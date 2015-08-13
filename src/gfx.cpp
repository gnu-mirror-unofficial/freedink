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
#include "SDL_syswm.h"
#include "SDL_image.h"
#include "SDL2_rotozoom.h"

#include "freedink_xpm.h"
#include "io_util.h"
#include "gfx.h"
#include "gfx_fade.h"
#include "gfx_fonts.h"
#include "gfx_palette.h"
#include "gfx_sprites.h"
#include "gfx_tiles.h"
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
SDL_Color GFX_real_pal[256];

/* Skip flipping the double buffer for this frame only - used when
   setting up show_bmp and copy_bmp */
/*bool*/int abort_this_flip = /*false*/0;


/* True color fade in [0,256]; 0 is completely dark, 256 is unaltered */
double truecolor_fade_brightness = 256;
/* Time elapsed since last fade computation; 0 is disabled */
Uint32 truecolor_fade_lasttick = 0;

FPSmanager framerate_manager;


/**
 * Check if the graphics system is initialized, so we know if we can
 * use it to display error messages to the user
 */
static enum gfx_init_state init_state = GFX_NOT_INITIALIZED;
enum gfx_init_state gfx_get_init_state()
{
  return init_state;
}

/**
 * Graphics subsystem initalization
 */
int gfx_init(enum gfx_windowed_state windowed, char* splash_path)
{
  /* Initialization in progress */
  init_state = GFX_INITIALIZING_VIDEO;

  /* Init graphics subsystem */
  if (SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
    {
      log_set_init_error_msg("Video initialization error: %s", SDL_GetError());
      return -1;
    }

  log_info("Truecolor mode: %s", truecolor ? "on" : "off");

  window = SDL_CreateWindow(PACKAGE_STRING,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    640, 480,
    windowed == GFX_WINDOWED ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN_DESKTOP);
  /* Note: SDL_WINDOW_FULLSCREEN[!_DESKTOP] may not respect aspect ratio */
  if (window == NULL)
    {
      log_set_init_error_msg("Unable to create 640x480 window: %s\n", SDL_GetError());
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
      log_set_init_error_msg("Unable to create renderer: %s\n", SDL_GetError());
      return -1;
    }
  // Specify aspect ratio
  SDL_RenderSetLogicalSize(renderer, 640, 480);

  SDL_RendererInfo info;
  SDL_GetRendererInfo(renderer, &info);
  log_info("Renderer driver: %s", info.name);
  /* Uint32 flags a mask of supported renderer flags; see Remarks for details */
  for (unsigned int i = 0; i < info.num_texture_formats; i++)
    log_info("Renderer texture formats: %s",
	     SDL_GetPixelFormatName(info.texture_formats[i]));
  log_info("Renderer max texture width: %d", info.max_texture_width);
  log_info("Renderer max texture height: %d", info.max_texture_height);

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
      log_set_init_error_msg("Unable to create render texture: %s", SDL_GetError());
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
  gfx_palette_get_phys(GFX_real_pal);

  /* Initialize graphic buffers */
  /* When a new image is loaded in DX, it's color-converted using the
     main palette (possibly altering the colors to match the palette);
     currently we emulate that by wrapping SDL_LoadBMP, converting
     image to the internal palette at load time - and we never change
     the buffer's palette again, so we're sure there isn't any
     conversion even if we change the screen palette: */
  if (!truecolor) {
    SDL_SetPaletteColors(GFX_backbuffer->format->palette, GFX_real_pal, 0, 256);

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
    char* fullpath = paths_dmodfile(splash_path);
    if (!exist(fullpath))
      {
	free(fullpath);
	fullpath = paths_fallbackfile(splash_path);
      }
    SDL_Surface* splash = load_bmp(fullpath);
    free(fullpath);
    if (splash == NULL)
      {
	log_error("Cannot load base graphics %s", splash_path);
      }
    else
      {
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

    flip_it();
  }


  /* Fonts system, default fonts */
  init_state = GFX_INITIALIZING_FONTS;
  if (gfx_fonts_init() < 0)
    return -1; /* error message set in gfx_fonts_init */

  /* Compute fade cache if necessary */
  gfx_fade_init();

  /* make all pointers to NULL */
  memset(&gfx_tiles, 0, sizeof(gfx_tiles));
  memset(&k, 0, sizeof(k));
  memset(&GFX_k, 0, sizeof(GFX_k));
  memset(&seq, 0, sizeof(seq));

  /* Load the tiles from the BMPs */
  tiles_load_default();

  SDL_initFramerate(&framerate_manager);
  /* The official v1.08 .exe runs 50-60 FPS in practice, despite the
     documented intent of running 83 FPS (or 12ms delay). */
  /* SDL_setFramerate(manager, 83); */
  SDL_setFramerate(&framerate_manager, FPS);
  
  init_state = GFX_INITIALIZED;
  return 0;
}

/**
 * Unload graphics subsystem
 */
void gfx_quit()
{
  init_state = GFX_QUITTING;

  gfx_fade_quit();

  gfx_fonts_quit();

  tiles_unload_all();
  sprites_unload();
  
  if (GFX_backbuffer   != NULL) SDL_FreeSurface(GFX_backbuffer);
  if (GFX_background    != NULL) SDL_FreeSurface(GFX_background);
  if (GFX_tmp1  != NULL) SDL_FreeSurface(GFX_tmp1);
  if (GFX_tmp2 != NULL) SDL_FreeSurface(GFX_tmp2);

  GFX_backbuffer = NULL;
  GFX_background = NULL;
  GFX_tmp1 = NULL;
  GFX_tmp2 = NULL;

  init_state = GFX_NOT_INITIALIZED;
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


/* LoadBMP wrapper. Load a new graphic from file, and apply the
   reference palette so that all subsequent blits are faster (color
   convertion is avoided) - although the initial loading time will be
   somewhat longer. */
static SDL_Surface* load_bmp_internal(char *filename, SDL_RWops *rw, int from_mem) {
  SDL_Surface *image;

  if (from_mem == 1)
    {
      image = IMG_Load_RW(rw, 1);
    }
  else
    {
      ciconvert(filename);
      image = IMG_Load(filename);
    }

  if (image == NULL)
    {
      /* fprintf(stderr, "load_bmp_internal: %s\n", SDL_GetError()); */
      /* Maybe it's just because we're at the end of a sequence */
      return NULL;
    }

  if (!truecolor)
    {
      /* Make a copy of the surface using the screen format (in
	 particular: same color depth, which is needed when importing
	 24bit graphics in 8bit mode). */
      /* This copy is also necessary to make a palette conversion from
	 the Dink palette (the one from the .bmp) to the
	 'DX-bug-messed' Dink palette (GFX_real_pal with overwritten
	 indexes 0 and 255). */
      SDL_Surface *converted = SDL_ConvertSurface(image, GFX_tmp2->format, 0);

      /* TODO: the following is probably unnecessary, I think that's
	 exactly what SDL_DisplayFormat does: convert the surface to
	 the screen's logical palette. Cf. test/sdl/paltest.c. */
      {
	/* In the end, the image must use the reference palette: that
	   way no mistaken color conversion will occur during blits to
	   other surfaces/buffers.  Blits should also be faster(?).
	   Alternatively we could replace SDL_BlitSurface with a
	   wrapper that sets identical palettes before the blits. */
	SDL_SetPaletteColors(converted->format->palette, GFX_real_pal, 0, 256);
	
	/* Blit the copy back to the original, with a potentially
	   different palette, which triggers color conversion to
	   image's palette. */
	gfx_blit_nocolorkey(image, NULL, converted, NULL);
      }
      SDL_FreeSurface(image);
      image = NULL;

      return converted;
    }
  else
    {
      /* In truecolor mode, converting a 8bit image to truecolor does
	 not bring noticeable performance increase or decrease, but
	 does increase memory usage by at least 10MB so let's use the
	 loaded image as-is. No need for palette conversion either. */
      return image;
    }

}

/* LoadBMP wrapper, from file */
SDL_Surface* load_bmp(char *filename)
{
  return load_bmp_internal(filename, NULL, 0);
}

/* LoadBMP wrapper, from FILE pointer */
SDL_Surface* load_bmp_from_fp(FILE* in)
{
  if (in == NULL)
    return NULL;
  SDL_RWops *rw = SDL_RWFromFP(in, /*autoclose=*/SDL_TRUE);
  return load_bmp_internal(NULL, rw, 1);
}

/* LoadBMP wrapper, from memory */
SDL_Surface* load_bmp_from_mem(SDL_RWops *rw)
{
  return load_bmp_internal(NULL, rw, 1);
}


/**
 * Temporary disable src's transparency and blit it to dst
 */
int gfx_blit_nocolorkey(SDL_Surface *src, SDL_Rect *src_rect,
			 SDL_Surface *dst, SDL_Rect *dst_rect)
{
  int retval = -1;
  if (src == NULL) {
    log_error("attempting to blit a NULL surface");
    return retval;
  }

  Uint32 colorkey;
  SDL_BlendMode blendmode;
  Uint32 rle_flags = src->flags & SDL_RLEACCEL;
  int has_colorkey = (SDL_GetColorKey(src, &colorkey) != -1);
  SDL_GetSurfaceBlendMode(src, &blendmode);

  if (has_colorkey)
    SDL_SetColorKey(src, SDL_FALSE, 0);
  SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);
  retval = SDL_BlitSurface(src, src_rect, dst, dst_rect);
  
  SDL_SetSurfaceBlendMode(src, blendmode);
  if (has_colorkey)
    SDL_SetColorKey(src, SDL_TRUE, colorkey);
  if (rle_flags)
    SDL_SetSurfaceRLE(src, 1);

  return retval;
}

/**
 * Blit and resize so that 'src' fits in 'dst_rect'
 */
int gfx_blit_stretch(SDL_Surface *src_surf, SDL_Rect *src_rect,
		     SDL_Surface *dst_surf, SDL_Rect *dst_rect)
{
  int retval = -1;

  SDL_Rect src_rect_if_null;
  if (src_rect == NULL)
    {
      src_rect = &src_rect_if_null;
      src_rect->x = 0;
      src_rect->y = 0;
      src_rect->w = src_surf->w;
      src_rect->h = src_surf->h;
    }

  if (src_rect->w == 0 || src_rect->h == 0)
    return 0;  // OK, ignore drawing, and don't mess sx/sy below

  double sx = 1.0 * dst_rect->w / src_rect->w;
  double sy = 1.0 * dst_rect->h / src_rect->h;
  /* In principle, double's are precise up to 15 decimal digits */
  if (fabs(sx-1) > 1e-10 || fabs(sy-1) > 1e-10)
    {
      SDL_Surface *scaled = zoomSurface(src_surf, sx, sy, SMOOTHING_OFF);

      /* Keep the same transparency / alpha parameters (SDL_gfx bug,
	 report submitted to the author: SDL_gfx adds transparency to
	 non-transparent surfaces) */
      Uint8 r, g, b, a;
      Uint32 colorkey;
      int colorkey_enabled = (SDL_GetColorKey(src_surf, &colorkey) != -1);
      SDL_GetRGBA(colorkey, src_surf->format, &r, &g, &b, &a);

      SDL_SetColorKey(scaled, colorkey_enabled,
		      SDL_MapRGBA(scaled->format, r, g, b, a));
      /* Don't mess with alpha transparency, though: */
      /* int alpha_flag = src->flags & SDL_SRCALPHA; */
      /* int alpha = src->format->alpha; */
      /* SDL_SetAlpha(scaled, alpha_flag, alpha); */
      
      src_rect->x = (int) round(src_rect->x * sx);
      src_rect->y = (int) round(src_rect->y * sy);
      src_rect->w = (int) round(src_rect->w * sx);
      src_rect->h = (int) round(src_rect->h * sy);
      retval = SDL_BlitSurface(scaled, src_rect, dst_surf, dst_rect);
      SDL_FreeSurface(scaled);
    }
  else
    {
      /* No scaling */
      retval = SDL_BlitSurface(src_surf, src_rect, dst_surf, dst_rect);
    }
  return retval;
}


/**
 * Refresh the physical screen, and apply a new palette or fade effect
 * if needed
 */
void flip_it(void)
{
  /* For now we do all operations on the CPU side and perform a big
     texture update at each frame; this is necessary to perform
     palette changes. */
  // TODO SDL2: implement truecolor mode on GPU side

  if (truecolor_fade_brightness < 256)
    gfx_fade_apply(truecolor_fade_brightness);

  /* Convert to destination buffer format */
  SDL_Surface* source = GFX_backbuffer;
  if (!truecolor) {
    /* Convert 8-bit buffer for truecolor texture upload */
	  source = rgba_screen;

    /* Use "physical" screen palette */
    SDL_Color pal_copy[256];
    SDL_Color pal_phys[256];
    memcpy(pal_copy, GFX_backbuffer->format->palette->colors, sizeof(pal_copy));
    gfx_palette_get_phys(pal_phys);
    SDL_SetPaletteColors(GFX_backbuffer->format->palette, pal_phys, 0, 256);

    if (SDL_BlitSurface(GFX_backbuffer, NULL, rgba_screen, NULL) < 0) {
      log_error("ERROR: 8-bit->truecolor conversion failed: %s", SDL_GetError());
    }
    SDL_SetPaletteColors(GFX_backbuffer->format->palette, pal_copy, 0, 256);
  }

  SDL_UpdateTexture(render_texture, NULL, source->pixels, source->pitch);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, render_texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void gfx_toggle_fullscreen()
{
  SDL_SetWindowFullscreen(window,
    (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)
    ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
  // Note: software renderer is buggy in SDL 2.0.2: it doesn't resize the surface
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
    int sum = 0;
    int i = 0;
    SDL_Surface* s = NULL;
    for (; i < GFX_TILES_NB_SETS+1; i++)
      {
	s = gfx_tiles[i];
	if (s != NULL)
	  sum += s->h * s->pitch;
      }
    log_debug("GFX tiles  = %8d", sum);
    total += sum;
  }

  log_debug("GFX total  = %8d (+ ~150kB fonts)", total);
}

void gfx_vlineRGB(SDL_Surface* s, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b) {
	SDL_Rect dst = { x, y1, 1, y2-y1 };
	SDL_FillRect(GFX_background, &dst, SDL_MapRGB(s->format, r, g, b));
}
void gfx_hlineRGB(SDL_Surface* s, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b) {
	SDL_Rect dst = { x1, y, x2-x1, 1 };
	SDL_FillRect(GFX_background, &dst, SDL_MapRGB(s->format, r, g, b));
}

void draw_box(rect box, int color)
{
/*   DDBLTFX     ddbltfx; */
  
/*   ddbltfx.dwSize = sizeof(ddbltfx); */
/*   ddbltfx.dwFillColor = color; */
  
/*   ddrval = lpDDSBack->Blt(&box ,NULL, NULL, DDBLT_COLORFILL| DDBLT_WAIT, &ddbltfx); */
  // GFX
  {
    SDL_Rect dst;
    dst.x = box.left; dst.y = box.top;
    dst.w = box.right - box.left;
    dst.h = box.bottom - box.top;
    SDL_FillRect(GFX_backbuffer, &dst, color);
  }
}

void fill_screen(int num)
{
  /* Warning: palette indexes 0 and 255 are hard-coded
     to black and white (cf. gfx_palette.c). */
  if (!truecolor)
    SDL_FillRect(GFX_background, NULL, num);
  else
    SDL_FillRect(GFX_background, NULL, SDL_MapRGB(GFX_background->format,
						GFX_real_pal[num].r,
						GFX_real_pal[num].g,
						GFX_real_pal[num].b));
}


/* Used to implement DinkC's copy_bmp_to_screen(). Difference with
   show_cmp: does not set showb.* (wait for button), install the image
   to lpDDSTwo (background) and not lpDDSBack (screen double
   buffer) */
void copy_bmp(char* name)
{
  char* fullpath = paths_dmodfile(name);
  SDL_Surface* image = IMG_Load(fullpath);
  if (image == NULL)
    {
      log_error("Couldn't load '%s': %s", name, SDL_GetError());
      return;
    }
  
  /* Set physical screen palette */
  if (!truecolor)
    {
      gfx_palette_set_from_surface(image);
      /* Grab back palette with DX bug overwrite */
      SDL_Color phys_pal[256];
      gfx_palette_get_phys(phys_pal);

      /* In case the DX bug messed the palette, let's convert the
	 image to the new palette. This also converts 24->8bit if
	 necessary. */
      {
	SDL_Surface* converted =  SDL_CreateRGBSurface(0, image->w,image->h,
						       8, 0,0,0,0);
	SDL_SetPaletteColors(converted->format->palette, phys_pal, 0, 256);
	SDL_BlitSurface(image, NULL, converted, NULL);
	SDL_FreeSurface(image);
	image = converted;
      }

      /* Next blit without palette conversion */
      SDL_SetPaletteColors(image->format->palette, GFX_real_pal, 0, 256);
    }

  SDL_BlitSurface(image, NULL, GFX_background, NULL);
  SDL_FreeSurface(image);

  abort_this_flip = /*true*/1;
}

void show_bmp(char* name, int script)
{
  char* fullpath = paths_dmodfile(name);
  SDL_Surface* image = IMG_Load(fullpath);
  if (image == NULL)
    {
      log_error("Couldn't load '%s': %s", name, SDL_GetError());
      return;
    }
  
  /* Set physical screen palette */
  if (!truecolor)
    {
      gfx_palette_set_from_surface(image);
      SDL_Color phys_pal[256];
      gfx_palette_get_phys(phys_pal);

      /* In case the DX bug messed the palette, let's convert the
	 image to the new palette. This also converts 24->8bit if
	 necessary. */
      {
	SDL_Surface* converted = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_INDEX8, 0);
	SDL_SetPaletteColors(converted->format->palette, phys_pal, 0, 256);
	SDL_BlitSurface(image, NULL, converted, NULL);
	SDL_FreeSurface(image);
	image = converted;
      }

      /* Next blit without palette conversion */
      SDL_SetPaletteColors(image->format->palette, GFX_real_pal, 0, 256);
    }

  SDL_BlitSurface(image, NULL, GFX_tmp1, NULL);
  SDL_FreeSurface(image);

  // After show_bmp(), and before the flip_it() call in updateFrame(),
  // other parts of the code will draw sprites on lpDDSBack and mess
  // the showbmp(). So skip the next flip_it().
  abort_this_flip = /*true*/1;
}
