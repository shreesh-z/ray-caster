#ifndef OBJECT_H
#define OBJECT_H

#include "GameMap.h"
#include "custom_math.h"
#include <SDL2/SDL.h>
	
	//all kinds of map objects
	class MapObject{
		public:
			//position and angle
			double x, y, ang;
			//dimension of the square of the object
			int objDim;
			
			//move the object acc. to the given displacement
				//acc. to the walls in the map, and acc. to other objects in the map
			//clipping of displacement is performed by the next funct
			int move(GameMap *gMap, MapObject **agent_arr, int agent_cnt,
						double dx, double dy, double dang, bool rotate);
			
			//tells if the current position of the object is valid or invalid
			bool tryMove(GameMap *gMap, MapObject **agent_arr, int agent_cnt);
			
			//only applicable to enemies
			virtual bool follow_player(GameMap *gmap, MapObject **agent_arr, int agent_cnt, 
								int tile_radius, double speed, double angVel, double dt) = 0;
			
			//only applicable to enemies
			virtual void sprite3D(GameMap *gMap, SDL_Surface *screenSurf, MapObject *player, double spread) = 0;
			
			//draw 2D sprite on the top-down view screen
			virtual void sprite2D(SDL_Surface *screenSurf, MapObject *player) = 0;
	};
	
	//special map object: the player
	class Player : public MapObject{
		
		public:
			Player(double x_, double y_, double ang_, int objDim_);
			Player();
			
			//draws the player at the center of the top-down view screen
			void sprite2D( SDL_Surface *screenSurf, MapObject *player);
			
			//does nothing, returns nothing
			void sprite3D( GameMap *gMap, SDL_Surface *screenSurf, MapObject *player, double spread );
			//does nothing, returns false
			bool follow_player(GameMap *gmap, MapObject **agent_arr, int agent_cnt, 
								int tile_radius, double speed, double angVel, double dt);
	};
	
	//the enemies
	class Agent : public MapObject{
		private:
			
			//if agent is supposed supposed to follow the player, and whether agent has seen the player yet
			//also whether player has been seen at least once
			bool has_seen, follow_player_flag, has_seen_once;
			//the sprite of the agent
			SDL_Surface *spriteSurf;
			
			//check is agent can see the player
			bool check_for_player(GameMap *gMap, MapObject *player, int tile_radius);
			
		public:
			
			Agent(SDL_Surface *surf, double posX, double posY, double ang_, int objDim_);
			
			//this is implemented properly
			bool follow_player(GameMap *gmap, MapObject **agent_arr, int agent_cnt, 
								int tile_radius, double speed, double angVel, double dt);
			
			//reset and forget you saw the player
			void reset();
			//reset, forget you saw the player, dont follow player again ever
			void reset_to_idle();
			
			//display agent sprite on the screen in the player's field of view
			void sprite3D(GameMap *gMap, SDL_Surface *screenSurf, MapObject *player, double spread);
			//display agent sprite on the 2D top down view iff agent has seen the player
			void sprite2D(SDL_Surface *screenSurf, MapObject *player);
	};
	
#endif