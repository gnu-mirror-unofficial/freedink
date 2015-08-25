#include "IOGfxSurfaceGL2.h"

#include "SDL.h"
#include "log.h"
#include "IOGfxGLFuncs.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

IOGfxSurfaceGL2::IOGfxSurfaceGL2(IOGfxDisplayGL2* display, GLuint texture, int w, int h, SDL_Color colorkey)
	: display(display), texture(texture), w(w), h(h), colorkey(colorkey) {
}

IOGfxSurfaceGL2::~IOGfxSurfaceGL2() {
	display->gl->DeleteTextures(1, &texture);
}

/* Function specifically made for Dink'C fill_screen() */
void IOGfxSurfaceGL2::fill_screen(int num, SDL_Color* palette) {
}

void IOGfxSurfaceGL2::fillRect(const SDL_Rect *rect, Uint8 r, Uint8 g, Uint8 b) {
}

void IOGfxSurfaceGL2::vlineRGB(Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b) {
}
void IOGfxSurfaceGL2::hlineRGB(Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b) {
}


void IOGfxSurfaceGL2::drawBox(rect box, int color) {
}

int IOGfxSurfaceGL2::internalBlit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect, bool useColorKey) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blitStretch: passed a NULL surface");
	if (dstrect == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blitStretch: passed a NULL dstrect");
	IOGfxSurfaceGL2* src_surf = dynamic_cast<IOGfxSurfaceGL2*>(src);
	GLuint src_tex = src_surf->texture;

	IOGfxGLFuncs* gl = display->gl;
	GLuint fbo;
	// bind this as framebuffer
	gl->GenFramebuffers(1, &fbo);
	gl->BindFramebuffer(GL_FRAMEBUFFER, fbo);
	gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texture, 0);
	GLenum status;
	if ((status = gl->CheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		log_error("glCheckFramebufferStatus: error 0x%x", status);
		switch (status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			log_error("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			log_error("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			log_error("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			log_error("GL_FRAMEBUFFER_UNSUPPORTED");
			break;
		}
		return -1;
	}

	// bind src_tex as texture
	gl->UseProgram(display->program);

	gl->ActiveTexture(GL_TEXTURE0);
	gl->Uniform1i(display->uniform_texture, /*GL_TEXTURE*/0);
	gl->BindTexture(GL_TEXTURE_2D, src_tex);

	glm::mat4 projection = glm::ortho(0.0f, 1.0f*display->w, 0.0f, 1.0f*display->h);
	glm::mat4 m_transform;
	m_transform = glm::translate(glm::mat4(1.0f), glm::vec3(dstrect->x,dstrect->y, 0.0))
		* glm::scale(glm::mat4(1.0f), glm::vec3(dstrect->w, dstrect->h, 0.0));
	glm::mat4 mvp = projection * m_transform; // * view * model * anim;
	gl->UniformMatrix4fv(display->uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

	if (useColorKey && src_surf->colorkey.a == SDL_ALPHA_TRANSPARENT)
		gl->Uniform3f(display->uniform_colorkey,
				src_surf->colorkey.r/255.0, src_surf->colorkey.g/255.0, src_surf->colorkey.b/255.0);
	else
		gl->Uniform3f(display->uniform_colorkey, -1,-1,-1);

	// draw
	gl->EnableVertexAttribArray(display->attribute_v_coord);
	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	gl->BindBuffer(GL_ARRAY_BUFFER, display->vboSpriteVertices);
	gl->VertexAttribPointer(
		display->attribute_v_coord, // attribute
		4,                 // number of elements per vertex, here (x,y,z,t)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);

	GLuint vboCroppedSpriteTexcoords;
	gl->EnableVertexAttribArray(display->attribute_v_texcoord);
	if (srcrect == NULL) {
		gl->BindBuffer(GL_ARRAY_BUFFER, display->vboSpriteTexcoords);
	} else {
		float x1 = 1.0 * srcrect->x / src_surf->w;
		float y1 = 1.0 * srcrect->y / src_surf->h;
		float x2 = 1.0 * (srcrect->x+srcrect->w) / src_surf->w;
		float y2 = 1.0 * (srcrect->y+srcrect->h) / src_surf->h;
		GLfloat croppedSpriteTexcoords[] = {
			x1, y1,
			x2, y1,
			x1, y2,
			x2, y2,
		};
		gl->GenBuffers(1, &vboCroppedSpriteTexcoords);
		gl->BindBuffer(GL_ARRAY_BUFFER, vboCroppedSpriteTexcoords);
		gl->BufferData(GL_ARRAY_BUFFER, sizeof(croppedSpriteTexcoords), croppedSpriteTexcoords, GL_STATIC_DRAW);
	}
	gl->VertexAttribPointer(
		display->attribute_v_texcoord, // attribute
		2,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);

	/* Push each element in buffer_vertices to the vertex shader */
	gl->DrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (srcrect != NULL)
		gl->DeleteBuffers(1, &vboCroppedSpriteTexcoords);
	gl->DisableVertexAttribArray(display->attribute_v_coord);
	gl->DisableVertexAttribArray(display->attribute_v_texcoord);

	gl->DeleteFramebuffers(1, &fbo);
	gl->BindFramebuffer(GL_FRAMEBUFFER, 0);

	return 0;
}

int IOGfxSurfaceGL2::blit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blit: passed a NULL surface");
	IOGfxSurfaceGL2* src_surf = dynamic_cast<IOGfxSurfaceGL2*>(src);

	SDL_Rect dstrect_if_not_null;
	if (dstrect == NULL) {
		dstrect = &dstrect_if_not_null;
		dstrect->x = 0;
		dstrect->y = 0;
	}
	// Force no-stretch blit
	if (srcrect == NULL) {
		dstrect->w = src_surf->w;
		dstrect->h = src_surf->h;
	} else {
		dstrect->w = srcrect->w;
		dstrect->h = srcrect->h;
	}

	return internalBlit(src, srcrect, dstrect, true);
}


int IOGfxSurfaceGL2::blitStretch(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	return internalBlit(src, srcrect, dstrect, true);
}

int IOGfxSurfaceGL2::blitNoColorKey(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blitStretch: passed a NULL surface");
	IOGfxSurfaceGL2* src_surf = dynamic_cast<IOGfxSurfaceGL2*>(src);

	SDL_Rect dstrect_if_not_null;
	if (dstrect == NULL) {
		dstrect = &dstrect_if_not_null;
		dstrect->x = 0;
		dstrect->y = 0;
	}
	// Force no-stretch blit
	if (srcrect == NULL) {
		dstrect->w = src_surf->w;
		dstrect->h = src_surf->h;
	} else {
		dstrect->w = srcrect->w;
		dstrect->h = srcrect->h;
	}

	return internalBlit(src, srcrect, dstrect, false);
}

unsigned int IOGfxSurfaceGL2::getMemUsage() {
	return 0;
}
