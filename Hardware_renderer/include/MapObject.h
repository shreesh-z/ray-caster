#ifndef OBJECT_H
#define OBJECT_H

#include "GameMap.h"
#include "custom_math.h"
#include <SDL2/SDL.h>
#include <vector>
#include <queue>
	
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
			int move(GameMap *gMap, std::vector<MapObject*> &agent_arr,
						double dx, double dy, double dang, bool rotate);
			
			//tells if the current position of the object is valid or invalid
			bool tryMove(GameMap *gMap, std::vector<MapObject*> &agent_arr);
			
			//only applicable to enemies
			virtual bool follow_player(GameMap *gmap, std::vector<MapObject*> &agent_arr, double dt) = 0;
			
			//draw 2D sprite on the top-down view screen
			virtual void sprite2D(SDL_Renderer *renderer) = 0;
			
			virtual void double_speed() = 0;
	};
	
	//special map object: the player
	class Player : public MapObject{
		
		public:
			Player(double x_, double y_, double ang_, int objDim_);
			Player();
			
			//draws the player at the center of the top-down view screen
			void sprite2D( SDL_Renderer *renderer );
			
			//does nothing, returns false
			bool follow_player(GameMap *gmap, std::vector<MapObject*> &agent_arr, double dt);
			
			void double_speed();
	};
	
	//the enemies
	class Agent : public MapObject{
		private:
			
			double speed, angVel;
			int tile_radius;
			
			//if agent is supposed supposed to follow the player, and whether agent has seen the player yet
			//also whether player has been seen at least once
			bool has_seen, follow_player_flag, has_seen_once;
			
			//check is agent can see the player
			bool check_for_player(GameMap *gMap, MapObject *player);
			
		public:
		
			//the sprite of the agent
			SDL_Texture *spriteText;
			
			//the x and y diffs between enemy and player (from enemy's POV) and the full distance
			double diffX_player, diffY_player, diff_hypot;
			
			Agent(SDL_Texture *sprite, double posX, double posY, double ang_, int objDim_,
				int tile_radius_, double speed_, double angVel_ );
			
			//this is implemented properly
			bool follow_player(GameMap *gmap, std::vector<MapObject*> &agent_arr, double dt);
			
			//reset and forget you saw the player
			void reset();
			//reset, forget you saw the player, dont follow player again ever
			void reset_to_idle();
			
			//display agent sprite on the 2D top down view iff agent has seen the player
			void sprite2D(SDL_Renderer *renderer);
			
			void double_speed();
	};
	
	//comparator class for placing agents into distance based priority queue
	class CompareObjects{
		public:
			bool operator()( Agent *player, Agent *agent );
	};
	
	//to place enemies into a priority queue and draw them on the screen starting from the farthest away from player to nearest
	void draw_3D_sprites( SDL_Renderer *renderer, GameMap *gMap, MapObject *player, std::vector<MapObject*> &agent_arr, double spread );
	
#endif