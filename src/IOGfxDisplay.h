#ifndef IOGFXDISPLAY_H
#define IOGFXDISPLAY_H

#include "SDL.h"

#include "IOGfxSurface.h"

class IOGfxDisplay {
public:
	int w, h;
	Uint32 flags;
	SDL_Window* window;

	IOGfxDisplay(int w, int h, Uint32 flags);
	virtual ~IOGfxDisplay();
	virtual bool open() = 0;
	virtual void close() = 0;

	virtual bool createWindow();
	virtual void logWindowInfo();

	virtual void clearWindow() = 0;
	virtual void flip(IOGfxSurface* backbuffer) = 0;
};

extern void gfx_toggle_fullscreen();
extern void flip_it(void);

#endif
