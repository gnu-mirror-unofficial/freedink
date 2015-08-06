/**
 * Displayed screen

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2005, 2007, 2008, 2009, 2014, 2015  Sylvain Beucler

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

#include "game_engine.h"
#include "live_sprites_manager.h"
#include "live_screen.h"
#include "editor_screen.h"
#include "hardness_tiles.h"
#include "gfx_sprites.h"
#include "log.h"
#include "dinkvar.h"

/* base editor screen */
struct screen cur_ed_screen;

/* hardness */
unsigned char screen_hitmap[600+1][400+1]; /* hit_map */

int playx = 620;
int playl = 20;
int playy = 400;

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
	      //Msg("has spindex of %d is_warp is %d",spr[h].sp_index,cur_ed_screen.sprite[spr[h].sp_index].is_warp);
	      if (cur_ed_screen.sprite[spr[h].sp_index].hard == 0)
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
	  screen_hitmap[offx +x][offy+y] = hmap.htile[  realhard(til)  ].hm[x][y];
    }
}


//add hardness from a sprite
void add_hardness (int sprite, int num)
{
  int xx;
  for (xx = spr[sprite].x + k[getpic(sprite)].hardbox.left; xx < spr[sprite].x + k[getpic(sprite)].hardbox.right; xx++)
    {
      int yy;
      for (yy = spr[sprite].y + k[getpic(sprite)].hardbox.top; yy < spr[sprite].y + k[getpic(sprite)].hardbox.bottom; yy++)
	{
	  if ( (xx-20 > 600) | (xx-20 < 0)| (yy > 400) | (yy < 0))
	    ; /* Nothing */
	  else
	    screen_hitmap[xx-20][yy] = num;
	}
    }
}




/**
 * Check whether planned new position (x1,y1) is solid
 * 
 * Only used in 'check_if_move_is_legal'
 *
 * Returns: 0 if not solid, !0 otherwise
 */
unsigned char get_hard(int x1, int y1)
{
  // TODO: break dependency: pass screenlock as argument
  if (screenlock)
    {
      if (x1 < 0)        x1 = 0;
      else if (x1 > 599) x1 = 599;

      if (y1 < 0)        y1 = 0;
      else if (y1 > 399) y1 = 399;
    }
  if ((x1 < 0) || (y1 < 0) || (x1 > 599) || (y1 > 399))
    return 0;
  
  int value = screen_hitmap[x1][y1];
  return(value);
}
