#ifndef IOGFXDISPLAY_H
#define IOGFXDISPLAY_H

#include "SDL.h"

#include "IOGfxSurface.h"

class IOGfxDisplay {
public:
	int w, h;
	bool truecolor;
	Uint32 flags;
	bool initializedVideo;
	SDL_Window* window;
	/* True color fade in [0,256]; 0 is completely dark, 256 is unaltered */
	double brightness; // TODO: move to unsigned short

public:
	IOGfxDisplay(int w, int h, bool truecolor, Uint32 flags);
	virtual ~IOGfxDisplay();

	virtual bool open();
	virtual void close();
	virtual Uint32 getFormat();

	virtual void clear() = 0;
	/* Refresh the physical screen, applying brightness and palette */
	virtual void flip(IOGfxSurface* backbuffer) = 0;
	virtual void onSizeChange(int w, int h) = 0;
	virtual IOGfxSurface* upload(SDL_Surface* s) = 0;

	bool createWindow();
	void logWindowInfo();
	void toggleFullScreen();
	void toggleScreenKeyboard();
};

extern void flip_it(void);

#endif
