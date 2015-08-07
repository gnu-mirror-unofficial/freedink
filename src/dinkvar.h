/**
 * Header for code common to FreeDink and FreeDinkedit

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

#ifndef _DINKVAR_H
#define _DINKVAR_H

#include "SDL.h"
#include "rect.h"

extern int base_timing;
extern int dinkspeed;
extern int flife;
extern int flub_mode;
extern int show_inventory;

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
extern /*bool*/int abort_this_flip;
extern int push_active;
extern int move_screen;
extern int move_counter;
extern /*bool*/int transition_in_progress;


extern int *pplayer_map;

/* Startup */
extern void pre_figure_out(char* line);
extern void figure_out(char* line);

extern /*bool*/int dinkedit;
extern int draw_screen_tiny;
extern int cur_map;
/*bool*/int get_box (int h, rect * box_crap, rect * box_real);

extern void check_seq_status(int h);
extern void draw_sprite_game(SDL_Surface *GFX_lpdest, int h);

extern void show_bmp(char name[80], int showdot, int script);
extern void copy_bmp( char name[80]);
extern int hurt_thing(int h, int damage, int special);
extern void check_sprite_status_full(int sprite_no);

/*
 * Editor
 */

extern void check_sprite_status(int h);
extern void check_frame_status(int h, int frame);

#endif
