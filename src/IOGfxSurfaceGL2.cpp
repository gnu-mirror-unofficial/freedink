#include "IOGfxSurfaceGL2.h"

#include "SDL.h"
#include "log.h"
#include "IOGfxGLFuncs.h"

IOGfxSurfaceGL2::IOGfxSurfaceGL2(SDL_Surface* surf, IOGfxGLFuncs* gl)
	: gl(gl) {
	gl->GenTextures(1, &texture);
	gl->BindTexture(GL_TEXTURE_2D, texture);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	log_info("surf->w=%d", surf->w);
	log_info("surf->h=%d", surf->h);
	log_info("surf->format->bits=%d", surf->format->BitsPerPixel);
	if (surf->format->BitsPerPixel == 24) {
		gl->TexImage2D(GL_TEXTURE_2D, // target
				0,	     // level, 0 = base, no minimap,
				GL_RGB,  // internalformat
				surf->w, // width
				surf->h, // height
				0,	     // border, always 0 in OpenGL ES
				GL_RGB,  // format
				GL_UNSIGNED_BYTE, // type
				surf->pixels);
	} else if (surf->format->BitsPerPixel == 32) {
		gl->TexImage2D(GL_TEXTURE_2D, // target
				0,	     // level, 0 = base, no minimap,
				GL_RGBA, // internalformat
				surf->w, // width
				surf->h, // height
				0,	     // border, always 0 in OpenGL ES
				GL_RGBA, // format
				GL_UNSIGNED_BYTE, // type
				surf->pixels);
	} else {
		log_error("Unsupported image format: %d-bit", surf->format->BitsPerPixel);
	}
	SDL_FreeSurface(surf);
}

IOGfxSurfaceGL2::~IOGfxSurfaceGL2() {
	gl->DeleteTextures(1, &texture);
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

int IOGfxSurfaceGL2::blit(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blit: passed a NULL surface");
	//GLuint src_tex = dynamic_cast<IOGfxSurfaceGL2*>(src)->texture;
	return -1;
}

int IOGfxSurfaceGL2::blitStretch(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blitStretch: passed a NULL surface");
	//GLuint src_tex = dynamic_cast<IOGfxSurfaceGL2*>(src)->texture;
	return -1;
}


int IOGfxSurfaceGL2::blitNoColorKey(IOGfxSurface* src, const SDL_Rect* srcrect, SDL_Rect* dstrect) {
	if (src == NULL)
		return SDL_SetError("IOGfxSurfaceGL2::blitNoColorKeyt: passed a NULL surface");
	//GLuint src_tex = dynamic_cast<IOGfxSurfaceGL2*>(src)->texture;
	return -1;
}

unsigned int IOGfxSurfaceGL2::getMemUsage() {
	return 0;
}
