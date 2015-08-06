/**
 * Display game texts

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2014  Sylvain Beucler

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

#include "text.h"

#include "dinkvar.h"
#include "live_sprites_manager.h"
#include "gfx.h"
#include "gfx_sprites.h"
#include "talk.h"
#include "log.h"

#define TEXT_MIN 2700
#define TEXT_TIMER 77

/* say_text, say_text_xy */
int add_text_sprite(char* text, int script, int sprite_owner, int mx, int my)
{
  int tsprite = add_sprite(mx, my, 8, 0, 0);
  if (tsprite == 0)
    {
      log_error("Couldn't say something, out of sprites.");
      return 0;
    }

  strncpy(spr[tsprite].text, text, 200-1); // TODO: currently truncated to 199 chars
  spr[tsprite].text[200-1] = '\0';

  *plast_text = tsprite;
  spr[tsprite].kill = strlen(text) * TEXT_TIMER;
  if (spr[tsprite].kill < TEXT_MIN)
    spr[tsprite].kill = TEXT_MIN;
  spr[tsprite].damage = -1;
  spr[tsprite].owner = sprite_owner;
  spr[tsprite].hard = 1;
  spr[tsprite].script = script;
  spr[tsprite].nohit = 1;

  return tsprite;
}

int say_text(char* text, int sprite_owner, int script)
{
  int tsprite;
  if (sprite_owner == 1000)
    tsprite = add_text_sprite(text, script, 1000, 100, 100);
  else
    tsprite = add_text_sprite(text, script, sprite_owner,
			      spr[sprite_owner].x, spr[sprite_owner].y);
  
  if (tsprite == 0)
    return 0;
  
  //set X offset for text, using strength var since it's unused
  spr[tsprite].strength = 75;
  check_seq_status(spr[spr[tsprite].owner].seq);
  spr[tsprite].defense = ( ( k[getpic(spr[tsprite].owner)].box.bottom
			     - k[getpic(spr[tsprite].owner)].yoffset )
			   + 100 );
  
  spr[tsprite].x = spr[spr[tsprite].owner].x - spr[tsprite].strength;
  spr[tsprite].y = spr[spr[tsprite].owner].y - spr[tsprite].defense;
  
  return tsprite;
}

int say_text_xy(char* text, int mx, int my, int script)
{
  int sprite_owner = 1000;
  return add_text_sprite(text, script, sprite_owner, mx, my);
}
