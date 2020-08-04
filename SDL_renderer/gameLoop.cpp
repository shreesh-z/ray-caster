//game Loop and Rendering
#include <SDL2/SDL.h>
#include <stdio.h>
#include <set>
#include <chrono>
#define PI 3.1415926535897932384

#include "GameMap.h"

const int SCREEN_WIDTH = 448;
const int SCREEN_HEIGHT = 448;

//the window on which everything is displayed
SDL_Window *window = NULL;
//the surface of the main screen
SDL_Surface *screenSurf = NULL;

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
	GameMap gMap("./Images/tester.bmp");
	
	//to get a map image that can be displayed
	SDL_Surface *mapSurf = SDL_CreateRGBSurface(0, 448, 448, 32, 0, 0, 0, 0);
	
	gMap.drawMap(mapSurf);
	
	//initializing the player
	gMap.initPlayer(74, 74, PI/2, 10);
	
	//init SDL
	if( !init_SDL() ){
		printf("Unable to start correctly.\n");
	}else{
		
		//to run the game loop
		bool running = true;
		
		//to access events in the event queue
		SDL_Event e;
		
		//to hold key presses
		std::set<int> keys = {};
		
		//to calculate time difference between each frame
		std::chrono::steady_clock::time_point oldTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point newTime = std::chrono::steady_clock::now();
		
		while( running ){
			
			//accessing events from the event queue
			while( SDL_PollEvent( &e ) != 0 ){
				//only check for quit event for now
				if( e.type == SDL_QUIT ){
					running = false;
				}else if( e.type == SDL_KEYDOWN ){
					//cap max number of keys pressed to 5
					//add keypresses to set
					if( keys.size() < 5 )
						keys.insert(e.key.keysym.sym);
					
				}else if( e.type == SDL_KEYUP ){
					//remove keys from set if they are lifted
					if( !keys.empty() ){
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
			gMap.movePlayer(keys, 60, 3, dt);
			
			//display map onto the screen
			SDL_BlitSurface( mapSurf, NULL, screenSurf, NULL );
			
			//draw player on the screen
			gMap.drawPlayer(screenSurf);
			
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