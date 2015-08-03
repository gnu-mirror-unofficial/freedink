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

#ifndef _SCREEN_H
#define _SCREEN_H

#include "rect.h"
#include "dinkc_sp_custom.h"
#include "gfx_tiles.h"

#define MAX_SPRITES_EDITOR 99

#define MAX_SPRITES_AT_ONCE 300

extern char current_map[50];

struct sp
{
  int x,moveman;
  int y;
  int mx,my;
  int lpx[51],lpy[51];
  int speed;
  int brain;
  int seq_orig,dir;
  int seq;
  int frame;
  unsigned long delay;
  int pseq;
  int pframe;
  
  /*BOOL*/int active;
  int attrib;
  unsigned long wait;
  int timer;
  int skip;
  int skiptimer;
  int size;
  int que;
  int base_walk;
  int base_idle;
  int base_attack;
  
  int base_hit;
  int last_sound;
  int hard;
  rect alt;
  int althard;
  int sp_index;  /* editor_sprite */
  /*BOOL*/int nocontrol;
  int idle;
  int strength;
  int damage;
  int defense;
  int hitpoints;
  int exp;
  int gold;
  int base_die;
  int kill;
  Uint32 kill_timer;
  int script_num;
  char text[200];
  int owner;
  int script;
  int sound;
  int callback;
  int freeze;
  /*bool*/int move_active;
  int move_script;
  int move_dir;
  int move_num;
  /*BOOL*/int move_nohard;
  int follow;
  int nohit;
  /*BOOL*/int notouch;
  unsigned long notouch_timer;
  /*BOOL*/int flying;
  int touch_damage;
  int brain_parm;
  int brain_parm2;
  /*BOOL*/int noclip;
  /*BOOL*/int reverse;
  /*BOOL*/int disabled;
  int target;
  int attack_wait;
  int move_wait;
  int distance;
  int last_hit;
  /*BOOL*/int live;
  int range;
  int attack_hit_sound;
  int attack_hit_sound_speed;
  int action;
  int nodraw;
  int frame_delay;
  int picfreeze;
  /* v1.08 */
  int bloodseq;
  int bloodnum;
  dinkc_sp_custom custom;
};
extern struct sp spr[];

// sub struct for hardness map
struct mega_y
{
	unsigned char y[401];
};

// struct for hardness map
struct hit_map
{
	struct mega_y x[601];
};
extern struct hit_map hm;

extern int last_sprite_created;


struct sprite_placement
{
  int x, y;
  int seq, frame, type;  /* DinkC: editor_seq, editor_frame, editor_type */
  int size;
  BOOL_1BYTE active;
  int rotation, special, brain;
  
  char script[13+1]; /* attached DinkC script */
  int speed, base_walk, base_idle, base_attack, base_hit, timer, que;
  int hard;
  rect alt; /* trim left/top/right/bottom */
  int is_warp;
  int warp_map;
  int warp_x;
  int warp_y;
  int parm_seq;
  
  int base_die, gold, hitpoints, strength, defense, exp, sound, vision, nohit, touch_damage;
  int buff[5];
};

/* one screen from map.dat */
struct small_map
{
  struct tile t[12*8+1]; // 97 background tiles
  struct sprite_placement sprite[100+1];
  char script[20+1]; /* script to run when entering the script */
};


extern void screen_init();
extern int load_map_to(char* path, const int num, struct small_map* screen);
extern void save_map(const int num);
extern void screen_rank_map_sprites(int rank[]);
extern void screen_rank_game_sprites(int rank[]);
extern void fill_hard_sprites(void);
extern void fill_whole_hard(void);

#endif
