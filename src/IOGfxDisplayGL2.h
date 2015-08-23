/**
 * Graphics primitives with OpenGL (ES) 2

 * Copyright (C) 2015  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public
 * License along with GNU FreeDink.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef IOGRAPHICS_H
#define IOGRAPHICS_H

#include "SDL.h"

#include "IOGfxDisplay.h"

class IOGraphicsGLFuncs;

class IOGfxDisplayGL2 : public IOGfxDisplay {
public:
	SDL_GLContext glcontext;
	IOGraphicsGLFuncs* gl;
	SDL_Texture* screen;

	IOGfxDisplayGL2(int w, int h, bool truecolor, Uint32 flags);
	~IOGfxDisplayGL2();

	virtual bool open();
	virtual void close();

	virtual void clear();
	virtual void flip(IOGfxSurface* backbuffer);
	virtual void onSizeChange(int w, int h);
	virtual IOGfxSurface* upload(SDL_Surface* image);

	bool createOpenGLContext();
	void logOpenGLInfo();

	SDL_Surface* screenshot();
	void screenshot(const char* out_filename);
};

#endif
