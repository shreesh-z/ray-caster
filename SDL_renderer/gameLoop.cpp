//game Loop and Rendering
#include <SDL2/SDL.h>
#include <stdio.h>
#include <set>
#include <chrono>
#define PI 3.1415926535897932384

#include "GameMap.h"
#include "MapObject.h"
#include "custom_math.h"

const unsigned BLOCK_DIM = 64;
const unsigned TILESHIFT = 6;
const int DEPTH_OF_FIELD = 20;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//the window on which everything is displayed
SDL_Window *window = NULL;
//the surface of the main screen
SDL_Surface *screenSurf = NULL;

void castRays(GameMap *gMap, MapObject *player, SDL_Surface *screenSurf, int angRange, int depth, double wallColorRatio);
void input(GameMap *gMap, MapObject *player, std::set<int> keys, double speed, double angVel, double dt);
void sprite2D(SDL_Surface *screenSurf, SDL_Surface *spriteSurf, MapObject *player);
void sprite3D(GameMap *gMap, SDL_Surface *screenSurf, SDL_Surface *spriteSurf, MapObject *player, double x, double y, double spread);

//Initializing SDL
bool init_SDL(){
	
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
		printf( "SDL couldnt initialize. SDL error: %s\n", SDL_GetError() );
		return false;
	}else{
		//creating window
		window = SDL_CreateWindow( "Ray Caster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
						SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		
		if( window == NULL ){
			printf( "Window couldnt be created. SDL error: %s\n", SDL_GetError() );
			return false;
		}else{
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
			//getting the window surface
			screenSurf = SDL_GetWindowSurface( window );
		}
	}
	return true;
}

void close_SDL(){
	
	SDL_DestroyWindow( window );
	window = NULL;
	
	SDL_Quit();
}

int main( int argc, char* args[] ){
		
	//creating a map object
	GameMap gMap("./Images/levelTrial3.bmp");
	//GameMap gMap("./Images/spriteTest.bmp");
	
	
	//to get a map image that can be displayed
	SDL_Surface *mapSurf = SDL_CreateRGBSurface(0, 448, 448, 32, 0, 0, 0, 0);
	SDL_Surface *spriteSurf = SDL_LoadBMP("./Images/sprite3.bmp");
	
	gMap.drawFullMap(mapSurf);
	
	//initializing the player
	//gMap.initPlayer(80, 80, (7*PI)/2, 10);
	MapObject player(80.0, 80.0, (7*PI)/2, 10);
	Agent enemy( spriteSurf, 256.0, 256.0, 0.0 );
	
	//init SDL
	if( !init_SDL() ){
		printf("Unable to start correctly.\n");
	}else{
		
		/*printf("A Simple Ray Casting Engine - written by Shreesh\n");
		printf("Use direction keys to move; a, s to turn\n");
		printf("Press space to double movement speed\n");
		printf("Press escape to exit\n");
		printf("Press Enter to continue\n");*/
		
		//to run the game loop
		bool running = true;
		
		bool show3D = true;
		
		//to access events in the event queue
		SDL_Event e;
		
		//to hold key presses
		std::set<int> keys = {};
		
		//to calculate time difference between each frame
		std::chrono::steady_clock::time_point oldTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point newTime = std::chrono::steady_clock::now();
		
		//to show sky and ground in 3D scene
		SDL_Rect sky, ground;
		sky.x = 0; sky.y = 0; sky.h = screenSurf->h >> 1; sky.w = screenSurf->w;
		ground.x = 0; ground.y = screenSurf->h >> 1; ground.h = ground.y; ground.w = screenSurf->w;
		
		while( running ){
			
			//accessing events from the event queue
			while( SDL_PollEvent( &e ) != 0 ){
				//only check for quit event for now
				if( e.type == SDL_QUIT ){
					
					running = false;
					
				}else if( e.type == SDL_KEYDOWN ){
					
					
					if(e.key.keysym.sym == SDLK_ESCAPE){
						
						running = false;
						break;
						
					}else if(e.key.keysym.sym == SDLK_TAB ){
						
						show3D = !show3D;
						
					}else if( keys.size() < 5 )
						//cap max number of keys pressed to 5
						//add keypresses to set
						keys.insert(e.key.keysym.sym);
					
				}else if( e.type == SDL_KEYUP ){
					//remove keys from set if they are lifted
					if(e.key.keysym.sym == SDLK_TAB)
						
						continue;
						
					else if( !keys.empty() ){
						
						keys.erase(e.key.keysym.sym);
						
					}
				}
			}
			
			//get time now
			newTime = std::chrono::steady_clock::now();
			//calculate time difference in mus
			double dt = std::chrono::duration_cast<std::chrono::microseconds>(newTime - oldTime).count();
			//convert to time difference in seconds
			dt *= 0.000001;
			oldTime = newTime;
			
			//move player according to the keys pressed
			input( &gMap, &player, keys, 240, 1.5, dt );
			
			//display map onto the screen
			//SDL_BlitSurface( mapSurf, NULL, screenSurf, NULL );
			
			//draw player on the screen
			//gMap.drawPlayer(screenSurf);
			
			if( show3D ){
				SDL_FillRect( screenSurf, &sky, SDL_MapRGB(screenSurf->format, 255, 255, 255 ) );
				SDL_FillRect( screenSurf, &ground, SDL_MapRGB(screenSurf->format, 50, 50, 50 ) );
				
				castRays( &gMap, &player, screenSurf, 30, DEPTH_OF_FIELD, 0.75 );
				enemy.follow_player( &gMap, &player, 10, 100, 1.5, dt );
				enemy.sprite3D( &gMap, screenSurf, &player, 0.5235987755982988);
				//sprite3D( &gMap, screenSurf, spriteSurf, &player, 256, 256, 0.5235987755982988 );	//30degs
			}else{
				SDL_FillRect(screenSurf, NULL, SDL_MapRGB(screenSurf->format, 0, 0, 0));
				gMap.draw2DMap( screenSurf , player.x, player.y );
				player.draw2DMap( screenSurf );
			}
			
			//update the window
			SDL_UpdateWindowSurface( window );
		}
	}
	
	//wrapping up everything
	SDL_FreeSurface(mapSurf);
	mapSurf = NULL;
	close_SDL();
	
	return 0;
}

/*void sprite3D(GameMap *gMap, SDL_Surface *screenSurf, SDL_Surface *spriteSurf, MapObject *player, double x, double y, double spread){
	
	//axes are normal cartesian coordinates, have to flip y axis
	double diffX = (x - player->x);
	double diffY = -(y - player->y);
	
	double dist = std::hypot( diffX , diffY );
	
	//dont do anything if the sprite is very far away
	if( ((int)dist >> TILESHIFT) > DEPTH_OF_FIELD )
		return;
	
	//The first mod2PI brings the difference back to the original modulo 2pi
	//The second modPI brings the diff in the range (-pi, pi)
	
	double sprite_ang = real_atan( diffX, diffY );
	
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
			
			cast_horiz_ray( gMap, player->x, player->y, rAng, DEPTH_OF_FIELD, &mapX, &mapY, &hDist);
			cast_vert_ray( gMap, player->x, player->y, rAng, DEPTH_OF_FIELD, &mapX, &mapY, &vDist);
			
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
}*/

void input(GameMap *gMap, MapObject *player, std::set<int> keys, double speed, double angVel, double dt){
	double moveX, moveY, moveAng;
	moveX = moveY = moveAng = 0.0;
	
	if( keys.count(SDLK_SPACE) != 0 ){
		speed *= 2;
		angVel *= 2;
	}
	if( keys.count(SDLK_LSHIFT) != 0){
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
			
		}
	}
	
	player->move(gMap, moveX, moveY, moveAng, true);
}

void castRays(GameMap *gMap, MapObject *player, SDL_Surface *screenSurf, int angRange, int depth, double wallColorRatio){
	
	//dimensions of the screen
	int wid = screenSurf->w, hig = screenSurf->h;
	
	//range of angles in which the rays are casted (in radians)
	double radAngRange = (PI*angRange)/180.0;
	//num of rays casted
	int rayCount = wid;
	
	//distance away from the player where the "screen" is situated.
	//all rays are projected onto this screen
	double screenDist = (double)wid/( 2*std::tan(radAngRange) );
	
	//the height of the slice of the wall where the ray hits
	int rayHig;
	
	//map indices where the ray hits the wall
	int vmapX, vmapY, hmapX, hmapY;
	
	vmapX = vmapY = hmapX = hmapY = 0;
	
	//angle at which ray casting starts (from left to right)
	double rayAng = player->ang + radAngRange;
	
	for( int i = 0; i < rayCount; i++ ){
	
		double rAng = mod2PI(rayAng);
		
		double hDist, vDist;
		
		cast_horiz_ray( gMap, player->x, player->y, rAng, depth, &hmapX, &hmapY, &hDist );
		cast_vert_ray( gMap, player->x, player->y, rAng, depth, &vmapX, &vmapY, &vDist );
		
		//which among the two is smallest
		double finalDist;
		//color of wall which the ray hit
		Uint8 wallColor[3];
		wallColor[0] = wallColor[1] = wallColor[2] = 0;
		
		if( vDist > hDist ){
			finalDist = hDist;
			Block *wall = gMap->horiz_wall_at( hmapY, hmapX );
			
			if( wall != NULL ){
				for( int j = 0; j < 3; j++ )
					wallColor[j] = wall->colors[j];
			}
		}else{
			finalDist = vDist;
			Block *wall = gMap->vert_wall_at( vmapY, vmapX );
			
			if( wall != NULL ){
				for( int j = 0; j < 3; j++ )
					wallColor[j] = wallColorRatio*wall->colors[j];
			}
		}
		
		//removing fish eye effect
		finalDist *= std::cos(rAng - player->ang);
		
		if( finalDist == 0.0 ){
			//if intersecting with a wall, reject ray
			////this avoids zero division////
			rayAng += (2*radAngRange)/rayCount;
			continue;
		}
		
		rayHig = ( BLOCK_DIM * screenDist )/finalDist;
		
		//capping ray height at screen height
		rayHig = rayHig > hig ? hig : rayHig;
		
		SDL_Rect slice;
		slice.y = (hig - rayHig) >> 1; slice.x = i;
		slice.h = rayHig; slice.w = 1;
		
		SDL_FillRect( screenSurf, &slice, SDL_MapRGB(screenSurf->format, wallColor[0], wallColor[1], wallColor[2] ) );
		
		rayAng -= (2*radAngRange)/rayCount;
	}
}