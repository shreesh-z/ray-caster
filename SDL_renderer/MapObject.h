#ifndef OBJECT_H
#define OBJECT_H

#include "GameMap.h"
#include "custom_math.h"
#include <SDL2/SDL.h>
	
	class MapObject{
		public:
			double x, y, ang;
			int objDim;
			
			MapObject(double x_, double y_, double ang_, int objDim_);
			MapObject();
			
			void update(double x_, double y_, double ang_, int objDim_);
			
			void move(GameMap *gMap, double dx, double dy, double dang, bool rotate);
			
			bool tryMove(GameMap *gMap);
			
			//draws at original x,y position on the screen
			void draw(SDL_Surface *screenSurf);
			
			//draws at the center of the screen (for camera follow view)
			void draw2DMap(SDL_Surface *screenSurf);
			
			void print();
			
	};
	
	class Agent : public MapObject{
		private:
			bool has_seen, follow_player_flag;
			SDL_Surface *spriteSurf;
			bool check_for_player(MapObject *player, int tile_radius);
			
		public:
			Agent(SDL_Surface *surf, double posX, double posY, double ang_);
			void follow_player(GameMap *gmap, MapObject *player, int tile_radius, double speed, double angVel, double dt);
			void reset();
			void reset_to_idle();
			
			void sprite3D(GameMap *gMap, SDL_Surface *screenSurf, MapObject *player, double spread);
	};
	
#endif