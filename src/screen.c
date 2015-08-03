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
#include "paths.h"
#include "map.h"

char current_map[50] = "map.dat";
struct screen cur_screen;

struct sp spr[MAX_SPRITES_AT_ONCE]; //max sprite control systems at once

/* hardness */
struct hit_map hm;

int last_sprite_created;

void screen_init() {
  memset(&spr, 0, sizeof(spr));
  memset(&hm, 0, sizeof(hm));
  memset(&cur_screen, 0, sizeof(cur_screen));
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
	  if (already_checked[h1] == 0 && cur_screen.sprite[h1].active)
	    {
	      int height;
	      if (cur_screen.sprite[h1].que != 0)
		height = cur_screen.sprite[h1].que;
	      else
		height = cur_screen.sprite[h1].y;
	      
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
	      //Msg("has spindex of %d is_warp is %d",spr[h].sp_index,cur_screen.sprite[spr[h].sp_index].is_warp);
	      if (cur_screen.sprite[spr[h].sp_index].hard == 0)
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



/**
 * Load 1 screen from specified map.dat in specified memory buffer
 */
int load_map_to(char* path, const int num, struct screen* screen)
{
  char skipbuf[10000]; // more than any fseek we do

  FILE *f = NULL;
  long holdme,lsize;
  f = paths_dmodfile_fopen(path, "rb");
  if (!f)
    {
      log_error("Cannot find %s file!!!", path);
      return -1;
    }
  lsize = 31280; // sizeof(struct small_map); // under i386, not portable
  holdme = (lsize * (num-1));
  fseek(f, holdme, SEEK_SET);
  //Msg("Trying to read %d bytes with offset of %d",lsize,holdme);

  /* Portably load map structure from disk */
  int i = 0;
  fread(skipbuf, 20, 1, f); // unused 'name' field
  for (i = 0; i < 97; i++)
    {
      screen->t[i].square_full_idx0 = read_lsb_int(f);
      fread(skipbuf, 4, 1, f); // unused 'property' field
      screen->t[i].althard = read_lsb_int(f);
      fread(skipbuf, 6, 1, f); // unused 'more2', 'more3', 'more4' fields
      fread(skipbuf, 2, 1, f); // reproduce memory alignment
      fread(skipbuf, 60, 1, f); // unused 'buff' field
    }
  // offset 7780
  
  fread(skipbuf, 160, 1, f); // unused 'v' field
  fread(skipbuf, 80, 1, f);  // unused 's' field
  // offset 8020
  
  /* struct sprite_placement sprite[101]; */
  /* size = 220 */
  for (i = 0; i < 101; i++)
    {
      screen->sprite[i].x = read_lsb_int(f);
      screen->sprite[i].y = read_lsb_int(f);
      screen->sprite[i].seq = read_lsb_int(f);
      screen->sprite[i].frame = read_lsb_int(f);
      screen->sprite[i].type = read_lsb_int(f);
      screen->sprite[i].size = read_lsb_int(f);
      
      screen->sprite[i].active = fgetc(f);
      fread(skipbuf, 3, 1, f); // reproduce memory alignment
      // offset 28
      
      screen->sprite[i].rotation = read_lsb_int(f);
      screen->sprite[i].special = read_lsb_int(f);
      screen->sprite[i].brain = read_lsb_int(f);
      
      fread(screen->sprite[i].script, 14, 1, f);
      screen->sprite[i].script[14-1] = '\0'; // safety
      fread(skipbuf, 38, 1, f); // unused hit/die/talk fields
      // offset 92
      
      screen->sprite[i].speed = read_lsb_int(f);
      screen->sprite[i].base_walk = read_lsb_int(f);
      screen->sprite[i].base_idle = read_lsb_int(f);
      screen->sprite[i].base_attack = read_lsb_int(f);
      screen->sprite[i].base_hit = read_lsb_int(f);
      screen->sprite[i].timer = read_lsb_int(f);
      screen->sprite[i].que = read_lsb_int(f);
      screen->sprite[i].hard = read_lsb_int(f);
      // offset 124
      
      screen->sprite[i].alt.left = read_lsb_int(f);
      screen->sprite[i].alt.top = read_lsb_int(f);
      screen->sprite[i].alt.right = read_lsb_int(f);
      screen->sprite[i].alt.bottom = read_lsb_int(f);
      // offset 140
      
      screen->sprite[i].is_warp = read_lsb_int(f);
      screen->sprite[i].warp_map = read_lsb_int(f);
      screen->sprite[i].warp_x = read_lsb_int(f);
      screen->sprite[i].warp_y = read_lsb_int(f);
      screen->sprite[i].parm_seq = read_lsb_int(f);
      // offset 160
      
      screen->sprite[i].base_die = read_lsb_int(f);
      screen->sprite[i].gold = read_lsb_int(f);
      screen->sprite[i].hitpoints = read_lsb_int(f);
      screen->sprite[i].strength = read_lsb_int(f);
      screen->sprite[i].defense = read_lsb_int(f);
      screen->sprite[i].exp = read_lsb_int(f);
      screen->sprite[i].sound = read_lsb_int(f);
      screen->sprite[i].vision = read_lsb_int(f);
      screen->sprite[i].nohit = read_lsb_int(f);
      screen->sprite[i].touch_damage = read_lsb_int(f);
      // offset 200
      
      int j = 0;
      for (j = 0; j < 5; j++)
	screen->sprite[i].buff[j] = read_lsb_int(f);
    }
  // offset 30204
  
  fread(screen->script, 21, 1, f);
  screen->script[21-1] = '\0'; // safety
  fread(skipbuf, 1018, 1, f); // unused hit/die/talk fields
  fread(skipbuf, 1, 1, f); // reproduce memory alignment
  // offset 31280
  
  fclose(f);
  return 0;
}

/**
 * Save screen number 'num' in the map. Only used by the editor.
 */
void save_map(const int num)
{
  char skipbuf[10000]; // more than any fseek we do
  memset(skipbuf, 0, 10000);

  FILE *f = NULL;
  long holdme,lsize;

  log_info("Saving map data..");
  if (num > 0)
    {
      f = paths_dmodfile_fopen(current_map, "r+b");
      if (f == NULL)
	{
	  perror("Cannot save map");
	  return;
	}
      lsize = 31280; // sizeof(struct small_map); // under ia32, not portable
      holdme = (lsize * (num-1));
      fseek(f, holdme, SEEK_SET);


      /* Portably dump map structure */
      int i = 0;
      char name[20] = "Smallwood";
      fwrite(name, 20, 1, f);
      for (i = 0; i < 97; i++)
	{
	  write_lsb_int(cur_screen.t[i].square_full_idx0, f);
	  fwrite(skipbuf, 4, 1, f); // unused 'property' field
	  write_lsb_int(cur_screen.t[i].althard, f);
	  fwrite(skipbuf, 6, 1, f); // unused 'more2', 'more3', 'more4' fields
	  fwrite(skipbuf, 2, 1, f); // reproduce memory alignment
	  fwrite(skipbuf, 60, 1, f); // unused 'buff' field
	}
      // offset 7780

      fwrite(skipbuf, 160, 1, f); // unused 'v' field
      fwrite(skipbuf, 80, 1, f);  // unused 's' field
      // offset 8020

      /* struct sprite_placement sprite[101]; */
      /* size = 220 */
      for (i = 0; i < 101; i++)
	{
	  write_lsb_int(cur_screen.sprite[i].x, f);
	  write_lsb_int(cur_screen.sprite[i].y, f);
	  write_lsb_int(cur_screen.sprite[i].seq, f);
	  write_lsb_int(cur_screen.sprite[i].frame, f);
	  write_lsb_int(cur_screen.sprite[i].type, f);
	  write_lsb_int(cur_screen.sprite[i].size, f);

	  fputc(cur_screen.sprite[i].active, f);
	  fwrite(skipbuf, 3, 1, f); // reproduce memory alignment
	  // offset 28
	  
	  write_lsb_int(cur_screen.sprite[i].rotation, f);
	  write_lsb_int(cur_screen.sprite[i].special, f);
	  write_lsb_int(cur_screen.sprite[i].brain, f);

	  fwrite(cur_screen.sprite[i].script, 14, 1, f);
	  fwrite(skipbuf, 38, 1, f); // reproduce memory alignment
	  // offset 92

	  write_lsb_int(cur_screen.sprite[i].speed, f);
	  write_lsb_int(cur_screen.sprite[i].base_walk, f);
	  write_lsb_int(cur_screen.sprite[i].base_idle, f);
	  write_lsb_int(cur_screen.sprite[i].base_attack, f);
	  write_lsb_int(cur_screen.sprite[i].base_hit, f);
	  write_lsb_int(cur_screen.sprite[i].timer, f);
	  write_lsb_int(cur_screen.sprite[i].que, f);
	  write_lsb_int(cur_screen.sprite[i].hard, f);
	  // offset 124

	  write_lsb_int(cur_screen.sprite[i].alt.left, f);
	  write_lsb_int(cur_screen.sprite[i].alt.top, f);
	  write_lsb_int(cur_screen.sprite[i].alt.right, f);
	  write_lsb_int(cur_screen.sprite[i].alt.bottom, f);
	  // offset 140

	  write_lsb_int(cur_screen.sprite[i].is_warp, f);
	  write_lsb_int(cur_screen.sprite[i].warp_map, f);
	  write_lsb_int(cur_screen.sprite[i].warp_x, f);
	  write_lsb_int(cur_screen.sprite[i].warp_y, f);
	  write_lsb_int(cur_screen.sprite[i].parm_seq, f);
	  // offset 160
  
	  write_lsb_int(cur_screen.sprite[i].base_die, f);
	  write_lsb_int(cur_screen.sprite[i].gold, f);
	  write_lsb_int(cur_screen.sprite[i].hitpoints, f);
	  write_lsb_int(cur_screen.sprite[i].strength, f);
	  write_lsb_int(cur_screen.sprite[i].defense, f);
	  write_lsb_int(cur_screen.sprite[i].exp, f);
	  write_lsb_int(cur_screen.sprite[i].sound, f);
	  write_lsb_int(cur_screen.sprite[i].vision, f);
	  write_lsb_int(cur_screen.sprite[i].nohit, f);
	  write_lsb_int(cur_screen.sprite[i].touch_damage, f);
	  // offset 200

	  int j = 0;
	  for (j = 0; j < 5; j++)
	    write_lsb_int(cur_screen.sprite[i].buff[j], f);
	}
      // offset 30204
      
      fwrite(cur_screen.script, 21, 1, f);
      fwrite(skipbuf, 1018, 1, f); // unused random/load/buffer fields
      fwrite(skipbuf, 1, 1, f); // reproduce memory alignment
      // offset 31280

      fclose(f);
    }

  log_info("Done saving map data..");
}
