#include <stdio.h>
#include <cmath>
#include "MapObject.h"

#define PI 3.1415926535897932384

const unsigned TILESHIFT = 6;
const unsigned BLOCK_DIM = 64;
const int DEPTH_OF_FIELD = 20;

MapObject::MapObject(double x_, double y_, double ang_, int objDim_){
	x = x_; y = y_;
	ang = ang_;
	objDim = objDim_;
}
MapObject::MapObject() : MapObject::MapObject(0, 0, 0, 10) {}

void MapObject::print(){
	printf( "%lf %lf %lf %d \n", x, y, ang, objDim );
}

void MapObject::update(double x_, double y_, double ang_, int objDim_){
	x = x_; y = y_;
	ang = ang_;
	objDim = objDim_;
}

void MapObject::draw(SDL_Surface *screenSurf){
	//drawing player square
	SDL_Rect playRect;
	playRect.x = x - (objDim >> 1);
	playRect.y = y - (objDim >> 1);
	playRect.w = objDim; playRect.h = objDim;
	SDL_FillRect( screenSurf, &playRect, SDL_MapRGB(screenSurf->format, 255, 255, 0 ) );
	
	playRect.x = x + objDim*std::cos(ang);
	playRect.y = y - objDim*std::sin(ang);
	playRect.w = 5; playRect.h = 5;
	SDL_FillRect( screenSurf, &playRect, SDL_MapRGB(screenSurf->format, 255, 0, 0 ) );
}

//draws at the center of the screen
void MapObject::draw2DMap(SDL_Surface *screenSurf){
	SDL_Rect playRect;
	playRect.x = ( ( screenSurf->w ) >> 1 ) - ( objDim >> 2 ) ;
	playRect.y = ( ( screenSurf->h ) >> 1 ) - ( objDim >> 2 );
	playRect.w = objDim >> 1; playRect.h = objDim >> 1;
	SDL_FillRect( screenSurf, &playRect, SDL_MapRGB(screenSurf->format, 255, 255, 0 ) );
	
	playRect.x = (screenSurf->w >> 1) + objDim*std::cos(ang);
	playRect.y = (screenSurf->h >> 1) - objDim*std::sin(ang);
	playRect.w = 2; playRect.h = 2;
	SDL_FillRect( screenSurf, &playRect, SDL_MapRGB(screenSurf->format, 255, 0, 0 ) );
}

int MapObject::move(GameMap *gMap, MapObject **agent_arr, int agent_cnt, double dx, double dy, double dang, bool rotate){
	
	ang = mod2PI( ang + dang );
	
	double moveX = dx;
	double moveY = dy;
	
	if( rotate ){
		//projecting motion in object's reference frame axes to grid axes
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
	
	double bxl, bxh, byl, byh;	//bounds of the agent
	bxl = this->x - (this->objDim >> 1);
	bxh = this->x + (this->objDim >> 1);
	byl = this->y - (this->objDim >> 1);
	byh = this->y + (this->objDim >> 1);
	
	for( int a = 0; a < agent_cnt; a++ ){	//starts from 1 as 0 is player
		
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

//##########################################AGENT##############################################################

//main constructor
Agent::Agent(SDL_Surface *surf, double posX, double posY, double ang_, int objDim_){
	spriteSurf = surf;
	objDim = objDim_;
	x = posX; y = posY; ang = ang_;
	this->reset();
	
	//hardcoded to always follow player
	follow_player_flag = true;
}

//agents only forgets that it has seen player
void Agent::reset(){
	has_seen = false;
}

//agent forgets that it has seen player, and also stops following
void Agent::reset_to_idle(){
	this->reset();
	follow_player_flag = false;
}

bool Agent::check_for_player(MapObject *player, int tile_radius){
	
	if( has_seen ) return true;
	
	int diffX = (int)(x - player->x) >> TILESHIFT;
	int diffY = (int)(y - player->y) >> TILESHIFT;
	
	has_seen = std::hypot(diffX, diffY) < tile_radius;
	
	return has_seen;
}

bool Agent::follow_player(GameMap *gMap, MapObject **agent_arr, int agent_cnt,
						int tile_radius, double speed, double angVel, double dt){
	if( follow_player_flag ){
		if( check_for_player( agent_arr[0], tile_radius ) ){
			
			double moveX, moveY, movAng;
			moveX = moveY = movAng = 0.0;
			
			//this finds the angle between the player and the agent
			//agent_arr[0] is the player
			double diffX = agent_arr[0]->x - this->x;
			double diffY = -(agent_arr[0]->y - this->y);
			
			double dist = std::hypot( diffX, diffY );
			if( dist < this->objDim + agent_arr[0]->objDim )
				return true;
			
			double ang_diff = modPI( mod2PI( real_atan( diffX, diffY ) - this->ang ) );
			
			if( ang_diff > 0 )
				movAng += angVel*dt;
			else
				movAng -= angVel*dt;
			
			moveX += std::cos(this->ang)*speed*dt;
			moveY -= std::sin(this->ang)*speed*dt;
			
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

void Agent::sprite3D(GameMap *gMap, SDL_Surface *screenSurf, MapObject *player, double spread){
	
	//axes are normal cartesian coordinates, have to flip y axis
	double diffX = (this->x - player->x);
	double diffY = -(this->y - player->y);
	
	double dist = std::hypot( diffX , diffY );
	
	//dont do anything if the sprite is very far away
	if( ((int)dist >> TILESHIFT) > DEPTH_OF_FIELD )
		return;
	
	double sprite_ang = real_atan( diffX, diffY );
	
	//The first mod2PI brings the difference back to the original modulo 2pi
	//The second modPI brings the diff in the range (-pi, pi)
	double ang_diff = modPI( mod2PI( sprite_ang - player->ang ) );
	
	//angular size of sprite
	double half_ang_sprite_size = std::atan( (double)(spriteSurf->w >> 1) / dist );
	
	if( std::fabs(ang_diff) - std::fabs(half_ang_sprite_size) < spread ){
		
		//horiz position on screen at which sprite is centered
		//-(ang_diff/spread) because larger angles are placed nearer to the left on the screen
		double pos = (double)(screenSurf->w >> 1)*( 1.0 - ang_diff/spread ); 
		
		//distance away from the screen
		double screenDist = (double)screenSurf->w/( 2*std::tan(spread) );
		
		//dimensions of the sprite
		int sprite_wid = (int)(spriteSurf->w*screenDist/dist);
		int sprite_hig = (int)(spriteSurf->h*screenDist/dist);
		
		//the maximum screen dimension
		int max_screen_dim = screenSurf->w > screenSurf->h ? screenSurf->w : screenSurf->h;
		
		//capping the sprite dims to the max screen dim
		sprite_wid = sprite_wid > max_screen_dim ? max_screen_dim : sprite_wid;
		sprite_hig = sprite_hig > max_screen_dim ? max_screen_dim : sprite_hig;
		
		//angle at which the sprite starts displaying
		double start_ang = mod2PI( sprite_ang + half_ang_sprite_size );
		
		//steps at which rays are casted to check if the sprite is to be clipped by a wall
		double ang_step = (2.0*half_ang_sprite_size)/(double)sprite_wid;
		
		for( int i = 0; i < sprite_wid; i++ ){
			
			//only placeholders
			int mapX, mapY;
			
			//angle at which ray is casted, modulo 2pi
			double rAng = mod2PI(start_ang - i*ang_step);
			
			double vDist, hDist, finalDist;
			int off;
			
			cast_horiz_ray( gMap, player->x, player->y, rAng, DEPTH_OF_FIELD, &mapX, &mapY, &hDist, &off);
			cast_vert_ray( gMap, player->x, player->y, rAng, DEPTH_OF_FIELD, &mapX, &mapY, &vDist, &off);
			
			//just like normal ray casting
			finalDist = vDist > hDist ? hDist : vDist;
			
			if( finalDist > dist ){
				
				SDL_Rect srcRect;
				srcRect.x = (int)((double)(spriteSurf->w*i)/(double)sprite_wid);
				srcRect.y = 0;
				srcRect.w = ((double)spriteSurf->w/(double)sprite_wid) + 1;
				srcRect.h = spriteSurf->h;
				
				SDL_Rect dstRect;
				dstRect.x = (int)pos - (sprite_wid >> 1) + i;
				dstRect.y = (screenSurf->h - sprite_hig) >> 1;
				dstRect.h = sprite_hig;
				dstRect.w = 1;
				
				SDL_BlitScaled(spriteSurf, &srcRect, screenSurf, &dstRect);
			}
		}
	}
}

void Agent::sprite2D(SDL_Surface *screenSurf, MapObject *player){
	SDL_Rect aRect;
	int screen_x = ( screenSurf->w >> 1 ) + ( (int)( (this->x - player->x) ) >> 1 );
	int screen_y = ( screenSurf->h >> 1 ) + ( (int)( (this->y - player->y) ) >> 1 );
	aRect.x = screen_x - ( this->objDim >> 2 );
	aRect.y = screen_y - ( this->objDim >> 2 );
	aRect.w = this->objDim >> 1;
	aRect.h = this->objDim >> 1;
	
	//if rectangle out of bounds;
	if( aRect.x + aRect.w < 0 or aRect.y + aRect.h < 0 or aRect.x > screenSurf->w or aRect.y > screenSurf->h )
		return;
	
	SDL_BlitScaled( this->spriteSurf, NULL, screenSurf, &aRect );
}