/**
 * Interface for graphics primitives

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

#ifndef IOGFX_H
#define IOGFX_H

#include "rect.h"

class IOGfxPrimitives {
public:
	virtual ~IOGfxPrimitives() = 0;
};

#include "SDL.h"
extern SDL_Surface* load_bmp(char *filename);
extern SDL_Surface* load_bmp_from_fp(FILE* in);
extern SDL_Surface* load_bmp_from_mem(SDL_RWops *rw);
extern int gfx_blit_nocolorkey(SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect);
extern int gfx_blit_stretch(SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect);
extern void gfx_toggle_fullscreen(void);
extern void gfx_vlineRGB(SDL_Surface* s, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b);
extern void gfx_hlineRGB(SDL_Surface* s, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b);
extern void draw_box(rect box, int color);
extern void fill_screen(int num);
extern void copy_bmp(char* name);
extern void show_bmp(char* name, int script);

#endif
