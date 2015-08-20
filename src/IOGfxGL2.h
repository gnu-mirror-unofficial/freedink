/**
 * Graphics primitives with OpenGL (ES) 2

 * Copyright (C) 2015  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software: you can redistribute it and/or
 * dmodify it under the terms of the GNU Affero General Public License
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

#include "IOGfx.h"

class IOGraphicsGLFuncs;

class IOGfxGL2 : public IOGfx {
public:
	int w, h;
	Uint32 flags;
	
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* screen;
	SDL_GLContext glcontext;
	IOGraphicsGLFuncs* gl;

	IOGfxGL2(int w, int h, Uint32 flags);
	~IOGfxGL2();
	bool open();
	void close();

	bool createWindow();
	void logWindowInfo();

	bool createOpenGLContext();
	void logOpenGLInfo();

	void clearWindow();
	SDL_Surface* screenshot();
	void screenshot(const char* out_filename);
};

#endif
