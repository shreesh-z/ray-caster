#include <stdio.h>
#include <cmath>
#include "MapObject.h"
#include "GameMap.h"

#define PI 3.1415926535897932384

const unsigned TILESHIFT = 6;

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
	playRect.x = ( screenSurf->w - objDim ) >> 1;
	playRect.y = ( screenSurf->h - objDim ) >> 1;
	playRect.w = objDim >> 1; playRect.h = objDim >> 1;
	SDL_FillRect( screenSurf, &playRect, SDL_MapRGB(screenSurf->format, 255, 255, 0 ) );
	
	playRect.x = (screenSurf->w >> 1) + objDim*std::cos(ang);
	playRect.y = (screenSurf->h >> 1) - objDim*std::sin(ang);
	playRect.w = 2; playRect.h = 2;
	SDL_FillRect( screenSurf, &playRect, SDL_MapRGB(screenSurf->format, 255, 0, 0 ) );
}

void MapObject::move(GameMap *gMap, double dx, double dy, double dang, bool rotate){
	
	ang += dang;
	if( ang > 2*PI )	//angle is modulo 2pi
		ang -= 2*PI;
	else if( ang < 0 )
		ang += 2*PI;
	
	double moveX = dx;
	double moveY = dy;
	
	if( rotate ){
		//projecting motion in object's reference frame axes to grid axes
		moveX = std::cos(ang)*dx + std::sin(ang)*dy;
		moveY = -std::sin(ang)*dx + std::cos(ang)*dy;
	}
	
	if( moveX == 0.0 && moveY == 0.0 )
		return;
	
	x += moveX; y += moveY;
	
	if( tryMove(gMap) )
		return;
	
	//clipping x direction movement
	x -= moveX;
	
	if( tryMove(gMap) )
		return;
	
	//clipping y direction movement
	x += moveX;
	y -= moveY;
	
	if( tryMove(gMap) )
		return;
	
	//clipping full movement if nothing works
	x -= moveX;
	
	return;
}

bool MapObject::tryMove(GameMap *gMap){
	int xl, xh, yl, yh;
	xl = ((int)x - (objDim >> 1)) >> TILESHIFT;
	xh = ((int)x + (objDim >> 1)) >> TILESHIFT;
	yl = ((int)y - (objDim >> 1)) >> TILESHIFT;
	yh = ((int)y + (objDim >> 1)) >> TILESHIFT;
	
	for( int y_ = yl; y_ <= yh; y_++ ){
		for( int x_ = xl; x_ <= xh; x_++ ){
			if( gMap->solid_block_at( y_, x_ ) )	// that gMap function also does out of bounds checking
				return false;
		}
	}
	
	return true;
}

/*void Agent::reset(){
	has_seen = false;
}

bool Agent::check_for_player(MapObject *player){
	if( has_seen ) return true;
	int diffX = (int)(x - player.x) >> TILESHIFT;
	int diffY = (int)(y - player.y) >> TILESHIFT;
	if( std::hypot(diffX, diffY) < 10 )
		has_seen = true;
	return has_seen;
}

Agent::follow_player(MapObject *player){
	if( check_player( player ) ){
		
	}
}*/