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
#include "IOGfxGLFuncs.h"

class IOGfxGLFuncs;

class IOGfxDisplayGL2 : public IOGfxDisplay {
public:
	SDL_GLContext glcontext;
	IOGfxGLFuncs* gl;
	SDL_Texture* screen;

	GLuint vboSpriteVertices, vboSpriteTexcoords;
	GLuint program, program_fillRect;
	GLint attribute_v_coord, attribute_v_texcoord;
	GLint uniform_mvp, uniform_texture, uniform_colorkey;
	GLint attribute_fillRect_v_coord;
	GLint uniform_fillRect_mvp, uniform_fillRect_color;


	IOGfxDisplayGL2(int w, int h, bool truecolor, Uint32 flags);
	~IOGfxDisplayGL2();

	virtual bool open();
	virtual void close();
	virtual void logDisplayInfo();

	virtual void clear();
	virtual void flipStretch(IOGfxSurface* backbuffer);
	virtual void onSizeChange(int w, int h);
	virtual IOGfxSurface* upload(SDL_Surface* image);
	virtual IOGfxSurface* alloc(int surfW, int surfH);
	virtual void flipDebug(IOGfxSurface* backbuffer);

	virtual SDL_Surface* screenshot(SDL_Rect* rect);

	bool createOpenGLContext();
	void logOpenGLInfo();

	bool createSpriteVertices();
	bool createSpriteTexcoords();
	GLuint createShader(const char* source, GLenum type);
	void infoLog(GLuint object);
	bool createPrograms();
	GLuint createProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
	GLint getAttribLocation(GLuint program, const char* name);
	GLint getUniformLocation(GLuint program, const char* name);
	void androidWorkAround();

};

#endif
