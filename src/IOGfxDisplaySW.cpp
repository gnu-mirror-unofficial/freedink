#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "IOGfxDisplaySW.h"
#include "IOGfxSurfaceSW.h"
#include "gfx_palette.h"

#include "log.h"

IOGfxDisplaySW::IOGfxDisplaySW(int w, int h, Uint32 flags)
	: IOGfxDisplay(w, h, flags), renderer(NULL) {
}

IOGfxDisplaySW::~IOGfxDisplaySW() {
}

bool IOGfxDisplaySW::open() {
	if (!createWindow()) return false;
	logWindowInfo();

	if (!createRenderer()) return false;
	logRenderersInfo();

	if (!createRenderTexture()) return false;
	logRenderTextureInfo();

	return true;
}

void IOGfxDisplaySW::close() {
	if (render_texture) SDL_DestroyTexture(render_texture);
	render_texture = NULL;

	if (renderer) SDL_DestroyRenderer(renderer);
	renderer = NULL;

	if (window) SDL_DestroyWindow(window);
	window = NULL;
}

void IOGfxDisplaySW::clearWindow() {
    SDL_RenderClear(renderer);
}

bool IOGfxDisplaySW::createRenderer() {
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
	/* TODO SDL2: make it configurable for speed: nearest/linear/DX-specific-best */
	//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
	/* TODO SDL2: make render driver configurable to ease software-mode testing */
	renderer = SDL_CreateRenderer(window, -1/*autoselect*/, 0);
	/* TODO SDL2: optionally pass SDL_RENDERER_PRESENTVSYNC to 3rd param */
	if (renderer == NULL) {
		log_error("Unable to create renderer: %s\n", SDL_GetError());
		return false;
	}
	// Specify aspect ratio
	SDL_RenderSetLogicalSize(renderer, w, h);
	return true;
}

static void logRendererInfo(SDL_RendererInfo* info) {
	log_info("  Renderer driver: %s", info->name);
	log_info("  Renderer flags:");
	if (info->flags & SDL_RENDERER_SOFTWARE)
		log_info("    SDL_RENDERER_SOFTWARE");
	if (info->flags & SDL_RENDERER_ACCELERATED)
		log_info("    SDL_RENDERER_ACCELERATED");
	if (info->flags & SDL_RENDERER_PRESENTVSYNC)
		log_info("    SDL_RENDERER_PRESENTVSYNC");
	if (info->flags & SDL_RENDERER_TARGETTEXTURE)
		log_info("    SDL_RENDERER_TARGETTEXTURE");
	log_info("  Renderer texture formats:");
	for (unsigned int i = 0; i < info->num_texture_formats; i++)
		log_info("    %s", SDL_GetPixelFormatName(info->texture_formats[i]));
	log_info("  Renderer max texture width: %d", info->max_texture_width);
	log_info("  Renderer max texture height: %d", info->max_texture_height);
}

void IOGfxDisplaySW::logRenderersInfo() {
	log_info("Available renderers:");
	for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		log_info("%d:\n", i);
		logRendererInfo(&info);
	}

	log_info("current:\n");
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	logRendererInfo(&info);
}

/**
 * Screen-like destination texture
 */
bool IOGfxDisplaySW::createRenderTexture() {
  render_texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, w, h);
  if (render_texture == NULL) {
    log_error("Unable to create render texture: %s", SDL_GetError());
    return false;
  }

  Uint32 render_texture_format;
  Uint32 Rmask, Gmask, Bmask, Amask; int bpp;
  SDL_QueryTexture(render_texture, &render_texture_format, NULL, NULL, NULL);
  SDL_PixelFormatEnumToMasks(render_texture_format, &bpp,
    &Rmask, &Gmask, &Bmask, &Amask);
  rgba_screen = SDL_CreateRGBSurface(0, w, h, bpp, Rmask, Gmask, Bmask, Amask);

  return true;
}

void IOGfxDisplaySW::logRenderTextureInfo() {
  Uint32 format;
  int access, w, h;
  SDL_QueryTexture(render_texture, &format, &access, &w, &h);
  log_info("Render texture: format: %s", SDL_GetPixelFormatName(format));
  char* str_access;
  switch(access) {
  case SDL_TEXTUREACCESS_STATIC:
    str_access = "SDL_TEXTUREACCESS_STATIC"; break;
  case SDL_TEXTUREACCESS_STREAMING:
    str_access = "SDL_TEXTUREACCESS_STREAMING"; break;
  case SDL_TEXTUREACCESS_TARGET:
    str_access = "SDL_TEXTUREACCESS_TARGET"; break;
  default:
    str_access = "Unknown!"; break;
  }
  log_info("Render texture: access: %s", str_access);
  log_info("Render texture: width : %d", w);
  log_info("Render texture: height: %d", h);
}

void IOGfxDisplaySW::center_game_display(SDL_Renderer *rend, SDL_Rect* rect) {
	int rend_w, rend_h;
	SDL_RenderGetLogicalSize(rend, &rend_w, &rend_h);
	double game_ratio = 1.0 * w / h;
	double rend_ratio = 1.0 * rend_w / rend_h;
	if (game_ratio < rend_ratio) {
		// left/right bars
		rect->w = w * rend_h / h;
		rect->h = rend_h;
		rect->x = (rend_w - rect->w) / 2;
		rect->y = 0;
	} else {
		// top/bottom bars
		rect->w = rend_w;
		rect->h = h * rend_w / w;
		rect->x = 0;
		rect->y = (rend_h - rect->h) / 2;
	}
}

void IOGfxDisplaySW::flip(IOGfxSurface* backbuffer) {
	/* Convert to destination buffer format */
	SDL_Surface* source = dynamic_cast<IOGfxSurfaceSW*>(backbuffer)->s;

	if (source->format->format == SDL_PIXELFORMAT_INDEX8) {
		/* Convert 8-bit buffer for truecolor texture upload */

		/* Use "physical" screen palette */
		SDL_Color pal_bak[256];
		SDL_Color pal_phys[256];
		memcpy(pal_bak, source->format->palette->colors, sizeof(pal_bak));
		gfx_palette_get_phys(pal_phys);
		SDL_SetPaletteColors(source->format->palette, pal_phys, 0, 256);

		if (SDL_BlitSurface(source, NULL, rgba_screen, NULL) < 0) {
			log_error("ERROR: 8-bit->truecolor conversion failed: %s", SDL_GetError());
		}
		SDL_SetPaletteColors(source->format->palette, pal_bak, 0, 256);

		source = rgba_screen;
	}

	SDL_UpdateTexture(render_texture, NULL, source->pixels, source->pitch);
	SDL_Rect dst;
	center_game_display(renderer, &dst);
	SDL_RenderCopy(renderer, render_texture, NULL, &dst);
	SDL_RenderPresent(renderer);
}
