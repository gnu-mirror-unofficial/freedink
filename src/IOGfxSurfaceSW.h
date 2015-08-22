#ifndef IOGFXSURFACESW_H
#define IOGFXSURFACESW_H

#include "IOGfxSurface.h"
#include "SDL.h"

class IOGfxSurfaceSW: public IOGfxSurface {
public:
	SDL_Surface* s;
	virtual ~IOGfxSurfaceSW();
};

#endif
