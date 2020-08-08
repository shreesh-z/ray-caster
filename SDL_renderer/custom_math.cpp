#include "custom_math.h"

#define PI 3.1415926535897932384
const unsigned BLOCK_DIM = 64;
const unsigned TILESHIFT = 6;

//returns actual angle for given base and height of triangle
double real_atan(double diffX, double diffY){
	double sprite_ang = std::atan( diffY / diffX );
	if( sprite_ang > 0 ){
		if( diffY < 0 && diffX < 0 )
			sprite_ang += PI;	//3rd quadrant
	}else{
		if( diffY > 0 && diffX < 0 )
			sprite_ang += PI;	//2nd quadrant
		else //if( diffY < 0 && diffX > 0 )
			sprite_ang += 2*PI;	//4th quadrant
	}
	//unchanged if 1st quadrant
	return sprite_ang;
}

//assumes angle is a sum/difference of two modulo 2pi numbers
double mod2PI( double ang ){
	if( ang < 0 )
		return ang + 2*PI;
	else if( ang > 2*PI )
		return ang - 2*PI;
	else
		return ang;
}
//assumes angle is modulo 2pi
double modPI( double ang ){
	if( ang > PI ){
		return ang - 2*PI;
	}else
		return ang;
}

bool cast_horiz_ray( GameMap *gMap, double posX, double posY, double rAng, int depth, int *mapX, int *mapY, double *dist ){
	double tanAng = std::tan(rAng);
	int dof = 0;
	double rayX, rayY, xOffs, yOffs;
	
	//first, checking for horizontal wall collisions            
	if ( rAng > PI ){	//looking down
		
		//projecting the ray onto the vertical grid line
		rayY = (double)( ( ( (int)(posY) >> TILESHIFT ) << TILESHIFT ) + BLOCK_DIM );
		//calculating the point of intersection with nearest horiz. wall
		rayX = posX - (rayY - posY)/tanAng;
		
		//calculating the offsets for further ray casting
		yOffs = (double)BLOCK_DIM;
		xOffs = -yOffs/tanAng;
		
	}else if ( rAng < PI && rAng > 0 ){ //looking up
		rayY = (double)( ( (int)(posY) >> TILESHIFT ) << TILESHIFT );
		rayX = posX + (posY - rayY)/tanAng;
		yOffs = -(double)BLOCK_DIM;
		xOffs = -yOffs/tanAng;
		
	}else{
		//if the ray is perfectly left or right, dont' cast it,
		//as it will never meet a horizontal wall
		rayX = posX;
		rayY = posY;
		dof = depth;
	}
	
	
	//casting until depth of field is reached
	while( dof < depth ){
		//extracting the map indices from ray positions
		*mapX = (int)(rayX) >> TILESHIFT;
		*mapY = (int)(rayY) >> TILESHIFT;
		
		//if the ray hit a horiz. wall
		if( gMap->solid_horiz_wall_at( *mapY, *mapX ) )
			break;
		//if not, keep going
		else{
			rayX += xOffs;
			rayY += yOffs;
		}
		dof++;
	}
	
	*dist = std::hypot(posX - rayX, posY - rayY);
	
	return dof == depth;
}

bool cast_vert_ray( GameMap *gMap, double posX, double posY, double rAng, int depth, int *mapX, int *mapY, double *dist ){
	double tanAng = std::tan(rAng);
	int dof = 0;
	double rayX, rayY, xOffs, yOffs;
	
	if( rAng > PI/2 && rAng < (3*PI)/2 ){   //looking left
			
		//projecting the ray onto a horizontal grid line
		rayX = (double)( ( (int)posX >> TILESHIFT ) << TILESHIFT );
		//calculating the point of intersection with the nearest vert. wall
		rayY = posY + (posX - rayX)*tanAng;
		
		//calculating the offsets for further ray casting
		xOffs = -(double)BLOCK_DIM;
		yOffs = -xOffs*tanAng;
		
	}else if( rAng > (3*PI)/2 || rAng < PI*2 ){  //looking right
		rayX = (double)( ( ( (int)posX >> TILESHIFT ) << TILESHIFT ) + BLOCK_DIM );
		rayY = posY - (rayX - posX)*tanAng;
		xOffs = (double)BLOCK_DIM;
		yOffs = -xOffs*tanAng;
	}else{
		//if the ray is perfectly up or down, don't cast it,
		//as it will never meet a vertical wall
		rayX = posX;
		rayY = posY;
		dof = depth;
	}
		
	//casting until depth of field is reached
	while( dof < depth ){
		//extracting the map indices from ray positions
		*mapX = (int)(rayX) >> TILESHIFT;
		*mapY = (int)(rayY) >> TILESHIFT;
		
		//if the ray hit a vert. wall
		if( gMap->solid_vert_wall_at( *mapY, *mapX ) )
			break;
		//if not, keep going
		else{
			rayX += xOffs;
			rayY += yOffs;
		}
		dof++;
	}
	
	*dist = std::hypot(posX - rayX, posY - rayY);
	
	return dof == depth;
}