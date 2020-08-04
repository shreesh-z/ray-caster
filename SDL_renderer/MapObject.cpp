#include <stdio.h>
#include "MapObject.h"

MapObject::MapObject(double x_, double y_, double ang_, double objDim_){
	x = x_; y = y_;
	ang = ang_;
	objDim = objDim_;
}
MapObject::MapObject() : MapObject::MapObject(0, 0, 0, 10) {}

void MapObject::print(){
	printf( "%lf %lf %lf %lf \n", x, y, ang, objDim );
}

void MapObject::update(double x_, double y_, double ang_, double objDim_){
	x = x_; y = y_;
	ang = ang_;
	objDim = objDim_;
}