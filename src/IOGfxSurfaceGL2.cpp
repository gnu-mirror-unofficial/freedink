#include "IOGfxSurfaceGL2.h"

#include "SDL.h"
#include "log.h"
#include "IOGfxGLFuncs.h"

IOGfxSurfaceGL2::IOGfxSurfaceGL2(IOGfxDisplayGL2* display, GLuint texture, int w, int h)
	: display(display), texture(texture), w(w), h(h) {
}

IOGfxSurfaceGL2::~IOGfxSurfaceGL2() {
	display->gl->DeleteTextures(1, &texture);
}

/* Function specifically made for Dink'C fill_screen() */
void IOGfxSurfaceGL2::fill_screen(int num, SDL_Color* palette) {
}

void IOGfxSurfaceGL2::fillRect(const SDL_Rect *rect, Uint8 r, Uint8 g, Uint8 b) {
}

void IOGfxSurfaceGL2::vlineRGB(Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b) {
}
void IOGfxSurfaceGL2::hlineRGB(Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b) {
}


void IOGfxSurfaceGL2::drawBox(rect box, int color) {
}

int IOGfxSurfaceGL2::blit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blit: passed a NULL surface");
	//GLuint src_tex = dynamic_cast<IOGfxSurfaceGL2*>(src)->texture;
	return -1;
}

int IOGfxSurfaceGL2::blitStretch(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blitStretch: passed a NULL surface");
	//GLuint src_tex = dynamic_cast<IOGfxSurfaceGL2*>(src)->texture;
	return -1;
}


int IOGfxSurfaceGL2::blitNoColorKey(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blitNoColorKeyt: passed a NULL surface");
	//GLuint src_tex = dynamic_cast<IOGfxSurfaceGL2*>(src)->texture;
	return -1;
}

unsigned int IOGfxSurfaceGL2::getMemUsage() {
	return 0;
}
