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

//SDL_Renderer* renderer = NULL;

void castRays(GameMap *gMap, MapObject *player, SDL_Surface *screenSurf, int angRange, int depth);
void input(GameMap *gMap, MapObject *player, MapObject **agent_arr, int agent_cnt,
			std::set<int> keys, double speed, double angVel, double dt);
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
			
			//renderer = SDL_CreateRenderer ( window, -1, SDL_RENDERER_ACCELERATED );
			//getting the window surface
			screenSurf = SDL_GetWindowSurface( window );
			
			//renderer = SDL_GetRenderer( window );
		}
	}
	return true;
}

void close_SDL(){
	
	SDL_DestroyWindow( window );
	window = NULL;
	
	SDL_Quit();
}

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

int main( int argc, char* args[] ){
	
	//the image holding all the wall textures
	SDL_Surface *wall_textures = SDL_LoadBMP("./Images/walls.bmp");
	
	//the image holding the darkened versions of the wall textures
	SDL_Surface *dark_wall_textures = SDL_CreateRGBSurface( 0, wall_textures->w, wall_textures->h, 24, 0, 0, 0, 0 );
	
	//change the pixels in the dark texture image
	create_dark_walls( wall_textures, dark_wall_textures, 0.75 );
	
	//creating a map object
	GameMap gMap( "./Images/levelTrial3.bmp", wall_textures, dark_wall_textures, 0.75 );
	//GameMap gMap("./Images/spriteTest.bmp");
	
	
	//to get a map image that can be displayed
	SDL_Surface *mapSurf = SDL_CreateRGBSurface(0, 448, 448, 32, 0, 0, 0, 0);
	SDL_Surface *spriteSurf = SDL_LoadBMP("./Images/sprite3.bmp");
		
	gMap.drawFullMap(mapSurf);
	
	//initializing the player
	//gMap.initPlayer(80, 80, (7*PI)/2, 10);
	MapObject player(80.0, 80.0, (7*PI)/2, 10);
	Agent enemy( spriteSurf, 256.0, 256.0, 0.0, 10 );
	
	int agent_cnt = 1;
	MapObject **agent_arr = new MapObject*[agent_cnt];
	agent_arr[0] = &player;
	agent_arr[1] = &enemy;
	
	//init SDL
	if( !init_SDL() ){
		printf("Unable to start correctly.\n");
	}else{
		
		/*printf("A Simple Ray Casting Engine - written by Shreesh\n");
		printf("Use direction keys to move; a, s to turn\n");
		printf("Press space to double movement speed\n");
		printf("Press escape to exit\n");
		printf("Press Enter to continue\n");*/
		
		//SDL_Surface *screen = SDL_CreateRGBSurface(0, screenSurf->w, screenSurf->h, 24, 0,0,0,0);
		
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
			input( &gMap, &player, agent_arr, agent_cnt, keys, 240, 1.5, dt );
			
			enemy.follow_player( &gMap, agent_arr, agent_cnt, 10, 100, 1.5, dt );
						
			if( show3D ){
				
				SDL_FillRect( screenSurf, &sky, SDL_MapRGB(screenSurf->format, 255, 255, 255 ) );
				SDL_FillRect( screenSurf, &ground, SDL_MapRGB(screenSurf->format, 50, 50, 50 ) );
				
				castRays( &gMap, &player, screenSurf, 30, DEPTH_OF_FIELD );
				enemy.sprite3D( &gMap, screenSurf, &player, 0.5235987755982988);
			}else{
				SDL_FillRect(screenSurf, NULL, SDL_MapRGB(screenSurf->format, 0, 0, 0));
				gMap.draw2DMap( screenSurf , player.x, player.y );
				player.draw2DMap( screenSurf );
				enemy.sprite2D( screenSurf, &player );
			}
			
			//update the window
			SDL_UpdateWindowSurface( window );
		}
	}
	
	//wrapping up everything
	SDL_FreeSurface(mapSurf);
	SDL_FreeSurface(wall_textures);
	SDL_FreeSurface(dark_wall_textures);
	mapSurf = NULL;
	close_SDL();
	
	return 0;
}

void input(GameMap *gMap, MapObject *player, MapObject **agent_arr, int agent_cnt,
			std::set<int> keys, double speed, double angVel, double dt){
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
	
	player->move(gMap, agent_arr, agent_cnt, moveX, moveY, moveAng, true);
}

void castRays(GameMap *gMap, MapObject *player, SDL_Surface *screenSurf, int angRange, int depth){
	
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
		
		wall->blit_wall_to_screen( screenSurf, &slice, offset_x, offset_y, isVertical );
		
	}
}