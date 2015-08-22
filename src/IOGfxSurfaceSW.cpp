#include "IOGfxSurfaceSW.h"

#include "SDL.h"

IOGfxSurfaceSW::IOGfxSurfaceSW(SDL_Surface* s) {
	this->s = s;
}

IOGfxSurfaceSW::~IOGfxSurfaceSW() {
	SDL_FreeSurface(s);
}

void IOGfxSurfaceSW::fill(int num, SDL_Color* palette) {
	/* Warning: palette indexes 0 and 255 are hard-coded
	   to black and white (cf. gfx_palette.c). */
	if (s->format->format == SDL_PIXELFORMAT_INDEX8)
		SDL_FillRect(s, NULL, num);
	else
		SDL_FillRect(s, NULL, SDL_MapRGB(s->format, palette[num].r, palette[num].g, palette[num].b));
}
