#include <stdio.h>
#include <cmath>
#define PI 3.1415926535897932384

//Refer to this header file for documentation
//since header file already includes SDL, no need to include again
#include "GameMap.h"
#include "MapObject.h"

//for a general block
Block::Block(Uint8 R_, Uint8 G_, Uint8 B_, bool isWall_){
	colors[0] = R_;
	colors[1] = G_;
	colors[2] = B_;
	isWall = isWall_;
}
//default block is an empty block
Block::Block() : Block(0, 0, 0, false){}

//creates the game map by importing a bitmap file
//uses SDL's internal mechanisms to read the bitmap file and create the map array
GameMap::GameMap(const char *imgPath){
	
	//The surface of the image
	SDL_Surface *mapImg = SDL_LoadBMP( imgPath );
	
	//If an error occurs
	if( mapImg == NULL ){
		printf("Unable to load image. SDL error: %s\n", SDL_GetError() );
		return;
	}
	
	//dimensions of the map in number of blocks
	mapDims[0] = mapImg->h;
	mapDims[1] = mapImg->w;
	
	//initializing the main map array
	mapArr = new Block*[mapDims[0]*mapDims[1]];
	
	//initializing the wall counts
	H_WALL_CNT = mapDims[0] + 1;
	V_WALL_CNT = mapDims[1] + 1;
	
	//initializing the wall arrays
	mapVLines = new Block*[mapDims[0]*V_WALL_CNT];
	mapHLines = new Block*[H_WALL_CNT*mapDims[1]];
	
	//Getting raw pixel data from the map image surface
	SDL_LockSurface(mapImg);
	Uint8 *pixel = ((Uint8*)mapImg->pixels);
	SDL_UnlockSurface(mapImg);

	
	//iterating over the raw pixel data to figure out where the walls are
	//An (R, G, B) of (0, 0, 0) represents an empty block
	for( int i = 0; i < mapImg->h; i++ ){
		for( int j = 0; j < mapImg->w*3; j += 3 ){		//3 bytes per pixel
			
			//initializing an empty block
			Block *newBlock = new Block();
			
			for( int k = 0; k < 3; k++ ){
				
				//setting block's color tuple to pixel's color tuple
				newBlock->colors[k] = pixel[i*mapImg->pitch + j + k];
				
				//if any tuple value is nonzero, it's a wall block
				if( newBlock->colors[k] != 0 )
					newBlock->isWall = true;
			}
			
			mapArr[i*mapDims[1] + (int)(j/3)] = newBlock;
		}
	}
	
	//initializing these two arrays with empty blocks
	for( int i = 0; i < mapDims[0]*V_WALL_CNT; i++ ){
		mapVLines[i] = new Block();
	}
	for( int i = 0; i < mapDims[1]*H_WALL_CNT; i++ ){
		mapHLines[i] = new Block();
	}
	
	//setting walls around solid blocks as solid walls
	//if the wall is around an empty block, it's also set as empty
	for( int i = 0; i < mapDims[0]; i++ ){
		for( int j = 0; j < mapDims[1]; j++){
			
			//only set walls around a solid block to be solid
			if( mapArr[i*mapDims[1] + j]->isWall ){
				mapVLines[V_WALL_CNT*i + j] = mapArr[i*mapDims[1] + j];
				mapVLines[V_WALL_CNT*i + j + 1] = mapArr[i*mapDims[1] + j];
				
				mapHLines[mapDims[1]*i + j] = mapArr[i*mapDims[1] + j];
				mapHLines[mapDims[1]*(i + 1) + j] = mapArr[i*mapDims[1] + j];
			}
		}
	}
}

//to print the map to the console, only for debugging
void GameMap::printMap(){
	
	printf("Whole map:\n");
	for( int i = 0; i < mapDims[0]; i++ ){
		for( int j = 0; j < mapDims[1]; j++ ){
			//prints 1 if it's a solid block, 0 if it's empty
			printf( "%d ", (int)mapArr[i*mapDims[1] + j]->isWall );
		}
		printf("\n");
	}
	printf("\n");
	
	printf("Vertical walls:\n");
	for( int i = 0; i < mapDims[0]; i++ ){
		for( int j = 0; j < V_WALL_CNT; j++ ){
			//prints 1 if it's a solid wall, 0 if empty
			printf( "%d ", (int)mapVLines[i*V_WALL_CNT + j]->isWall );
		}
		printf("\n");
	}
	printf("\n");
	
	printf("Horizontal walls:\n");
	for( int i = 0; i < H_WALL_CNT; i++ ){
		for( int j = 0; j < mapDims[1]; j++ ){
			//prints 1 if it's a solid wall, 0 if empty
			printf( "%d ", (int)mapHLines[i*mapDims[1] + j]->isWall );
		}
		printf("\n");
	}
	printf("\n");
}

//draws the map onto the surface passed to the function
void GameMap::drawMap(SDL_Surface* mapSurf){
	for( int i = 0; i < mapDims[0]; i++ ){
		for( int j = 0; j < mapDims[1]; j++ ){
			SDL_Rect blockRect;
			blockRect.x = j*BLOCK_DIM;
			blockRect.y = i*BLOCK_DIM;
			blockRect.w = BLOCK_DIM; blockRect.h = BLOCK_DIM;
			
			SDL_FillRect( mapSurf, &blockRect, SDL_MapRGB(mapSurf->format,
						mapArr[i*mapDims[1] + j]->colors[0],
						mapArr[i*mapDims[1] + j]->colors[1],
						mapArr[i*mapDims[1] + j]->colors[2])
						);
		}
	}
}

void GameMap::drawPlayer(SDL_Surface *screenSurf){
	//drawing player square
	SDL_Rect playRect;
	playRect.x = player.x-player.objDim/2;
	playRect.y = player.y-player.objDim/2;
	playRect.w = player.objDim; playRect.h = player.objDim;
	SDL_FillRect( screenSurf, &playRect, SDL_MapRGB(screenSurf->format, 255, 255, 0 ) );
	
	playRect.x = player.x + player.objDim*std::cos(player.ang);
	playRect.y = player.y - player.objDim*std::sin(player.ang);
	playRect.w = 5; playRect.h = 5;
	SDL_FillRect( screenSurf, &playRect, SDL_MapRGB(screenSurf->format, 255, 0, 0 ) );
}

void GameMap::initPlayer(double x, double y, double ang, double playerDim){
	player.update(x, y, ang, playerDim);
}

void GameMap::movePlayer(std::set<int> keys, double speed, double angVel, double dt){
	
	double moveX, moveY;
	moveX = moveY = 0.0;
	
	if( keys.count(SDLK_SPACE) != 0 ){
		speed *= 2;
		angVel *= 2;
	}
	for( int n : keys ){
		switch(n){
			
			case SDLK_a:
				player.ang += angVel*dt;
				if( player.ang > 2*PI )
					player.ang -= 2*PI;
				break;
			
			case SDLK_s:
				player.ang -= angVel*dt;
				if( player.ang < 2*PI )
					player.ang += 2*PI;
				break;
				
			case SDLK_UP:
				moveX += std::cos(player.ang)*speed*dt;
				moveY -= std::sin(player.ang)*speed*dt;
				break;
			
			case SDLK_DOWN:
				moveX -= std::cos(player.ang)*speed*dt;
				moveY += std::sin(player.ang)*speed*dt;
				break;
				
			case SDLK_RIGHT:
				moveX += std::sin(player.ang)*speed*dt;
				moveY += std::cos(player.ang)*speed*dt;
				break;
			
			case SDLK_LEFT:
				moveX -= std::sin(player.ang)*speed*dt;
				moveY -= std::cos(player.ang)*speed*dt;
				break;
		}
	}
	
	player.x += moveX; player.y += moveY;
	
	if( tryMove(player) )
		return;
	
	//clipping x direction movement
	player.x -= moveX;
	
	if( tryMove(player) )
		return;
	
	//clipping y direction movement
	player.x += moveX;
	player.y -= moveY;
	
	if( tryMove(player) )
		return;
	
	//clipping full movement
	player.x -= moveX;
	
	return;
}

bool GameMap::tryMove(MapObject player){
	int xl, xh, yl, yh;
	xl = (int)((player.x - player.objDim/2)/BLOCK_DIM);
	xh = (int)((player.x + player.objDim/2)/BLOCK_DIM);
	yl = (int)((player.y - player.objDim/2)/BLOCK_DIM);
	yh = (int)((player.y + player.objDim/2)/BLOCK_DIM);
	
	for( int y = yl; y <= yh; y++ ){
		for( int x = xl; x <= xh; x++ ){
			if( mapArr[y*mapDims[1] + x]->isWall )
				return false;
		}
	}
	
	return true;
}
//};
/*
int main( int argc, char* args[] ){
	GameMap g("./Images/tester.bmp");
	g.printMap();
	return 0;
}*/