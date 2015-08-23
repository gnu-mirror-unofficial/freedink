#ifndef IOGFXDISPLAYSW_H
#define IOGFXDISPLAYSW_H

#include "IOGfxDisplay.h"

class IOGfxDisplaySW: public IOGfxDisplay {
private:
	SDL_Renderer* renderer;
	/* Streaming texture to push software buffer -> hardware */
	SDL_Texture* render_texture;
	/* Intermediary texture to convert 8bit->32bit in non-truecolor */
	SDL_Surface *rgba_screen;

public:
	IOGfxDisplaySW(int w, int h, Uint32 flags);
	virtual ~IOGfxDisplaySW();

	virtual bool open();
	virtual void close();

	virtual void clear();
	virtual void flip(IOGfxSurface* backbuffer);
	virtual void onSizeChange(int w, int h);

	bool createRenderer();
	void logRenderersInfo();

	bool createRenderTexture();
	void logRenderTextureInfo();

	void center_game_display(SDL_Renderer *rend, SDL_Rect* rect);
};

#endif
