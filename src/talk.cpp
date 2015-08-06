/**
 * Talk interface for DinkC

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2005, 2007, 2008, 2009, 2010, 2012, 2014  Sylvain Beucler

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

#include "talk.h"

#include "SDL.h"
#include "log.h"
#include "game_engine.h" /* play */
#include "dinkvar.h" /* check_seq_status */
#include "gfx.h" /* GFX_lpDDSBack */
#include "gfx_fonts.h"
#include "gfx_sprites.h" /* GFX_k */
#include "input.h"
#include "sfx.h"

struct talk_struct talk;

void talk_start(int script, int nb_choices) {
  talk.last = nb_choices;
  talk.cur = 1;
  talk.active = /*true*/1;
  talk.page = 1;
  talk.cur_view = 1;
  talk.script = script;

  int ret = SDL_SetRelativeMouseMode(SDL_TRUE);
  if (ret == -1)
    log_error("Relative mouse positionning not supported on this platform.");
  // TODO INPUT: relative mode is messed with pen tablets

}

void talk_stop() {
  talk.active = /*false*/0;
  SDL_SetRelativeMouseMode(SDL_FALSE);
  // Avoid spurious mouse events in case when we set relative mouse
  // mode back and forth in a single frame during talk_stop/talk_start
  // (aka text submenu):
  SDL_PumpEvents();
}

void talk_clear()
{
  memset(&talk, 0, sizeof(talk));
  play.mouse = 0;
}
