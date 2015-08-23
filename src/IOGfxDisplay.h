#ifndef IOGFXDISPLAY_H
#define IOGFXDISPLAY_H

#include "SDL.h"

#include "IOGfxSurface.h"

class IOGfxDisplay {
public:
	int w, h;
	Uint32 flags;
	bool initializedVideo;
	SDL_Window* window;

public:
	IOGfxDisplay(int w, int h, Uint32 flags);
	virtual ~IOGfxDisplay();

	virtual bool open();
	virtual void close();

	virtual void clear() = 0;
	virtual void flip(IOGfxSurface* backbuffer) = 0;
	virtual void onSizeChange(int w, int h) = 0;

	bool createWindow();
	void logWindowInfo();
	void toggleFullScreen();
	void toggleScreenKeyboard();
};

extern void flip_it(void);

#endif
