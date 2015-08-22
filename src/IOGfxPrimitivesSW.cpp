#include "IOGfxPrimitivesSW.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL2_rotozoom.h"

#include "log.h"
#include "paths.h"
#include "io_util.h"

#include "gfx.h"
#include "gfx_palette.h"
#include "gfx_fade.h"

IOGfxPrimitivesSW::IOGfxPrimitivesSW() {
}

IOGfxPrimitivesSW::~IOGfxPrimitivesSW() {
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
      /* Make a copy of the surface using the screen format: same
         palette (with dithering), and same color depth (needed when
         importing 24bit graphics in 8bit mode). */
      SDL_PixelFormat* fmt = gfx_palette_get_phys_format();
      SDL_Surface *converted = SDL_ConvertSurface(image, fmt, 0);
	  SDL_FreeFormat(fmt);
      SDL_FreeSurface(image);
	  image = converted;
	  /* Disable palette conversion in future blits */
      SDL_SetPaletteColors(image->format->palette,
						   GFX_backbuffer->format->palette->colors, 0, 256);
      return image;
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


void gfx_center_game_display(SDL_Renderer *rend, SDL_Rect* rect) {
	int rend_w, rend_h;
	SDL_RenderGetLogicalSize(rend, &rend_w, &rend_h);
	int game_w = GFX_RES_W;
	int game_h = GFX_RES_H;
	double game_ratio = 1.0 * game_w / game_h;
	double rend_ratio = 1.0 * rend_w / rend_h;
	if (game_ratio < rend_ratio) {
		// left/right bars
		rect->w = game_w * rend_h / game_h;
		rect->h = rend_h;
		rect->x = (rend_w - rect->w) / 2;
		rect->y = 0;
	} else {
		// top/bottom bars
		rect->w = rend_w;
		rect->h = game_h * rend_w / game_w;
		rect->x = 0;
		rect->y = (rend_h - rect->h) / 2;
	}
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
  SDL_Rect dst;
  gfx_center_game_display(renderer, &dst);
  SDL_RenderCopy(renderer, render_texture, NULL, &dst);
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
						GFX_ref_pal[num].r,
						GFX_ref_pal[num].g,
						GFX_ref_pal[num].b));
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

      /* In case the DX bug messed the palette, let's convert the
	 image to the new palette. This also converts 24->8bit if
	 necessary. */
	  SDL_PixelFormat* fmt = gfx_palette_get_phys_format();
	  SDL_Surface *converted = SDL_ConvertSurface(image, fmt, 0);
	  SDL_FreeFormat(fmt);
	  SDL_FreeSurface(image);
	  image = converted;

      /* Next blit without palette conversion */
      SDL_SetPaletteColors(image->format->palette,
						   GFX_backbuffer->format->palette->colors, 0, 256);
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

      /* In case the DX bug messed the palette, let's convert the
	 image to the new palette. This also converts 24->8bit if
	 necessary. */
	  SDL_PixelFormat* fmt = gfx_palette_get_phys_format();
	  SDL_Surface *converted = SDL_ConvertSurface(image, fmt, 0);
	  SDL_FreeFormat(fmt);
	  SDL_FreeSurface(image);
	  image = converted;
	  /* Disable palette conversion in future blits */
      SDL_SetPaletteColors(image->format->palette,
						   GFX_backbuffer->format->palette->colors, 0, 256);
    }

  SDL_BlitSurface(image, NULL, GFX_tmp1, NULL);
  SDL_FreeSurface(image);

  // After show_bmp(), and before the flip_it() call in updateFrame(),
  // other parts of the code will draw sprites on lpDDSBack and mess
  // the showbmp(). So skip the next flip_it().
  abort_this_flip = /*true*/1;
}
