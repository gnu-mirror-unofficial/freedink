#include "ImageLoader.h"

#include "SDL.h"
#include "SDL_image.h"

/* Reference palette: this is the canonical Dink palette, loaded from
   TS01.bmp (for freedink) and esplash.bmp (for freedinkedit). The
   physical screen may be changed (e.g. show_bmp()), but this
   canonical palette will stay constant. */
SDL_Color GFX_ref_pal[256];
/* Reference blit surface that all others should mimic to avoid palette dithering;
 * GFX_ref_pal may change, but not this surface's palette */
// TODO: make IOGfxDisplaySW->blit *not* do any palette conversion, so we can get rid of this global
SDL_Surface* ImageLoader::blitFormat = NULL;

/**
 * Load image to the current display format (indexed or truecolor)
 * This isn't centralized in display->upload() because dir.ff are handled differently,
 * and this is done independently of the target display renderer.
 */
SDL_Surface* ImageLoader::loadToBlitFormat(FILE* in) {
	if (in == NULL)
		return NULL;

	SDL_RWops* rw = SDL_RWFromFP(in, /*autoclose=*/SDL_TRUE);
	SDL_Surface* image = IMG_Load_RW(rw, 1);
	if (blitFormat->format->format == SDL_PIXELFORMAT_INDEX8) {
		/* Make a copy of the surface using the screen format: same
    	   palette (with dithering), and same color depth (needed when
		   importing 24bit graphics in 8bit mode). */
		SDL_PixelFormat* fmt = SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
		if (!fmt->palette)  // fmt is cached ref from SDL2
			fmt->palette = SDL_AllocPalette(256);
		memcpy(fmt->palette->colors, GFX_ref_pal, sizeof(GFX_ref_pal));
		SDL_Surface *converted = SDL_ConvertSurface(image, fmt, 0);
		SDL_FreeFormat(fmt);
		SDL_FreeSurface(image);
		image = converted;
		/* Disable palette conversion in future blits */
		SDL_SetPaletteColors(image->format->palette, blitFormat->format->palette->colors, 0, 256);
	}

	/* Disable alpha in 32bit BMPs, like the original engine */
	SDL_SetSurfaceBlendMode(image, SDL_BLENDMODE_NONE);

	return image;
}
