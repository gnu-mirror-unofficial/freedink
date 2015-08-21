/**
 * Access to GL functions without linking to system-specific library

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

#include "IOGfxGLFuncs.h"
#include "SDL.h"

IOGraphicsGLFuncs::IOGraphicsGLFuncs() {
	Clear = (void (APIENTRY *)(GLbitfield))SDL_GL_GetProcAddress("glClear");
	ClearColor = (void (APIENTRY *)(GLclampf, GLclampf, GLclampf, GLclampf))SDL_GL_GetProcAddress("glClearColor");
	ReadPixels = (void (APIENTRY *)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels))SDL_GL_GetProcAddress("glReadPixels");
}
