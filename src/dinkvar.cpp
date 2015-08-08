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
