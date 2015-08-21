#ifndef TILESETSMANAGER_H
#define TILESETSMANAGER_H

#include "SDL.h"
#include "gfx_tiles.h"

class BgTilesetsManager {
public:
	SDL_Surface* slots[GFX_TILES_NB_SETS+1]; /* 1-indexed array */

	BgTilesetsManager();
	~BgTilesetsManager();
	void loadDefault();
	void loadSlot(int slot, char* relpath);
	void unloadAll();
	int getMemUsage();
};

#endif
