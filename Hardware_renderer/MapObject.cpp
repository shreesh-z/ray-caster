#include <stdio.h>
#include <cmath>
#include "MapObject.h"

#define PI 3.1415926535897932384

const unsigned TILESHIFT = 6;
const unsigned BLOCK_DIM = 64;
const int DEPTH_OF_FIELD = 20;



int MapObject::move(GameMap *gMap, MapObject **agent_arr, int agent_cnt, double dx, double dy, double dang, bool rotate){
	
	ang = mod2PI( ang + dang );
	
	double moveX = dx;
	double moveY = dy;
	
	if( rotate ){
		//projecting motion in object's reference frame axes to grid axes
		//done for player input
		moveX = std::cos(ang)*dx + std::sin(ang)*dy;
		moveY = -std::sin(ang)*dx + std::cos(ang)*dy;
	}
	
	if( moveX == 0.0 && moveY == 0.0 )
		return -1;	//no motion, no clipping
	
	x += moveX; y += moveY;
	
	if( tryMove(gMap, agent_arr, agent_cnt) )
		return 0;	//no clipping
	
	//clipping x direction movement
	x -= moveX;
	
	if( tryMove(gMap, agent_arr, agent_cnt) )
		return 1;	//clipping in x direction
	
	//clipping y direction movement
	x += moveX;
	y -= moveY;
	
	if( tryMove(gMap, agent_arr, agent_cnt) )
		return 2;	//clipping in y direction
	
	//clipping full movement if nothing works
	x -= moveX;
	
	return 3;	//full clipping
}

bool MapObject::tryMove(GameMap *gMap, MapObject **agent_arr, int agent_cnt){
	int xl, xh, yl, yh;
	
	//the indices in the map array where the four corners of the object's square lie
	xl = ((int)this->x - (this->objDim >> 1)) >> TILESHIFT;
	xh = ((int)this->x + (this->objDim >> 1)) >> TILESHIFT;
	yl = ((int)this->y - (this->objDim >> 1)) >> TILESHIFT;
	yh = ((int)this->y + (this->objDim >> 1)) >> TILESHIFT;
	
	for( int y_ = yl; y_ <= yh; y_++ ){
		for( int x_ = xl; x_ <= xh; x_++ ){
			if( gMap->solid_block_at( y_, x_ ) )	// that gMap function also does out of bounds checking
				return false;
		}
	}
	
	//don't check player collisions with other objects on the map
	if( this == agent_arr[0] )
		return true;
	
	double bxl, bxh, byl, byh;	//actual bounds of the agent (not in the mapa array like before)
	bxl = this->x - (this->objDim >> 1);
	bxh = this->x + (this->objDim >> 1);
	byl = this->y - (this->objDim >> 1);
	byh = this->y + (this->objDim >> 1);
	
	for( int a = 0; a < agent_cnt; a++ ){
		
		if( this == agent_arr[a] )
			continue;
		
		double axl, axh, ayl, ayh;
		axl = agent_arr[a]->x - (agent_arr[a]->objDim >> 1);
		axh = agent_arr[a]->x + (agent_arr[a]->objDim >> 1);
		ayl = agent_arr[a]->y - (agent_arr[a]->objDim >> 1);
		ayh = agent_arr[a]->y + (agent_arr[a]->objDim >> 1);
		
		for( double i = bxl; i < bxh; i += 1.0 ){
			for( double j = byl; j < byh; j += 1.0 ){
				//if object's's rect is intersecting another object's rect
				if( axl < i and i < axh  and  ayl < j and j < ayh )
					return false;
			}
		}
	}
	
	return true;
}

Player::Player(double x_, double y_, double ang_, int objDim_){
	x = x_; y = y_;
	ang = ang_;
	objDim = objDim_;
}
//default object has dimension 10
Player::Player() : Player::Player(0, 0, 0, 10) {}

//draws at the center of the screen
void Player::sprite2D(SDL_Renderer *renderer, MapObject *player){
	
	SDL_Rect playRect;
	int wid, hig;
	SDL_GetRendererOutputSize( renderer, &wid, &hig );
	//objDim/4 is done because top-down view is drawn at half size
	playRect.x = ( ( wid ) >> 1 ) - ( objDim >> 2 );
	playRect.y = ( ( hig ) >> 1 ) - ( objDim >> 2 );
	playRect.w = objDim >> 1; playRect.h = objDim >> 1;
	
	SDL_SetRenderDrawColor( renderer, 255, 255, 0, 255 );
	SDL_RenderFillRect( renderer, &playRect );
	
	SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
	SDL_RenderDrawLine( renderer,
			wid >> 1, hig >> 1,
			(wid >> 1) + objDim*std::cos(ang), (hig >> 1) - objDim*std::sin(ang) );
	
}

//does nothing
bool Player::follow_player(GameMap *gmap, MapObject **agent_arr, int agent_cnt, double dt){
	return false;
}	

//does nothing
void Player::sprite3D(GameMap *gMap, SDL_Renderer *renderer, MapObject *player, double spread){}

//##########################################AGENT##############################################################

//main constructor
Agent::Agent(SDL_Texture *sprite, double posX, double posY, double ang_, int objDim_,
			int tile_radius_, double speed_, double angVel_){
	spriteText = sprite;
	objDim = objDim_;
	x = posX; y = posY; ang = ang_;
	tile_radius = tile_radius_;
	speed = speed_;
	angVel = angVel_;
	
	this->reset();
	has_seen_once = false;
	
	//hardcoded to always follow player
	follow_player_flag = true;
}

//agents only stops seeing player
void Agent::reset(){
	has_seen = false;
}

//agent forgets that it has seen player, and also stops following
void Agent::reset_to_idle(){
	this->reset();
	has_seen_once = false;
	follow_player_flag = false;
}

bool Agent::check_for_player(GameMap *gMap, MapObject *player){
	
	//if( has_seen ) return true;
	
	int diffX = (int)(x - player->x) >> TILESHIFT;
	int diffY = (int)(y - player->y) >> TILESHIFT;
	
	//tile distance between player and agent
	double dist = std::hypot(diffX, diffY);
	
	if( dist < tile_radius ){
		//if tile distance is less than radius of sight of agent, next check if player is blocked by a wall
		//for this we cast a ray from the agent to the player and check for wall collisions
		double diffX_ = player->x - this->x;
		double diffY_ = -(player->y - this->y);
		//angle at which ray is casted
		double rayAng = real_atan( diffX_, diffY_ );
		
		//distances from walls
		double vDist, hDist, finalDist;
		//placeholders, these are not used at all
		int a, b, c;
		cast_vert_ray( gMap, this->x, this->y, rayAng, tile_radius, &a, &b, &vDist, &c );
		cast_horiz_ray( gMap, this->x, this->y, rayAng, tile_radius, &a, &b, &hDist, &c );
		
		finalDist = vDist > hDist ? hDist : vDist;
		
		//real (map) distance between the agent and the player
		double real_dist = std::hypot( diffX_, diffY_ );
		
		//if wall is farther away than the player		
		has_seen = has_seen ? true : finalDist > real_dist;
	
	}else if( dist > ( tile_radius << 2 ) )
		//if the player is too far away, forget that you have seen them
		has_seen = false;
	
	//if agent has seen player at least once before resetting
	if( !has_seen_once and has_seen ){
		has_seen_once = true;
	}
	
	return has_seen;
}

bool Agent::follow_player(GameMap *gMap, MapObject **agent_arr, int agent_cnt, double dt){
	
	if( follow_player_flag ){
		if( check_for_player( gMap, agent_arr[0] ) ){
			
			double moveX, moveY, movAng;
			moveX = moveY = movAng = 0.0;
			
			//this finds the angle between the player and the agent
			//(agent_arr[0] is always the player)
			double diffX = agent_arr[0]->x - this->x;
			double diffY = -(agent_arr[0]->y - this->y);
			
			double dist = std::hypot( diffX, diffY );
			
			//if the agent and the player have collided, return true and don't move
			//returning true ends the game anyway, so no need to move
			if( dist < this->objDim + agent_arr[0]->objDim )
				return true;
			
			//player is at which side of the agent, left or right
			//first find the difference mod 2PI, then convert it to lie in (-pi, pi)
			double ang_diff = modPI( mod2PI( real_atan( diffX, diffY ) - this->ang ) );
			
			if( ang_diff > 0 )		//if player is to the left
				movAng += angVel*dt;
			else					//if player is to the right
				movAng -= angVel*dt;
			
			//move in the direction of the current angle
			moveX += std::cos(this->ang)*speed*dt;
			moveY -= std::sin(this->ang)*speed*dt;
			
			//clip the motion
			int clip = this->move( gMap, agent_arr, agent_cnt, moveX, moveY, movAng, false);
			
			//to compensate for collisions with wall
			if( clip > 0 ){
				if( clip == 1 ){	//x got clipped
					
					//undoing y motion
					this->y -= moveY;
					
					//defining new motion along the wall
					if( this->ang < PI )	//facing upward
						moveY = -speed*dt;
					else					//facing downward
						moveY = speed*dt;
					
					moveX = 0.0;
					
					this->move( gMap, agent_arr, agent_cnt, moveX, moveY, 0.0, false);
					
				}else if( clip == 2 ){	//y got clipped
					
					//undoing x motion
					this->x -= moveX;
					
					//defining new motion along wall
					if( this->ang > PI/2 && this->ang < (3*PI)/2 ) //facing left
						moveX = -speed*dt;
					else										//facing right
						moveX = speed*dt;
					
					moveY = 0.0;
					
					this->move( gMap, agent_arr, agent_cnt, moveX, moveY, 0.0, false);
					
				}else{
					//if it encounters an inescapable space, it just resets
					this->reset();
				}
			}
		}
	}
	return false;
}

//PLEASE NOTE THAT THE POINT OF VIEW OF THIS FUNCTION IS OF THE PLAYER, NOT THE AGENT.
//ALL ANGLES CALCULATED ARE OF THE AGENT ACC. TO THE PLAYER
void Agent::sprite3D(GameMap *gMap, SDL_Renderer *renderer, MapObject *player, double spread){
	
	int wid, hig;
	SDL_GetRendererOutputSize( renderer, &wid, &hig );
	
	//axes are normal cartesian coordinates, have to flip y axis
	double diffX = (this->x - player->x);
	double diffY = -(this->y - player->y);
	
	//distance between the player and the agent
	double dist = std::hypot( diffX , diffY );
	
	//dont do anything if the sprite is very far away
	if( ((int)dist >> TILESHIFT) > DEPTH_OF_FIELD )
		return;
	
	//angle at which the agent lies acc. to the player
	double sprite_ang = real_atan( diffX, diffY );
	
	//The first mod2PI brings the difference back to the original modulo 2pi
	//The second modPI brings the diff in the range (-pi, pi)
	double ang_diff = modPI( mod2PI( sprite_ang - player->ang ) );
	
	int spriteTextWid, spriteTextHig;
	SDL_QueryTexture( spriteText, NULL, NULL, &spriteTextWid, &spriteTextHig );
	
	//angular size of sprite
	double half_ang_sprite_size = std::atan( (double)(spriteTextWid >> 1) / dist );
	
	//if the sprite lies in the field of view of the player
	//the abs is calculated so that if part of the sprite lies inside the FOV, it is still drawn
	if( std::fabs(ang_diff) - std::fabs(half_ang_sprite_size) < spread ){
		
		//NOW WE CAST THE RAYS TO DRAW THE SPRITE
		//rays are cast so that wall clipping can be performed where sprites are being blocked by walls
		
		//horiz position on screen at which sprite is centered
		//-(ang_diff/spread) because larger angles are placed nearer to the left on the screen
		double pos = (double)(wid >> 1)*( 1.0 - ang_diff/spread ); 
		
		//distance away from the screen
		double screenDist = (double)wid/( 2*std::tan(spread) );
		
		//dimensions of the sprite
		int sprite_wid = (int)(spriteTextWid*screenDist/dist);
		int sprite_hig = (int)(spriteTextHig*screenDist/dist);
		
		//the maximum screen dimension
		int max_screen_dim = wid > hig ? wid : hig;
		
		//capping the sprite dims to the max screen dim
		sprite_wid = sprite_wid > max_screen_dim ? max_screen_dim : sprite_wid;
		sprite_hig = sprite_hig > max_screen_dim ? max_screen_dim : sprite_hig;
		
		//angle at which the sprite starts displaying
		double start_ang = mod2PI( sprite_ang + half_ang_sprite_size );
		
		//steps at which rays are casted to check if the sprite is to be clipped by a wall
		double ang_step = (2.0*half_ang_sprite_size)/(double)sprite_wid;
		
		for( int i = 0; i < sprite_wid; i++ ){
			
			//only placeholders
			int mapX, mapY, off;
			
			//angle at which ray is casted, modulo 2pi
			double rAng = mod2PI(start_ang - i*ang_step);
			
			double vDist, hDist, finalDist;
			
			cast_horiz_ray( gMap, player->x, player->y, rAng, DEPTH_OF_FIELD, &mapX, &mapY, &hDist, &off);
			cast_vert_ray( gMap, player->x, player->y, rAng, DEPTH_OF_FIELD, &mapX, &mapY, &vDist, &off);
			
			//just like normal ray casting
			finalDist = vDist > hDist ? hDist : vDist;
			
			//draw a slice of the sprite IFF A WALL IS NOT BLOCKING the sprite at this slice
			if( finalDist > dist ){
				
				SDL_Rect srcRect;
				srcRect.x = (int)((double)(spriteTextWid*i)/(double)sprite_wid);
				srcRect.y = 0;
				srcRect.w = ((double)spriteTextWid/(double)sprite_wid) + 1;
				srcRect.h = spriteTextHig;
				
				SDL_Rect dstRect;
				dstRect.x = (int)pos - (sprite_wid >> 1) + i;
				dstRect.y = (hig - sprite_hig) >> 1;
				dstRect.h = sprite_hig;
				dstRect.w = 1;
				
				SDL_RenderCopy( renderer, spriteText, &srcRect, &dstRect);
			}
		}
	}
}

void Agent::sprite2D(SDL_Renderer *renderer, MapObject *player){
	
	int wid, hig;
	SDL_GetRendererOutputSize( renderer, &wid, &hig );
	
	//this is done to dissuade cheating by just checking the top down view to look through walls
	//If an agent hasn't seen the player yet, dont draw it on the screen
	if( !has_seen_once )
		return;
	
	SDL_Rect aRect;
	int screen_x = ( wid >> 1 ) + ( (int)( (this->x - player->x) ) >> 1 );
	int screen_y = ( hig >> 1 ) + ( (int)( (this->y - player->y) ) >> 1 );
	aRect.x = screen_x - ( this->objDim >> 2 );
	aRect.y = screen_y - ( this->objDim >> 2 );
	aRect.w = this->objDim >> 1;
	aRect.h = this->objDim >> 1;
	
	//if rectangle out of bounds, dont draw
	if( aRect.x + aRect.w < 0 or aRect.y + aRect.h < 0 or aRect.x > wid or aRect.y > hig )
		return;
	
	SDL_RenderCopy( renderer, this->spriteText, NULL, &aRect );
	
	SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
	SDL_RenderDrawLine( renderer, screen_x, screen_y,
					screen_x + objDim*std::cos(ang), screen_y - objDim*std::sin(ang) );
}