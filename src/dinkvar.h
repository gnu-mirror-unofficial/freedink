/**
 * Header for code common to FreeDink and FreeDinkedit

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

#ifndef _DINKVAR_H
#define _DINKVAR_H

/* for RECT ?? */
/* #include <windows.h> */

/* #include <ddraw.h> */
/* #include <dinput.h> */
/* #include <mmsystem.h> */

#include <limits.h>

#include "SDL.h"
#include "SDL2_framerate.h"
#include "game_engine.h"
#include "rect.h"
#include "dinkc.h"
#include "screen.h"

struct attackinfo_struct
{
	int time;
	/*bool*/int active;
	int script;
	/*bool*/int hitme;
	int last_power;
	Uint32 wait;
	Uint32 pull_wait;
};


extern int GetKeyboard(int key);
extern int add_sprite(int x1, int y, int brain,int pseq, int pframe );
extern int add_sprite_dumb(int x1, int y, int brain,int pseq, int pframe,int size);
extern void check_seq_status(int h);
/* extern void dderror(HRESULT hErr); */
//extern void draw_sprite_game(LPDIRECTDRAWSURFACE lpdest,int h);
extern void draw_sprite_game(SDL_Surface *GFX_lpdest, int h);
extern void duck_brain(int h);
extern /*BOOL*/int init_mouse();
extern int load_script(char filename[15], int sprite, /*bool*/int set_sprite);
extern /*bool*/int locate(int script, char proc[20]);
extern void process_callbacks(void);
extern void run_script (int script);
extern void update_status_all(void);

extern /*bool*/int abort_this_flip;
extern int base_timing;
extern struct attackinfo_struct bow;
extern int dinkspeed;
extern int flife;
extern int flub_mode;
extern int fps_final;
extern int show_inventory;
extern int stop_entire_game;
extern int getpic(int h);

/* show_bmp() currently ran */
struct show_bmp
{
	/*bool*/int active;
	/*bool*/int showdot;
	int reserved;
	int script;
	int stime;
	int picframe;
};
extern struct show_bmp showb;


extern int mbase_count;
extern unsigned long mold;

extern int *pupdate_status;
extern int playl;
extern int playx;
extern int playy;
extern int *pplayer_map;
extern int screenlock;
extern Uint32 thisTickCount;
extern Uint32 lastTickCount;
extern FPSmanager framerate_manager;
extern /*bool*/int transition_in_progress;

/* extern HRESULT ddrval; */
/* extern LPDIRECTDRAWPALETTE lpDDPal; /\* The primary surface palette *\/ */
/* extern PALETTEENTRY    pe[256]; */


/* Game state */
extern void attach(void);
extern /*bool*/int windowed;
extern int fcur_weapon, fcur_magic;
extern int push_active;
extern int move_screen;
extern int move_counter;

/* Player */
extern /*bool*/int inside_box(int x1, int y1, rect box);
extern int walk_off_screen;

/* Sprites - state */
extern void changedir( int dir1, int k,int base);

/* Sprites - global */
extern void kill_sprite_all (int sprite);
extern int find_sprite(int editor_sprite);

/* Scripts */
extern void kill_all_scripts_for_real(void);
extern void kill_returning_stuff(int script);

/* Map */
extern unsigned char get_hard(int x1, int y1);
extern unsigned char get_hard_play(int h, int x1, int y1);
extern void load_hard(void);

/* OS */
extern int bActive; // is application active?
extern char *command_line; // command line params, used by doInit


/* Startup */
extern void pre_figure_out(char* line);
extern void figure_out(char* line);

/* Metadata */
extern int burn_revision;

/*
 * Game & editor
 */
struct ts_block
{
  unsigned char hm[50+1][50+1];  // tile hardness/hitmap
  BOOL_1BYTE used;
};

//struct for hardness info, INDEX controls which hardness each block has.  800 max
//types available.
#define HARDNESS_NB_TILES 800
struct hardness
{
  struct ts_block htile[HARDNESS_NB_TILES];
  /* default hardness for each background tile square, 12*8=96 tiles
     per screen but indexed % 128 in the code (so 128*(41-1)+12*8=5216
     used indexes instead of 12*8*41=3936). */
  short btile_default[GFX_TILES_NB_SQUARES];
};

/*bool*/int get_box (int h, rect * box_crap, rect * box_real);
extern /*bool*/int dinkedit;
extern int draw_screen_tiny;
extern int cur_map;
extern struct hardness hmap;


/*
 * Editor
 */

extern void check_sprite_status(int h);
extern void add_hardness(int sprite, int num);
extern /*bool*/int kill_last_sprite(void);
extern void check_frame_status(int h, int frame);
extern void flip_it_second(void);
extern int realhard(int tile);
extern void save_hard(void);

extern void fill_screen(int num);


extern void show_bmp(char name[80], int showdot, int script);
extern void copy_bmp( char name[80]);
extern /*bool*/int text_owned_by(int sprite);
extern void fill_hardxy(rect box);
extern int does_sprite_have_text(int sprite);
extern int change_sprite(int h,  int val, int * change);
extern int change_sprite_noreturn(int h,  int val, int * change);
extern int change_edit_char(int h,  int val, unsigned char * change);
extern int change_edit(int h,  int val, unsigned short * change);
extern int hurt_thing(int h, int damage, int special);
extern void random_blood(int mx, int my, int h);
extern void check_sprite_status_full(int sprite_no);


/* Game modes */
extern int mode;
extern int keep_mouse;
// + talk.active

extern void set_mode(int new_mode);
extern void set_keep_mouse(int on);
#endif
