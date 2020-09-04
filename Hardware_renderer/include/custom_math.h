#ifndef RAY_CAST_H
#define RAY_CAST_H

#include "GameMap.h"
#include <set>
	
	double real_atan(double diffX, double diffY);
	double mod2PI( double ang );
	double modPI( double ang );
	bool cast_vert_ray( GameMap *gMap, double posX, double posY, double rAng, int depth,
						int *mapX, int *mapY, double *dist, int *offset );
	bool cast_horiz_ray( GameMap *gMap, double posX, double posY, double rAng, int depth,
						int *mapX, int *mapY, double *dist, int *offset );

#endif