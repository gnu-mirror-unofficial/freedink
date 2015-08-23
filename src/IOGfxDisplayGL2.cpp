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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "IOGfxDisplayGL2.h"
#include "IOGfxGLFuncs.h"

#include "log.h"

IOGfxDisplayGL2::IOGfxDisplayGL2(int w, int h, bool truecolor, Uint32 flags)
	: IOGfxDisplay(w, h, truecolor, flags | SDL_WINDOW_OPENGL),
	  glcontext(NULL), gl(NULL), screen(NULL) {
}

IOGfxDisplayGL2::~IOGfxDisplayGL2() {
	close();
}

bool IOGfxDisplayGL2::open() {
	if (!IOGfxDisplay::open()) return false;

	if (!createOpenGLContext()) return false;
	logOpenGLInfo();

	return true;
}

void IOGfxDisplayGL2::close() {
	if (gl) delete gl;
	gl = NULL;
	if (glcontext) SDL_GL_DeleteContext(glcontext);
	glcontext = NULL;

	IOGfxDisplay::close();
}

bool IOGfxDisplayGL2::createOpenGLContext() {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	glcontext = SDL_GL_CreateContext(window);
	if (glcontext == NULL) {
		log_error("Could not create OpenGL context: %s", SDL_GetError());
		return false;
	}
	gl = new IOGraphicsGLFuncs();
	return true;
}

void IOGfxDisplayGL2::logOpenGLInfo() {
	int major, minor, profile;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
	const char* profile_str = "";
	if (profile & SDL_GL_CONTEXT_PROFILE_CORE)
		profile_str = "CORE";
	if (profile & SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
		profile_str = "COMPATIBILITY";
	if (profile & SDL_GL_CONTEXT_PROFILE_ES)
		profile_str = "ES";

	log_info("OpenGL %d.%d %s", major, minor, profile_str);
}

void IOGfxDisplayGL2::clear() {
	gl->ClearColor(0,0,1,1);
	gl->Clear(GL_COLOR_BUFFER_BIT);
}

SDL_Surface* IOGfxDisplayGL2::screenshot() {
	// assume 4-bytes alignment
	SDL_Surface* surface = SDL_CreateRGBSurface(0,
		w, h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
	);
	SDL_LockSurface(surface);
	gl->ReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	SDL_UnlockSurface(surface);
	return surface;
}

void IOGfxDisplayGL2::screenshot(const char* output_file) {
	SDL_Surface* surface = screenshot();
	SDL_SaveBMP(surface, output_file);
	SDL_FreeSurface(surface);
}

void IOGfxDisplayGL2::flip(IOGfxSurface* backbuffer) {
	// TODO
}

void IOGfxDisplayGL2::onSizeChange(int w, int h) {
	// TODO
}
