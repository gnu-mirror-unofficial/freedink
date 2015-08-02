/**
 * Game main loop

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2012, 2014  Sylvain Beucler

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

#include "app.h"
#include "bgm.h"
#include "dinkc_console.h"
#include "dinkvar.h"
#include "game_engine.h"
#include "gfx.h"
#include "input.h"
#include "log.h"
#include "screen.h"
#include "sfx.h"
#include "talk.h"
#include "update_frame.h"

#include "SDL.h"

/* Text choice selection (e.g. "Load game" in the title screen) */
void freedink_update_mouse_text_choice(int dx, int dy) {
  play.mouse += dy;
}
void freedink_update_mouse(SDL_Event* ev)
{
  if (ev->type != SDL_MOUSEMOTION)
    return;

  /* Players controls mouse */
  if ((mode == 1 || keep_mouse)
      && (spr[1].active == 1 && spr[1].brain == 13))
    {
      spr[1].x = ev->motion.x;
      spr[1].y = ev->motion.y;
      
      /* Clip the cursor to our client area */
      if (spr[1].x > (640-1)) spr[1].x = 640-1;
      if (spr[1].y > (480-1)) spr[1].y = 480-1;
      if (spr[1].x < 0) spr[1].x = 0;
      if (spr[1].y < 0) spr[1].y = 0;
    }

  if (talk.active)
    freedink_update_mouse_text_choice(ev->motion.xrel, ev->motion.yrel);
}

/**
 * doInit - do work required for every instance of the application:
 *                create the window, initialize data
 */
static void freedink_init()
{
  /* Notify other apps that FreeDink is playing */
  log_path(/*true*/1);

  /* Game-specific initialization */
  /* Start with this initialization as it resets structures that are
     filled in other subsystems initialization */
  game_init();

  if (sound_on)
    bgm_init();

  //Activate dink, but don't really turn him on
  //spr[1].active = TRUE;
  spr[1].timer = 33;
	
  // ** SETUP **
  last_sprite_created = 1;
  set_mode(0);

  //lets run our init script
  int script = load_script("main", 0, /*true*/1);
  locate(script, "main");
  run_script(script);

  /* lets attach our vars to the scripts, they must be declared in the
     main.c DinkC script */
  attach();
}

/* Global game shortcuts */
static void freedink_input_global_shortcuts(SDL_Event* ev) {
  if (ev->type != SDL_KEYDOWN)
    return;
  if (ev->key.repeat)
    return;

  if (ev->key.keysym.scancode == SDL_SCANCODE_RETURN)
    {
      gfx_toggle_fullscreen();
    }
  else if (ev->key.keysym.sym == SDLK_q)
    {
      //shutdown game
      SDL_Event ev;
      ev.type = SDL_QUIT;
      SDL_PushEvent(&ev);
    }
  else if (ev->key.keysym.sym == SDLK_d)
    {
      /* Debug mode */
      if (!debug_mode)
	log_debug_on();
      else
	log_debug_off();
    }
  else if (ev->key.keysym.sym == SDLK_c)
    {
      if (!console_active)
	dinkc_console_show();
      else
	dinkc_console_hide();
    }
  else if (ev->key.keysym.sym == SDLK_m)
    {
      //shutdown music
      StopMidi();
    }
}

static void freedink_input(SDL_Event* ev) {
  // Show IME (virtual keyboard) on Android
  if (ev->type == SDL_KEYDOWN && ev->key.keysym.scancode == SDL_SCANCODE_MENU) {
    if (!SDL_IsScreenKeyboardShown(window))
      SDL_StartTextInput();
    else
      SDL_StopTextInput();
  }

  if (ev->type == SDL_KEYUP
      && input_getscancodestate(ev->key.keysym.scancode) == SDL_PRESSED) {
    // always tell the game when the player releases a key
    input_update(ev);
  } else if ((ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP)
      && ev->key.keysym.mod & KMOD_ALT) {
    freedink_input_global_shortcuts(ev);
  } else if (console_active) {
    dinkc_console_process_key(ev);
  } else {
    // forward all events to the game
    input_update(ev);
  }

  // Process mouse even in console mode
  freedink_update_mouse(ev);
}

static void freedink_quit() {
  game_quit();
  bgm_quit();
}

/**
 * Bootstrap
 */
int main(int argc, char* argv[])
{
  return app_start(argc, argv,
		   "Tiles/Splash.bmp",
		   freedink_init,
		   freedink_input,
		   updateFrame,
		   freedink_quit);
}
