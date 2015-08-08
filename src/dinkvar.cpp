/**
 * Common code for FreeDink and FreeDinkedit

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2003  Shawn Betts
 * Copyright (C) 2005, 2006  Dan Walma
 * Copyright (C) 2005, 2007, 2008, 2009, 2010, 2012, 2014, 2015  Sylvain Beucler

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


#include "SDL.h"
#include "SDL_image.h"

#include "live_sprites_manager.h"
#include "live_screen.h"
#include "dinkini.h"
#include "gfx.h"
#include "gfx_sprites.h"
#include "gfx_palette.h"
#include "bgm.h"
#include "str_util.h"
#include "paths.h"
#include "log.h"

int dinkspeed = 3;
int show_inventory = 0; // display inventory?

int flub_mode = -500;

int push_active = 1;

int base_timing = 0;

int move_screen = 0;
int move_counter = 0;
int cur_map;
int* pplayer_map;


/* Screen transition */
/*bool*/int transition_in_progress = /*false*/0;



void draw_sprite_game(SDL_Surface *GFX_lpdest, int h)
{
  if (spr[h].brain == 8)
    return; // text
  if (spr[h].nodraw == 1)
    return; // invisible

  rect box_crap,box_real;

  if (get_box(h, &box_crap, &box_real, false))
    {
      /* Generic scaling */
      /* Not perfectly accurate yet: move a 200% sprite to the border
	 of the screen to it is clipped: it's scaled size will slighly
	 vary. Maybe we need to clip the source zone before scaling
	 it.. */
      // error checking for invalid rectangle
      if (box_crap.left >= box_crap.right || box_crap.top >= box_crap.bottom)
	return;
      
      SDL_Rect src, dst;
      int retval = 0;
      src.x = box_real.left;
      src.y = box_real.top;
      src.w = box_real.right - box_real.left;
      src.h = box_real.bottom - box_real.top;
      dst.x = box_crap.left;
      dst.y = box_crap.top;
      dst.w = box_crap.right - box_crap.left;
      dst.h = box_crap.bottom - box_crap.top;

      retval = gfx_blit_stretch(GFX_k[getpic(h)].k, &src, GFX_lpdest, &dst);
      
      if (retval < 0) {
	log_error("Could not draw sprite %d: %s", getpic(h), SDL_GetError());
	/* If we failed, then maybe the sprite was actually loaded
	   yet, let's try now */
	if (spr[h].pseq != 0)
	  check_seq_status(spr[h].pseq);
    }
  }
}

        int hurt_thing(int h, int damage, int special)
        {
                //lets hurt this sprite but good
                if (damage < 1) return(0);
                int num = damage - spr[h].defense;

                //      Msg("num is %d.. defense was %d.of sprite %d", num, spr[h].defense, h);
                if (num < 1) num = 0;

                if (num == 0)
                {
                        if ((rand() % 2)+1 == 1) num = 1;
                }

                spr[h].damage += num;
                return(num);
                //draw blood here
        }
