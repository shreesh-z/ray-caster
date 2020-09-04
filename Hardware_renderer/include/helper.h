#ifndef HELPER_H
#define HELPER_H
#include "GameMap.h"
#include "MapObject.h"
#include <SDL2/SDL.h>
#include <vector>

	void castRays(GameMap *gMap, MapObject *player, SDL_Renderer *renderer, int angRange, int depth);
	bool input(GameMap *gMap, MapObject *player, std::vector<MapObject*> &agent_arr,
				std::set<int> keys, double speed, double angVel, double dt);
	bool checkWhiteBlock( GameMap *gMap, MapObject *player );
	void create_dark_walls(SDL_Surface *wall_textures, SDL_Surface *dark_wall_textures, double wallColorRatio);
	void frame_rate( SDL_Renderer *renderer, SDL_Texture *numbers, int fps );

#endif