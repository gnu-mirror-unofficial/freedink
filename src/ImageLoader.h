#ifndef SPRITELOADER_H
#define SPRITELOADER_H

#include "SDL.h"

extern SDL_Color GFX_ref_pal[256];

class ImageLoader {
public:
	static SDL_Surface* blitFormat;
	static SDL_Surface* loadToBlitFormat(FILE* in);
	static SDL_Surface* convertToPaletteFormat(SDL_Surface* image);
};

#endif
