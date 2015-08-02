/**
 * Game-specific processing

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2008, 2009, 2010, 2014  Sylvain Beucler

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

#include <stdlib.h>  /* srand */
#include <time.h>  /* time */
#include <string.h>  /* memset */
#include "game_engine.h"
#include "screen.h" /* hm */
#include "dinkvar.h"  /* hmap, pam */
#include "freedink.h"  /* add_time_to_saved_game */
#include "input.h"
#include "log.h"
#include "sfx.h"
#include "bgm.h"
#include "gfx.h"
#include "gfx_sprites.h"
#include "savegame.h"
#include "meminfo.h"

/* Engine variables directly mapped with DinkC variables */
int *pvision, *plife, *presult, *pspeed, *ptiming, *plifemax,
  *pexper, *pstrength, *pcur_weapon,*pcur_magic, *pdefense,
  *pgold, *pmagic, *plevel, *plast_text, *pmagic_level;
int *pupdate_status, *pmissile_target, *penemy_sprite,
  *pmagic_cost, *pmissle_source;


int flife, fexp, fstrength, fdefense, fgold, fmagic, fmagic_level, flifemax, fraise, last_magic_draw;

int fcur_weapon, fcur_magic;



struct wait_for_button wait4b;



char* dversion_string;

int last_saved_game = 0;
char save_game_info[200] = "Level &level";

time_t time_start;


int smooth_follow = 0;



/** Fadedown / fadeup **/
/* Tell the engine that we're warping */
/* (quick fadedown + black + fadeup) */
int process_warp = 0;
/* Tell the engine that we're fading down */
/* or fading up */
int process_downcycle = 0;
int process_upcycle = 0;
/* When the fadedown must finish: */
unsigned long cycle_clock = 0;
/* What script to resume after fade: */
int cycle_script = 0;

/* Base for Dink's push sequences */
unsigned int dink_base_push = 310;

/* Engine is currently executing a screen's main() - display "Please
   Wait" when loading graphics and postpone other scripts */
/*bool*/int screen_main_is_running = /*false*/0;
static int please_wait_toggle_frame = 7;


static int high_speed = 0;
struct player_info play;

void game_restart()
{
  int mainscript;
  while (kill_last_sprite());
  kill_repeat_sounds_all();
  kill_all_scripts_for_real();
  set_mode(0);
  screenlock = 0;

  /* Reset all game state */
  memset(&play, 0, sizeof(play));

  memset(&hm, 0, sizeof(hm));
  input_set_default_buttons();

  mainscript = load_script("main", 0, /*true*/1);

  locate(mainscript, "main");
  run_script(mainscript);
  //lets attach our vars to the scripts
  attach();
}

/**
 * Fake SDL_GetTicks if the player is in high-speed mode.  Make sure
 * you call it once per frame.
 */
Uint32 game_GetTicks()
{
  static Uint32 last_sdl_ticks = 0;
  static Uint32 high_ticks = 0;

  Uint32 cur_sdl_ticks = SDL_GetTicks();
  /* Work-around incorrect initial value */
  if (last_sdl_ticks == 0)
    last_sdl_ticks = cur_sdl_ticks - 1;
    
  /* If high speed, then count every tick as triple (so add 2x) */
  if (high_speed)
    {
      high_ticks += 2 * (cur_sdl_ticks - last_sdl_ticks);
    }
  
  last_sdl_ticks = cur_sdl_ticks;
  return cur_sdl_ticks + high_ticks;
}

void game_set_high_speed()
{
  if (high_speed == 0)
    {
      SDL_setFramerate(&framerate_manager, 3*FPS);
      high_speed = 1;
    }
}

void game_set_normal_speed()
{
  if (high_speed == 1)
    {
      SDL_setFramerate(&framerate_manager, FPS);
      high_speed = 0;
    }
}


int next_raise(void)
{
  int crap = *plevel;
  int num = ((100 * crap) * crap);
  
  if (num > 99999) num = 99999;
  return(num);
}


/**
 * Add experience - no "did the player really kill this enemy?"
 * checks
 */
void add_exp_force(int num, int source_sprite)
{
  if (num > 0)
    {
      //add experience
      *pexper += num;

      int crap2 = add_sprite(spr[source_sprite].x, spr[source_sprite].y, 8, 0, 0);
      spr[crap2].y -= k[seq[spr[source_sprite].pseq].frame[spr[source_sprite].pframe]].yoffset;
      spr[crap2].x -= k[seq[spr[source_sprite].pseq].frame[spr[source_sprite].pframe]].xoffset;
      spr[crap2].y -= k[seq[spr[source_sprite].pseq].frame[spr[source_sprite].pframe]].box.bottom / 3;
      spr[crap2].x += k[seq[spr[source_sprite].pseq].frame[spr[source_sprite].pframe]].box.right / 5;
      spr[crap2].y -= 30;
      spr[crap2].speed = 1;
      spr[crap2].hard = 1;
      spr[crap2].brain_parm = 5000;
      spr[crap2].my = -1;
      spr[crap2].kill = 1000;
      spr[crap2].dir = 8;
      spr[crap2].damage = num;
      
      if (*pexper > 99999)
	*pexper = 99999;
    }
}

void add_exp(int num, int killed_sprite)
{
  if (spr[killed_sprite].last_hit != 1)
    return;
  
  add_exp_force(num, killed_sprite);
}


/**
 * Resurrect sprites that were temporarily disabled
 * (editor_type(6/7/8))
 */
void fix_dead_sprites()
{
  int i;

  for (i = 1; i < 100; i++)
    {
      int type = play.spmap[*pplayer_map].type[i];

      // Resurrect sprites after 5mn
      if (type == 6)
	{
	  if  ((thisTickCount > (play.spmap[*pplayer_map].last_time + 300000))
	       || (thisTickCount + 400000 < play.spmap[*pplayer_map].last_time + 300000))
	    {
	      //this sprite can come back online now
	      play.spmap[*pplayer_map].type[i] = 0;
	    }
	}

      // Resurrect sprites after 3mn
      if (type == 7)
	{
	  if (thisTickCount > (play.spmap[*pplayer_map].last_time + 180000))
	    {
	      //this sprite can come back online now
	      play.spmap[*pplayer_map].type[i] = 0;
	    }
	}

      // Resurrect sprites after 1mn
      if (type == 8)
	{
	  if (thisTickCount > (play.spmap[*pplayer_map].last_time + 60000))
	    {
	      //this sprite can come back online now
	      play.spmap[*pplayer_map].type[i] = 0;
	    }
	}
    }
}


/**
 * Load 1 screen from map.dat, which contains all 768 game screens
 */
int game_load_map(int num)
{
  if (load_map_to(current_map, num, &pam) < 0)
    return -1;
  
  spr[1].move_active = 0;
  if (dversion >= 108)
    spr[1].move_nohard = 0;
  spr[1].freeze = 0;
  screenlock = 0;

  fill_whole_hard();

  fix_dead_sprites();
  check_midi();

  return 0;
}


/**
 * Remember last time we entered this screen (so we can disable
 * sprites for some minutes, e.g. monsters)
 */
void update_screen_time()
{
  //Msg("Cur time is %d", play.spmap[*pplayer_map].last_time);
  //Msg("Map is %d..", *pplayer_map);
  play.spmap[*pplayer_map].last_time = thisTickCount;
  //Msg("Time was saved as %d", play.spmap[*pplayer_map].last_time);
}


void update_play_changes( void )
{
  int j;
        for (j = 1; j < 100; j++)
        {
                if (pam.sprite[j].active)
                        if (play.spmap[*pplayer_map].type[j] != 0)
                        {
                                //lets make some changes, player has extra info
                                if (play.spmap[*pplayer_map].type[j] == 1)
                                {
                                        pam.sprite[j].active = 0;

                                }

                                if (play.spmap[*pplayer_map].type[j] == 2)
                                {
                                        pam.sprite[j].type = 1;
                    pam.sprite[j].hard = 1;
                                }
                                if (play.spmap[*pplayer_map].type[j] == 3)
                                {

                                        //              Msg("Changing sprite %d", j);
                                        pam.sprite[j].type = 0;
                                        pam.sprite[j].hard = 1;

                                }

                                if (play.spmap[*pplayer_map].type[j] == 4)
                                {
                                        pam.sprite[j].type = 1;
                    pam.sprite[j].hard = 0;
                                }

                                if (play.spmap[*pplayer_map].type[j] == 5)
                                {
                                        pam.sprite[j].type = 0;
                    pam.sprite[j].hard = 0;
                                }

                                if (play.spmap[*pplayer_map].type[j] == 6)
                                {
                                        pam.sprite[j].active = 0;

                                }
                                if (play.spmap[*pplayer_map].type[j] == 7)
                                {
                                        pam.sprite[j].active = 0;

                                }
                                if (play.spmap[*pplayer_map].type[j] == 8)
                                {
                                        pam.sprite[j].active = 0;

                                }

                                pam.sprite[j].seq = play.spmap[*pplayer_map].seq[j];
                                pam.sprite[j].frame = play.spmap[*pplayer_map].frame[j];
                                strcpy(pam.sprite[j].script, "");


                        }


        }
}


/**
 * Load screen sprites: draw sprites on background buffer, ordered by queue
 * and configure the others (sounds, scripts, etc.).
 *
 * Also cf. game_place_sprites_background(...) and editor's
 * place_sprites(...).
 */
void game_place_sprites()
{
  update_play_changes();
  
  int rank[MAX_SPRITES_EDITOR];
  screen_rank_map_sprites(rank);
  
  int r1 = 0;  
  for (; r1 < MAX_SPRITES_EDITOR && rank[r1] > 0; r1++)
    {
      //Msg("Ok, rank[%d] is %d.",oo,rank[oo]);
      int j = rank[r1];
      
      if (pam.sprite[j].active == 1
	  && (pam.sprite[j].vision == 0 || pam.sprite[j].vision == *pvision))
	{
	  check_seq_status(pam.sprite[j].seq);
	  
	  //we have instructions to make a sprite
	  if (pam.sprite[j].type == 0 || pam.sprite[j].type == 2)
	    {
	      //make it part of the background (much faster)
	      int sprite = add_sprite_dumb(pam.sprite[j].x,pam.sprite[j].y, 0,
					   pam.sprite[j].seq,pam.sprite[j].frame,
					   pam.sprite[j].size);

	      spr[sprite].hard = pam.sprite[j].hard;
	      spr[sprite].sp_index = j;
	      rect_copy(&spr[sprite].alt , &pam.sprite[j].alt);
	      
	      check_sprite_status_full(sprite);

	      if (pam.sprite[j].type == 0)
		draw_sprite_game(GFX_lpDDSTwo, sprite);
	      
	      if (spr[sprite].hard == 0)
		{
		  /*if (pam.sprite[j].is_warp == 0)
		    add_hardness(sprite, 1); else */
		  add_hardness(sprite, 100 + j);
		}
	      spr[sprite].active = 0;
	    }

	  if (pam.sprite[j].type == 1)
	    {
	      //make it a living sprite
	      int sprite = add_sprite_dumb(pam.sprite[j].x,pam.sprite[j].y, 0,
					   pam.sprite[j].seq,pam.sprite[j].frame,
					   pam.sprite[j].size);
	      
	      spr[sprite].hard = pam.sprite[j].hard;
	      
	      //assign addition parms to the new sprite
	      spr[sprite].sp_index = j;
	      
	      spr[sprite].brain = pam.sprite[j].brain;
	      spr[sprite].speed = pam.sprite[j].speed;
	      spr[sprite].base_walk = pam.sprite[j].base_walk;
	      spr[sprite].base_idle = pam.sprite[j].base_idle;
	      spr[sprite].base_attack = pam.sprite[j].base_attack;
	      spr[sprite].base_hit = pam.sprite[j].base_hit;
	      spr[sprite].hard = pam.sprite[j].hard;
	      spr[sprite].timer = pam.sprite[j].timer;
	      spr[sprite].que = pam.sprite[j].que;
	      
	      
	      spr[sprite].sp_index = j;
	      
	      rect_copy(&spr[sprite].alt , &pam.sprite[j].alt);
	      
	      spr[sprite].base_die = pam.sprite[j].base_die;
	      spr[sprite].strength = pam.sprite[j].strength;
	      spr[sprite].defense = pam.sprite[j].defense;
	      spr[sprite].gold = pam.sprite[j].gold;
	      spr[sprite].exp = pam.sprite[j].exp;
	      spr[sprite].nohit = pam.sprite[j].nohit;
	      spr[sprite].touch_damage = pam.sprite[j].touch_damage;
	      spr[sprite].hitpoints = pam.sprite[j].hitpoints;
	      spr[sprite].sound = pam.sprite[j].sound;
	      check_sprite_status_full(sprite);
	      if (pam.sprite[j].is_warp == 0 && spr[sprite].sound != 0)
		{
		  //make looping sound
		  log_debug("making sound with sprite %d..", sprite);
		  SoundPlayEffect( spr[sprite].sound,22050, 0,sprite, 1);
		}
	      if (spr[sprite].brain == 3)
		{
		  // Duck
		  check_seq_status(21);
		  check_seq_status(23);
		  check_seq_status(24);
		  check_seq_status(26);
		  check_seq_status(27);
		  check_seq_status(29);
		  // Headless duck
		  check_seq_status(111);
		  check_seq_status(113);
		  check_seq_status(117);
		  check_seq_status(119);
		  // Duck head
		  check_seq_status(121);
		  check_seq_status(123);
		  check_seq_status(127);
		  check_seq_status(129);
		}
	      
	      if (spr[sprite].hard == 0)
		{
		  /*  if (pam.sprite[j].is_warp == 0)
			add_hardness(sprite, 1);
		      else */
		  add_hardness(sprite, 100+j);
		}
	      
	      //does it need a script loaded?
	      if (strlen(pam.sprite[j].script) > 1)
		{
		  spr[sprite].script = load_script(pam.sprite[j].script, sprite, /*true*/1);
		}
	    }
	  //Msg("I just made sprite %d because rank[%d] told me to..",sprite,j);
					       }
    }
}

/**
 * Draw background sprites and background (not looking at
 * non-background sprites), ordered by queue.
 * 
 * Also cf. game_place_sprites(...) and editor's place_sprites(...).
 */
void game_place_sprites_background()
{
  int rank[MAX_SPRITES_EDITOR];
  screen_rank_map_sprites(rank);

  int r1 = 0;
  for (; r1 < MAX_SPRITES_EDITOR && rank[r1] > 0; r1++)
    {
      //Msg("Ok, rank[%d] is %d.",oo,rank[oo]);
      int j = rank[r1];
      
      if (pam.sprite[j].active == 1
	  && (pam.sprite[j].vision == 0 || pam.sprite[j].vision == *pvision))
	{
	  if (pam.sprite[j].type == 0)
	    {
	      //we have instructions to make a sprite
	      check_seq_status(pam.sprite[j].seq);
	      
	      //make it part of the background (much faster)
	      int sprite = add_sprite_dumb(pam.sprite[j].x,pam.sprite[j].y, 0,
					   pam.sprite[j].seq,pam.sprite[j].frame,
					   pam.sprite[j].size);

	      check_sprite_status_full(sprite);
	      draw_sprite_game(GFX_lpDDSTwo, sprite);
	      spr[sprite].active = 0;
	    }
	}
    }
}

void fill_back_sprites()
{
  int rank[MAX_SPRITES_EDITOR];
  screen_rank_map_sprites(rank);

  int r1 = 0;
  for (; r1 < MAX_SPRITES_EDITOR && rank[r1] > 0; r1++)
    {
      //Msg("Ok, rank[%d] is %d.",oo,rank[oo]);
      int j = rank[r1];

      if (pam.sprite[j].active == 1
	  && (pam.sprite[j].vision == 0 || pam.sprite[j].vision == *pvision))
	{



	  if (pam.sprite[j].type != 1 && pam.sprite[j].hard == 0)
	    {
	      //make it part of the background (much faster)
	      int sprite = add_sprite_dumb(pam.sprite[j].x,pam.sprite[j].y, 0,
					   pam.sprite[j].seq,pam.sprite[j].frame,
					   pam.sprite[j].size);

	      spr[sprite].hard = pam.sprite[j].hard;
	      spr[sprite].sp_index = j;
	      rect_copy(&spr[sprite].alt , &pam.sprite[j].alt);

	      check_sprite_status_full(sprite);




	      if (spr[sprite].hard == 0)
		{
		  /*if (pam.sprite[j].is_warp == 0)
		    add_hardness(sprite, 1); else */
		  add_hardness(sprite,100+j);
		}
	      spr[sprite].active = 0;
	    }
	}
    }
}


/* Draw the background from tiles */
void draw_map_game(void)
{
  *pvision = 0;
                
  while (kill_last_sprite());
  kill_repeat_sounds();
  kill_all_scripts();

  gfx_tiles_draw_screen();
                
  if (strlen(pam.script) > 1)
    {
      int script_id = load_script(pam.script,0, /*true*/1);
                        
      if (script_id > 0) 
	{
	  locate(script_id, "main");
	  screen_main_is_running = /*true*/1;
	  run_script(script_id);
	  screen_main_is_running = /*false*/0;
	}
    }

  // lets add the sprites hardness to the real hardness, adding it's
  // own uniqueness to our collective.
  game_place_sprites();
  
  thisTickCount = game_GetTicks();
                
  // Run active sprites' scripts
  init_scripts();

  // Display some memory stats after loading a screen
  if (debug_mode) {
    meminfo_log_mallinfo();
    gfx_log_meminfo();
    sfx_log_meminfo();
  }
}
        
/* It's used at: freedink.cpp:restoreAll(), DinkC's draw_background(),
   stop_entire_game(). What's the difference with draw_map_game()?? */
void draw_map_game_background(void)
{
  gfx_tiles_draw_screen();
  game_place_sprites_background();
}



/**
 * Display a flashing "Please Wait" anim directly on the screen, just
 * before switching to a screen that requires loading new graphics
 * from the disk.
 */
static void draw_wait()
{
  if (screen_main_is_running) {
    if (seq[423].frame[please_wait_toggle_frame] != 0)
      {
	SDL_Rect dst = { 232, 0, -1, -1 };
	SDL_BlitSurface(GFX_k[seq[423].frame[please_wait_toggle_frame]].k, NULL,
			GFX_lpDDSBack, &dst);
	flip_it();
      }
    if (please_wait_toggle_frame == 7)
      please_wait_toggle_frame = 8;
    else
      please_wait_toggle_frame = 7;
  }
}


void game_init()
{
  /* Clean the game state structure - done by C++ but not
     automatically done by C, and this causes errors. TODO: fix the
     errors properly instead of using this dirty trick. */
  memset(&play, 0, sizeof(play));

  gfx_sprites_loading_listener = draw_wait;
  
  if (dversion >= 108)
    dversion_string = "v1.08 FreeDink";
  else
    dversion_string = "v1.07 FreeDink";

  srand((unsigned)time(NULL));

  dinkc_init();
}

void game_quit()
{
  kill_all_scripts_for_real();

  int i = 0;
  for (i = 1; i < MAX_SPRITES_AT_ONCE; i++)
    {
      if (spr[i].custom != NULL)
	dinkc_sp_custom_free(spr[i].custom);
      spr[i].custom = NULL;
    }

  dinkc_quit();

  if (last_saved_game > 0)
    {
      log_info("Modifying saved game.");

      if (!add_time_to_saved_game(last_saved_game))
	log_error("Error modifying saved game.");
      last_saved_game = 0;
    }
}
