#ifndef GAME_MAP
#define GAME_MAP

#include <SDL2/SDL.h>
#include "blocks.h"

class GameMap{
	private:
		//dimension of the map by num of blocks in x and y directions
		int mapDims[2];
		//the map array of blocks
		Block **mapArr;
		//the vert and horiz walls in the map
		Block **mapVLines, **mapHLines;
		
		Block *texture_blocks[5];
		
		//Number of horiz and vertical walls
		int H_WALL_CNT, V_WALL_CNT;
		
		//MapObject player;
		
		int mapZoom;
	
	public:
		//Width of a wall
		const unsigned BLOCK_DIM = 64;
		const unsigned TILESHIFT = 6;
		
		//creates the game map by importing a bitmap file
		//uses SDL's internal mechanisms to read the bitmap file and create the map array
		GameMap(SDL_Surface *mapImg, SDL_Surface *wall_textures, SDL_Surface *dark_wall_textures, double wallColorRatio);
		//~GameMap();
		
		//prints the map to the console
		void printMap();
		//draws the map onto the screen surface passed to it
		void drawFullMap(SDL_Surface *screenSurf);
		
		Block* block_at( int x, int y );
		Block* horiz_wall_at( int x, int y );
		Block* vert_wall_at( int x, int y );
		bool solid_block_at( int y, int x );
		bool solid_horiz_wall_at( int x, int y );
		bool solid_vert_wall_at( int x, int y );
		
		void draw2DMap(SDL_Surface *screenSurf, int posX, int posY);
};

#endif