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
			
			for( int k = 2; k >= 0; k-- ){
				
				//setting block's color tuple to pixel's color tuple
				newBlock->colors[k] = pixel[i*mapImg->pitch + j + 2 - k];
				
				//if any tuple value is nonzero, it's a wall block
				if( newBlock->colors[k] != 0 )
					newBlock->isWall = true;
				else
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
				if( player.ang < 0 )
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
	xl = (int)(player.x - player.objDim/2) >> TILESHIFT;
	xh = (int)(player.x + player.objDim/2) >> TILESHIFT;
	yl = (int)(player.y - player.objDim/2) >> TILESHIFT;
	yh = (int)(player.y + player.objDim/2) >> TILESHIFT;
	
	for( int y = yl; y <= yh; y++ ){
		for( int x = xl; x <= xh; x++ ){
			if( mapArr[y*mapDims[1] + x]->isWall )
				return false;
		}
	}
	
	return true;
}

void GameMap::castRays(SDL_Surface *screenSurf, int angRange, int depth, double wallColorRatio){
	int wid = screenSurf->w, hig = screenSurf->h;
	double radAngRange = (PI*angRange)/180.0; //angle in radians
	int rayCount = wid;
	double screenDist = (double)wid/( 2*std::tan(radAngRange) );
	//printf("%lf, %lf\n", radAngRange, screenDist);
	//int rayWid = 1;
	int rayHig;
	//double rayCenterX = 0.5;
	
	double hRayX, hRayY, vRayX, vRayY, h_xOffs, h_yOffs, v_xOffs, v_yOffs;
	int vmapX, vmapY, hmapX, hmapY;
	
	vmapX = vmapY = hmapX = hmapY = 0;
	
	double rayAng = player.ang + radAngRange;
	
	Uint8 wallColor[3];
	
	for( int i = 0; i < rayCount; i++ ){
	
		double rAng = rayAng;
		if( rAng > 2*PI )
			rAng -= 2*PI;
		if( rAng < 0 )
			rAng += 2*PI;
		
		//printf("Reached\n");
		
		double tanAng = std::tan(rAng);
		int dof = 0;
		
		//first, checking for horizontal wall collisions            
		if ( rAng > PI ){                 //looking down
			//projecting the ray onto the vertical grid line
			hRayY = (double)( ( ( (int)(player.y) >> TILESHIFT ) << TILESHIFT ) + BLOCK_DIM );
			//calculating the point of intersection with nearest horiz. wall
			hRayX = player.x - (hRayY - player.y)/tanAng;
			
			//calculating the offsets for further ray casting
			h_yOffs = (double)BLOCK_DIM;
			h_xOffs = -h_yOffs/tanAng;
			
		}else if ( rAng < PI and rAng > 0 ){ //looking up
			hRayY = (double)( ( (int)(player.y) >> TILESHIFT ) << TILESHIFT );
			hRayX = player.x + (player.y - hRayY)/tanAng;
			h_yOffs = -(double)BLOCK_DIM;
			h_xOffs = -h_yOffs/tanAng;
			
		}else{
			//if the ray is perfectly left or right, dont' cast it,
			//as it will never meet a horizontal wall
			hRayX = player.x;
			hRayY = player.y;
			dof = depth;
		}
		
		
			//casting until depth of field is reached
		while( dof < depth ){
			//extracting the map indices from ray positions
			hmapX = (int)(hRayX) >> TILESHIFT;
			hmapY = (int)(hRayY) >> TILESHIFT;
			
			//if the ray hit a horiz. wall
			if( hmapX >= 0 && hmapX < mapDims[1] 
				&& hmapY >= 0 && hmapY < mapDims[0]
				&& mapHLines[hmapY*mapDims[1] + hmapX]->isWall )
				break;
			//if not, keep going
			else{
				hRayX += h_xOffs;
				hRayY += h_yOffs;
			}
			dof++;
		}
		
		dof = 0;
		
		//then, checking for vertical wall collisions
		if( rAng > PI/2 && rAng < (3*PI)/2 ){   //looking left
			//projecting the ray onto a horizontal grid line
			vRayX = (double)( ( (int)player.x >> TILESHIFT ) << TILESHIFT );
			//calculating the point of intersection with the nearest vert. wall
			vRayY = player.y + (player.x - vRayX)*tanAng;
			
			//calculating the offsets for further ray casting
			v_xOffs = -(double)BLOCK_DIM;
			v_yOffs = -v_xOffs*tanAng;
			
		}else if( rAng > (3*PI)/2 || rAng < PI*2 ){  //looking right
			vRayX = (double)( ( ( (int)player.x >> TILESHIFT ) << TILESHIFT ) + BLOCK_DIM );
			vRayY = player.y - (vRayX - player.x)*tanAng;
			v_xOffs = (double)BLOCK_DIM;
			v_yOffs = -v_xOffs*tanAng;
		}else{
			//if the ray is perfectly up or down, don't cast it,
			//as it will never meet a vertical wall
			vRayX = player.x;
			vRayY = player.y;
			dof = depth;
		}
			
		//casting until depth of field is reached
		while( dof < depth ){
			//extracting the map indices from ray positions
			vmapX = (int)(vRayX) >> TILESHIFT;
			vmapY = (int)(vRayY) >> TILESHIFT;
			
			//if the ray hit a vert. wall
			if( vmapX >= 0 && vmapX < mapDims[1] 
				&& vmapY >= 0 && vmapY < mapDims[0]
				&& mapVLines[vmapY*V_WALL_CNT + vmapX]->isWall )
				break;
			//if not, keep going
			else{
				vRayX += v_xOffs;
				vRayY += v_yOffs;
			}
			dof++;
		}
		
		double vDist = std::fabs((player.x - vRayX)/std::cos(rAng));
		double hDist = std::fabs((player.x - hRayX)/std::cos(rAng));
		double finalDist;
		
		if( vDist > hDist ){
			finalDist = hDist;
			for( int j = 0; j < 3; j++ )
				wallColor[j] = mapHLines[hmapY*mapDims[1] + hmapX]->colors[j];
		}else{
			finalDist = vDist;
			for( int j = 0; j < 3; j++ )
				wallColor[j] = wallColorRatio*mapVLines[vmapY*V_WALL_CNT + vmapX]->colors[j];
		}
		
		finalDist *= std::cos(rAng - player.ang);
		
		if( finalDist == 0.0 ){
			//printf("Reached\n");
			rayAng += (2*radAngRange)/rayCount;
			continue;
		}
		
		rayHig = ( BLOCK_DIM * screenDist )/finalDist;
		
		rayHig = rayHig > hig ? hig : rayHig;
		
		SDL_Rect slice;
		slice.y = (hig - rayHig) >> 1; slice.x = i;
		//slice.x = 0; slice.y = i;
		slice.h = rayHig; slice.w = 2;
		
		//printf("%d, %d, %d, %d\n", slice.x, slice.y, slice.w, slice. h);
		
		SDL_FillRect( screenSurf, &slice, SDL_MapRGB(screenSurf->format, wallColor[0], wallColor[1], wallColor[2] ) );
		
		rayAng -= (2*radAngRange)/rayCount;
	}
}
//};
/*
int main( int argc, char* args[] ){
	GameMap g("./Images/tester.bmp");
	g.printMap();
	return 0;
}*/