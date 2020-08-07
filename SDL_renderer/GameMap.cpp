#include <stdio.h>
#include <cmath>
#define PI 3.1415926535897932384

//Refer to this header file for documentation
//since header file already includes SDL, no need to include again
#include "GameMap.h"

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
	
	//default is 16, can't be changed as of now
	mapZoom = 16;
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
			
			for( int k = 2; k >= 0; k-- ){
				
				//setting block's color tuple to pixel's color tuple
				//bytes are stored in reverse order in the surface, ie BGR
				newBlock->colors[k] = pixel[i*mapImg->pitch + j + 2 - k];
				
				//if any tuple value is nonzero, it's a wall block
				if( newBlock->colors[k] != 0 )
					newBlock->isWall = true;
				else
					//setting it to a dark gray so that
					//when draw distance is reached, a full black isn't displayed
					newBlock->colors[k] = 50;
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
	//if the wall is around an empty block, nothing is done
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

GameMap::~GameMap(){
	for( int i = 0; i < mapDims[0]*mapDims[1]; i++ ){
		delete mapArr[i];
	}
	delete mapArr;
	delete mapVLines;
	delete mapHLines;
}


//for other components to extract array elements
Block* GameMap::block_at( int y, int x ){
	if( x >= 0 && x < mapDims[1] && y >= 0 && y < mapDims[0] )
		return mapArr[y*mapDims[1] + x];
	else
		return NULL;
}

Block* GameMap::horiz_wall_at( int y, int x ){
	if( x >= 0 && x < mapDims[1] && y >= 0 && y < H_WALL_CNT )
		return mapHLines[y*mapDims[1] + x];
	else
		return NULL;
}

Block* GameMap::vert_wall_at( int y, int x ){
	if( x >= 0 && x < V_WALL_CNT && y >= 0 && y < mapDims[0] )
		return mapVLines[y*V_WALL_CNT + x];
	else
		return NULL;
}

bool GameMap::solid_block_at( int y, int x ){
	if( x >= 0 && x < mapDims[1] && y >= 0 && y < mapDims[0] )
		return mapArr[y*mapDims[1] + x]->isWall;
	else
		return false;
}

bool GameMap::solid_horiz_wall_at( int y, int x ){
	if( x >= 0 && x < mapDims[1] && y >= 0 && y < H_WALL_CNT )
		return mapHLines[y*mapDims[1] + x]->isWall;
	else
		return false;
}

bool GameMap::solid_vert_wall_at( int y, int x ){
	if( x >= 0 && x < V_WALL_CNT && y >= 0 && y < mapDims[0] )
		return mapVLines[y*V_WALL_CNT + x]->isWall;
	else
		return false;
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

//draws the full map onto the surface passed to the function
void GameMap::drawFullMap(SDL_Surface* mapSurf){
	
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

void GameMap::draw2DMap(SDL_Surface *screenSurf, int posX, int posY){
	
	int mapX = posX >> TILESHIFT;
	int mapY = posY >> TILESHIFT;
	int xOffs = (posX - (mapX << TILESHIFT) ) >> 1;
	int yOffs = (posY - (mapY << TILESHIFT) ) >> 1;
	
	int mapxL, mapxH, mapyL, mapyH;
	mapxL = mapX - mapZoom; mapyL = mapY - mapZoom;
	mapxH = mapX + mapZoom; mapyH = mapY + mapZoom;
	if( mapxL < 0 ) mapxL = 0;
	if( mapxH >= mapDims[1] ) mapxH = mapDims[1] - 1;
	if( mapyL < 0 ) mapyL = 0;
	if( mapyH >= mapDims[0] ) mapyH = mapDims[0] - 1;
	
	xOffs = ((screenSurf->w) >> 1) - xOffs;
	yOffs = ((screenSurf->h) >> 1) - yOffs;
	
	xOffs -= (mapX - mapxL) << (TILESHIFT - 1);
	yOffs -= (mapY - mapyL) << (TILESHIFT - 1);
	
	for( int i = mapyL; i <= mapyH; i++ ){
		for( int j = mapxL; j <= mapxH; j++ ){
			SDL_Rect blockRect;
			blockRect.x = xOffs + ((j - mapxL) << (TILESHIFT - 1));
			blockRect.y = yOffs + ((i - mapyL) << (TILESHIFT - 1));
			blockRect.w = BLOCK_DIM >> 1; blockRect.h = BLOCK_DIM >> 1;
			
			SDL_FillRect( screenSurf, &blockRect, SDL_MapRGB(screenSurf->format,
						mapArr[i*mapDims[1] + j]->colors[0],
						mapArr[i*mapDims[1] + j]->colors[1],
						mapArr[i*mapDims[1] + j]->colors[2])
						);
		}
	}
}