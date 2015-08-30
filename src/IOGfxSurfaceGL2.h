#ifndef IOGFXSURFACEGL2_H
#define IOGFXSURFACEGL2_H

#include "IOGfxSurface.h"

#include "SDL.h"

#include "IOGfxGLFuncs.h"

class IOGfxDisplayGL2;
class IOGfxSurfaceGL2: public IOGfxSurface {
public:
	IOGfxDisplayGL2* display;
	GLuint texture;
	SDL_Color colorkey;

	IOGfxSurfaceGL2(IOGfxDisplayGL2* display, GLuint texture, int w, int h, SDL_Color colorkey);
	virtual ~IOGfxSurfaceGL2();
	virtual void fill_screen(int num, SDL_Color* palette);
	virtual int fillRect(const SDL_Rect *rect, Uint8 r, Uint8 g, Uint8 b);
	int internalBlit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect, bool useColorKey);
	virtual int blit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect);
	virtual int blitStretch(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect);
	virtual int blitNoColorKey(IOGfxSurface *src, const SDL_Rect *srcrect, SDL_Rect *dstrect);
	virtual unsigned int getMemUsage();
};

#endif
