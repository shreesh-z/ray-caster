#include <helper.h>
#define PI 3.1415926535897932384

const unsigned BLOCK_DIM = 64;
const unsigned TILESHIFT = 6;
const int DEPTH_OF_FIELD = 20;

void create_dark_walls(SDL_Surface *wall_textures, SDL_Surface *dark_wall_textures, double wallColorRatio){
	
	SDL_LockSurface(wall_textures);
	Uint8 *pixel = ((Uint8*)wall_textures->pixels);
	SDL_UnlockSurface(wall_textures);
	
	SDL_LockSurface(dark_wall_textures);
	Uint8 *dark_pixel = ((Uint8*)dark_wall_textures->pixels);
	SDL_UnlockSurface(dark_wall_textures);
	
	for( int i = 0; i < dark_wall_textures->h; i++ ){
		for( int j = 0; j < dark_wall_textures->pitch; j+=3 ){
			for( int k = 0; k < 3; k++ )
				dark_pixel[i*dark_wall_textures->pitch + j + k] 
					= (Uint8)( wallColorRatio*pixel[i*wall_textures->pitch + j + k] );
		}
	}
	
}

void frame_rate( SDL_Renderer *renderer, SDL_Texture *numbers, int fps ){
	
	if( fps > 99 )
		return;
	int digs[2];
	
	//extracting digits
	digs[0] = (int)( fps/10 );
	digs[1] = fps % 10;
	
	int wid, hig;
	SDL_GetRendererOutputSize( renderer, &wid, &hig );
	
	SDL_Rect srcRect, dstRect;
	srcRect.x = 5*digs[1]; srcRect.y = 0;
	srcRect.w = 4; srcRect.h = 7;
	
	//draw at the top right corner of the screen
	dstRect.x = wid - 25; dstRect.y = 0;
	dstRect.w = 20; dstRect.h = 35;
	SDL_RenderCopy( renderer, numbers, &srcRect, &dstRect );
	
	srcRect.x = 5*digs[0]; srcRect.y = 0;
	srcRect.w = 5; srcRect.h = 7;
	
	dstRect.x = wid - 50; dstRect.y = 0;
	dstRect.w = 25; dstRect.h = 35;
	
	SDL_RenderCopy( renderer, numbers, &srcRect, &dstRect );
	
}

bool checkWhiteBlock( GameMap *gMap, MapObject *player ){
	int mapX, mapY;
	
	//finding where the point right in front of the player lies in the map array
	mapX = (int)( player->x + std::cos(player->ang)*(double)player->objDim ) >> TILESHIFT;
	mapY = (int)( player->y - std::sin(player->ang)*(double)player->objDim ) >> TILESHIFT;
	
	Block *block = gMap->block_at( mapY, mapX );
	
	if( block != NULL ){
		if( block->isWall ){
			//if this block is fully white, return true ( player wins )
			return block->colors[0] == 255 and block->colors[1] == 255 and block->colors[2] == 255;
		}
	}
	
	return false;
}

bool input(GameMap *gMap, MapObject *player, std::vector<MapObject*> &agent_arr,
			std::set<int> keys, double speed, double angVel, double dt){
	
	double moveX, moveY, moveAng;
	moveX = moveY = moveAng = 0.0;
	
	bool touched = false;
	
	if( keys.count(SDLK_SPACE) != 0 ){
		speed *= 2;
		angVel *= 2;
	}
	if( keys.count(SDLK_LSHIFT) != 0 or keys.count(SDLK_RSHIFT) != 0 ){
		speed /= 2;
		angVel /= 2;
	}
	
	for(int n : keys){
		switch(n){
			
			case SDLK_a:
				moveAng += angVel*dt;
				break;
			
			case SDLK_s:
				moveAng -= angVel*dt;
				break;
				
			case SDLK_UP:
				moveX += speed*dt;
				break;
			
			case SDLK_DOWN:
				moveX -= speed*dt;
				break;
			
			case SDLK_RIGHT:
				moveY += speed*dt;
				break;
			
			case SDLK_LEFT:
				moveY -= speed*dt;
				break;
				
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				touched = checkWhiteBlock( gMap, player );

		}
	}
	
	player->move(gMap, agent_arr, moveX, moveY, moveAng, true);
	
	return touched;
}

void castRays(GameMap *gMap, MapObject *player, SDL_Renderer *renderer, int angRange, int depth){
	
	//dimensions of the screen
	int wid, hig;
	SDL_GetRendererOutputSize( renderer, &wid, &hig );
	
	//range of angles in which the rays are casted (in radians)
	double radAngRange = (PI*angRange)/180.0;
	//num of rays casted
	int rayCount = wid;
	
	//distance away from the player where the "screen" is situated.
	//all rays are projected onto this screen
	double screenDist = (double)wid/( 2*std::tan(radAngRange) );
	
	//the height of the slice of the wall where the ray hits
	double rayHig;
	
	//map indices where the ray hits the wall
	int vmapX, vmapY, hmapX, hmapY;
	
	vmapX = vmapY = hmapX = hmapY = 0;
	
	for( int i = 0; i < rayCount; i++ ){
		
		//angle is calculated by atan
		double rAng = mod2PI( player->ang + modPI( real_atan( screenDist, (double) ( ( wid >> 1 ) - i ) ) ) );
		
		double hDist, vDist;
		
		//the horiz offset at which a slice of the wall texture is to be
		//picked
		int v_offset, h_offset, offset_x;
		
		cast_horiz_ray( gMap, player->x, player->y, rAng, depth, &hmapX, &hmapY, &hDist, &h_offset );
		cast_vert_ray( gMap, player->x, player->y, rAng, depth, &vmapX, &vmapY, &vDist, &v_offset );
		
		//which among the two distances is smallest
		double finalDist;
		
		//the wall where the ray hit
		Block *wall = NULL;
		bool isVertical = false;
		
		if( vDist > hDist ){
			finalDist = hDist;
			offset_x = h_offset;
			wall = gMap->horiz_wall_at( hmapY, hmapX );
			
		}else{
			finalDist = vDist;
			offset_x = v_offset;
			wall = gMap->vert_wall_at( vmapY, vmapX );
			isVertical = true;
			
		}
		
		//removing fish eye effect
		finalDist *= std::cos(rAng - player->ang);
		
		if( finalDist == 0.0 ){
			//skip drawing this ray
			continue;
		}
		
		rayHig = ( (double)BLOCK_DIM * screenDist )/finalDist;
		
		//if slice height is lesser than screen height, texture doesn't get clipped
		//but if slice height is greater than, then the texture has to be clipped
		//from above and below to coincide with the field of view of the player
		int offset_y = 0;
		
		if( rayHig > hig ){
			offset_y = (int)(((rayHig - (double)hig)*(double)BLOCK_DIM)/(2.0*rayHig));
		}
		
		//capping ray height at screen height
		rayHig = rayHig > hig ? (double)hig : rayHig;
		
		SDL_Rect slice;
		slice.y = (hig - (int)rayHig) >> 1; slice.x = i;
		slice.h = (int)rayHig; slice.w = 1;
		
		wall->blit_wall_to_screen( renderer, &slice, offset_x, offset_y, isVertical );
		
	}
}