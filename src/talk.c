/**
 * Dialog choices

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
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

#include "talk.h"

#include "SDL.h"
#include "log.h"
#include "game_engine.h" /* play */
#include "dinkvar.h" /* check_seq_status */
#include "gfx.h" /* GFX_lpDDSBack */
#include "gfx_fonts.h"
#include "gfx_sprites.h" /* GFX_k */
#include "input.h"
#include "sfx.h"

struct talk_struct talk;

void talk_start(int script, int nb_choices) {
  talk.last = nb_choices;
  talk.cur = 1;
  talk.active = /*true*/1;
  talk.page = 1;
  talk.cur_view = 1;
  talk.script = script;

  int ret = SDL_SetRelativeMouseMode(SDL_TRUE);
  if (ret == -1)
    log_error("Relative mouse positionning not supported on this platform.");
  // TODO INPUT: relative mode is messed with pen tablets

}

void talk_stop() {
  talk.active = /*false*/0;
  SDL_SetRelativeMouseMode(SDL_FALSE);
}

void talk_clear()
{
  memset(&talk, 0, sizeof(talk));
  play.mouse = 0;
}
void talk_process()
{
  int px = 48, py = 44;

  int sx = 184;
  int sy = 94, sy_hold, sy_ho;
/*   int spacing = 12; */
  int curxl = 126;
  int curxr = 462;
  int curyr = 200;
  int curyl = 200;

  int y_last = 0, y_hold = 0, y_ho;
/*   HDC         hdc; */
  rect rcRect;
  int i;
  int x_depth = 335;
  if (talk.newy != -5000)
    sy = talk.newy;

  sy_hold = sy;
  sy_ho = sy;

  check_seq_status(30);

  int fake_page;
/*  again: */
/*   ddrval = lpDDSBack->BltFast( px, py, k[seq[30].frame[2]].k, */
/* 			       &k[seq[30].frame[2]].box  , DDBLTFAST_SRCCOLORKEY  ); */
/*   if (ddrval == DDERR_WASSTILLDRAWING) goto again; */
  // GFX
  {
    SDL_Rect dst;
    dst.x = px; dst.y = py;
    SDL_BlitSurface(GFX_k[seq[30].frame[2]].k, NULL, GFX_lpDDSBack, &dst);
  }

/*  again2:	 */
/*   ddrval = lpDDSBack->BltFast( px+169, py+42, k[seq[30].frame[3]].k, */
/* 			       &k[seq[30].frame[3]].box  , DDBLTFAST_SRCCOLORKEY  ); */
/*   if (ddrval == DDERR_WASSTILLDRAWING) goto again2; */
  // GFX
  {
    SDL_Rect dst;
    dst.x = px + 169; dst.y = py + 42;
    SDL_BlitSurface(GFX_k[seq[30].frame[3]].k, NULL, GFX_lpDDSBack, &dst);
  }

/*  again3: */
/*   ddrval = lpDDSBack->BltFast( px+169+180, py+1, k[seq[30].frame[4]].k, */
/* 			       &k[seq[30].frame[4]].box  , DDBLTFAST_SRCCOLORKEY  ); */
/*   if (ddrval == DDERR_WASSTILLDRAWING) goto again3; */
  // GFX
  {
    SDL_Rect dst;
    dst.x = px+169+180; dst.y = py+1;
    if (SDL_BlitSurface(GFX_k[seq[30].frame[4]].k, NULL, GFX_lpDDSBack, &dst) < 0)
      log_error("Could not draw sprite %d: %s", seq[30].frame[4], SDL_GetError());
  }


  int talk_hold = talk.cur;
  if (sjoy.rightd) talk.cur++;
  if (sjoy.downd) talk.cur++;
  if (sjoy.upd) talk.cur--;
  if (sjoy.leftd) talk.cur--;

  if (play.mouse > 20)
    {
      talk.cur++;
      play.mouse = 0;
    }

  if (play.mouse < -20)
    {
      talk.cur--;
      play.mouse = 0;
    }


  if (talk_hold != talk.cur)
    {
      if (talk.cur >= talk.cur_view)
	if (talk.cur <= talk.cur_view_end)
	  SoundPlayEffect(11, 22050,0,0,0);
    }

/*   if (lpDDSBack->GetDC(&hdc) == DD_OK) */
/*     {       */

/*       SelectObject (hdc, hfont_small); */
      // FONTS
      //FONTS_SetFont(FONTS_hfont_small);
/*       SetBkMode(hdc, TRANSPARENT);  */



      /* Print dialog title, if any */
      if (strlen(talk.buffer) > 0)
	{
	  rect_set(&rcRect, sx, 94, 463, 400);
	  /* if using an explicit "set_y" after "choice_start()": */
	  if (talk.newy != -5000)
	    rcRect.bottom = talk.newy + 15;

/* 	  SetTextColor(hdc,RGB(8,14,21)); */
	  // FONTS
	  FONTS_SetTextColor(8, 14, 21);
/* 	  DrawText(hdc,talk.buffer,strlen(talk.buffer),&rcRect,DT_VCENTER | DT_CENTER | DT_WORDBREAK); */
	  // FONTS
	  //printf("(%dx%d)x(%dx%d)\n", rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);
	  print_text_wrap(talk.buffer, &rcRect, 1, 0, FONT_DIALOG);


	   /* Same of in text_draw, except for #1 and default */
	   // FONTS:
	   // support for custom colors
	   if (talk.color >= 1 && talk.color <= 15)
	     FONTS_SetTextColorIndex(talk.color);
	   else
	     {
	       if (dversion >= 108)
		 FONTS_SetTextColor(255, 255, 255);
	       else
		 FONTS_SetTextColor(255, 255, 2);
	    }

	  rect_offset(&rcRect, 1, 1);
/* 	  DrawText(hdc,talk.buffer,strlen(talk.buffer),&rcRect,DT_VCENTER | DT_CENTER | DT_WORDBREAK);	 */
	  // FONTS
	  print_text_wrap(talk.buffer, &rcRect, 1, 0, FONT_DIALOG);

/* 	  SetTextColor(hdc,RGB(8,14,21)); */
	  // FONTS
	  FONTS_SetTextColor(8, 14, 21);
	}




      //tabulate distance needed by text, LORDII experience helped here
      //recal:
      for (i = talk.cur_view; i < talk.last; i++)
	{
	  rect_set(&rcRect,sx,y_hold,463,x_depth+100);
/* 	  y_hold = DrawText(hdc,talk.line[i],lstrlen(talk.line[i]),&rcRect,DT_CALCRECT | DT_CENTER | DT_WORDBREAK); */
	  // FONTS
	  /* Don't print, only check the height in pixel: */
	  y_hold = print_text_wrap(talk.line[i], &rcRect, 1, 1, FONT_DIALOG);
	  sy_hold += y_hold;

	  //Msg("Sy_hold = %d (%d)", sy_hold,i);

	  if (sy_hold > x_depth)
	    {

	      talk.cur_view_end = i-1;
	      //Msg("Sy is over, sp cur_view is %d ", talk.cur_view_end);
	      goto death;
	    }
	}

      talk.cur_view_end = i;

      if (talk.cur_view == 1 && talk.cur_view_end == talk.last)
	{
	  //Msg("Small enough to fit on one screen, lets center it!");
	  sy += ( (x_depth - sy_hold) / 2) - 20;
	}
    death:
      if (talk.cur > talk.last)
	{
	  SoundPlayEffect(11, 22050,0,0,0);

	  talk.cur = 1;

	}
      if (talk.cur < 1)
	{
	  SoundPlayEffect(11, 22050,0,0,0);

	  talk.cur = talk.last;
	}


      //if (talk.cur_view_end != talk.last)
      {
	//Msg("Talkcur is %d, talk cur view is %d", talk.cur, talk.cur_view);
	//total options too large for page, lets scroll


	if (talk.cur > talk.cur_view_end)
	  {
	    //     Msg("advancing page:  talkcur is %d, changing cur_view to same", talk.cur, talk.cur_view);
	    talk.cur_view = talk.cur;
	    talk.page ++;

	    // Msg("Page advanced to %d. (cur_end is %d, cur is %d)", talk.page,talk.cur_view_end, talk.cur);
	    goto fin;
	  }



	if (talk.cur < talk.cur_view)
	  {
	    //	Msg("Turning back the clock from page %d..", talk.page);

	    talk.cur_view = 1;
	    // talk.cur = 1;

	    talk.page--;
	    log_info("Page backed to %d.", talk.page);
	    fake_page = 1;
	    for (i = 1; i < talk.last; i++)
	      {
		rect_set(&rcRect,sx,sy_ho,463,x_depth);

/* 		y_ho = DrawText(hdc,talk.line[i],lstrlen(talk.line[i]),&rcRect,DT_CALCRECT | DT_CENTER | DT_WORDBREAK); */
		// FONTS
		/* Don't print, only check the height in pixel: */
		y_ho = print_text_wrap(talk.line[i], &rcRect, 1, 1, FONT_DIALOG);
		sy_ho += y_ho;
		//Msg("adding y_yo %d.. (on %d)", y_ho,i);
		if (sy_ho > x_depth)
		  {
		    /*if (fake_page == talk.page)
		      {
		      goto fin;
		      }
		    */
		    fake_page++;
		    sy_ho = sy+ y_ho;
		    //Msg("Does fake page (%d) match desired page (%d) %d", fake_page, talk.page, i);
		  }
		if (fake_page == talk.page)
		  {
		    talk.cur_view = i;
		    talk.cur_view_end = talk.cur;
		    //Msg("Going to fin with end being %d, and.cur being %d.  View is %d.",
		    //		   talk.cur_view_end, talk.cur, talk.cur_view);
		    goto fin;
		  }

		//         Msg("Second: Sy is over, sp cur_view is %d", talk.cur_view_end);
	      }
	    talk.cur_view_end = i;
	  }
      }

      //Msg("talk last is %d.  cur_view_end is %d, Cur is %d", talk.last, talk.cur_view_end, talk.cur);

      //	 talk.cur_view_end = talk.last;

      for ( i = talk.cur_view; i <= talk.cur_view_end; i++)
	{
	  //lets figure out where to draw this line

	  rect_set(&rcRect, sx, sy, 463, x_depth + 100);
/* 	  SetTextColor(hdc,RGB(8,14,21)); */
	  // FONTS
	  FONTS_SetTextColor(8, 14, 21);
/* 	  DrawText(hdc,talk.line[i],lstrlen(talk.line[i]),&rcRect, DT_CENTER | DT_WORDBREAK); */
	  // FONTS
	  print_text_wrap(talk.line[i], &rcRect, 1, 0, FONT_DIALOG);
	  rect_offset(&rcRect, -2, -2);
/* 	  DrawText(hdc,talk.line[i],lstrlen(talk.line[i]),&rcRect,DT_CENTER | DT_WORDBREAK); */
	  // FONTS
	  print_text_wrap(talk.line[i], &rcRect, 1, 0, FONT_DIALOG);

	  rect_offset(&rcRect, 1, 1);
	  if (i == talk.cur)
	    {
	      curyl = sy-4;
	      curyr = sy-4;

/* 	      SetTextColor(hdc,RGB(255,255,255)); */
	      // FONTS
	      FONTS_SetTextColor(255, 255, 255);
	    }
	  else
	    {
/* 	      SetTextColor(hdc,RGB(255,255,2)); */
	      // FONTS
	      FONTS_SetTextColor(255, 255, 2);
	    }
/* 	  y_last = DrawText(hdc,talk.line[i],lstrlen(talk.line[i]),&rcRect,DT_CENTER | DT_WORDBREAK); */
	  // FONTS
	  y_last = print_text_wrap(talk.line[i], &rcRect, 1, 0, FONT_DIALOG);
	  sy += y_last;
	}

    fin:
      //	   dum =  GetTextFace(hdc,100,shit) ;
/*       lpDDSBack->ReleaseDC(hdc); */

      if (talk.timer < thisTickCount)
	{
	  talk.curf++;
	  talk.timer = thisTickCount+100;
	}


      if (talk.curf == 0) talk.curf = 1;

      if (talk.curf > 7) talk.curf = 1;
/*     again4: */
/*       ddrval = lpDDSBack->BltFast( curxl, curyl, k[seq[456].frame[talk.curf]].k, */
/* 				   &k[seq[456].frame[talk.curf]].box  , DDBLTFAST_SRCCOLORKEY  ); */
/*       if (ddrval == DDERR_WASSTILLDRAWING) goto again4; */
      // GFX
      {
	SDL_Rect dst;
	dst.x = curxl; dst.y = curyl;
	SDL_BlitSurface(GFX_k[seq[456].frame[talk.curf]].k, NULL, GFX_lpDDSBack, &dst);
      }

/*     again5: */
/*       ddrval = lpDDSBack->BltFast( curxr, curyr, k[seq[457].frame[talk.curf]].k, */
/* 				   &k[seq[456].frame[talk.curf]].box  , DDBLTFAST_SRCCOLORKEY  ); */
/*       if (ddrval == DDERR_WASSTILLDRAWING) goto again5; */
      // GFX
      {
	SDL_Rect dst;
	dst.x = curxr; dst.y = curyr;
	SDL_BlitSurface(GFX_k[seq[457].frame[talk.curf]].k, NULL, GFX_lpDDSBack, &dst);
      }
/*   } */


  if ((sjoy.button[ACTION_ATTACK]) | (mouse1))
    {
      mouse1 = /*false*/0;
      talk_stop();
      *presult = talk.line_return[talk.cur];
      SoundPlayEffect(17, 22050,0,0,0);

      if (talk.script != 0)
	{
	  //we need to continue a script
	  run_script(talk.script);

	}
    }
}
