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

#include "log.h"

#ifdef __EMSCRIPTEN__
// static access mandatory since ~1.38.21
// might be fixed directly in the SDL2 port, cf.
// https://groups.google.com/d/msg/emscripten-discuss/d3damBim3Mw/mrvJdC49FQAJ
#include <GLES2/gl2.h>
#define GETPROCADDRESS(x) x
#else
#define GETPROCADDRESS(x) SDL_GL_GetProcAddress(#x)
#endif

IOGfxGLFuncs::IOGfxGLFuncs() {
	GetError = (GLenum (APIENTRY*)(void))GETPROCADDRESS(glGetError);
	Enable = (void (APIENTRY*)(GLenum cap))GETPROCADDRESS(glEnable);
	GetIntegerv = (void (APIENTRY*)(GLenum pname, GLint* params))GETPROCADDRESS(glGetIntegerv);
	BlendFunc = (void (APIENTRY*)(GLenum sfactor, GLenum dfactor))GETPROCADDRESS(glBlendFunc);

	Clear = (void (APIENTRY*)(GLbitfield))GETPROCADDRESS(glClear);
	ClearColor = (void (APIENTRY*)(GLclampf, GLclampf, GLclampf, GLclampf))GETPROCADDRESS(glClearColor);
	Viewport = (void (APIENTRY*)(GLint x, GLint y, GLsizei width, GLsizei height))GETPROCADDRESS(glViewport);

	ReadPixels = (void (APIENTRY*)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels))GETPROCADDRESS(glReadPixels);

	GenTextures = (void (APIENTRY*)(GLsizei n, GLuint *textures))GETPROCADDRESS(glGenTextures);
	BindTexture = (void (APIENTRY*)(GLenum target, GLuint texture))GETPROCADDRESS(glBindTexture);
	DeleteTextures = (void (APIENTRY*)(GLsizei n, const GLuint *textures))GETPROCADDRESS(glDeleteTextures);
	TexParameteri = (void (APIENTRY*)(GLenum target, GLenum pname, GLint param))GETPROCADDRESS(glTexParameteri);
	TexImage2D = (void (APIENTRY*)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels))GETPROCADDRESS(glTexImage2D);

	GenBuffers = (void (APIENTRY*)(GLsizei n, GLuint* buffers))GETPROCADDRESS(glGenBuffers);
	BindBuffer = (void (APIENTRY*)(GLenum target, GLuint buffer))GETPROCADDRESS(glBindBuffer);
	BufferData = (void (APIENTRY*)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage))GETPROCADDRESS(glBufferData);
	DeleteBuffers = (void (APIENTRY*)(GLsizei n, const GLuint* buffers))GETPROCADDRESS(glDeleteBuffers);

	IsShader = (GLboolean (APIENTRY*)(GLuint shader))GETPROCADDRESS(glIsShader);
	CreateShader = (GLuint (APIENTRY*)(GLenum type))GETPROCADDRESS(glCreateShader);
	ShaderSource = (void (APIENTRY*)(GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths))GETPROCADDRESS(glShaderSource);
	CompileShader = (void (APIENTRY*)(GLuint shader))GETPROCADDRESS(glCompileShader);
	GetShaderiv = (void (APIENTRY*)(GLuint shader, GLenum pname, GLint* param))GETPROCADDRESS(glGetShaderiv);
	GetShaderInfoLog = (void (APIENTRY*)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog))GETPROCADDRESS(glGetShaderInfoLog);
	AttachShader = (void (APIENTRY*)(GLuint program, GLuint shader))GETPROCADDRESS(glAttachShader);
	DeleteShader = (void (APIENTRY*)(GLuint shader))GETPROCADDRESS(glDeleteShader);

	IsProgram = (GLboolean (APIENTRY*)(GLuint program))GETPROCADDRESS(glIsProgram);
	CreateProgram = (GLuint (APIENTRY*)(void))GETPROCADDRESS(glCreateProgram);
	GetProgramiv = (void (APIENTRY*)(GLuint program, GLenum pname, GLint* param))GETPROCADDRESS(glGetProgramiv);
	LinkProgram = (void (APIENTRY*)(GLuint program))GETPROCADDRESS(glLinkProgram);
	GetProgramInfoLog = (void (APIENTRY*)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog))GETPROCADDRESS(glGetProgramInfoLog);
	DeleteProgram = (void (APIENTRY*)(GLuint program))GETPROCADDRESS(glDeleteProgram);

	GetAttribLocation = (GLint (APIENTRY*)(GLuint program, const GLchar* name))GETPROCADDRESS(glGetAttribLocation);
	GetUniformLocation = (GLint (APIENTRY*)(GLuint program, const GLchar* name))GETPROCADDRESS(glGetUniformLocation);

	UseProgram = (void (APIENTRY*)(GLuint program))GETPROCADDRESS(glUseProgram);
	ActiveTexture = (void (APIENTRY*)(GLenum texture))GETPROCADDRESS(glActiveTexture);
	Uniform1i = (void (APIENTRY*)(GLint location, GLint v0))GETPROCADDRESS(glUniform1i);
	Uniform1f = (void (APIENTRY*)(GLint location, GLfloat v0))GETPROCADDRESS(glUniform1f);
	Uniform3f = (void (APIENTRY*)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2))GETPROCADDRESS(glUniform3f);
	Uniform4f = (void (APIENTRY*)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))GETPROCADDRESS(glUniform4f);
	UniformMatrix3fv = (void (APIENTRY*)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))GETPROCADDRESS(glUniformMatrix3fv);
	UniformMatrix4fv = (void (APIENTRY*)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))GETPROCADDRESS(glUniformMatrix4fv);

	EnableVertexAttribArray = (void (APIENTRY*)(GLuint))GETPROCADDRESS(glEnableVertexAttribArray);
	VertexAttribPointer = (void (APIENTRY*)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer))GETPROCADDRESS(glVertexAttribPointer);
	DisableVertexAttribArray = (void (APIENTRY*)(GLuint))GETPROCADDRESS(glDisableVertexAttribArray);

	GenFramebuffers = (void (APIENTRY*)(GLsizei n, GLuint* framebuffers))GETPROCADDRESS(glGenFramebuffers);
	BindFramebuffer = (void (APIENTRY*)(GLenum target, GLuint framebuffer))GETPROCADDRESS(glBindFramebuffer);
	FramebufferTexture2D = (void (APIENTRY*)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))GETPROCADDRESS(glFramebufferTexture2D);
	CheckFramebufferStatus = (GLenum (APIENTRY*)(GLenum target))GETPROCADDRESS(glCheckFramebufferStatus);
	DeleteFramebuffers = (void (APIENTRY*)(GLsizei n, const GLuint* framebuffers))GETPROCADDRESS(glDeleteFramebuffers);

	DrawArrays = (void (APIENTRY*)(GLenum mode, GLint first, GLsizei count))GETPROCADDRESS(glDrawArrays);
}

void IOGfxGLFuncs::logGetError() {
	switch (this->GetError()) {
	case GL_NO_ERROR:
		log_info("GL_NO_ERROR");
		break;
	case GL_INVALID_ENUM:
		log_info("GL_INVALID_ENUM");
		break;
	case GL_INVALID_VALUE:
		log_info("GL_INVALID_VALUE");
		break;
	case GL_INVALID_OPERATION:
		log_info("GL_INVALID_OPERATION");
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		log_info("GL_INVALID_FRAMEBUFFER_OPERATION");
		break;
	case GL_OUT_OF_MEMORY:
		log_info("GL_OUT_OF_MEMORY");
		break;
	}
}
