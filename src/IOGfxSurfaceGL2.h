#ifndef IOGFXSURFACESW_H
#define IOGFXSURFACESW_H

#include "IOGfxSurface.h"

#include "SDL.h"

#include "IOGfxGLFuncs.h"

class IOGfxDisplayGL2;
class IOGfxSurfaceGL2: public IOGfxSurface {
public:
	IOGfxDisplayGL2* display;
	GLuint texture;
	int w, h;
	SDL_Color colorkey;

	IOGfxSurfaceGL2(IOGfxDisplayGL2* display, GLuint texture, int w, int h, SDL_Color colorkey);
	virtual ~IOGfxSurfaceGL2();
	virtual void fill_screen(int num, SDL_Color* palette);
	virtual void fillRect(const SDL_Rect *rect, Uint8 r, Uint8 g, Uint8 b);
	virtual void vlineRGB(Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b);
	virtual void hlineRGB(Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b);
	virtual void drawBox(rect box, int color);
	int internalBlit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect, bool useColorKey);
	virtual int blit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect);
	virtual int blitStretch(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect);
	virtual int blitNoColorKey(IOGfxSurface *src, const SDL_Rect *srcrect, SDL_Rect *dstrect);
	virtual unsigned int getMemUsage();
};

#endif
