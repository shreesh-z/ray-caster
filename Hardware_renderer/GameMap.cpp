#include <stdio.h>
#include <cmath>
#include <string>
#define PI 3.1415926535897932384

//Refer to this header file for documentation
//since header file already includes SDL, no need to include again
#include "GameMap.h"

//creates the game map by importing a bitmap file
//uses SDL's internal mechanisms to read the bitmap file and create the map array
GameMap::GameMap(SDL_Surface *mapImg, SDL_Texture *wall_textures, SDL_Texture *dark_wall_textures, double wallColorRatio){
	
	//default is 16, can't be changed as of now
	mapZoom = 16;
	
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
			
			//if wall is textured
			bool is_textured = false;
			
			//proxy color
			Uint8 color;
			//colors of the wall in RGB
			Uint8 colors[3];
			
			//checking if the block has textured walls
			for( int k = 0; k < 3; k++ ){
				
				color = pixel[i*mapImg->pitch + j + 2 - k];
				colors[k] = color;
				//red = 255
				//blue is used to encode which texture file to choose
				//blue values of 5 to 10 are considered, so a max of 6 textures are supported as of now
				is_textured = ( k == 0 && color == 255 ) || 
								( k == 1 && color == 0 ) ||
								( k == 2 && color >= 5 && color <= 10 );
			}
			
			Block *newBlock = NULL;
			
			if( is_textured ){
				
				//converting the color into an offset
				color -= 5;
				
				//creating a textured block
				newBlock = new TextureBlock( wall_textures, dark_wall_textures, color );
				
				for( int k = 0; k < 3; k++ )
					newBlock->colors[k] = colors[k];
			
			}else{
				
				//if this block has the player or an agent, set it to an empty block
				if( ( colors[0] == 0 and colors[1] == 255 and colors[2] == 1 )
					or( colors[0] == 1 and colors[1] == 0 and colors[2] == 255 ) ){
					colors[0] = colors[1] = colors[2] = 0;
				}
				
				//creating a colored block
				newBlock = new ColorBlock( colors[0], colors[1], colors[2], false, wallColorRatio );
				
				for( int k = 0; k < 3; k++ ){
					
					//if any tuple value is nonzero, it's a wall block
					if( newBlock->colors[k] != 0 )
						newBlock->isWall = true;
					else
						//setting it to a dark gray so that
						//when draw distance is reached, a full black isn't displayed
						newBlock->colors[k] = 50;
				}
			}
			
			mapArr[i*mapDims[1] + (int)(j/3)] = newBlock;
		}
	}
	
	//initializing these two arrays with empty blocks
	for( int i = 0; i < mapDims[0]*V_WALL_CNT; i++ ){
		mapVLines[i] = new ColorBlock(wallColorRatio);
	}
	for( int i = 0; i < mapDims[1]*H_WALL_CNT; i++ ){
		mapHLines[i] = new ColorBlock(wallColorRatio);
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
/*void GameMap::drawFullMap(SDL_Surface* mapSurf){
	
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
}*/

//center the display at posX, posY
void GameMap::draw2DMap(SDL_Renderer *renderer, int posX, int posY){
	
	//map positions of given surface at which this display should be centered
	int mapX = posX >> TILESHIFT;
	int mapY = posY >> TILESHIFT;
	
	//The posX, posY lies inside a block. The offsets are for how far away from the center of the screen the 
	//top let corner of said block will lie on the screen (after some processing)
	
	//right now xoffs and yoffs are distances from the posX and posY to the block's top left corner
	int xOffs = (posX - (mapX << TILESHIFT) );
	int yOffs = (posY - (mapY << TILESHIFT) );
	
	//the bounds of the map portion that will be displayed on the screen
	int mapxL, mapxH, mapyL, mapyH;
	
	//the bounds are decided by the amount of zoom
	mapxL = mapX - mapZoom; mapyL = mapY - mapZoom;
	mapxH = mapX + mapZoom; mapyH = mapY + mapZoom;
	
	//if zooming takes us out of bounds, cap it to min/max
	if( mapxL < 0 ) mapxL = 0;
	if( mapxH >= mapDims[1] ) mapxH = mapDims[1] - 1;
	if( mapyL < 0 ) mapyL = 0;
	if( mapyH >= mapDims[0] ) mapyH = mapDims[0] - 1;
	
	int wid, hig;
	SDL_GetRendererOutputSize( renderer, &wid, &hig );
	
	//to center xOffs and yOffs on the screen
	//now, xoffs and yoffs are only offsets of the block in which posX and posY lie
	xOffs = ((wid - xOffs) >> 1);// - xOffs;
	yOffs = ((hig - yOffs) >> 1);// - yOffs;
	
	//we have to subract the lengths of the blocks that will lie to the left and top of the posX, posY block
	//to get the offset of the first block that will be drawn onto the screen.
	xOffs -= (mapX - mapxL) << (TILESHIFT - 1);
	yOffs -= (mapY - mapyL) << (TILESHIFT - 1);
	
	for( int i = mapyL; i <= mapyH; i++ ){
		for( int j = mapxL; j <= mapxH; j++ ){
			
			SDL_Rect blockRect;
			blockRect.x = xOffs + ((j - mapxL) << (TILESHIFT - 1));
			blockRect.y = yOffs + ((i - mapyL) << (TILESHIFT - 1));
			blockRect.w = BLOCK_DIM >> 1; blockRect.h = BLOCK_DIM >> 1;
			
			//the block will handle the drawing
			mapArr[i*mapDims[1] + j]->blit_wall_to_2d_screen( renderer, &blockRect );
			
		}
	}
}