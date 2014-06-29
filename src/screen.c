/**
 * Screen sprites and hardness

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2005, 2007, 2008, 2009, 2014  Sylvain Beucler

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

#include "dinkvar.h"
#include "screen.h"
#include "gfx.h"
#include "sfx.h"
#include "log.h"

struct sp spr[MAX_SPRITES_AT_ONCE]; //max sprite control systems at once

/* hardness */
struct hit_map hm;

/* dink.dat */
struct map_info map;

char current_map[50] = "map.dat";
char current_dat[50] = "dink.dat";

int last_sprite_created;

void screen_init() {
  memset(&spr, 0, sizeof(spr));
  memset(&hm, 0, sizeof(hm));
}

/**
 * Fills a int[MAX_SPRITES_EDITOR] with the index of the current
 * screen's sprites, sorted by ascending height/queue.
 */
void screen_rank_map_sprites(int* rank)
{
  memset(rank, 0, MAX_SPRITES_EDITOR * sizeof(int));

  int r1 = 0;
  int already_checked[MAX_SPRITES_EDITOR+1];
  memset(already_checked, 0, sizeof(already_checked));
  for (r1 = 0; r1 < MAX_SPRITES_EDITOR; r1++)
    {
      int highest_sprite = 22000; //more than it could ever be
      rank[r1] = 0;
      
      int h1;
      for (h1 = 1; h1 <= MAX_SPRITES_EDITOR; h1++)
	{
	  if (already_checked[h1] == 0 && pam.sprite[h1].active)
	    {
	      int height;
	      if (pam.sprite[h1].que != 0)
		height = pam.sprite[h1].que;
	      else
		height = pam.sprite[h1].y;
	      
	      if (height < highest_sprite)
		{
		  highest_sprite = height;
		  rank[r1] = h1;
		}
	    }
	}
      if (rank[r1] != 0)
	already_checked[rank[r1]] = 1;
    }
}

/**
 * Fills a int[MAX_SPRITES_AT_ONCE] with the index of the current
 * screen's sprites, sorted by ascending height/queue.
 */
void screen_rank_game_sprites(int* rank)
{
  memset(rank, 0, MAX_SPRITES_AT_ONCE * sizeof(int));

  int r1 = 0;
  int already_checked[MAX_SPRITES_AT_ONCE+1];
  memset(already_checked, 0, sizeof(already_checked));
  for (r1 = 0; r1 < last_sprite_created; r1++)
    {
      int highest_sprite = 22000; //more than it could ever be
      rank[r1] = 0;

      int h1;
      for (h1 = 1; h1 <= last_sprite_created; h1++)
	{
	  if (already_checked[h1] == 0 && spr[h1].active)
	    {
	      int height;
	      if (spr[h1].que != 0)
		height = spr[h1].que;
	      else
		height = spr[h1].y;
	      
	      if (height < highest_sprite)
		{
		  highest_sprite = height;
		  rank[r1] = h1;
		}
	    }
	}
      if (rank[r1] != 0)
	already_checked[rank[r1]] = 1;
    }
}


void fill_hard_sprites()
{
  int rank[MAX_SPRITES_AT_ONCE];
  screen_rank_game_sprites(rank);

  int r1 = 0;
  for (; r1 < last_sprite_created && rank[r1] > 0; r1++)
    {
      int h = rank[r1];
      if (spr[h].active)
	{
	  // Msg("proccesing sprite %d", h);
	  if (spr[h].sp_index != 0)
	    {
	      //Msg("has spindex of %d is_warp is %d",spr[h].sp_index,pam.sprite[spr[h].sp_index].is_warp);
	      if (pam.sprite[spr[h].sp_index].hard == 0)
		{
		  add_hardness(h,100+spr[h].sp_index);
		  //Msg("added warp hardness for %d", spr[h].sp_index);
		}
	    }
	  else
	    {
	      if (spr[h].hard == 0)
		{
		  //Msg("adding a new sprite hardness %d (not from editor)", h);
		  add_hardness(h, 1);
		}
	    }
	}
    }
}


void fill_whole_hard(void)
{
  int til;
  for (til=0; til < 96; til++)
    {
      int offx = (til * 50 - ((til / 12) * 600));
      int offy = (til / 12) * 50;
      int x, y;
      for (x = 0; x < 50; x++)
	for (y = 0; y < 50; y++)
	  hm.x[offx +x].y[offy+y] = hmap.htile[  realhard(til)  ].x[x].y[y];
    }
}
