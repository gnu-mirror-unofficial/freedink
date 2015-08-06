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
extern int screenlock;

/* Player */
extern /*bool*/int inside_box(int x1, int y1, rect box);
extern int walk_off_screen;

/* Sprites - state */
extern void changedir( int dir1, int k,int base);

/* Sprites - global */
extern void kill_sprite_all (int sprite);

/* Scripts */
extern void kill_all_scripts_for_real(void);
extern void kill_returning_stuff(int script);

/* OS */
extern int bActive; // is application active?
extern char *command_line; // command line params, used by doInit


/* Startup */
extern void pre_figure_out(char* line);
extern void figure_out(char* line);

/* Metadata */
extern int burn_revision;

/*bool*/int get_box (int h, rect * box_crap, rect * box_real);
extern /*bool*/int dinkedit;
extern int draw_screen_tiny;
extern int cur_map;


/*
 * Editor
 */

extern void check_sprite_status(int h);
extern void check_frame_status(int h, int frame);
extern void flip_it_second(void);

extern void fill_screen(int num);


extern void show_bmp(char name[80], int showdot, int script);
extern void copy_bmp( char name[80]);
extern void fill_hardxy(rect box);
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
