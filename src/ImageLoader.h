#ifndef SPRITELOADER_H
#define SPRITELOADER_H

#include "SDL.h"

extern SDL_Color GFX_ref_pal[256];

class ImageLoader {
public:
	static SDL_Surface* loadToFormat(FILE* in, SDL_PixelFormat* refFmt);
};

#endif
