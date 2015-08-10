#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gettext.h>
#define _(String) gettext (String)

#include "AppFreeDink.h"
#include "bgm.h"
#include "dinkc_console.h"
#include "game_engine.h"
#include "live_sprites_manager.h"
#include "editor_screen.h"
#include "gfx.h"
#include "input.h"
#include "log.h"
#include "sfx.h"
#include "talk.h"
#include "update_frame.h"

#include "SDL.h"


#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __ANDROID__
#include <string.h> /* strerror */
#include <errno.h>
#endif


/* Text choice selection (e.g. "Load game" in the title screen) */
void freedink_update_mouse_text_choice(int dx, int dy) {
	play.mouse += dy;
}
void freedink_update_mouse(SDL_Event* ev) {
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

/* Global game shortcuts */
static void freedink_input_global_shortcuts(SDL_Event* ev) {
	if (ev->type != SDL_KEYDOWN)
		return;
	if (ev->key.repeat)
		return;
	
	if (ev->key.keysym.scancode == SDL_SCANCODE_RETURN) {
		gfx_toggle_fullscreen();
	} else if (ev->key.keysym.sym == SDLK_q) {
		//shutdown game
		SDL_Event ev;
		ev.type = SDL_QUIT;
		SDL_PushEvent(&ev);
    } else if (ev->key.keysym.sym == SDLK_d) {
		/* Debug mode */
		if (!debug_mode)
			log_debug_on();
		else
			log_debug_off();
    } else if (ev->key.keysym.sym == SDLK_c) {
		if (!console_active)
			dinkc_console_show();
		else
			dinkc_console_hide();
    } else if (ev->key.keysym.sym == SDLK_m) {
		//shutdown music
		StopMidi();
    }
}

	
AppFreeDink::AppFreeDink() {
	description = _("Starts the Dink Smallwood game or one of its D-Mods.");
	splash_path = "Tiles/Splash.bmp";
	dinkini_playmidi = true;
}

/**
 * do work required for every instance of the application: create the
 * window, initialize data
 */
void AppFreeDink::init() {
	/* Notify other apps that FreeDink is playing */
	writeDinksmallwoodIni(true);

	/* Game-specific initialization */
	/* Start with this initialization as it resets structures that are
	   filled in other subsystems initialization */
	game_init(opt_version);

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

AppFreeDink::~AppFreeDink() {
	bgm_quit();
	game_quit();
	writeDinksmallwoodIni(/*playing=*/0);
}


void AppFreeDink::input(SDL_Event* ev) {
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
			   && (ev->key.keysym.mod & KMOD_ALT)) {
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

void AppFreeDink::logic() {
	updateFrame();
}

/**
 * Save where Dink is installed in a .ini file, read by third-party
 * applications like the DinkFrontEnd. Also notify whether Dink is
 * running or not.
 */
void AppFreeDink::writeDinksmallwoodIni(bool playing) {
	if (g_b_no_write_ini)
		return; //fix problem with NT security if -noini is set
	/* TODO: saves it in the user home instead. */
	
#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__
	char windir[MAX_PATH];
	char inifile[MAX_PATH];
	GetWindowsDirectory(windir, MAX_PATH);
	sprintf(inifile, "%s\\dinksmallwood.ini", windir);
	
	unlink(inifile);
	
	FILE* f = fopen(inifile, "w");
	if (f == NULL) {
		log_error("Couldn't write dinksmallwood.ini: %s", strerror(errno));
		return;
	}
	fprintf(f, "[Dink Smallwood Directory Information for the CD to read]\n");
	fprintf(f, "%s", SDL_GetBasePath());
	fprintf(f, "\n");
	if (playing)
		fprintf(f, "TRUE\n");
	else
		fprintf(f, "FALSE\n");
	fclose(f);
#endif
}