#ifndef BLOCKS_H
#define BLOCKS_H

#include <SDL2/SDL.h>

	class Block{
			
		public:
		
			SDL_Surface *wall_texture;
			
			//Holds the color tuple for the wall colors
			Uint8 colors[3];
			
			//If this block is a wall
			bool isWall;
			
			virtual void blit_wall_to_screen( SDL_Surface *screenSurf, SDL_Rect *dstRect,
									int offset, int offset_y, bool vert );
									
			virtual void blit_wall_to_2d_screen( SDL_Surface *screenSurf, SDL_Rect *dstRect );
	};

	class ColorBlock : public Block{
		
		public:
		
			Uint8 dark_colors[3];
			
			//constructor for a general solid colored block
			ColorBlock(Uint8 R_, Uint8 G_, Uint8 B_, bool isWall_, double wallColorRatio);
			ColorBlock(double wallColorRatio);
			
			void blit_wall_to_screen( SDL_Surface *screenSurf, SDL_Rect *dstRect,
								int offset, int offset_y, bool vert );
			
			void blit_wall_to_2d_screen( SDL_Surface *screenSurf, SDL_Rect *dstRect );
	};

	class TextureBlock : public Block{
			
		public:
			
			SDL_Surface *dark_wall_texture;
			int texture_offset;
			
			TextureBlock(SDL_Surface *wall_textures, SDL_Surface *dark_wall_textures,
						int texture_offset_ );
			
			void blit_wall_to_screen( SDL_Surface *screenSurf, SDL_Rect *dstRect,
									int offset, int offset_y, bool vert );
			
			void blit_wall_to_2d_screen( SDL_Surface *screenSurf, SDL_Rect *dstRect );
	};

#endif