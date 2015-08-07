/**
 * System initialization, common to FreeDink and FreeDinkEdit

 * Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015  Sylvain Beucler

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
#include <math.h>
#include <getopt.h>
#include <unistd.h> /* chdir */
#include <xalloc.h>

#include <locale.h>
#include "gettext.h"
#define _(String) gettext (String)

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL2_framerate.h"
#include "game_engine.h"
#include "live_screen.h"
#include "dinkini.h"
#include "dinkvar.h"
#include "EditorMap.h"
#include "hardness_tiles.h"
#include "gfx.h"
#include "gfx_fonts.h"
#include "gfx_palette.h"
#include "gfx_sprites.h"
#include "gfx_tiles.h"
#include "fastfile.h"
#include "sfx.h"
#include "bgm.h"
#include "input.h"
#include "io_util.h"
#include "paths.h"
#include "log.h"
#include "app.h"
#include "msgbox.h"

#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __ANDROID__
#include <string.h> /* strerror */
#include <errno.h>
#endif

static int g_b_no_write_ini = 0; // -noini passed to command line?
static char* init_error_msg = NULL;

// TODO: move me to game_engine.c (and -7 as a game-specific CLI option)
int dversion = 108;
bool dinkedit = false;

FPSmanager framerate_manager;

void init_set_error_msg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vasprintf(&init_error_msg, fmt, ap);
  va_end(ap);
}

/**
 * Prints the version on the standard ouput. Based on the homonymous
 * function from ratpoison
 */
void
print_version ()
{
  printf("%s %s\n", PACKAGE_NAME, VERSION);
  printf("FreeDink is free software, and you are welcome to redistribute it\n");
  printf("under certain conditions; see the GNU GPL for details.\n");
  printf("http://gnu.org/licenses/gpl.html\n");
  printf("There is NO WARRANTY, to the extent permitted by law.\n");
  exit(EXIT_SUCCESS);
}


/**
 * Prints the version on the standard ouput. Based on the homonymous
 * function from ratpoison
 */
void
print_help (int argc, char *argv[])
{
  printf(_("Usage: %s [OPTIONS]...\n"), argv[0]);
  if (!dinkedit)
    printf(_("Starts the Dink Smallwood game or one of its D-Mods."));
  else
    printf(_("Edit the Dink Smallwood game or one of its D-Mods."));
  printf("\n");
  /* TODO: Display the default configuration here */
  printf(_("  -h, --help            Display this help screen\n"));
  printf(_("  -v, --version         Display the version\n"));
  printf("\n");
  printf(_("  -g, --game <dir>      Specify a DMod directory\n"));
  printf(_("  -r, --refdir <dir>    Specify base directory for dink/graphics, D-Mods, etc.\n"));
  printf("\n");
  printf(_("  -d, --debug           Explain what is being done\n"));
  printf(_("  -i, --noini           Do not attempt to write dinksmallwood.ini\n"));
  printf(_("  -j, --nojoy           Do not attempt to use joystick\n"));
  printf(_("  -s, --nosound         Do not play sound\n"));
  printf(_("  -t, --truecolor       Allow more colours (for recent D-Mod graphics)\n"));
  printf(_("  -w, --window          Use windowed mode instead of screen mode\n"));
  printf(_("  -7, --v1.07           Enable v1.07 compatibility mode\n"));
  printf("\n");

  /* printf ("Type 'info freedink' for more information\n"); */

  /* TRANSLATORS: The placeholder indicates the bug-reporting address
     for this package.  Please add _another line_ saying "Report
     translation bugs to <...>\n" with the address for translation
     bugs (typically your translation team's web or email
     address).  */
  printf(_("Report bugs to %s.\n"), PACKAGE_BUGREPORT);

  exit(EXIT_SUCCESS);
}



/*
* Release all objects we use
*/
void app_quit()
{
  SDL_Event ev;
  ev.type = SDL_QUIT;
  SDL_PushEvent(&ev);

  log_path(/*playing=*/0);

  sfx_quit();

  FastFileFini();

  dinkini_quit();

  input_quit();

  gfx_quit();

  //SDL_QuitSubSystem(SDL_INIT_EVENTTHREAD);
  SDL_QuitSubSystem(SDL_INIT_TIMER);

  SDL_Quit();

  if (init_error_msg != NULL)
    free(init_error_msg);

  paths_quit();
}

/**
 * This function is called if the initialization function fails
 */
int initFail(char *message)
{
  msgbox(message);
  return EXIT_FAILURE; /* used when "return initFail(...);" */
}


/**
 * Check the command line arguments and initialize the required global
 * variables
 */
static int check_arg(int argc, char *argv[])
{
  int c;
  char *refdir_opt = NULL;
  char *dmoddir_opt = NULL;
  int debug_p = 0;

  /* Options '-debug', '-game', '-noini', '-nojoy', '-nosound' and
     '-window' (with one dash '-' only) are required to maintain
     backward compatibility with the original game */
  struct option long_options[] =
    {
      {"debug",     no_argument,       NULL, 'd'},
      {"refdir",    required_argument, NULL, 'r'},
      {"game",      required_argument, NULL, 'g'},
      {"help",      no_argument,       NULL, 'h'},
      {"noini",     no_argument,       NULL, 'i'},
      {"nojoy",     no_argument,       NULL, 'j'},
      {"nosound",   no_argument,       NULL, 's'},
      {"version",   no_argument,       NULL, 'v'},
      {"window",    no_argument,       NULL, 'w'},
      {"v1.07",     no_argument,       NULL, '7'},
      {"truecolor", no_argument,       NULL, 't'},
      {"nomovie"  , no_argument,       NULL, ','},
      {0, 0, 0, 0}
    };

  char short_options[] = "dr:g:hijsvw7t";

  /* Loop through each argument */
  while ((c = getopt_long_only (argc, argv, short_options, long_options, NULL)) != EOF)
    {
      switch (c) {
      case 'd':
	debug_p = 1;
        /* Enable early debugging, before we can locate DEBUG.txt */
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
	break;
      case 'r':
	refdir_opt = strdup(optarg);
	break;
      case 'g':
	dmoddir_opt = strdup(optarg);
	break;
      case 'h':
	print_help(argc, argv);
	break;
      case 'j':
	joystick = 0;
	break;
      case 'i':
	g_b_no_write_ini = 1;
	break;
      case 's':
	sound_on = 0;
	break;
      case 'v':
	print_version();
	break;
      case 'w':
	windowed = 1;
	break;
      case '7':
	dversion = 107;
	break;
      case 't':
	truecolor = 1;
	break;
      case ',':
        printf(_("Note: -nomovie is accepted for compatiblity, but has no effect.\n"));
	break;
      default:
	exit(EXIT_FAILURE);
      }
    }

  if (optind < argc) {
    fprintf(stderr, "Invalid additional argument: ");
    while (optind < argc)
      fprintf(stderr, "%s ", argv[optind++]);
    printf(" (did you forget '--game'?)\n");
    exit(EXIT_FAILURE);
  }

  log_init();

  paths_init(argv[0], refdir_opt, dmoddir_opt);

  free(refdir_opt);
  free(dmoddir_opt);

  if (debug_p == 1)
    {
      /* Remove DEBUG.TXT when starting Dink (but not when toggling debug) */
      char* fullpath = paths_dmodfile("DEBUG.TXT");
      remove(fullpath);
      free(fullpath);

      log_debug_on();
    }

  log_info("Game directory is '%s'.", paths_getdmoddir());
  return 1;
}


/**
 * What to do if running out of memory
 */
void xalloc_die (void) {
  fprintf(stderr, "Memory exhausted!");
  app_quit();
  abort();
}

void app_loop(void (*input_hook)(SDL_Event* ev), void (*logic_hook)()) {
  /* Main loop */
  int run = 1;
  while(run)
    {
      /* Controller: dispatch events */
      SDL_Event event;
      SDL_Event* ev = &event;
      input_reset();
      while (SDL_PollEvent(ev))
	{
	  switch(ev->type)
	    {
	    case SDL_QUIT:
	      run = 0;
	      break;
	    default:
	      input_hook(ev);
	      break;
	    }
	}

      /* Main app logic */
      logic_hook();

      /* Clean-up finished sounds: normally this is done by
	 SDL_mixer but since we're using effects tricks to
	 stream&resample sounds, we need to do this manually. */
      sfx_cleanup_finished_channels();
    }
}

/**
 * chdir to ease locating resources
 */
static void app_chdir() {
#ifdef __ANDROID__
  /* SD Card - SDL_AndroidGetExternalStoragePath()
     == /storage/sdcard0/Android/data/org.freedink/files */
  log_info("Android external storage: %s\n", SDL_AndroidGetExternalStoragePath());
  if (SDL_AndroidGetExternalStoragePath() == NULL)
    log_error("Could not get external storage path '%s': %s'",
	      SDL_AndroidGetExternalStoragePath(),
	      SDL_GetError());
  int state = SDL_AndroidGetExternalStorageState();
  log_info("- read : %s\n", (state&SDL_ANDROID_EXTERNAL_STORAGE_READ) ? "yes":"no");
  log_info("- write: %s\n", (state&SDL_ANDROID_EXTERNAL_STORAGE_WRITE) ? "yes":"no");

  /* SDL_AndroidGetInternalStoragePath() == /data/data/org.freedink/files/ */
  log_info("Android internal storage: %s\n", SDL_AndroidGetInternalStoragePath());

  if (chdir(SDL_AndroidGetExternalStoragePath()) < 0)
    log_error("Could not chdir to '%s': %s'",
	      SDL_AndroidGetExternalStoragePath(),
	      strerror(errno));
#elif defined _WIN32 || defined __WIN32__ || defined __CYGWIN__
  /* .exe's directory */
  log_info("Woe exe dir: %s\n", SDL_GetBasePath());
  if (chdir(SDL_GetBasePath()) < 0)
    log_error("Could not chdir to '%s': %s'",
	      SDL_GetBasePath(),
	      strerror(errno));
#endif
}

/* freedink and freedinkedit's common init procedure. This procedure
   will also initialize each subsystem as needed (eg InitSound) */
int app_start(int argc, char *argv[],
	      char* splash_path,
	      void(*init_hook)(),
	      void(*input_hook)(SDL_Event* ev),
	      void(*logic_hook)(),
	      void(*quit_hook)())
{
  /* chdir to resource paths under woe&android */
  app_chdir();

  /* Reset debug levels */
  log_debug_off();

  /** i18n **/
  /* Only using LC_MESSAGES because LC_CTYPE (part of LC_ALL) may
     bring locale-specific behavior in the DinkC parsers. If that's a
     problem we may use some gnulib modules
     (cf. (gettext.info.gz)Triggering) */
  /* Ex. with scanf("%f"...):
     LANG=C            1.1 -> 1.100000
     LANG=C            1,1 -> 1.000000
     LANG=fr_FR.UTF-8  1.1 -> 1,000000
     LANG=fr_FR.UTF-8  1,1 -> 1,100000 */
  /* setlocale (LC_ALL, ""); */
  setlocale(LC_MESSAGES, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  bindtextdomain(PACKAGE "-gnulib", LOCALEDIR);
  textdomain(PACKAGE);

  /* SDL can display messages in either ASCII or UTF-8. Thus we need
     gettext to output translations in UTF-8. */
  /* That's a problem for console messages in locales that are not
     UTF-8-encoded. If this is really a problem, we'll have to perform
     some character conversion directly. We can't create a separate
     message catalog for those, because several console messages are
     also displayed on the SDL screen and need a separate conversion
     anyway. */
  bind_textdomain_codeset(PACKAGE, "UTF-8");


  if (!check_arg(argc, argv))
    return EXIT_FAILURE;

  /* Same for this D-Mod's .mo (after options are parsed) */
  char* dmod_localedir = paths_dmodfile("l10n");
  bindtextdomain(paths_getdmodname(), dmod_localedir);
  bind_textdomain_codeset(paths_getdmodname(), "UTF-8");
  free(dmod_localedir);


  /* SDL */
  /* Init timer subsystem */
  if (SDL_Init(SDL_INIT_TIMER) == -1) {
    init_set_error_msg("Timer initialization error: %s\n", SDL_GetError());
    return initFail(init_error_msg);
  }

  /* Quits in case we couldn't do it properly first (i.e. attempt to
     avoid stucking the user in 640x480 when crashing) */
  atexit(app_quit);

  /* GFX */
  if (gfx_init(windowed ? GFX_WINDOWED : GFX_FULLSCREEN,
	       splash_path) < 0)
    return initFail(init_error_msg);

  /* Joystick */
  input_init();

  /* SFX & BGM */
  sfx_init();

  SDL_initFramerate(&framerate_manager);
  /* The official v1.08 .exe runs 50-60 FPS in practice, despite the
     documented intent of running 83 FPS (or 12ms delay). */
  /* SDL_setFramerate(manager, 83); */
  SDL_setFramerate(&framerate_manager, FPS);

  dinkini_init();

  live_screen_init();

  //dinks normal walk
  log_info("Loading batch...");
  bool playmidi = !dinkedit;
  load_batch(playmidi);
  log_info(" done!");

  log_info("Loading hard...");
  load_hard();
  log_info(" done!");

  log_info("World data....");
  g_map.load();
  log_info(" done!");

  init_hook();

  app_loop(input_hook, logic_hook);

  quit_hook();

  app_quit();

  return EXIT_SUCCESS;
}

/**
 * Save where Dink is installed in a .ini file, read by third-party
 * applications like the DinkFrontEnd. Also notify whether Dink is
 * running or not.
 */
void log_path(/*bool*/int playing)
{
  if (g_b_no_write_ini)
    return; //fix problem with NT security if -noini is set
  /* TODO: saves it in the user home instead. Think about where to
     cleanly store additional DMods. */

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
