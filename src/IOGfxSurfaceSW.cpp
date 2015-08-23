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

void IOGfxSurfaceSW::vlineRGB(Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b) {
	SDL_Rect dst = { x, y1, 1, y2-y1 };
	SDL_FillRect(s, &dst, SDL_MapRGB(s->format, r, g, b));
}
void IOGfxSurfaceSW::hlineRGB(Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b) {
	SDL_Rect dst = { x1, y, x2-x1, 1 };
	SDL_FillRect(s, &dst, SDL_MapRGB(s->format, r, g, b));
}


void IOGfxSurfaceSW::drawBox(rect box, int color) {
	SDL_Rect dst;
	dst.x = box.left; dst.y = box.top;
	dst.w = box.right - box.left;
	dst.h = box.bottom - box.top;
	SDL_FillRect(s, &dst, color);
}
