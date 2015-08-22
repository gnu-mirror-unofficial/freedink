#ifndef IOGFXSURFACE_H
#define IOGFXSURFACE_H

#include "SDL.h"

class IOGfxSurface {
public:
	virtual ~IOGfxSurface();
	virtual void fill(int num, SDL_Color* palette) = 0;
};

#endif
