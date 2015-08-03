/**
 * FreeDink (not FreeDinkEdit) screen update

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2005, 2006  Dan Walma
 * Copyright (C) 2005, 2007, 2008, 2009, 2010, 2011, 2012, 2014  Sylvain Beucler

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* #include <windows.h> */
/* #include <ddraw.h> */
#include "SDL.h"
#include "SDL2_framerate.h"

#include "dinkvar.h"
#include "game_engine.h"
#include "map.h"
#include "screen.h"
#include "freedink.h"
#include "gfx.h"
/* For printing strings in debug mode */
#include "gfx_fonts.h"
#include "gfx_sprites.h"
#include "gfx_tiles.h"
#include "bgm.h"
#include "log.h"
#include "rect.h"
#include "dinkc_console.h"
#include "input.h"
#include "sfx.h"
#include "talk.h"
#include "status.h"

#include "update_frame.h"

static Uint32 fps_lasttick = 0;
static int frames = 0;
static int fps = 0;
static /*bool*/int turn_on_plane = /*FALSE*/0;
static /*bool*/int plane_process = /*TRUE*/1;

/* Fills 'struct seth_joy sjoy' with the current keyboard and/or
   joystick state */
/* TODO INPUT: group all input checks here, and switch to events
   processing rather than keystate parsing for 'justpressed' events
   (to avoid having to maintain a fake keyboard state in input.c) */
void check_joystick()
{
  /* Clean-up */
  /* Actions */
  {
    int a = ACTION_FIRST;
    for (a = ACTION_FIRST; a < ACTION_LAST; a++)
      sjoy.joybit[a] = 0;
  }
  
  /* Arrows */
  sjoy.right = 0;
  sjoy.left = 0;
  sjoy.up = 0;
  sjoy.down = 0;

  /* Arrows triggered (not maintained pressed) */
  sjoy.rightd = 0;
  sjoy.leftd = 0;
  sjoy.upd = 0;
  sjoy.downd = 0;
	
  if (joystick)
    {
      SDL_JoystickUpdate(); // required even with joystick events enabled
      Sint16 x_pos = 0, y_pos = 0;
      /* SDL counts buttons from 0, not from 1 */
      int i = 0;
      for (i = 0; i < NB_BUTTONS; i++)
	if (SDL_JoystickGetButton(jinfo, i))
	  sjoy.joybit[input_get_button_action(i)] = 1;

      x_pos = SDL_JoystickGetAxis(jinfo, 0);
      y_pos = SDL_JoystickGetAxis(jinfo, 1);
      /* Using thresold=10% (original game) is just enough to get rid
	 of the noise. Let's use 30% instead, otherwise Dink will go
	 diags too easily. */
      {
	Sint16 threshold = 32767 * 30/100;
	if (x_pos < -threshold) sjoy.left  = 1;
	if (x_pos > +threshold) sjoy.right = 1;
	if (y_pos < -threshold) sjoy.up    = 1;
	if (y_pos > +threshold) sjoy.down  = 1;
      }
    }
  
  if (input_getscancodestate(SDL_SCANCODE_LCTRL) || input_getscancodestate(SDL_SCANCODE_RCTRL)) sjoy.joybit[ACTION_ATTACK] = 1;
  if (input_getscancodestate(SDL_SCANCODE_SPACE)) sjoy.joybit[ACTION_TALK] = 1;
  if (input_getscancodestate(SDL_SCANCODE_LSHIFT) || input_getscancodestate(SDL_SCANCODE_RSHIFT)) sjoy.joybit[ACTION_MAGIC] = 1;
  if (input_getscancodestate(SDL_SCANCODE_RETURN)) sjoy.joybit[ACTION_INVENTORY] = 1;
  if (input_getscancodestate(SDL_SCANCODE_ESCAPE)) sjoy.joybit[ACTION_MENU] = 1;
  if (input_getscancodestate(SDL_SCANCODE_6)) sjoy.joybit[ACTION_MAP] = 1;
  if (input_getcharstate(SDLK_m)) sjoy.joybit[ACTION_MAP] = 1;
  if (input_getscancodestate(SDL_SCANCODE_7)) sjoy.joybit[ACTION_BUTTON7] = 1;
  
  {
    int a = ACTION_FIRST;
    for (a = ACTION_FIRST; a < ACTION_LAST; a++)
      {
	sjoy.button[a] = 0;
	if (sjoy.joybit[a] && sjoy.joybitold[a] == 0)
	  /* Button was just pressed */
	  sjoy.button[a] = 1;
	sjoy.joybitold[a] = sjoy.joybit[a];
      }
  }
  
  if (input_getscancodestate(SDL_SCANCODE_RIGHT) || sjoy.joybit[ACTION_RIGHT]) sjoy.right = 1;
  if (input_getscancodestate(SDL_SCANCODE_LEFT)  || sjoy.joybit[ACTION_LEFT])  sjoy.left  = 1;
  if (input_getscancodestate(SDL_SCANCODE_DOWN)  || sjoy.joybit[ACTION_DOWN])  sjoy.down  = 1;
  if (input_getscancodestate(SDL_SCANCODE_UP)    || sjoy.joybit[ACTION_UP])    sjoy.up    = 1;
  
  if (sjoy.right && sjoy.rightold == 0)
    sjoy.rightd = 1;
  sjoy.rightold = sjoy.right;
	
  if (sjoy.left && sjoy.leftold == 0)
    sjoy.leftd = 1;
  sjoy.leftold = sjoy.left;
  
  if (sjoy.up && sjoy.upold == 0)
    sjoy.upd = 1;
  sjoy.upold = sjoy.up;
	
  if (sjoy.down && sjoy.downold == 0)
    sjoy.downd = 1;
  sjoy.downold = sjoy.down;

  
  /* High speed */
  if (input_getscancodestate(SDL_SCANCODE_TAB) == 1)
    {
      game_set_high_speed();
    }
  else if (input_getscancodestate(SDL_SCANCODE_TAB) == 0)
    {
      game_set_normal_speed();
    }
  

  if (wait4b.active)
    {
      //check for dirs
      
      if (sjoy.rightd) wait4b.button = 16;
      if (sjoy.leftd)  wait4b.button = 14;
      if (sjoy.upd)    wait4b.button = 18;
      if (sjoy.downd)  wait4b.button = 12;
      
      sjoy.rightd = 0;
      sjoy.downd = 0;
      sjoy.upd = 0;
      sjoy.leftd = 0;
      
      //check buttons
      {
	int a = ACTION_FIRST;
	for (a = ACTION_FIRST; a < ACTION_LAST; a++)
	  {
	    if (sjoy.button[a])
	      //button was pressed
	      wait4b.button = a;
	    sjoy.button[a] = /*false*/0;
	  }
      }
      
      if (wait4b.button != 0)
	{
	  *presult = wait4b.button;
	  wait4b.active = /*false*/0;
	  run_script(wait4b.script);
	}
    }
}

void updateFrame()
{
  check_joystick();

  /* Refresh frame counter twice per second */
  if ((SDL_GetTicks() - fps_lasttick) > 500)
    {
      fps = frames * (1000/500);
      frames = 0;
      fps_lasttick = SDL_GetTicks();
    }
  frames++;
  

  char msg[100];
  int move_result ;
    
  int max_s;
  int rank[MAX_SPRITES_AT_ONCE];

  abort_this_flip = /*false*/0;

	
  /* This run prepares a screen transition (when Dink runs to the border) */
  /*bool*/int get_frame = /*false*/0;

  /* Screen transition preparation start point */
 trigger_start:
    

  
  lastTickCount = thisTickCount;
  thisTickCount = game_GetTicks();
	
	
      {
	//Use to test at 30 fps
	//Sleep(66);
	
	//redink1 - 'lock the framerate to 83 FPS'... Seth told me to.
	/* Beuc: that doesn't work. Waiting for 1ms is not guaranteed
	   to work accurately, most often computer will wait for
	   delays such as 10ms or 15ms. My tests give framerates of
	   respectively 50 and 60 FPS. */
/* Woe: */
/* 	while (thisTickCount - lastTickCount < 12) */
/* 	  { */
/* 	    Sleep (1); */
/* 	    thisTickCount = GetTickCount (); */
/* 	  } */

/* SDL */
/* 	while (thisTickCount - lastTickCount < 12) */
/* 	  { */
/* 	    SDL_Delay (1); */
/* 	    thisTickCount = SDL_GetTicks (); */
/* 	  } */

	/* SDL_gfx has a more clever algorithm, which accurately sets
	   the framerate to a fixed value. */
	SDL_framerateDelay(&framerate_manager);


	thisTickCount = game_GetTicks();

	fps_final = thisTickCount - lastTickCount;
	
	//redink1 changed to 12-12 from 10-15... maybe work better on faster computers?
	if (fps_final < 12)
	  fps_final = 12;
	if (fps_final > 68)
	  fps_final = 68;
	base_timing = fps_final / 3;
	if (base_timing < 4)
	  base_timing = 4;
	int junk3;
	
	//redink1 added these changes to set Dink's speed correctly, even on fast machines.
	//junk3 = (fps_average / dinkspeed) -  (fps_average / 8);
	if (dinkspeed <= 0)
	  junk3 = 0;
	
	else if (dinkspeed == 1)
	  junk3 = 12;
	
	else if (dinkspeed == 2)
	  junk3 = 6;
	
	else if (dinkspeed == 3)
	  junk3 = 3;
	
	else
	  junk3 = 1;
	
	//redink1... weird bug, only do this for normal mode, as it breaks mouse support
	//if (mode == 2)
	{
	  junk3 *= (base_timing / 4);
	}
	spr[1].speed = junk3;
      }
	
	
	if (showb.active)
	{
		process_show_bmp();
		if (!abort_this_flip)
		  flip_it();
		
		return;
	}
	
	mbase_count++;
	
	if (thisTickCount > mold+100)
	{
		mold = thisTickCount;
		if (bow.active) bow.hitme = /*true*/1;
		if (*pupdate_status == 1) update_status_all();
		
		
		update_sound();
		process_animated_tiles();
	}
	
	
	if (show_inventory)
	{
		process_item();
		return;
	}
	
	
	if (transition_in_progress) 
	  {
	    if (transition()) /* transition not finished */
	      goto flip;
	    else
	      return;
	  }
	
	
	/* Fade to black, etc. */
	if (process_upcycle) up_cycle();
	if (process_warp > 0) process_warp_man();
	if (process_downcycle) CyclePalette();
	
	
	if (plane_process)
	  {
	    max_s = last_sprite_created;
	    screen_rank_game_sprites(rank);
	  }
	else
	  {
	    //not processing planes
	    max_s = MAX_SPRITES_AT_ONCE;
	  }
	
	//Blit from Two, which holds the base scene.
	SDL_BlitSurface(GFX_lpDDSTwo, NULL, GFX_lpDDSBack, NULL);
	
	
	if (stop_entire_game == 1)
	{
		if (talk.active) talk_process();
		
		else
		{
			stop_entire_game = 0;
			
			draw_map_game_background();
			draw_status_all();
			
		}
		goto flip;
		
	}
	
	
	
	
	int j = 0;
	for (; j <= max_s; j++)
	{
		//h  = 1;
		int h = 0;
		if (plane_process)
		  h = rank[j];
		else
		  h = j;
		//Msg( "Ok, rank %d is %d", j,h);
		
		if (h > 0) 
			if (spr[h].active && spr[h].disabled == 0)
			{
				
				//check_sprite_status_full(h);
				
				spr[h].moveman = 0; //init thing that keeps track of moving path	
				spr[h].lpx[0] = spr[h].x;
				spr[h].lpy[0] = spr[h].y; //last known legal cords
				
				spr[h].skiptimer++;
				//inc delay, used by "skip" by all sprites
/* 				box_crap = k[getpic(h)].box; */
				if (spr[h].kill > 0)
				{
					if (spr[h].kill_timer == 0) spr[h].kill_timer = thisTickCount;
					if (spr[h].kill_timer + spr[h].kill < thisTickCount)
					{
						
						spr[h].active = /*false*/0;
						//          Msg("Killing sprite %d.", h);
						
						get_last_sprite();
						if (spr[h].callback > 0) 
						{
							//	Msg("Callback running script %d.", spr[h].script);
							
							run_script(spr[h].callback);
							
							
						}
						
						
					}
					
				}
				
				if (spr[h].timer > 0)
				{
					if (thisTickCount > spr[h].wait)
					{
						spr[h].wait = thisTickCount + spr[h].timer;
						
					}else
					{
						goto animate;
					}
					
				}
				
				
				//brains - predefined bahavior patterns available to any sprite
				
				if (spr[h].notouch) if (thisTickCount > spr[h].notouch_timer) spr[h].notouch = /*false*/0;
				if (get_frame == /*false*/0)
				{
					if (   (spr[h].brain == 1)/* || (spr[h].brain == 9) || (spr[h].brain == 3) */ )
					{
						
						run_through_touch_damage_list(h);
						
						
					}		
					
					if (spr[h].brain == 1)
					{
						if (process_warp == 0)
							human_brain(h);	
					}
					
					if (spr[h].brain == 2) bounce_brain(h);
					if (spr[h].brain == 0) no_brain(h);
					if (spr[h].brain == 3) duck_brain(h);
					if (spr[h].brain == 4) pig_brain(h);
					if (spr[h].brain == 5) one_time_brain(h);
					if (spr[h].brain == 6) repeat_brain(h);
					if (spr[h].brain == 7) one_time_brain_for_real(h);
					if (spr[h].brain == 8) text_brain(h);
					if (spr[h].brain == 9) pill_brain(h);
					if (spr[h].brain == 10) dragon_brain(h);
					if (spr[h].brain == 11) missile_brain(h, /*true*/1);
					if (spr[h].brain == 12) scale_brain(h);
					if (spr[h].brain == 13) mouse_brain(h);
					if (spr[h].brain == 14) button_brain(h);
					if (spr[h].brain == 15) shadow_brain(h);
					if (spr[h].brain == 16) people_brain(h);
					if (spr[h].brain == 17) missile_brain_expire(h);
				} else
				{
					goto past;
				}

animate:
				
				move_result = check_if_move_is_legal(h);
				
				if (flub_mode != -500)
				{
					log_debug("move result is %d", flub_mode);
					move_result = flub_mode;
					flub_mode = -500;
				}
				
				if (spr[h].brain == 1) if (move_result > 100)
				{
					if (pam.sprite[move_result-100].is_warp == 1)
						special_block(move_result - 100);
				}
				
				
				if (spr[h].reverse)
				{
					
					//reverse instructions
					if (spr[h].seq > 0)
					{
						if (spr[h].frame < 1)
						{
							// new anim
							spr[h].pseq = spr[h].seq;
							spr[h].pframe = seq[spr[h].seq].len;
							spr[h].frame = seq[spr[h].seq].len;
							if (spr[h].frame_delay != 0) spr[h].delay = (thisTickCount+ spr[h].frame_delay); else
								spr[h].delay = (thisTickCount + seq[spr[h].seq].delay[seq[spr[h].seq].len]);
						}   else
						{
							// not new anim
							
							//is it time?
							
							if (thisTickCount > spr[h].delay)
							{
								
								
								spr[h].frame--;
								
								
								if (spr[h].frame_delay != 0) spr[h].delay = (thisTickCount + spr[h].frame_delay); else
									
									spr[h].delay = (thisTickCount + seq[spr[h].seq].delay[spr[h].frame]);
								
								spr[h].pseq = spr[h].seq;
								spr[h].pframe = spr[h].frame;
								
								
								if (seq[spr[h].seq].frame[spr[h].frame]  < 2)
								{
									
									spr[h].pseq = spr[h].seq;
									spr[h].pframe = spr[h].frame+1;
									
									spr[h].frame = 0;
									spr[h].seq_orig = spr[h].seq;
									spr[h].seq = 0;
									spr[h].nocontrol = /*false*/0;
									
									
									if (h == 1) if (in_this_base(spr[h].seq_orig, dink_base_push))
										
									{
										
										
										play.push_active = /*false*/0;
										if (play.push_dir == 2) if (sjoy.down) play.push_active = /*true*/1;
										if (play.push_dir == 4) if (sjoy.left) play.push_active = /*true*/1;
										if (play.push_dir == 6) if (sjoy.right) play.push_active = /*true*/1;
										if (play.push_dir == 8) if (sjoy.up) play.push_active = /*true*/1;
										
										
										goto past;
										
									}
								}
								if (spr[h].seq > 0) if (seq[spr[h].seq].special[spr[h].frame] == 1)
								{
									//this sprite can damage others right now!
									//lets run through the list and tag sprites who were hit with their damage
									
									run_through_tag_list(h, spr[h].strength);
									
								}
								
								
								
								
							}
						}
					}
					
					
				} else
				{
					
					if (spr[h].seq > 0) if (spr[h].picfreeze == 0)
					{
						if (spr[h].frame < 1)
						{
							// new anim
							spr[h].pseq = spr[h].seq;
							spr[h].pframe = 1;
							spr[h].frame = 1;
							if (spr[h].frame_delay != 0)
							  spr[h].delay = thisTickCount + spr[h].frame_delay;
							else
							  spr[h].delay = (thisTickCount + seq[spr[h].seq].delay[1]);
						}   else
						{
							// not new anim
							
							//is it time?
							
							if (thisTickCount > spr[h].delay)
							{
								
								
								spr[h].frame++;
								if (spr[h].frame_delay != 0)
								  spr[h].delay = thisTickCount + spr[h].frame_delay;
								else
								  spr[h].delay = (thisTickCount + seq[spr[h].seq].delay[spr[h].frame]);
								
								spr[h].pseq = spr[h].seq;
								spr[h].pframe = spr[h].frame;
								
								if (seq[spr[h].seq].frame[spr[h].frame] == -1)
								{
									spr[h].frame = 1;
									spr[h].pseq = spr[h].seq;
									spr[h].pframe = spr[h].frame;
									if (spr[h].frame_delay != 0) spr[h].delay = thisTickCount + spr[h].frame_delay; else
										
										spr[h].delay = (thisTickCount + seq[spr[h].seq].delay[spr[h].frame]);
									
								}
								
								if (seq[spr[h].seq].frame[spr[h].frame]  < 1)
								{
									
									spr[h].pseq = spr[h].seq;
									spr[h].pframe = spr[h].frame-1;
									
									spr[h].frame = 0;
									spr[h].seq_orig = spr[h].seq;
									spr[h].seq = 0;
									spr[h].nocontrol = /*false*/0;
									
									
									if (h == 1) if (in_this_base(spr[h].seq_orig, dink_base_push))
										
									{
										
										
										play.push_active = /*false*/0;
										if (play.push_dir == 2) if (sjoy.down) play.push_active = /*true*/1;
										if (play.push_dir == 4) if (sjoy.left) play.push_active = /*true*/1;
										if (play.push_dir == 6) if (sjoy.right) play.push_active = /*true*/1;
										if (play.push_dir == 8) if (sjoy.up) play.push_active = /*true*/1;
										
										
										goto past;
										
									}
								}
								if (spr[h].seq > 0) if (seq[spr[h].seq].special[spr[h].frame] == 1)
								{
									//this sprite can damage others right now!
									//lets run through the list and tag sprites who were hit with their damage
									
									run_through_tag_list(h, spr[h].strength);
									
								}
								
								
								
								
							}
						}
					}
					
					
				}

past:
				check_seq_status(spr[h].seq);
				draw_sprite_game(GFX_lpDDSBack, h);
}
} /* for 0->max_s */

 
	if (mode == 0)
	{
		
	  memset(&spr[1], 0, sizeof(spr[1]));
		
		spr[1].speed = 3;
		/* init_mouse(hWndMain); */
		/* g_pMouse->Acquire(); */
		
		spr[1].timer = 0;
		spr[1].brain = 1;
		spr[1].hard = 1;
		spr[1].pseq = 2;
		spr[1].pframe = 1;
		spr[1].seq = 2;
		spr[1].dir = 2;
		spr[1].damage = 0;
		spr[1].strength = 10;
		spr[1].defense = 0;
		spr[1].skip = 0;
		rect_set(&spr[1].alt,0,0,0,0);
		spr[1].base_idle = 10;
		spr[1].base_walk = -1;
		spr[1].size = 100;		 
		spr[1].base_hit = 100;
		spr[1].active = /*TRUE*/1;

		SDL_WarpMouseInWindow(window, spr[1].x, spr[1].y);
		
		int crap2 = add_sprite(0,450,8,0,0);
		
		
		spr[crap2].hard = 1;
		spr[crap2].noclip = 1;
		strcpy(spr[crap2].text, dversion_string);
		
		spr[crap2].damage = -1;
		spr[crap2].owner = 1000;
		
		
		
		int scr = load_script("START",1000, /*true*/1);
		if (locate(scr, "MAIN") == /*false*/0)
		{
			log_error("Can't locate MAIN in script START!");
		}

		run_script(scr);
		set_mode(1);
		
	}
	
	
	
	if (mode == 2)
	{
	  set_mode(3);
	  game_load_map(map.loc[*pplayer_map]);
	  draw_map_game();
	  flife = *plife;
	}
	
	
	/* Screen transition? */
	if (spr[1].active && spr[1].brain == 1) {
	  if (did_player_cross_screen()) {
	    /* let's restart and draw the next screen,
	       did_player_cross_screen->grab_trick() screenshot'd the current one */
	    get_frame = 1;
	    goto trigger_start;
	  }
	}
	
	/* Screen transition */
	if (get_frame)
	{
	  get_frame = 0;
	  transition_in_progress = 1;

	  // GFX
	  {
	    SDL_Rect src, dst;
	    src.x = playl;
	    src.y = 0;
	    src.w = 620 - playl;
	    src.h = 400;
	    dst.x = dst.y = 0;
	    SDL_BlitSurface(GFX_lpDDSBack, &src, GFX_lpDDSTrick2, &dst);
	  }
	  
	  return;
	}
	
	
	
	if (screenlock == 1)
	{
		//Msg("Drawing screenlock.");
		drawscreenlock();
		
	}
	
	
	if (debug_mode)
	  {
	    FONTS_SetTextColor(200, 200, 200);
	    if (mode == 0) strcpy(msg,"");
	    if (mode == 1)
	      {
		int x;
		sprintf(msg,"X is %d y is %d  FPS: %d", spr[1].x, spr[1].y, fps);
		//let's add the key info to it.
		for (x = 0; x < 256; x++)
		  if (input_getcharstate(x))
		    sprintf(msg + strlen(msg), " (Key %i)", x);
	      }
	    if (mode == 3)
	      {
		sprintf(msg, "Sprites: %d  FPS: %d  Plane_process: %d"
			" Moveman X%d X%d: %d Y%d Y%d Map %d",
			last_sprite_created, fps/*_show*/, plane_process,
			spr[1].lpx[0], spr[1].lpy[0], spr[1].moveman,
			spr[1].lpx[1], spr[1].lpy[1], *pplayer_map);
	      }
	    
	    print_text_wrap_debug(msg, 0, 0);
	    if (strlen(last_debug) > 0)
	      {
		//let's also draw this...
		strcpy(msg, last_debug);
		print_text_wrap_debug(msg, 0, 20);
	      }
	  }

        /* Console */
        if (console_active == 1)
        {
	  char* line = dinkc_console_get_cur_line();
	  FONTS_SetTextColor(0, 0, 0);
	  print_text_wrap_debug(line, 20, 380);

	  char retval[20+1];
	  sprintf(retval, "%d", dinkc_console_get_last_retval());
	  FONTS_SetTextColor(255, 0, 0);
	  print_text_wrap_debug(retval, 20, 360);
	}

	int j2 = 0;
	for (; j2 <= max_s; j2++)
	  {
	    int h = 0;
	    if (plane_process)
	      h = rank[j2];
	    else
	      h = j2;
	    if (h > 0 && spr[h].active && spr[h].brain == 8)
	      text_draw(h);
	  }
    
    
	if (talk.active) talk_process();
	
	
	process_callbacks();
	
flip:

	if (!abort_this_flip)
		flip_it(); 
	
	if (turn_on_plane) plane_process = /*TRUE*/1;
	
} /* updateFrame */
