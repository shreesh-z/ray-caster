#include <blocks.h>
#include <SDL2/SDL.h>

const unsigned BLOCK_DIM = 64;
const unsigned TILESHIFT = 6;

//for a general block
ColorBlock::ColorBlock(Uint8 R_, Uint8 G_, Uint8 B_, bool isWall_, double wallColorRatio ){
	colors[0] = R_;	dark_colors[0] = (Uint8)(wallColorRatio*R_);
	colors[1] = G_; dark_colors[1] = (Uint8)(wallColorRatio*G_);
	colors[2] = B_; dark_colors[2] = (Uint8)(wallColorRatio*B_);
	
	wall_texture = NULL;
	
	isWall = isWall_;
	
	seen = false;
}
//default block is an empty block
ColorBlock::ColorBlock(double wallColorRatio) : ColorBlock(0, 0, 0, false, wallColorRatio){}

void ColorBlock::blit_wall_to_screen( SDL_Renderer *renderer, SDL_Rect *dstRect,
								int offset, int offset_y, bool isVert ){
	
	seen = true;
	
	//texture offset doesn't matter, just fill the rect with this block's color
	if( isVert )
		SDL_SetRenderDrawColor( renderer, dark_colors[0], dark_colors[1], dark_colors[2], 255 );
	else
		SDL_SetRenderDrawColor( renderer, colors[0], colors[1], colors[2], 255 );
	
	SDL_RenderFillRect( renderer, dstRect );
	
}

void ColorBlock::blit_wall_to_2d_screen( SDL_Renderer *renderer, SDL_Rect *dstRect ){
	if( seen ){	
		//directly fill with the block color
		SDL_SetRenderDrawColor( renderer, colors[0], colors[1], colors[2], 255 );
		SDL_RenderFillRect( renderer, dstRect );
	}
}

TextureBlock::TextureBlock( SDL_Texture *wall_textures, SDL_Texture *dark_wall_textures,
						int texture_offset_ ){
	
	wall_texture = wall_textures;
	
	dark_wall_texture = dark_wall_textures;
	
	//texture_offset * 64 is the actual position where this block's texture lies
	texture_offset = texture_offset_ << TILESHIFT;
	
	//textured blocks are always solid
	isWall = true;
	
	seen = false;
}

void TextureBlock::blit_wall_to_screen( SDL_Renderer *renderer, SDL_Rect *dstRect,
									int offset, int offset_y, bool isVert ){
	
	seen = true;
	
	SDL_Rect srcRect;
	
	//first the correct wall texture is found, then the horiz offset is applied
	srcRect.x = texture_offset + offset;
	srcRect.y = offset_y;

	srcRect.w = 1; srcRect.h = BLOCK_DIM - ( offset_y << 1 );
	
	if( isVert )
		SDL_RenderCopy( renderer, dark_wall_texture, &srcRect, dstRect );
	else
		SDL_RenderCopy( renderer, wall_texture, &srcRect, dstRect );
	
}

void TextureBlock::blit_wall_to_2d_screen( SDL_Renderer *renderer, SDL_Rect *dstRect ){
	
	if( !seen ) return;
	
	SDL_Rect srcRect;
	srcRect.x = texture_offset; srcRect.y = 0;
	srcRect.w = BLOCK_DIM; srcRect.h = BLOCK_DIM;
	
	//scaling is done, otherwise it doesn't work properly
	SDL_RenderCopy( renderer, wall_texture, &srcRect, dstRect );
}