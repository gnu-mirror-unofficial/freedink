/**
 * Emergency message boxes

 * Copyright (C) 2008, 2009, 2014  Sylvain Beucler

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

#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__
#define WIN32_LEAN_AND_MEAN
/* MessageBox */
#include <windows.h>
#else
#  ifdef HAVE_EXECLP
/* fork, waitpid, execlp */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#  endif
#endif

#include "SDL.h"

/**
 * Emergency message box for when even SDL_ShowSimpleMessageBox fails
 */
void msgbox_os(char *msg)
{
#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__

  /* WIN32 API */
  MessageBox(NULL, msg, PACKAGE_NAME, MB_OK);

#else
#  ifdef HAVE_EXECLP

  /* 'xmessage' basic (and ugly) X utility */
  pid_t pid = 0;
  if ((pid = fork()) < 0)
    perror("fork");
  else if (pid == 0)
    {
      /* child */
      /* Don't display xmessage errors, this would be misleading */
      fclose(stdout);
      fclose(stderr);
      if (execlp("xmessage", "xmessage", "-center", "-buttons", "OK:0", msg, NULL) < 0)
	perror("execlp");
      exit(EXIT_FAILURE);
    }
  else
    {
      /* father */
      pid_t child_pid = pid;
      int status = 0;
      waitpid(child_pid, &status, 0);
    }
#  else

  /* Add more OS-specific fallbacks here. */

#  endif
#endif
}


/**
 * Display an error for the user's immediate attention, during
 * initialization (so we can use the SDL window if needed)
 */
void msgbox(char* fmt, ...)
{
  va_list ap;

  char *buf = NULL;
  va_start(ap, fmt);

  if (fmt == NULL)
    fmt = "Unknown error!\n"
      "This means there's an internal error in FreeDink.\n"
      "Please report this bug to " PACKAGE_BUGREPORT " .";
  vasprintf(&buf, fmt, ap);
  va_end(ap);

  /* Display a SDL message box if possible, otherwise fall back to a
     system message box */
  fprintf(stderr, "%s\n", buf);
  SDL_Quit();
  if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PACKAGE_STRING, buf, NULL) < 0)
    msgbox_os(buf);

  free(buf);
}
