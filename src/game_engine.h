/**
 * Game-specific processing

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2008, 2009, 2012  Sylvain Beucler

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

#ifndef _GAME_ENGINE_H
#define _GAME_ENGINE_H

#include <time.h>

#include "SDL.h"
#include "io_util.h"
#include "dinkc.h"
#include "gfx_tiles.h"
#include "dinkc_sp_custom.h"

#define FPS 60

struct item_struct
{
  BOOL_1BYTE active;
  char name[10+1];
  int seq;
  int frame;
};

struct mydata
{
  unsigned char type[100];  // DinkC's editor_type(i)
  unsigned short seq[100];  // DinkC's editor_seq(i)
  unsigned char frame[100]; // DinkC's editor_frame(i)
  int last_time; // ticks when type 6, 7 or 8 was set
};

/* Game state. Saved games are generated by dumping this structure in
   to SAVEx.DAT, so DON'T DON'T DON'T change anything here ;)
   otherwise you'll change the saved games format. */
// for storing current tiles in save game
struct player_info_tile
{
  char file[50];
};
#define NB_MITEMS 8
#define NB_ITEMS 16
struct player_info
{
  int minutes;
  
  struct item_struct mitem[NB_MITEMS];
  struct item_struct item[NB_ITEMS];
  
  int curitem; // highlighted item in the inventory
  BOOL_1BYTE item_magic; // 1 if it's a magic item, 0 if regular

  struct mydata spmap[769];
  struct varman var[MAX_VARS];
  
  
  BOOL_1BYTE push_active;
  int push_dir;
  unsigned int push_timer;
  int last_talk;
  int mouse; /* vertical position of the mouse when selecting a dialog
		option */
  int last_map;
  
  /* v1.08: use wasted space for storing file location of map.dat,
     dink.dat, palette, and tiles */
  /* char cbuff[6000];*/
  char mapdat[50];
  char dinkdat[50];
  char palette[50];
  struct player_info_tile tile[GFX_TILES_NB_SETS+1];
  struct global_function func[100];
};

extern struct player_info play;
extern int last_sprite_created;

/* Engine variables directly mapped with DinkC variables */
extern int *pvision, *plife, *presult, *pspeed, *ptiming, *plifemax,
  *pexper, *pstrength, *pcur_weapon,*pcur_magic, *pdefense,
  *pgold, *pmagic, *plevel, *plast_text, *pmagic_level;
extern int *pupdate_status, *pmissile_target, *penemy_sprite,
  *pmagic_cost, *pmissle_source;


extern int flife, fexp, fstrength, fdefense, fgold, fmagic,
  fmagic_level, flifemax, fraise, last_magic_draw;


struct wait_for_button
{
	int script;
	int button;
	/*bool*/int active;
};
extern struct wait_for_button wait4b;


extern int last_saved_game;

extern char *dversion_string;
extern int dversion;
#define LEN_SAVE_GAME_INFO 200
extern char save_game_info[LEN_SAVE_GAME_INFO];

extern time_t time_start;

extern int smooth_follow;


/* Editor sprite containing the current warp (teleporter), 0 if no active warp: */
extern int process_warp;
extern int process_downcycle;
extern int process_upcycle;
extern unsigned long cycle_clock;
extern int cycle_script;

extern unsigned int dink_base_push;

extern /*bool*/int screen_main_is_running;

extern void game_init();
extern void game_quit();
extern void game_restart();

extern Uint32 game_GetTicks(void);
extern void game_set_high_speed(void);
extern void game_set_normal_speed(void);

extern int next_raise();
extern void add_exp_force(int num, int source_sprite);
extern void add_exp(int num, int killed_sprite);

extern void fix_dead_sprites();

extern int game_load_screen(int num);
extern void update_screen_time(void);
extern void update_play_changes( void );
extern void game_place_sprites(void);
extern void game_place_sprites_background(void);
extern void fill_back_sprites(void);

extern void draw_screen_game(void);
extern void draw_screen_game_background(void);
#endif
