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

#ifndef IOGRAPHICSGLFUNCS_H
#define IOGRAPHICSGLFUNCS_H

#ifndef APIENTRY
#  if defined(_WIN32)
#    define APIENTRY __stdcall
#  else
#    define APIENTRY
#  endif
#endif

typedef void GLvoid;
typedef int GLint;
typedef int GLsizei;
typedef float GLclampf;
typedef unsigned int GLenum;
typedef unsigned int GLbitfield;

#define GL_COLOR_BUFFER_BIT 0x00004000

#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909

#define GL_UNSIGNED_BYTE 0x1401

class IOGraphicsGLFuncs {
public:
	IOGraphicsGLFuncs();

	void (APIENTRY *Clear)(GLbitfield);
	void (APIENTRY *ClearColor)(GLclampf, GLclampf, GLclampf, GLclampf);
	void (APIENTRY *ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
};

#endif
