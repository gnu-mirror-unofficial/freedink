#ifndef IOGFXDISPLAYSW_H
#define IOGFXDISPLAYSW_H

#include "IOGfxDisplay.h"

class IOGfxDisplaySW: public IOGfxDisplay {
public:
	SDL_Renderer* renderer;
	/* Streaming texture to push software buffer -> hardware */
	SDL_Texture* render_texture = NULL;
	/* Intermediary texture to convert 8bit->32bit in non-truecolor */
	SDL_Surface *rgba_screen = NULL;

	IOGfxDisplaySW(int w, int h, Uint32 flags);
	virtual ~IOGfxDisplaySW();

	virtual bool open();
	virtual void close();
	virtual void clearWindow();
	virtual void flip(IOGfxSurface* backbuffer);

	bool createRenderer();
	void logRenderersInfo();

	bool createRenderTexture();
	void logRenderTextureInfo();

	void center_game_display(SDL_Renderer *rend, SDL_Rect* rect);
};

#endif
