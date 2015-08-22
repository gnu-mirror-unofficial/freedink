/**
 * Graphics

 * Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <math.h>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL2_framerate.h"

#include "freedink_xpm.h"
#include "DMod.h"
#include "io_util.h"
#include "gfx.h"
#include "IOGfxPrimitives.h"
#include "IOGfxDisplay.h"
#include "gfx_fade.h"
#include "gfx_fonts.h"
#include "gfx_palette.h"
#include "gfx_sprites.h"
#include "paths.h"
#include "log.h"


/* Is the screen depth more than 8bit? */
int truecolor = 0;

SDL_Surface *GFX_backbuffer = NULL; /* Backbuffer */

/* GFX_lpDDSTwo: holds the base scene */
/* Rationale attempt :*/
/* lpDDSTwo contains the background, which is reused for each new
   frame. It is overwritten when switching to another screen. However,
   it can change during a screen: 1) animated tiles (water & fire) 2)
   when a sprite is written on the background (eg when an enemy dies)
   3) with various hacks such as fill_screen() (and maybe
   copy_bmp_to_screen()). */
/* Those changes may conflict with each other (eg: an animated tile
   overwrites half the carcass of a dead enemy). We might want to fix
   that. */
/* After the background is done, all the other operations are applied
   on lpDDSBack, the double buffer which is directly used by the
   physical screen. */
SDL_Surface *GFX_background = NULL;

/* Beuc: apparently used for the scrolling screen transition and more
   generaly as temporary buffers. Only used by the game, not the
   editor. */
/* Used in freedink.cpp only + as a local/independent temporary buffer
   in show_bmp&copy_bmp&process_show_bmp&load_sprite* */
SDL_Surface *GFX_tmp1 = NULL;
/* Used in freedink.cpp and update_frame.cpp */
SDL_Surface *GFX_tmp2 = NULL;

/* Main window and associated renderer */
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
/* Streaming texture to push software buffer -> hardware */
SDL_Texture* render_texture = NULL;
/* Intermediary texture to convert 8bit->32bit in non-truecolor */
SDL_Surface *rgba_screen = NULL;

/* Reference palette: this is the canonical Dink palette, loaded from
   TS01.bmp (for freedink) and esplash.bmp (for freedinkedit). The
   physical screen may be changed (e.g. show_bmp()), but this
   canonical palette will stay constant. */
/* PALETTEENTRY  real_pal[256]; */
SDL_Color GFX_ref_pal[256];

/* Skip flipping the double buffer for this frame only - used when
   setting up show_bmp and copy_bmp */
/*bool*/int abort_this_flip = /*false*/0;


/* True color fade in [0,256]; 0 is completely dark, 256 is unaltered */
double truecolor_fade_brightness = 256;
/* Time elapsed since last fade computation; 0 is disabled */
Uint32 truecolor_fade_lasttick = 0;

FPSmanager framerate_manager;

void logRendererInfo(SDL_RendererInfo* info) {
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

void logRenderersInfo() {
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
 * Graphics subsystem initalization
 */
int gfx_init(bool windowed, char* splash_path)
{
  /* Init graphics subsystem */
  if (SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
    {
      log_error("Video initialization error: %s", SDL_GetError());
      return -1;
    }

  log_info("Truecolor mode: %s", truecolor ? "on" : "off");

  window = SDL_CreateWindow(PACKAGE_STRING,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    640, 480,
    windowed ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN_DESKTOP);
  /* Note: SDL_WINDOW_FULLSCREEN[!_DESKTOP] may not respect aspect ratio */
  if (window == NULL)
    {
      log_error("Unable to create 640x480 window: %s\n", SDL_GetError());
      return -1;
    }

  log_info("Video driver: %s", SDL_GetCurrentVideoDriver());
  if (0) {
    // Segfaults on quit:
    //SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "opengl");
    SDL_Surface* window_surface = SDL_GetWindowSurface(window);
    log_info("Video fall-back surface (unused): %s",
	     SDL_GetPixelFormatName(window_surface->format->format));
  }
  log_info("Video fall-back surface (unused): %s",
	   SDL_GetPixelFormatName(SDL_GetWindowPixelFormat(window)));

  /* Renderer */
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  /* TODO SDL2: make it configurable for speed: nearest/linear/DX-specific-best */
  //SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
  /* TODO SDL2: make render driver configurable to ease software-mode testing */
  renderer = SDL_CreateRenderer(window, -1/*autoselect*/, 0);
  /* TODO SDL2: optionally pass SDL_RENDERER_PRESENTVSYNC to 3rd param */
  if (renderer == NULL)
    {
      log_error("Unable to create renderer: %s\n", SDL_GetError());
      return -1;
    }
  // Specify aspect ratio
  SDL_RenderSetLogicalSize(renderer, 640, 480);

  logRenderersInfo();

  /* Window configuration */
  {
    SDL_Surface *icon = NULL;
    if ((icon = IMG_ReadXPMFromArray(freedink_xpm)) == NULL)
      {
	log_error("Error loading icon: %s", IMG_GetError());
      }
    else
      {
	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);
      }
  }

  /* Create destination surface */
  {
    Uint32 Rmask=0, Gmask=0, Bmask=0, Amask=0; int bpp=0;
    if (!truecolor) {
      SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_INDEX8, &bpp,
				 &Rmask, &Gmask, &Bmask, &Amask);
    } else {
      SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGB888, &bpp,
				 &Rmask, &Gmask, &Bmask, &Amask);
    }
    GFX_backbuffer = SDL_CreateRGBSurface(0, 640, 480, bpp,
      Rmask, Gmask, Bmask, Amask);
    log_info("Main buffer: %s %d-bit R=0x%08x G=0x%08x B=0x%08x A=0x%08x",
	     SDL_GetPixelFormatName(GFX_backbuffer->format->format),
	     bpp, Rmask, Gmask, Bmask, Amask);
  }

  /* Surface is then transfered to destination texture */
  {
    render_texture = SDL_CreateTexture(renderer,
      SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 640, 480);
    if (render_texture == NULL) {
      log_error("Unable to create render texture: %s", SDL_GetError());
      return -1;
    }

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


  /* Default palette */
  gfx_palette_reset();

  /* Create and set the physical palette */
  if (gfx_palette_set_from_bmp("Tiles/Ts01.bmp") < 0)
    log_error("Failed to load default palette from Tiles/Ts01.bmp");

  /* Set the reference palette */
  gfx_palette_get_phys(GFX_ref_pal);

  /* Initialize graphic buffers */
  /* When a new image is loaded in DX, it's color-converted using the
     main palette (possibly altering the colors to match the palette);
     currently we emulate that by wrapping SDL_LoadBMP, converting
     image to the internal palette at load time - and we never change
     the buffer's palette again, so we're sure there isn't any
     conversion even if we change the screen palette: */
  if (!truecolor) {
    SDL_SetPaletteColors(GFX_backbuffer->format->palette, GFX_ref_pal, 0, 256);

    Uint32 render_texture_format;
    SDL_QueryTexture(render_texture, &render_texture_format, NULL, NULL, NULL);
    Uint32 Rmask, Gmask, Bmask, Amask; int bpp;
    SDL_PixelFormatEnumToMasks(render_texture_format, &bpp,
      &Rmask, &Gmask, &Bmask, &Amask);
    rgba_screen = SDL_CreateRGBSurface(0, 640, 480, bpp,
      Rmask, Gmask, Bmask, Amask);
  }
  GFX_background    = SDL_ConvertSurface(GFX_backbuffer, GFX_backbuffer->format, 0);
  GFX_tmp1  = SDL_ConvertSurface(GFX_backbuffer, GFX_backbuffer->format, 0);
  GFX_tmp2 = SDL_ConvertSurface(GFX_backbuffer, GFX_backbuffer->format, 0);

  /* Display splash picture, as early as possible */
  {
    FILE* splash_file = paths_dmodfile_fopen(splash_path, "r");
    if (splash_file == NULL) {
      splash_file = paths_fallbackfile_fopen(splash_path, "r");
    }
    SDL_Surface* splash = load_bmp_from_fp(splash_file);
    if (splash == NULL) {
	  log_error("Cannot load base graphics %s", splash_path);
    } else {
	/* Copy splash to the background buffer so that D-Mod can
	   start an effect from it (e.g. Pilgrim Quest's burning
	   splash screen effect) */
	if (SDL_BlitSurface(splash, NULL, GFX_background, NULL) < 0)
	  log_error("Error blitting splash to temp buffer");
	SDL_FreeSurface(splash);
      }
    
    /* Copy splash screen (again) to the screen during loading time */
    if (SDL_BlitSurface(GFX_background, NULL, GFX_backbuffer, NULL) < 0)
      log_error("Error blitting splash to back buffer");

    SDL_RenderClear(renderer);
    flip_it();
	SDL_RenderPresent(renderer);
  }


  /* Fonts system, default fonts */
  if (gfx_fonts_init() < 0)
    return -1; /* error message set in gfx_fonts_init */

  /* Compute fade cache if necessary */
  gfx_fade_init();

  /* make all pointers to NULL */
  memset(&k, 0, sizeof(k));
  memset(&GFX_k, 0, sizeof(GFX_k));
  memset(&seq, 0, sizeof(seq));


  SDL_initFramerate(&framerate_manager);
  /* The official v1.08 .exe runs 50-60 FPS in practice, despite the
     documented intent of running 83 FPS (or 12ms delay). */
  /* SDL_setFramerate(manager, 83); */
  SDL_setFramerate(&framerate_manager, FPS);
  
  return 0;
}

/**
 * Unload graphics subsystem
 */
void gfx_quit()
{
  gfx_fade_quit();

  gfx_fonts_quit();

  sprites_unload();
  
  if (GFX_backbuffer   != NULL) SDL_FreeSurface(GFX_backbuffer);
  if (GFX_background    != NULL) SDL_FreeSurface(GFX_background);
  if (GFX_tmp1  != NULL) SDL_FreeSurface(GFX_tmp1);
  if (GFX_tmp2 != NULL) SDL_FreeSurface(GFX_tmp2);

  GFX_backbuffer = NULL;
  GFX_background = NULL;
  GFX_tmp1 = NULL;
  GFX_tmp2 = NULL;

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

/**
 * Print GFX memory usage
 */
void gfx_log_meminfo()
{
  int total = 0;

  {
    int sum = 0;
    sum = GFX_backbuffer->h * GFX_backbuffer->pitch;
    log_debug("GFX screen = %8d", sum);
    total += sum;
  }
  
  {
    int sum = 0;
    SDL_Surface* s = NULL;
    s = GFX_background;
    sum += s->h * s->pitch;
    s = GFX_tmp1;
    sum += s->h * s->pitch;
    s = GFX_tmp2;
    sum += s->h * s->pitch;
    log_debug("GFX buf    = %8d", sum);
    total += sum;
  }
  
  {
    int sum = 0;
    int i = 0;
    SDL_Surface* s = NULL;
    for (; i < MAX_SPRITES; i++)
      {
	s = GFX_k[i].k;
	if (s != NULL)
	  sum += s->h * s->pitch;
	// Note: this does not take SDL_RLEACCEL into account
      }
    log_debug("GFX bmp    = %8d", sum);
    total += sum;
  }

  {
	  int sum = g_dmod.bgTilesets.getMemUsage();
	  log_debug("GFX tiles  = %8d", sum);
	  total += sum;
  }

  log_debug("GFX total  = %8d (+ ~150kB fonts)", total);
}

void gfx_toggle_fullscreen()
{
  SDL_SetWindowFullscreen(window,
    (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)
    ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
  // Note: software renderer is buggy in SDL 2.0.2: it doesn't resize the surface
}
