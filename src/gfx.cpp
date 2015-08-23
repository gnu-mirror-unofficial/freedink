/**
 * Graphics data to sort out

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
#include "IOGfxSurfaceSW.h"
#include "IOGfxDisplaySW.h"
#include "ImageLoader.h"
#include "gfx_fonts.h"
#include "gfx_palette.h"
#include "gfx_sprites.h"
#include "paths.h"
#include "log.h"


/* Is the screen depth more than 8bit? */
int truecolor = 0;

/* Backbuffer */
SDL_Surface *GFX_backbuffer = NULL;
IOGfxSurface* IOGFX_backbuffer = NULL;

/* Base/background scene */
/* Contains the background, which is reused for each new frame. It is
   overwritten when switching to another screen. However, it can
   change during a screen:
   1) animated tiles (water & fire)
   2) when a sprite is written on the background (eg when an enemy dies)
   3) with various hacks such as fill_screen() and copy_bmp_to_screen() */
/* Those changes may conflict with each other (eg: an animated tile
   overwrites half the carcass of a dead enemy). */
/* After the background is done, all the other operations are applied
   on GFX_backbuffer, the double buffer which is directly used by the
   physical screen. */
SDL_Surface *GFX_background = NULL;
IOGfxSurface* IOGFX_background = NULL;

/* Temporary buffer for scrolling screen transition +
   show_bmp/process_show_bmp */
SDL_Surface *GFX_tmp1 = NULL;
/* Temporary buffer for scrolling screen transition +
   load_sprite* */
SDL_Surface *GFX_tmp2 = NULL;

/* Skip flipping the double buffer for this frame only - used when
   setting up show_bmp and copy_bmp */
/*bool*/int abort_this_flip = /*false*/0;


/* Time elapsed since last fade computation; 0 is disabled */
Uint32 truecolor_fade_lasttick = 0;

FPSmanager framerate_manager;

/* Main window and associated renderer */
IOGfxDisplay* g_display = NULL;


/**
 * Graphics subsystem initalization
 */
int gfx_init(bool windowed, char* splash_path) {
	g_display = new IOGfxDisplaySW(GFX_RES_W, GFX_RES_H, truecolor,
			windowed ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN_DESKTOP);
	/* Note: SDL_WINDOW_FULLSCREEN[!_DESKTOP] may not respect aspect ratio */
	g_display->open();

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
  IOGFX_backbuffer = new IOGfxSurfaceSW(GFX_backbuffer);

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
  }
  GFX_background    = SDL_ConvertSurface(GFX_backbuffer, GFX_backbuffer->format, 0);
  IOGFX_background = new IOGfxSurfaceSW(GFX_background);
  GFX_tmp1  = SDL_ConvertSurface(GFX_backbuffer, GFX_backbuffer->format, 0);
  GFX_tmp2 = SDL_ConvertSurface(GFX_backbuffer, GFX_backbuffer->format, 0);

  /* Display splash picture, as early as possible */
  {
	SDL_Surface* splash = NULL;
    FILE* in = paths_dmodfile_fopen(splash_path, "r");
    if (in == NULL) {
      in = paths_fallbackfile_fopen(splash_path, "r");
    }
    if (in != NULL) {
      SDL_RWops* rw = SDL_RWFromFP(in, /*autoclose=*/SDL_TRUE);
      SDL_Surface* image = IMG_Load_RW(rw, 1);
      splash = g_display->upload(image);
    }
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

    g_display->clear();
    g_display->flip(IOGFX_backbuffer);
  }


  /* Fonts system, default fonts */
  if (gfx_fonts_init() < 0)
    return -1; /* error message set in gfx_fonts_init */

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
  gfx_fonts_quit();

  sprites_unload();
  
  if (IOGFX_backbuffer != NULL) delete IOGFX_backbuffer;
  if (IOGFX_background != NULL) delete IOGFX_background;
  if (GFX_tmp1  != NULL) SDL_FreeSurface(GFX_tmp1);
  if (GFX_tmp2 != NULL) SDL_FreeSurface(GFX_tmp2);

  GFX_backbuffer = NULL;
  GFX_background = NULL;
  GFX_tmp1 = NULL;
  GFX_tmp2 = NULL;

  g_display->close();
  delete g_display;
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
