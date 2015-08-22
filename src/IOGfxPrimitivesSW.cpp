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
static SDL_Surface* load_bmp_internal(SDL_RWops *rw) {
  SDL_Surface *image = IMG_Load_RW(rw, 1);

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

/* LoadBMP wrapper, from FILE pointer */
SDL_Surface* load_bmp_from_fp(FILE* in)
{
  if (in == NULL)
    return NULL;
  SDL_RWops *rw = SDL_RWFromFP(in, /*autoclose=*/SDL_TRUE);
  return load_bmp_internal(rw);
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
void flip_it()
{
  /* For now we do all operations on the CPU side and perform a big
     texture update at each frame; this is necessary to perform
     palette changes. */

  if (truecolor_fade_brightness < 256)
    gfx_fade_apply(truecolor_fade_brightness);

  display->flip(IOGFX_backbuffer);
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
