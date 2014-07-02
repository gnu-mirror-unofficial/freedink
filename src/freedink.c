/**
 * FreeDink game-specific code

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

#include "freedink.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gettext.h"
#define _(String) gettext(String)

#include "fastfile.h"

#include "gfx.h"
#include "gfx_fonts.h"
#include "gfx_palette.h"
#include "gfx_sprites.h"
#include "gfx_tiles.h"
#include "gfx_fade.h"
#include "bgm.h"
#include "sfx.h"
#include "update_frame.h"
#include "app.h"
#include "io_util.h"
#include "paths.h"
#include "input.h"
#include "log.h"

#include "game_engine.h"
#include "screen.h"
#include "meminfo.h"
#include "dinkvar.h"
#include "dinkc_console.h"
#include "talk.h"
#include "text.h"

void move(int u, int amount, char kind,  char kindy);
void draw_box(rect box, int color);
void run_through_tag_list_push(int h);
int check_if_move_is_legal(int u);
void change_dir_to_diag( int *dir);
int hurt_thing(int h, int damage, int special);


static int but_timer = 0;

/* Fadedown/fadeup counter */
static int process_count = 0;

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

/* Get sprite #h, grab its text and display it */
void text_draw(int h)
{
	
	char crap[200];
	char *cr;
	rect rcRect;
	int color = 0;
	
	if (spr[h].damage == -1)
	{
		sprintf(crap, "%s", spr[h].text);
		cr = &crap[0];
		color = 14;
		while( cr[0] == '`') 
		{
			//color code at top
			if (cr[1] == '#') color = 13;
			if (cr[1] == '1') color = 1;
			if (cr[1] == '2') color = 2;
			if (cr[1] == '3') color = 3;
			if (cr[1] == '5') color = 5;
			if (cr[1] == '6') color = 6;
			if (cr[1] == '7') color = 7;
			if (cr[1] == '8') color = 8;
			if (cr[1] == '9') color = 9;
			if (cr[1] == '0') color = 10;
			if (cr[1] == '$') color = 14;
			if (cr[1] == '%') color = 15;
			
			if (dversion >= 108)
			  {
			    //support for additional colors
			    if (cr[1] == '@')
			      color = 12;
			    if (cr[1] == '!')
			      color = 11;
			  }

			if (cr[1] == '4') color = 4;
			cr = &cr[2];
		}
		
		//Msg("Final is %s.",cr);
		if (spr[h].owner == 1000)
		{
			rect_set(&rcRect,spr[h].x,spr[h].y,spr[h].x+620,spr[h].y+400);
		} else
		{
			
			rect_set(&rcRect,spr[h].x,spr[h].y,spr[h].x+150,spr[h].y+150);
			
			if (spr[h].x+150 > 620)
				rect_offset(&rcRect, ((spr[h].x+150)-620) - (((spr[h].x+150)-620) * 2), 0);
			
			
			
		}
		
	} else
	{
		
		
		sprintf(crap, "%d", spr[h].damage);
		cr = &crap[0];
		if (spr[h].brain_parm == 5000)
			color = 14;
		
		
		if (spr[h].y < 0) spr[h].y = 0;
		rect_set(&rcRect,spr[h].x,spr[h].y,spr[h].x+50 ,spr[h].y+50);
		
		
	}       


	/* During a fadedown/fadeup, use white text to mimic v1.07 */
	if (truecolor_fade_brightness < 256)
	  color = 15;
	
	
/* 	SetTextColor(hdc,RGB(8,14,21)); */
	// FONTS
	FONTS_SetTextColor(8, 14, 21);
	   if (spr[h].owner == 1200)
	   {
	     printf("1200 says %s\n", cr);
		   //this text has no sprite, and doesn't want to be centered.
/* 		   DrawText(hdc,cr,strlen(cr),&rcRect,DT_WORDBREAK); */
		   // FONTS
	     print_text_wrap(cr, &rcRect, 0, 0, FONT_DIALOG);
		   
		   rect_offset(&rcRect,-2,0);
/* 		   DrawText(hdc,cr,strlen(cr),&rcRect,DT_WORDBREAK); */
		   // FONTS
		   print_text_wrap(cr, &rcRect, 0, 0, FONT_DIALOG);
		   
		   rect_offset(&rcRect,1,1);
/* 		   DrawText(hdc,cr,strlen(cr),&rcRect,DT_WORDBREAK); */
		   // FONTS
		   print_text_wrap(cr, &rcRect, 0, 0, FONT_DIALOG);

		   rect_offset(&rcRect,0,-2);
/* 		   DrawText(hdc,cr,strlen(cr),&rcRect,DT_WORDBREAK); */
		   // FONTS
		   print_text_wrap(cr, &rcRect, 0, 0, FONT_DIALOG);
	   }
	   else
	   {
		   
/* 		   DrawText(hdc,cr,strlen(cr),&rcRect,DT_CENTER | DT_WORDBREAK); */
		   // FONTS
	     print_text_wrap(cr, &rcRect, 1, 0, FONT_DIALOG);

		   rect_offset(&rcRect,-2,0);
/* 		   DrawText(hdc,cr,strlen(cr),&rcRect,DT_CENTER | DT_WORDBREAK); */
		   // FONTS
		   print_text_wrap(cr, &rcRect, 1, 0, FONT_DIALOG);
		   
		   rect_offset(&rcRect,1,1);
/* 		   DrawText(hdc,cr,strlen(cr),&rcRect,DT_CENTER | DT_WORDBREAK); */
		   // FONTS
		   print_text_wrap(cr, &rcRect, 1, 0, FONT_DIALOG);

		   rect_offset(&rcRect,0,-2);
/* 		   DrawText(hdc,cr,strlen(cr),&rcRect,DT_CENTER | DT_WORDBREAK); */
		   // FONTS
		   print_text_wrap(cr, &rcRect, 1, 0, FONT_DIALOG);
	   }
	   
	   rect_offset(&rcRect,0,1);
	   
	   // FONTS:
	   // support for custom colors
	   if (color >= 1 && color <= 15)
	     FONTS_SetTextColorIndex(color);
	   else
	     FONTS_SetTextColor(255, 255, 255);

	   if (spr[h].owner == 1200)
	     {
/* 	       DrawText(hdc,cr,strlen(cr),&rcRect,DT_WORDBREAK); */
	       // FONTS
	       print_text_wrap(cr, &rcRect, 0, 0, FONT_DIALOG);
	     }
	   else
	     {
/* 	       DrawText(hdc,cr,strlen(cr),&rcRect,DT_CENTER | DT_WORDBREAK); */
	       // FONTS
	       print_text_wrap(cr, &rcRect, 1, 0, FONT_DIALOG);
	     }
}


void get_last_sprite(void)
{
  int i;
  for (i = MAX_SPRITES_AT_ONCE - 1; i > 2; i--)
    {
      if (spr[i].active)
	{
	  last_sprite_created = i;
	  //   Msg("last sprite created is %d.", i);
	  return;
	}
    }
}


// ********* CHECK TO SEE IF THIS CORD IS ON A HARD SPOT *********
/*bool*/int not_in_this_base(int seq, int base)
{
	
	int realbase = (seq / 10) * 10;
	
	
	if (realbase != base)
	{
		
		
		return(/*true*/1); 
	}
	else
	{
		return(/*false*/0);
	}
}

/*bool*/int in_this_base(int seq, int base)
{
	
	int realbase = (seq / 10) * 10;
	if (realbase == base)
	{
		
		//	Msg("TRUE - Ok, realbase is %d, compared to the base, which is %d.", realbase, base);
		return(/*true*/1); 
	}
	else
	{
		//	Msg("FALSE - Ok, realbase is %d, compared to the base, which is %d.", realbase, base);
		
		return(/*false*/0);
	}
}


void automove (int j)
{
	
	char kindx,kindy;
	int speedx = 0;
	int speedy = 0;
	
	
	
	
	if (spr[j].mx != 0)
	{ 
		if (spr[j].mx < 0)
			kindx = '-'; else kindx = '+';
		if (kindx == '-') speedx = (spr[j].mx - (spr[j].mx * 2)); else
			speedx = spr[j].mx;
	} else kindx = '0';
	
	if (spr[j].my != 0)
	{ 
		if (spr[j].my < 0)
			kindy = '-'; else kindy = '+';
		if (kindy == '-') speedy = (spr[j].my - (spr[j].my * 2)); else
			speedy = spr[j].my;
		
	} else kindy = '0';
	
	int speed = speedx;
	if (speedy > speedx) speed = speedy;
	if (speed > 0)
		move(j,speed,kindx,kindy);
	//move(j, 1, '+','+'); 
	
}


int autoreverse(int j)
{
	//Msg("reversing die %d",spr[j].dir);
	int r = ((rand() % 2)+1);	
	if ( (spr[j].dir == 1) || (spr[j].dir == 2) ) 
	{
		if (r == 1)
			return(8);
		if (r == 2)
			return(6);
		
	}
	
	if ( (spr[j].dir == 3) || (spr[j].dir == 6) ) 
	{
		if (r == 1)
			return(2);
		if (r == 2)
			
			return(4);
		
	}
	
    if ( (spr[j].dir == 9) || (spr[j].dir == 8) ) 
	{
		if (r == 1)
			return(2);
		if (r == 2)
			
			return(6);
		
		
	}
	
    if ( (spr[j].dir == 7) || (spr[j].dir == 4) ) 
	{
		if (r == 1)
			return(8);
		if (r == 2)
			return(6);
		
	}
	
	return(0);
}


int autoreverse_diag(int j)
{
	if (spr[j].dir == 0) spr[j].dir = 7;
	int r = ((rand() % 2)+1);	
	
	if ( (spr[j].dir == 1) || (spr[j].dir == 3) ) 
	{
		
		if (r == 1)
			return(9);
		if (r == 2)
			return(7);
	}
	
	if ( (spr[j].dir == 3) || (spr[j].dir == 6) ) 
	{
		if (r == 1)
			return(7);
		if (r == 2)
			return(1);
		
	}
	
    if ( (spr[j].dir == 9) || (spr[j].dir == 8) ) 
	{
		if (r == 1)
			return(1);
		if (r == 2)
			return(7);
	}
	
    if ( (spr[j].dir == 7) || (spr[j].dir == 4) ) 
	{
		if (r == 1)
			return(3);
		if (r == 2)
			return(9);
		
	}
	
	log_debug("Auto Reverse Diag was sent a dir %d sprite, base %d walk.",spr[j].dir, spr[j].base_walk);
	return(0);
}

void draw_damage(int h)
{
	
	int crap2 = add_sprite(spr[h].x,spr[h].y,8,0,0);
	
	spr[crap2].y -= k[seq[spr[h].pseq].frame[spr[h].pframe]].yoffset;
	spr[crap2].x -= k[seq[spr[h].pseq].frame[spr[h].pframe]].xoffset;
	spr[crap2].y -= k[seq[spr[h].pseq].frame[spr[h].pframe]].box.bottom / 3;
	spr[crap2].x += k[seq[spr[h].pseq].frame[spr[h].pframe]].box.right / 5;
	
	spr[crap2].speed = 1;
	spr[crap2].hard = 1;
	spr[crap2].brain_parm = h;  
	spr[crap2].my = -1;
	spr[crap2].kill = 1000;
	spr[crap2].dir = 8;
	spr[crap2].damage = spr[h].damage;
	
}


void add_kill_sprite(int h)
{
	if ( (spr[h].dir > 9) || (spr[h].dir < 1) )
	{
		log_error("Changing sprites dir from %d (!?) to 3.", spr[h].dir);
		spr[h].dir = 3;
		
	}
	
	
	int dir = spr[h].dir;
	int base = spr[h].base_die;
	
	//Msg("Base die is %d", base);
	if (base == -1) 
	{
		
	  if (seq[spr[h].base_walk+5].is_active)
		{
			add_exp(spr[h].exp, h);
			
			int crap2 = add_sprite(spr[h].x,spr[h].y,5,spr[h].base_walk +5,1);
			spr[crap2].speed = 0;
			spr[crap2].seq = spr[h].base_walk + 5;   
			// set corpse size to the original sprite size
			spr[crap2].size = spr[h].size;
			return;
		} else
		{
			dir = 0;
			base = 164;
			
		}
	}
	
	
	
	if (!seq[base+dir].is_active)
	{  
		
		if (dir == 1) dir = 9;
		else if (dir == 3) dir = 7;			
		else if (dir == 7) dir = 3;			
		else if (dir == 9) dir = 1;			
		
		else if (dir == 4) dir = 6;			
		else if (dir == 6) dir = 4;			
		else if (dir == 8) dir = 2;			
		else if (dir == 2) dir = 8;			
		
		
	}
	if (!seq[base+dir].is_active)
		
	{
		log_error("Can't make a death sprite for dir %d!", base+dir);
	}
	
	
	
	int crap2 = add_sprite(spr[h].x,spr[h].y,5,base +dir,1);
	spr[crap2].speed = 0;
	spr[crap2].base_walk = 0;
	spr[crap2].seq = base + dir;
	
	if (base == 164) spr[crap2].brain = 7;
	
	spr[crap2].size = spr[h].size;
	
	add_exp(spr[h].exp, h);
	
}


void done_moving(int h)
{
	
	spr[h].move_active = /*false*/0;
	
	spr[h].move_nohard = /*false*/0;
	
	if (spr[h].move_script > 0)
	{
		//	Msg("mover running script %d..", spr[h].move_script);
		run_script(spr[h].move_script);
	}
	
	
	
}

int get_distance_and_dir_smooth(int h, int h1, int *dir)
{
  unsigned int x_diff = abs(spr[h].x - spr[h1].x);
  unsigned int y_diff = abs(spr[h].y - spr[h1].y);

  if (spr[h].x < spr[h1].x)
    {
      if (spr[h].y < spr[h1].y)
	{
	  // 6, 3, 2
	  if (y_diff * 4 < x_diff)
	    *dir = 6;
	  else if (x_diff * 4 < y_diff)
	    *dir = 2;
	  else
	    *dir = 3;
	}
      else if (spr[h].y > spr[h1].y)
	{
	  // 4, 9, 8
	  if (y_diff * 4 < x_diff)
	    *dir = 6;
	  else if (x_diff * 4 < y_diff)
	    *dir = 8;
	  else
	    *dir = 9;
	}
      else
	{
	  *dir = 6;
	}
    }
  else if (spr[h].x > spr[h1].x)
    {
      if (spr[h].y < spr[h1].y)
	{
	  // 4, 1, 2
	  if (y_diff * 4 < x_diff)
	    *dir = 4;
	  else if (x_diff * 4 < y_diff)
	    *dir = 2;
	  else
	    *dir = 1;
	}
      else if (spr[h].y > spr[h1].y)
	{
	  // 4, 7, 8
	  if (y_diff * 4 < x_diff)
	    *dir = 4;
	  else if (x_diff * 4 < y_diff)
	    *dir = 8;
	  else
	    *dir = 7;
	}
      else
	{
	  *dir = 4;
	}
    }
  else
    {
      if (spr[h].y < spr[h1].y)
	*dir = 2;
      else if (spr[h].y > spr[h1].y)
	*dir = 8;
    }

  return (x_diff > y_diff) ? x_diff : y_diff;
}
int get_distance_and_dir_nosmooth(int h, int h1, int *dir)
{
  int distancex = 5000;
  int distancey = 5000;
  /* Arbitrarily set to 6 to avoid uninitialized values; don't set to
     5, because *dir is added to e.g. base_attack to get the right
     sequence - with 5, you get the dead/corpse sequence instead of an
     attack sequence.. */
  int dirx = 6;
  int diry = 6;

  if ((spr[h].x > spr[h1].x) && ((spr[h].x - spr[h1].x) < distancex))
    {
      distancex = spr[h].x - spr[h1].x;
      dirx = 4;
    }
  if ((spr[h].x < spr[h1].x) && ((spr[h1].x - spr[h].x) < distancex))
    {
      distancex = spr[h1].x - spr[h].x;
      dirx = 6;
    }

  if ((spr[h].y > spr[h1].y) && ((spr[h].y - spr[h1].y) < distancey))
    {
      distancey = spr[h].y - spr[h1].y;
      diry = 8;
    }
  if ((spr[h].y < spr[h1].y) && ((spr[h1].y - spr[h].y) < distancey))
    {
      distancey = spr[h1].y - spr[h].y;
      diry = 2;
    }

  if (distancex > distancey)
    {
      *dir = dirx;
      return distancex;
    }
  else
    {
      *dir = diry;
      return distancey;
    }
}
int get_distance_and_dir(int h, int h1, int *dir)
{
  if (smooth_follow == 1)
    return get_distance_and_dir_smooth(h, h1, dir);
  else
    return get_distance_and_dir_nosmooth(h, h1, dir);
}

void process_follow(int h)
{
	if (spr[h].follow > 299)
	{
		log_error("Sprite %d cannot 'follow' sprite %d??",h,spr[h].follow);
		return;
	}
	
	if (spr[spr[h].follow].active == /*false*/0)
	{
		log_debug("Killing follow");
		spr[h].follow = 0;
		return;
	}
	
	int dir;
	int distance = get_distance_and_dir(h, spr[h].follow, &dir);
	
	if (distance < 40) return;
	
	changedir(dir,h,spr[h].base_walk);
	automove(h);
	
	
}


void process_target(int h)
{
	if (spr[h].target > 299)
	{
		log_error("Sprite %d cannot 'target' sprite %d??",h,spr[h].follow);
		return;
	}
	
	if (spr[spr[h].target].active == /*false*/0)
	{
		log_debug("Killing target");
		spr[h].target = 0;
		return;
	}
	
	int dir;
	int distance = get_distance_and_dir(h, spr[h].target, &dir);
	
	if (distance < spr[h].distance) return;
	
	changedir(dir,h,spr[h].base_walk);
	
	automove(h);
	
	
}


/*bool*/int check_for_kill_script(int i)
{
	
	
	if (spr[i].script > 0)
	{
		//if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))
		
		if (locate(spr[i].script, "DIE")) run_script(spr[i].script);
		
		return(/*true*/1);	
	}
	
	return(/*false*/0);
}

/*bool*/int check_for_duck_script(int i)
{
	
	
	if (spr[i].script > 0)
	{
		//if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))
		
		if (locate(spr[i].script, "DUCKDIE")) run_script(spr[i].script);
		
		return(/*true*/1);	
	}
	
	return(/*false*/0);
}



void process_move(int h)
{
	
	//	Msg("Proccesing sprite %d, dir %d (script is %d)", h, spr[h].dir, spr[h].move_script);
	
	
	
	if ((spr[h].move_dir == 4) | (spr[h].move_dir == 1) | (spr[h].move_dir == 7) )
	{
		if (spr[h].x <= spr[h].move_num)
		{
			//done moving
			done_moving(h);
			return;
		}
		changedir(spr[h].move_dir,h,spr[h].base_walk);		
		automove(h);	
	}
	
	if ( (spr[h].move_dir == 6) | (spr[h].move_dir == 9) | (spr[h].move_dir == 3))
	{
		if (spr[h].x >= spr[h].move_num)
		{
			//done moving
			done_moving(h);
			return;
		}
		changedir(spr[h].move_dir,h,spr[h].base_walk);		
		automove(h);	
	}
	
	
	if (spr[h].move_dir == 2)
	{
		if (spr[h].y >= spr[h].move_num)
		{
			//done moving
			done_moving(h);
			return;
		}
		changedir(spr[h].move_dir,h,spr[h].base_walk);		
		automove(h);	
	}
	
	
	if (spr[h].move_dir == 8)
	{
		if (spr[h].y <= spr[h].move_num)
		{
			//done moving
			done_moving(h);
			return;
		}
		changedir(spr[h].move_dir,h,spr[h].base_walk);		
		automove(h);	
	}
	
	
}

void kill_sprite_all (int sprite)
{
        spr[sprite].active = /*false*/0;

        kill_text_owned_by(sprite);
        kill_scripts_owned_by(sprite);

}

void duck_brain(int h)
{
	int hold;
	
	
	if (   (spr[h].damage > 0) && (in_this_base(spr[h].pseq, 110)  ) )
	{
		
		check_for_duck_script(h);
		
		//hit a dead duck
		int crap2 = add_sprite(spr[h].x,spr[h].y,7,164,1);
                /* TODO: add_sprite might return 0, and the following
                   would trash spr[0] - cf. bugs.debian.org/688934 */
		spr[crap2].speed = 0;
		spr[crap2].base_walk = 0;
		spr[crap2].seq = 164;
		draw_damage(h);
		spr[h].damage = 0;
		add_exp(spr[h].exp, h);
		
		kill_sprite_all(h);
		
		return;
	}
	
	
	if (   (spr[h].damage > 0) && (in_this_base(spr[h].pseq, spr[h].base_walk)  ) )
	{
		//SoundPlayEffect( 1,3000, 800 );
		draw_damage(h);
		add_exp(spr[h].exp, h);
		spr[h].damage = 0;
		
		//lets kill the duck here, ha.
		check_for_kill_script(h);
		spr[h].follow = 0;
		int crap = add_sprite(spr[h].x,spr[h].y,5,1,1);
                /* TODO: add_sprite might return 0, and the following
                   would trash spr[0] - cf. bugs.debian.org/688934 */
		spr[crap].speed = 0;
		spr[crap].base_walk = 0;
		spr[crap].size = spr[h].size;						
		spr[crap].speed =  ((rand() % 3)+1);
		
		
		spr[h].base_walk = 110;
		spr[h].speed = 1;
		spr[h].timer = 0;
		spr[h].wait = 0;
		spr[h].frame = 0;
		
		if (spr[h].dir == 0) spr[h].dir = 1;
		if (spr[h].dir == 4) spr[h].dir = 7;
		if (spr[h].dir == 6) spr[h].dir = 3;
		
		changedir(spr[h].dir,h,spr[h].base_walk);
		spr[crap].dir = spr[h].dir;
		spr[crap].base_walk = 120;
		changedir(spr[crap].dir,crap,spr[crap].base_walk);
		
		
		automove(h);
		return;
	}
	
	
	if (spr[h].move_active)
	{
		process_move(h);
		return;
	}
	
	if (spr[h].freeze)
	{
		return;
	}
	
	
	if (spr[h].follow > 0)
	{
		process_follow(h);
		return;
	}
	
	
	
	if (spr[h].base_walk == 110)
	{
		if ( (rand() % 100)+1 == 1)
			random_blood(spr[h].x, spr[h].y-18, h);
		goto walk;
	}
	
	
	
	
	
	if (spr[h].seq == 0 ) 
	{
		
		if (((rand() % 12)+1) == 1 )
		{  
			hold = ((rand() % 9)+1);
			
			if ((hold != 2) && (hold != 8) && (hold != 5))
			{
				
				//Msg("random dir change started.. %d", hold);
				changedir(hold,h,spr[h].base_walk);
				
			}
			else
			{
				int junk = spr[h].size;
				
				if (junk >=  100)
					junk = 18000 - (junk * 50);
				
				if (junk < 100)
					junk = 16000 + (junk * 100);
				
				SoundPlayEffect( 1,junk, 800,h ,0);
				spr[h].mx = 0;
				spr[h].my = 0;
				spr[h].wait = thisTickCount + (rand() % 300)+200;
				
			}
			return;		
		} 
		
		if ((spr[h].mx != 0) || (spr[h].my != 0))
			
		{
			spr[h].seq = spr[h].seq_orig;
			
		}
		
	}
	
	
walk:
	if (spr[h].y > playy)
		
	{
		changedir(9,h,spr[h].base_walk);
	}         
	
	
	
	if (spr[h].x > playx-30)
		
	{
		changedir(7,h,spr[h].base_walk);
	}         
	
	if (spr[h].y < 10)
	{
		changedir(1,h,spr[h].base_walk);
	}         
	
	if (spr[h].x < 30) 
	{
		changedir(3,h,spr[h].base_walk);
	}         
	
	//   Msg("Duck dir is %d, seq is %d.", spr[h].dir, spr[h].seq);	
	automove(h);
	
	if (check_if_move_is_legal(h) != 0)
		
	{
		if (spr[h].dir != 0)
			changedir(autoreverse_diag(h),h,spr[h].base_walk);
	}
	
}
// end duck_brain

void change_dir_to_diag( int *dir)
{
	
	if (*dir == 8) *dir = 7;
	if (*dir == 4) *dir = 1;
	if (*dir == 2) *dir = 3;
	if (*dir == 6) *dir = 9;
	
}



void pill_brain(int h)
{
	int hold;
	
	if  (spr[h].damage > 0)
	{
		//got hit
		//SoundPlayEffect( 1,3000, 800 );
		if (spr[h].hitpoints > 0)
		{
			draw_damage(h);
			if (spr[h].damage > spr[h].hitpoints) spr[h].damage = spr[h].hitpoints;
			spr[h].hitpoints -= spr[h].damage;
			
			if (spr[h].hitpoints < 1)
			{
				//they killed it
				check_for_kill_script(h);
				
				if (spr[h].brain == 9)
				{
					if (spr[h].dir == 0) spr[h].dir = 3;
					change_dir_to_diag(&spr[h].dir);
					add_kill_sprite(h);
					spr[h].active = /*false*/0;
				}
				return;
				
			}
		}
		spr[h].damage = 0;
		
	}
	
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	
	
	
	if (spr[h].freeze) return;
	
	if (spr[h].follow > 0)
	{
		process_follow(h);
		
	}
	
	
	if (spr[h].target != 0) 
	{
		
		if (in_this_base(spr[h].seq, spr[h].base_attack))
		{
			//still attacking
			return;
		}
		
		
		
		
		
		int dir;
		if (spr[h].distance == 0) spr[h].distance = 5;
		int distance = get_distance_and_dir(h, spr[h].target, &dir);
		
		if (distance < spr[h].distance) if (spr[h].attack_wait < thisTickCount)
		{
			//	Msg("base attack is %d.",spr[h].base_attack);
			if (spr[h].base_attack != -1)
			{
			  //Msg("attacking with %d..", spr[h].base_attack+dir);

			  /* Enforce lateral (not diagonal) attack,
			     even in smooth_follow mode */
			  int attackdir = 6; // arbitrary initialized default
			  get_distance_and_dir_nosmooth(h, spr[h].target, &attackdir);
			  spr[h].dir = attackdir;
				
				spr[h].seq = spr[h].base_attack+spr[h].dir;
				spr[h].frame = 0;
				
				if (spr[h].script != 0)
				  {
				    if (locate(spr[h].script, "ATTACK"))
				      run_script(spr[h].script);
				    else
				      spr[h].move_wait = thisTickCount + ((rand() % 300)+10);
				  }
				return;
					
			}
			
		}
		
		
		
		if (spr[h].move_wait  < thisTickCount)
		{
			process_target(h);
			spr[h].move_wait = thisTickCount + 200;
			
		}
		else
		{
		/*	automove(h);
		
		  if (check_if_move_is_legal(h) != 0)
		  {
		  
			}
			*/
			
			goto walk_normal;
		}
		
		return;
	}
	
	
	
walk_normal:
	
	if (spr[h].base_walk != -1)
	{
		if ( spr[h].seq == 0) goto recal;
	}
	
	if (( spr[h].seq == 0) && (spr[h].move_wait < thisTickCount))
	{
recal:
	if (((rand() % 12)+1) == 1 )
	{  
		hold = ((rand() % 9)+1);
		if (  (hold != 4) &&   (hold != 6) &&  (hold != 2) && (hold != 8) && (hold != 5))
		{
			changedir(hold,h,spr[h].base_walk);
			spr[h].move_wait = thisTickCount +((rand() % 2000)+200);
			
		}
		
	} else
	{
		//keep going the same way
		if (in_this_base(spr[h].seq_orig, spr[h].base_attack)) goto recal;
		spr[h].seq = spr[h].seq_orig;
		if (spr[h].seq_orig == 0) goto recal;
	}
	
	}
	
    
	
	if (spr[h].y > (playy - 15))
		
	{
		changedir(9,h,spr[h].base_walk);
	}         
	
	if (spr[h].x > (playx - 15))
		
	{
		changedir(1,h,spr[h].base_walk);
	}         
	
	if (spr[h].y < 18)
	{
		changedir(1,h,spr[h].base_walk);
	}         
	
	if (spr[h].x < 18) 
	{
		changedir(3,h,spr[h].base_walk);
	}         
	
	automove(h);
	
	if (check_if_move_is_legal(h) != 0)
	{
		spr[h].move_wait = thisTickCount + 400;
		changedir(autoreverse_diag(h),h,spr[h].base_walk);
	}
	
	
	//				changedir(hold,h,spr[h].base_walk);
	
	
}

void find_action(int h)
{
	
	spr[h].action = (rand() % 2)+1;
	
	
	if (spr[h].action == 1)
	{
		//sit and think
		spr[h].move_wait = thisTickCount +((rand() % 3000)+400);
		if (spr[h].base_walk != -1)
		{
			int dir = (rand() % 4)+1;  
			
			spr[h].pframe = 1;
			if (dir == 1)  spr[h].pseq = spr[h].base_walk+1;
			if (dir == 2)  spr[h].pseq = spr[h].base_walk+3;
			if (dir == 3)  spr[h].pseq = spr[h].base_walk+7;
			if (dir == 4)  spr[h].pseq = spr[h].base_walk+9;
		}
		
		return;
	}
	
	if (spr[h].action == 2)
	{
		//move
		spr[h].move_wait = thisTickCount +((rand() % 3000)+500);
		int dir = (rand() % 4)+1;  
		spr[h].pframe = 1;
		if (dir == 1)  changedir(1,h,spr[h].base_walk);
		if (dir == 2)  changedir(3,h,spr[h].base_walk);
		if (dir == 3)  changedir(7,h,spr[h].base_walk);
		if (dir == 4)  changedir(9,h,spr[h].base_walk);
		return;
	}
	
	
	log_error("Internal error:  Brain 16, unknown action.");
}


void people_brain(int h)
{
	if  (spr[h].damage > 0)
	{
		//got hit
		//SoundPlayEffect( 1,3000, 800 );
		if (spr[h].hitpoints > 0)
		{
			draw_damage(h);
			if (spr[h].damage > spr[h].hitpoints) spr[h].damage = spr[h].hitpoints;
			spr[h].hitpoints -= spr[h].damage;
			
			if (spr[h].hitpoints < 1)
			{
				//they killed it
				check_for_kill_script(h);
				
				if (spr[h].brain == 16)
				{
					if (spr[h].dir == 0) spr[h].dir = 3;
					spr[h].brain = 0;
					change_dir_to_diag(&spr[h].dir);
					add_kill_sprite(h);
					spr[h].active = /*false*/0;
				}
				return;
				
			}
		}
		spr[h].damage = 0;
		
	}
	
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	
	
	
	if (spr[h].freeze) return;
	
	if (spr[h].follow > 0)
	{
		process_follow(h);
		return;
	}
	
	
	if ((spr[h].move_wait < thisTickCount) && (spr[h].seq == 0))
	{
		
		spr[h].action = 0;
	}
	
	
	
	if (spr[h].action == 0) find_action(h);
	
	
	if (spr[h].action != 2) 
	{
		spr[h].seq = 0;
		return;
		
	}
	if (spr[h].seq_orig != 0)
		if (spr[h].seq == 0) spr[h].seq = spr[h].seq_orig;
		
		
		if (spr[h].y > playy)
			
		{
			
			if ( ((rand() % 2)+1) == 1)
				changedir(9,h,spr[h].base_walk);
			else changedir(7,h,spr[h].base_walk);
			
			
		}         
		
		if (spr[h].x > playx)
			
		{
			if ( ((rand() % 2)+1) == 1)
				changedir(1,h,spr[h].base_walk);
			else changedir(7,h,spr[h].base_walk);
			
		}         
		
		if (spr[h].y < 20)
		{
			if ( ((rand() % 2)+1) == 1)
				changedir(1,h,spr[h].base_walk);
			else changedir(3,h,spr[h].base_walk);
		}         
		
		if (spr[h].x < 30) 
		{
			if ( ((rand() % 2)+1) == 1)
				changedir(3,h,spr[h].base_walk);
			else changedir(9,h,spr[h].base_walk);
		}         
		
		automove(h);
		
		if (check_if_move_is_legal(h) != 0)
		{
			if ((rand() % 3) == 2)
			{
				changedir(autoreverse_diag(h),h,spr[h].base_walk);
				
			} else
			{
				spr[h].move_wait = 0;
				spr[h].pframe = 1;
				spr[h].seq = 0;
			}
		}
		
		
		//				changedir(hold,h,spr[h].base_walk);
		
		
}


void no_brain(int h)
{
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	if (spr[h].freeze) return;
	
	if (spr[h].follow > 0)
	{
		process_follow(h);
		return;
	}
	
}


void shadow_brain(int h)
{
	if (spr[spr[h].brain_parm].active == /*false*/0)
	{
		spr[h].active = /*false*/0;
		return;
	}
	
	spr[h].x = spr[spr[h].brain_parm].x;
	spr[h].y = spr[spr[h].brain_parm].y;
	spr[h].size = spr[spr[h].brain_parm].size;
	
	if (spr[h].seq == 0) if (spr[h].seq_orig != 0) spr[h].seq = spr[h].seq_orig;
	
}



void dragon_brain(int h)
{
	int hold;
	
	
	if  (spr[h].damage > 0)
	{
		//got hit
		//SoundPlayEffect( 1,3000, 800 );
		if (spr[h].hitpoints > 0)
		{
			draw_damage(h);
			if (spr[h].damage > spr[h].hitpoints) spr[h].damage = spr[h].hitpoints;
			spr[h].hitpoints -= spr[h].damage;
			
			if (spr[h].hitpoints < 1)
			{
				//they killed it
				
				check_for_kill_script(h);
				if (spr[h].brain == 10)
				{
					add_kill_sprite(h);
					spr[h].active = /*false*/0;
					
				}
				
				return;
				
			}
		}
		spr[h].damage = 0;
	}
	
	
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	
	if (spr[h].freeze) return;
	
	
	if (spr[h].follow > 0)
	{
		process_follow(h);
		return;
	}
	
	if (spr[h].target != 0)
		if (spr[h].attack_wait < thisTickCount)
		{
			if (spr[h].script != 0) 
			{
				
				if (locate(spr[h].script, "ATTACK")) run_script(spr[h].script);
			}	
		}
		
		
		
		if (spr[h].seq == 0)
		{
recal:
		if (((rand() % 12)+1) == 1 )
		{  
			hold = ((rand() % 9)+1);
			if (  (hold != 1) &&   (hold != 3) &&  (hold != 7) && (hold != 9) && (hold != 5))
			{
				changedir(hold,h,spr[h].base_walk);
				
			}
			
		} else
		{
			//keep going the same way
			spr[h].seq = spr[h].seq_orig;
			if (spr[h].seq_orig == 0) goto recal;
		}
		
		}
		
		
		if (spr[h].y > playy)
			
		{
			changedir(8,h,spr[h].base_walk);
		}         
		
		if (spr[h].x > GFX_RES_W)
		{
			changedir(4,h,spr[h].base_walk);
		}         
		
		if (spr[h].y < 0)
		{
			changedir(2,h,spr[h].base_walk);
		}         
		
		if (spr[h].x < 0) 
		{
			changedir(6,h,spr[h].base_walk);
		}         
		
		automove(h);
		
		if (check_if_move_is_legal(h) != 0)
			
		{
			
			int mydir = autoreverse(h);
			
			//	Msg("Real dir now is %d, autoresver changed to %d.",spr[h].dir, mydir);
			
			changedir(mydir,h,spr[h].base_walk);
			
			log_debug("real dir changed to %d", spr[h].dir);
		}
		
}




void pig_brain(int h)
{
	int hold;
	
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	if (   (spr[h].damage > 0) )
	{
		//SoundPlayEffect( 1,3000, 800 );
		draw_damage(h);
		spr[h].hitpoints -= spr[h].damage;
		spr[h].damage = 0;
		if (spr[h].hitpoints < 1)
		{
			add_exp(spr[h].exp, h);
			spr[h].damage = 0;
			//lets kill the duck here, ha.
			check_for_kill_script(h);
			spr[h].speed = 0;
            spr[h].base_walk = -1;
			spr[h].seq = 164;
			spr[h].brain = 7;    
		}
		
		return;
	}
	
	
	if (spr[h].freeze) return;
	
	
	
	if (spr[h].seq == 0 ) 
	{
		
		if (((rand() % 12)+1) == 1 )
		{  
			hold = ((rand() % 9)+1);
			
			if (  (hold != 4) &&   (hold != 6) &&  (hold != 2) && (hold != 8) && (hold != 5))
			{
				changedir(hold,h,spr[h].base_walk);
				
			}
			else
			{
				int junk = spr[h].size;
				
				if (junk >=  100)
					junk = 18000 - (junk * 50);
				
				if (junk < 100)
					junk = 16000 + (junk * 100);
				
				
				hold = ((rand() % 4)+1);
				
				if (!playing(spr[h].last_sound)) spr[h].last_sound = 0;
				
				if (spr[h].last_sound == 0)
				{
					
					
					if (hold == 1) 
						spr[h].last_sound = SoundPlayEffect( 2,junk, 800 ,h,0);
					if (hold == 2) 
						spr[h].last_sound = SoundPlayEffect( 3,junk, 800,h ,0);
					if (hold == 3) 
						spr[h].last_sound = SoundPlayEffect( 4,junk, 800 ,h,0);
					if (hold == 4) 
						spr[h].last_sound = SoundPlayEffect( 5,junk, 800,h,0 );
					
				}
				
				spr[h].mx = 0;
				spr[h].my = 0;
				spr[h].wait = thisTickCount + (rand() % 300)+200;
				
			}
			
		} 
		else
		{
			
			if ((spr[h].mx != 0) || (spr[h].my != 0))
				
			{
				spr[h].seq = spr[h].seq_orig;
				
			}                                                                                                                                                                                                                                                                                                                          
			
		}
	}
	
	
	if (spr[h].y > (playy-k[getpic(h)].box.bottom / 4))
	{
		changedir(9,h,spr[h].base_walk);
	}         
	
	if (spr[h].x > (GFX_RES_W -k[getpic(h)].box.right-10))
	{
		changedir(1,h,spr[h].base_walk);
	}         
	
	if (spr[h].y < 10)
	{
		changedir(1,h,spr[h].base_walk);
	}         
	
	if (spr[h].x < 10) 
	{
		changedir(3,h,spr[h].base_walk);
	}         
	
	automove(h);
	
	if (check_if_move_is_legal(h) != 0)
		
	{
		changedir(autoreverse_diag(h),h,spr[h].base_walk);
	}
	
}
// end duck_brain



/**
 * Check if the sprite can pass or should be blocked
 *
 * Returns 0 = can pass, <>0 = should be blocked
 */
int check_if_move_is_legal(int u)
{
  if ((dversion >= 108) /* move_nohard is active for all movements, not just active moves */
      || (/* dversion == 107 && */ spr[u].move_active))
    if (spr[u].move_nohard == 1)
      return(0);

  if (u == 1 && in_this_base(spr[u].seq, dink_base_push))
    return(0);
	
  int hardness = 0;
	if (spr[u].moveman > 0)
	{
	  int i;
	  for (i = 1; i <= spr[u].moveman; i++)
	    {
	      hardness = get_hard(spr[u].lpx[i]-20, spr[u].lpy[i]);
	      if (hardness == 2 && spr[u].flying) 
		{
		  spr[u].moveman = 0;
		  if (dversion >= 108)
		    return 0;
		  else
		    return 2;
		}
	      if (hardness > 0)
		{
		  spr[u].x = spr[u].lpx[i-1];
		  spr[u].y = spr[u].lpy[i-1];
		  spr[u].moveman = 0;			
		  
		  if (push_active)
		    {
		      if (u == 1 && hardness != 2 && play.push_active == /*false*/0)
			{
			  if ((spr[u].dir == 2) | (spr[u].dir == 4) | (spr[u].dir == 6) | (spr[u].dir == 8))
			    {
			      //he  (dink)  is definatly pushing on something
			      play.push_active = /*true*/1;
			      play.push_dir = spr[u].dir;
			      play.push_timer = thisTickCount;
			    }
			}
		      else
			{
			  if (play.push_dir != spr[1].dir) play.push_active = /*false*/0;
			}
		    }
		  return(hardness);
		}
	    }
	}
	
	if (u == 1)  play.push_active = /*false*/0;
	return(0);
}


void move(int u, int amount, char kind,  char kindy)
{
	int mx = 0;
	int my = 0;	
	int i;
	/*bool*/int clearx;
	/*bool*/int cleary;
	clearx = /*false*/0;
	cleary = /*false*/0;
	
	for (i = 1; i <= amount; i++)
	{
		spr[u].moveman++;
		if (mx >= spr[u].mx) clearx = /*true*/1;
		if (my >= spr[u].my) clearx = /*true*/1;
		
		if ((clearx) && (cleary))
		{
			mx = 0;
			my = 0;
			clearx = /*false*/0;
			cleary = /*false*/0;
			
		}
		
		
		if (kind == '+')
		{
			if (mx < spr[u].mx)
				spr[u].x++;
			mx++;
			
		}
		if (kind == '-')
		{
			
			
			if (mx < (spr[u].mx - (spr[u].mx * 2)))
				spr[u].x--;
			mx++;
		}
		
		if (kindy == '+')
		{
			
			if (my < spr[u].my)
				spr[u].y++;
			my++;
		}
		if (kindy == '-')
		{
			
			if (my < (spr[u].my - (spr[u].my * 2)))
				spr[u].y--;
			my++;
		}
		
		spr[u].lpx[spr[u].moveman] = spr[u].x;
        spr[u].lpy[spr[u].moveman] = spr[u].y;
	}
	
	
}



void bounce_brain(int h)
{
	if (spr[h].y > (playy-k[getpic(h)].box.bottom))
	{
		spr[h].my -= (spr[h].my * 2);
	}         
	
	if (spr[h].x > (GFX_RES_W -k[getpic(h)].box.right))
	{
		spr[h].mx -= (spr[h].mx * 2);
	}         
	
	if (spr[h].y < 0)
	{
		spr[h].my -= (spr[h].my * 2);
	}         
	
	
	if (spr[h].x < 0) 
	{
		spr[h].mx -= (spr[h].mx * 2);
	}         
	
	
	spr[h].x += spr[h].mx;
	spr[h].y += spr[h].my;
	
	
}
//end bounce brain		

/* Capture the current's backbuffer game zone for screen transition */
void grab_trick(int trick)
{
  SDL_Rect src, dst;
  src.x = playl;
  src.y = 0;
  src.w = 620 - playl;
  src.h = 400;
  dst.x = dst.y = 0;
  SDL_BlitSurface(GFX_lpDDSBack, &src, GFX_lpDDSTrick, &dst);
  
  move_screen = trick;			
  move_counter = 0;
}

int did_player_cross_screen(int lock_sprite, int h)
{
  if (walk_off_screen == 1)
    return 0;
	
  int ret = 0;
  // DO MATH TO SEE IF THEY HAVE CROSSED THE SCREEN, IF SO LOAD NEW ONE
  if ((spr[h].x) < playl) 
    {
      if ((*pmap-1) >= 1 && map.loc[*pmap-1] > 0 && screenlock == 0)
	{
	  //move one map to the left
	  if (lock_sprite)
	    return 0;
	  
	  update_screen_time();
	  grab_trick(4);
	  *pmap -= 1;
	  game_load_map(map.loc[*pmap]);
	  if (map.indoor[*pmap] == 0)
	    play.last_map = *pmap;
	  
	  draw_map_game();
	  // compatibility: update Dink position *after* screen change
	  spr[h].x = 619;
	  spr[h].y = spr[h].lpy[0];
	  ret = 1;
	}
      else
	{
	  spr[h].x = playl;
	}
    }
  
  else if (spr[h].x > 619)
    {
      if ((*pmap+1) <= 24*32 && map.loc[*pmap+1] > 0 && screenlock == 0)
	{
	  //move one map to the right
	  if (lock_sprite)
	    return 0;
	  
	  update_screen_time();
	  grab_trick(6);
	  *pmap += 1;
	  game_load_map(map.loc[*pmap]);
	  if (map.indoor[*pmap] == 0)
	    play.last_map = *pmap;
	  
	  draw_map_game();
	  // compatibility: update Dink position *after* screen change
	  spr[h].x = playl;
	  spr[h].y = spr[h].lpy[0];
	  ret = 1;
	}
      else
	{
	  spr[h].x = 619;
	}
    }
  
  else if (spr[h].y < 0)
    {
      if ((*pmap-32) >= 1 && map.loc[*pmap-32] > 0 && screenlock == 0)
	{
	  //move one map up
	  if (lock_sprite)
	    return 0;

	  update_screen_time();
	  grab_trick(8);
	  *pmap -= 32;
	  game_load_map(map.loc[*pmap]);
	  if (map.indoor[*pmap] == 0)
	    play.last_map = *pmap;
	  
	  // compatibility: update Dink X position *before* screen change
	  // (shouldn't matter when though, since it's an Y motion)
	  spr[h].x = spr[h].lpx[0];
	  draw_map_game();
	  // compatibility: update Dink Y position *after* screen change
	  spr[h].y = 399;
	  ret = 1;
	}
      else
	{
	  spr[h].y = 0;
	}
    }
  
  
  else if (spr[h].y > 399)
    {
      if ((*pmap+32) <= 24*32 && map.loc[*pmap+32] > 0 && screenlock == 0)
	{
	  //move one map down
	  if (lock_sprite)
	    return 0;
	  
	  update_screen_time();
	  grab_trick(2);
	  *pmap += 32;
	  game_load_map(map.loc[*pmap]);
	  if (map.indoor[*pmap] == 0)
	    play.last_map = *pmap;
	  
	  draw_map_game();
	  // compatibility: update Dink position *after* screen change
	  spr[h].y = 0;
	  spr[h].x = spr[h].lpx[0];
	  ret = 1;
	}
      else
	{
	  spr[h].y = 399;
	}
    }
  return ret;
}


/*bool*/int run_through_tag_list_talk(int h)
{
	rect box;
	int amount, amounty;
	int i;

	for (i = 1; i <= last_sprite_created; i++)
	{
		
		if (spr[i].active) if (i != h) if (spr[i].brain != 8)
		{
			
			
			rect_copy(&box, &k[getpic(i)].hardbox);
			rect_offset(&box, spr[i].x, spr[i].y);
			
			rect_inflate(&box, 10,10);
			
			amount = 50;		
			amounty = 35;
			if (spr[h].dir == 6)
			{
				box.left -= amount;
			}
			
			if (spr[h].dir == 4)
			{
				box.right += amount;
			}
			
			
			if (spr[h].dir == 2)
			{
				box.top -= amounty;
			}
			
			if (spr[h].dir == 8)
			{
				box.bottom += amounty;
			}
			
			//		draw_box(box, 33);
			
			if (inside_box(spr[h].x, spr[h].y, box))
			{	
				//Msg("Talking to sprite %d", i);
				if (spr[i].script > 0)
				{
					//if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))
					//Msg("trying to find TALK in script %d", spr[i].script);
					if (locate(spr[i].script, "TALK")) 
					{
						kill_returning_stuff(spr[i].script);
						
						run_script(spr[i].script);
						return(/*true*/1);	
					}
					
					
				}
				
			}
			
			
		}
		
		
	}
	
	
	return(/*false*/0);
}





void make_missile(int x1, int y1, int dir, int speed, int seq, int frame, int strength)
{
	int crap = add_sprite(x1,y1,11,seq,frame);
	spr[crap].speed = speed;
	spr[crap].seq = seq;
	spr[crap].timer = 0;
	spr[crap].strength = strength;
	spr[crap].flying = /*true*/1;
	changedir(dir, crap, 430);
	
}




void missile_brain(int h, /*bool*/int repeat)
{
  rect box;
  int j;
  automove(h);
  
  *pmissle_source = h;
  int hard = check_if_move_is_legal(h);
  if (repeat && spr[h].seq == 0)
    spr[h].seq = spr[h].seq_orig;
  spr[1].hitpoints = *plife; 
  
  if (hard > 0 && hard != 2) 
    {      
      //lets check to see if they hit a sprites hardness
      if (hard > 100)
	{
	  int ii;
	  for (ii = 1; ii < last_sprite_created; ii++)
	    {
	      if (spr[ii].sp_index == hard-100)
		{
		  if (spr[ii].script > 0)
		    {
		      *pmissile_target = 1;
		      *penemy_sprite = 1;
		      
		      if (locate(spr[ii].script, "HIT"))
			{
			  kill_returning_stuff(spr[ii].script);
			  run_script(spr[ii].script);
			}
		    }
		  
		  if (spr[h].script > 0)
		    {
		      *pmissile_target = ii;
		      *penemy_sprite = 1;
		      if (locate(spr[h].script, "DAMAGE")) 
			{
			  kill_returning_stuff(spr[h].script);
			  run_script(spr[h].script);
			}
		    }
		  else
		    {
		      if (spr[h].attack_hit_sound == 0)
			SoundPlayEffect( 9,22050, 0 ,0,0);
		      else
			SoundPlayEffect( spr[h].attack_hit_sound,spr[h].attack_hit_sound_speed, 0 ,0,0);
		      
		      spr[h].active = 0;
		    }
		  
		  //run missile end	
		  return;
		}
	    }
	}
      //run missile end	
      
      if (spr[h].script > 0)
	{
	  *pmissile_target = 0;
	  if (locate(spr[h].script, "DAMAGE"))
	    run_script(spr[h].script);
	}
      else
	{
	  if (spr[h].attack_hit_sound == 0)
	    SoundPlayEffect(9, 22050, 0, 0, 0);
	  else
	    SoundPlayEffect(spr[h].attack_hit_sound,spr[h].attack_hit_sound_speed, 0, 0, 0);
	  
	  spr[h].active = 0;
	  return;
	}
    }
  
  if (spr[h].x > 1000) spr[h].active = /*false*/0;
  if (spr[h].y > 700) spr[h].active = /*false*/0;
  if (spr[h].y < -500) spr[h].active = /*false*/0;
  if (spr[h].x < -500) spr[h].active = /*false*/0;
  
  //did we hit anything that can die?
  
  for (j = 1; j <= last_sprite_created; j++)
    {
      if (spr[j].active && h != j && spr[j].nohit != 1 && spr[j].notouch == /*false*/0)
	if (spr[h].brain_parm != j && spr[h].brain_parm2!= j)
	  //if (spr[j].brain != 15) if (spr[j].brain != 11)
	  {
	    rect_copy(&box, &k[getpic(j)].hardbox);
	    rect_offset(&box, spr[j].x, spr[j].y);
	    
	    if (spr[h].range != 0)
	      rect_inflate(&box, spr[h].range,spr[h].range);
	    
	    if (debug_mode) draw_box(box, 33);
	    
	    if (inside_box(spr[h].x, spr[h].y, box))
	      {
		spr[j].notouch = /*true*/1;
		spr[j].notouch_timer = thisTickCount+100;
		spr[j].target = 1;
		*penemy_sprite = 1;
		//change later to reflect REAL target
		if (spr[h].script > 0)
		  {
		    *pmissile_target = j;
		    if (locate(spr[h].script, "DAMAGE"))
		      run_script(spr[h].script);
		  }
		else
		  {
		    if (spr[h].attack_hit_sound == 0)
		      SoundPlayEffect(9, 22050, 0, 0, 0);
		    else
		      SoundPlayEffect(spr[h].attack_hit_sound,spr[h].attack_hit_sound_speed, 0, 0,0);
		  }
		
		if (spr[j].hitpoints > 0 && spr[h].strength != 0)
		  {
		    int hit = 0;
		    if (spr[h].strength == 1)
		      hit = spr[h].strength - spr[j].defense;
		    else
		      hit = (spr[h].strength / 2)
			+ ((rand() % (spr[h].strength / 2)) + 1)
			- spr[j].defense;
		    
		    if (hit < 0)
		      hit = 0;
		    spr[j].damage += hit;
		    if (hit > 0)
		      random_blood(spr[j].x, spr[j].y-40, j);
		    spr[j].last_hit = 1;
		    //Msg("Damage done is %d..", spr[j].damage);
		  }
		
		if (spr[j].script > 0)
		  {
		    //CHANGED did = h
		    *pmissile_target = 1;
		    
		    if (locate(spr[j].script, "HIT"))
		      {
			kill_returning_stuff(spr[j].script);
			run_script(spr[j].script);
		      }
		  }
	      }
	    //run missile end	
	    
	  }
    }
}


void missile_brain_expire(int h)
{
	missile_brain(h, /*false*/0);
	if (spr[h].seq == 0) spr[h].active = 0;
	
}

void run_through_mouse_list(int h, /*bool*/int special)
{
  rect box;
  int i;

	for (i = 1; i <= last_sprite_created; i++)
	{
		
		if (spr[i].active) if (i != h) if
			((spr[i].touch_damage != 0) )
		{
			
			if (spr[i].touch_damage != -1) if (spr[h].notouch) return;
			rect_copy(&box, &k[getpic(i)].hardbox);
			rect_offset(&box, spr[i].x, spr[i].y);
			
			
			if (inside_box(spr[h].x, spr[h].y, box))
			{	
				
				if ((spr[i].touch_damage == -1) && (spr[i].script != 0))
				{
					log_info("running %d's script..", spr[i].script);
					if (locate(spr[i].script, "CLICK")) run_script(spr[i].script);
				} 
				else
				{
					if (spr[i].touch_damage == -1)
					{
						log_error("Sprites touch damage is set to -1 but there is no script set!");
					} else
					{
						//lets hurt the guy
					}
					
				}
				
				if (special) return;	
				
			}
			
			
		}
		
	}
	
	if (special) 		SoundPlayEffect(19, 22050, 0, 0,0);
	
}




void mouse_brain(int h)
{
	
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	
	int diag = 0;
	
	if (sjoy.right) diag++;
	if (sjoy.left) diag++;
	if (sjoy.down) diag++;
	if (sjoy.up) diag++;
	
	
    //*********************************PROCESS MOVEMENT							
	
	if (diag == 1)
	{
								
								if (sjoy.right)
								{
									move(h,spr[h].speed,'+','0');
									changedir(6,h,spr[h].base_walk);
								}
								
								
								if (sjoy.left) 
								{
									move(h,spr[h].speed,'-','0');
									changedir(4,h,spr[h].base_walk);
								}
								
								
								if (sjoy.down)
								{
									move(h,spr[h].speed,'0','+');
									changedir(2,h,spr[h].base_walk);
								}
								
								
								if (sjoy.up) 
								{
									move(h,spr[h].speed,'0','-');
									changedir(8,h,spr[h].base_walk);
								}
								
	}
	// ***************** DIAGONAL!!!!
	
	if (diag > 1)
	{
								
								if ( (sjoy.up) && (sjoy.left) ) 
								{
									changedir(7,h,spr[h].base_walk);
									move(h,spr[h].speed - (spr[h].speed / 3),'-','-');
									
								}
								
								if ( (sjoy.down) && (sjoy.left))
								{
									changedir(1,h,spr[h].base_walk);
									move(h,spr[h].speed - (spr[h].speed / 3),'-','+');
									
								}
								
								if ( (sjoy.down) && (sjoy.right))
								{
									changedir(3,h,spr[h].base_walk);
									move(h,spr[h].speed - (spr[h].speed / 3),'+','+');
								}
								
								
								if ( (sjoy.up) && (sjoy.right))
								{
									changedir(9,h,spr[h].base_walk);
									move(h,spr[h].speed - (spr[h].speed / 3),'+','-');
								}
								
	}
	
	
	
	
	if ( (sjoy.button[ACTION_ATTACK] == /*TRUE*/1) | (mouse1) )
	{
		
		log_info("running through mouse list..");
		run_through_mouse_list(h, /*true*/1);
		sjoy.button[ACTION_ATTACK] = /*false*/0;
		mouse1 = /*false*/0;
							 
	}
	
}

void process_bow( int h)
{
	int timetowait = 100;
	
	
	if (bow.wait < thisTickCount)
	{
		if (sjoy.right) spr[h].dir = 6;
		if (sjoy.left) spr[h].dir = 4;
		if (sjoy.up) spr[h].dir = 8;
		if (sjoy.down) spr[h].dir = 2;
	}
	
	
	
	if (sjoy.right) if (sjoy.up) 
	{
		spr[h].dir = 9;
		bow.wait = thisTickCount + timetowait;
	}
	if (sjoy.left) if (sjoy.up) 
	{
		spr[h].dir = 7;
		bow.wait = thisTickCount + timetowait;
	}
	if (sjoy.right) if (sjoy.down) 
	{
		spr[h].dir = 3;
		bow.wait = thisTickCount + timetowait;
		
	}
	if (sjoy.left) if (sjoy.down) 
	{
		spr[h].dir = 1;
		bow.wait = thisTickCount + timetowait;
		
	}
	spr[h].pseq = 100+spr[h].dir;
	
	
	if (bow.pull_wait < thisTickCount)
	{
		bow.pull_wait = thisTickCount + 10;
		if (bow.hitme) bow.time += 7;
		
		
		//	bowsound->SetFrequency(22050+(bow.time*10));
		
		if (bow.time > 500) bow.time = 500;
		spr[h].pframe = (bow.time / 100)+1;
	}
	
	
	if (!sjoy.joybitold[ACTION_ATTACK])
	{
		bow.active = /*false*/0;
		bow.last_power = bow.time;
		run_script(bow.script);
		//     bowsound->Stop();
		return;
	}
	
}



/**
 * Player
 */
void human_brain(int h)
{
  int diag;
  int crap;
  /*BOOL*/int bad;
  
  if (mode == 0)
    goto b1end;
  
  if (spr[h].move_active)
    {
      process_move(h);
      return;
    }
	
  if (spr[h].damage > 0)
    {
      draw_damage(h);
      
      *plife -= spr[h].damage;
      
      spr[h].damage = 0;
      if (*plife < 0)
	*plife = 0;
		
      int hurt = (rand() % 2)+1;
		
      if (hurt == 1)
	SoundPlayEffect(15, 25050, 2000, 0,0);
      if (hurt == 2)
	SoundPlayEffect(16, 25050, 2000, 0,0);
		
      //draw blood
    }
  
  if (play.push_active)
    {
      if (play.push_dir == 2 && !sjoy.down) 
	{
	  spr[h].nocontrol = /*false*/0;
	  play.push_active = /*false*/0;
	}
      
      if (play.push_dir == 4 && !sjoy.left) 
	{
	  spr[h].nocontrol = /*false*/0;
	  play.push_active = /*false*/0;
	}
      if (play.push_dir == 6 && !sjoy.right) 
	{
	  spr[h].nocontrol = /*false*/0;
	  play.push_active = /*false*/0;
	}
      
      if (play.push_dir == 8 && !sjoy.up) 
	{
	  spr[h].nocontrol = /*false*/0;
	  play.push_active = /*false*/0;
	}
    }
  
  if (spr[h].nocontrol)
    return;
	
  if (talk.active)
    goto freeze;
	
  if (spr[h].freeze)
    {
      //they are frozen
      if (sjoy.button[ACTION_TALK] == 1)
	{
	  //they hit the talk button while frozen, lets hurry up the process
	  int jj;
	  for (jj = 1; jj <= last_sprite_created; jj++)
	    {
	      // Msg("Checking %d, brain %d, script %d, my freeze is %d",jj, spr[jj].brain, spr[jj].script, spr[h].freeze);
	      if (spr[jj].brain == 8 && spr[jj].script == play.last_talk)
		{
		  //this sprite owns its freeze
		  spr[jj].kill_timer = 1;
		  //force the message to be over
		}
	    }
	}
      goto freeze;
    }


  //******************************  KEYS THAT CAN BE PRESSED AT ANY TIME **************
  
  if (bow.active)
    {
      //bow is active!!
      process_bow(h);
      return;
    }

  if (play.push_active && thisTickCount > play.push_timer + 600)
    {
      spr[h].seq = dink_base_push + spr[h].dir;
      spr[h].frame = 1;
      spr[h].nocontrol = /*true*/1;
      //play.push_active = /*false*/0;
      run_through_tag_list_push(h);
      
      return;
    }
  
  if ((sjoy.button[ACTION_TALK] == 1))
    {
      if (!run_through_tag_list_talk(h))
	{
	  int did_dnotalk = 0;
	  if (dversion >= 108)
	    {
	      // addition of 'not talking to anything' script
	      int sc = load_script ("dnotalk", 0, /*false*/0);
	      if (sc != 0 && locate (sc, "MAIN"))
		{
		  run_script (sc);
		  did_dnotalk = 1;
		}
	    }

	  if (did_dnotalk == 0)
	    {
	      kill_text_owned_by(h);	
	      int randy = (rand() % 6)+1;
	      if (randy == 1) say_text(_("`$I don't see anything here."), h, 0);
	      if (randy == 2) say_text(_("`$Huh?"), h, 0);
	      if (randy == 3) say_text(_("`$I'm fairly sure I can't talk to or use that."), h, 0);
	      if (randy == 4) say_text(_("`$What?"), h, 0);
	      if (randy == 5) say_text(_("`$I'm bored."), h, 0);
	      if (randy == 6) say_text(_("`$Not much happening here."), h, 0);
	    }
	}
    }
	
  if ((sjoy.button[ACTION_ATTACK] == 1) && (weapon_script != 0))
    {
      if (spr[h].base_hit > 0)
	{
	  if (locate(weapon_script, "USE"))
	    run_script(weapon_script);
	  goto b1end;
	}
    }
  
  //added AGAIN 10-19-99
  //Let's check keys for getting hit
  if (thisTickCount > but_timer && console_active == 0)
    {
      int scancode;
      for (scancode = 0; scancode < SDL_NUM_SCANCODES; scancode++)
	{ 
	  if (scancode == SDL_SCANCODE_SPACE
	      || scancode == SDL_SCANCODE_6
	      || scancode == SDL_SCANCODE_7
	      || scancode == SDL_SCANCODE_LEFT
	      || scancode == SDL_SCANCODE_UP
	      || scancode == SDL_SCANCODE_RIGHT
	      || scancode == SDL_SCANCODE_DOWN
	      || scancode == SDL_SCANCODE_RETURN
	      || scancode == SDL_SCANCODE_TAB
	      || scancode == SDL_SCANCODE_ESCAPE
	      || scancode == SDL_SCANCODE_LCTRL
	      || scancode == SDL_SCANCODE_RCTRL
	      || scancode == SDL_SCANCODE_LSHIFT
	      || scancode == SDL_SCANCODE_RSHIFT
	      || scancode == SDL_SCANCODE_LALT
	      || scancode == SDL_SCANCODE_RALT
	      || SDL_GetKeyFromScancode(scancode) == SDLK_m)
	    continue;
	  
	  char scriptname[30];
	  if (input_getscancodestate(scancode))
	    {
	      /* Get the same keycodes than the original Dink engines
		 for letters, that is, uppercase ascii rather than
		 lowercase ascii */
	      int code;
	      int keycode = SDL_GetKeyFromScancode(scancode);
	      if (keycode >= SDLK_a && keycode <= SDLK_z)
		code = 'A' + (keycode - SDLK_a);
	      else
		code = scancode;
	      
	      sprintf(scriptname, "key-%d", code);
	      but_timer = thisTickCount+200;
	      
	      int script = load_script(scriptname, 1, /*false*/0);
	      if (locate(script, "MAIN"))
		{
		  run_script(script);
		  goto b1end;
		}
	    }
	}
    }
  
  enum buttons_actions actions[5];
  enum buttons_actions actions_script[5];
  int nb_actions = 1;
  actions[0] = ACTION_MAP;
  actions_script[0] = 6;
  if (dversion >= 108)
    {
      nb_actions = 5;
      actions[1] = ACTION_BUTTON7;
      actions_script[1] = 7;
      actions[2] = ACTION_BUTTON8;
      actions_script[2] = 8;
      actions[3] = ACTION_BUTTON9;
      actions_script[3] = 9;
      actions[4] = ACTION_BUTTON10;
      actions_script[4] = 10;
    }
  int i = 0;
  for (i = 0; i < nb_actions; i++)
    {
      // button6.c, button7.c, ..., button10.c
      if (sjoy.button[actions[i]] == 1)
	{
	  char script_filename[6+2+1]; // 'button' + '7'..'10' + '\0' (no '.c')
	  sprintf(script_filename, "button%d", actions_script[i]);
	  int mycrap = load_script(script_filename, 1, /*false*/0);
	  if (locate(mycrap, "MAIN"))
	    run_script(mycrap);
	  goto b1end;
	}
    }
  
  if (magic_script != 0 && sjoy.joybit[ACTION_MAGIC])
    goto shootm;

  if (sjoy.button[ACTION_MAGIC] == 1)
    {
      if (magic_script == 0)
	{
	  if (dversion >= 108)
	    {
	      // addition of 'no magic' script
	      int sc = load_script ("dnomagic", 0, /*false*/0);
	      if (sc != 0 && locate (sc, "MAIN"))
		{
		  run_script (sc);
		  goto b1end;
		}
	    }

	  int randy = (rand() % 6)+1;
	  kill_text_owned_by(h);	
	  if (randy == 1) say_text(_("`$I don't know any magic."), h, 0);
	  if (randy == 2) say_text(_("`$I'm no wizard!"), h, 0);
	  if (randy == 3) say_text(_("`$I need to learn magic before trying this."), h, 0);
	  if (randy == 4) say_text(_("`$I'm gesturing wildly to no avail!"), h, 0);
	  if (randy == 5) say_text(_("`$Nothing happened."), h, 0);
	  if (randy == 6) say_text(_("`$Hocus pocus!"), h, 0);
	  goto b1end;
	}
		
      //player pressed 1
      //lets magiced something
shootm:	
      if (*pmagic_level >= *pmagic_cost)
	{
	  if (locate(magic_script, "USE"))
	    run_script(magic_script);
	  goto b1end;	
	} 
    }
  
  if (sjoy.button[ACTION_INVENTORY])
    {
      if (dversion >= 108)
	{
	  // addition of 'enter key/inventory' script
	  int sc = load_script ("button4", 0, /*false*/0);
	  if (sc != 0 && locate (sc, "MAIN"))
	    {
	      run_script (sc);
	      return;
	    }
	}
      
      show_inventory = 1;
      SoundPlayEffect(18, 22050,0,0,0);
      return;
    }
  
  if (sjoy.button[ACTION_MENU] == 1)
    {
      if (!showb.active && !bow.active && !talk.active)
	{
	  int sc = load_script("escape", 1000, /*false*/0);
	  if (sc != 0 && locate(sc, "MAIN"))
	    run_script(sc);
	  return;
	}
    }
  
  if (console_active == 0)
    {
      if (input_getcharstate(SDLK_b))
	ResumeMidi();
      if (input_getcharstate(SDLK_n))
	PauseMidi();
    }
  
  if (spr[h].skip > 0
      && spr[h].skip <= spr[h].skiptimer)
    {
      spr[h].skiptimer = 0;
      goto b1end;
    }
  
  
  diag = 0;
  if (sjoy.right) diag++;
  if (sjoy.left) diag++;
  if (sjoy.down) diag++;
  if (sjoy.up) diag++;

  
  //*********************************PROCESS MOVEMENT
  
  if (diag == 1)
    {
      if (sjoy.right)
	{
	  move(h,spr[h].speed,'+','0');
	  changedir(6,h,spr[h].base_walk);
	}
      
      if (sjoy.left) 
	{
	  move(h,spr[h].speed,'-','0');
	  changedir(4,h,spr[h].base_walk);
	}
      
      if (sjoy.down)
	{
	  move(h,spr[h].speed,'0','+');
	  changedir(2,h,spr[h].base_walk);
	}
      
      if (sjoy.up) 
	{
	  move(h,spr[h].speed,'0','-');
	  changedir(8,h,spr[h].base_walk);
	}
    }
  
  // ***************** DIAGONAL!!!!
  if (diag > 1 && diag < 3)
    {
      if (sjoy.up && sjoy.left)
	{
	  changedir(7,h,spr[h].base_walk);
	  move(h,spr[h].speed - (spr[h].speed / 3),'-','-');
	}
      
      if (sjoy.down && sjoy.left)
	{
	  changedir(1,h,spr[h].base_walk);
	  move(h,spr[h].speed - (spr[h].speed / 3),'-','+');
	}
      
      if (sjoy.down && sjoy.right)
	{
	  changedir(3,h,spr[h].base_walk);
	  move(h,spr[h].speed - (spr[h].speed / 3),'+','+');
	}
      
      if (sjoy.up && sjoy.right)
	{
	  changedir(9,h,spr[h].base_walk);
	  move(h,spr[h].speed - (spr[h].speed / 3),'+','-');
	}		
    }
  	
  bad = 0;
  if (sjoy.right) bad = 1;
  if (sjoy.left) bad = 1;
  if (sjoy.up) bad = 1;
  if (sjoy.down) bad = 1;
  
  if (bad)
    {
      if (spr[h].idle)
	{
	  spr[h].frame = 1;
	  spr[h].idle = /*FALSE*/0;
	}
      goto badboy;
    }
		
  if (not_in_this_base(spr[h].seq, spr[h].base_idle)) //unccoment to allow walk anim to end before idle anim to start
    {
    freeze:
      if (spr[h].dir == 1) spr[h].dir = 2;
      if (spr[h].dir == 3) spr[h].dir = 2;
      if (spr[h].dir == 7) spr[h].dir = 8;
      if (spr[h].dir == 9) spr[h].dir = 8;
      
      if (spr[h].base_idle != 0)
	changedir(spr[h].dir,h,spr[h].base_idle);
      spr[h].idle = /*TRUE*/1;   
    }
  
 badboy: 
 b1end:
  
  if (spr[h].dir == 2 || spr[h].dir == 4 || spr[h].dir == 6 || spr[h].dir == 8)
    goto smoothend;

  crap = check_if_move_is_legal(h);
  if (crap != 0)
    {
      // TODO: crap-100 may be negative...
      if (pam.sprite[crap-100].is_warp != 0)
	flub_mode = crap;
		  
      //hit something, can we move around it?
		  
      if (spr[h].seq == spr[h].base_walk + 4
	  || spr[h].seq == spr[h].base_walk + 6)
	{
	  int hardm = get_hard_play(h, spr[h].x, spr[h].y-1);
	  if (hardm == 0)
	    spr[h].y -= 1;
	}

      if (spr[h].seq == spr[h].base_walk + 8
	  || spr[h].seq == spr[h].base_walk + 2)
	{
	  int hardm = get_hard_play(h, spr[h].x-1, spr[h].y);
	  if (hardm == 0)
	    spr[h].x -= 1;
	}


      if (spr[h].seq == spr[h].base_walk + 9)
	{
	  int hardm = get_hard_play(h, spr[h].x+1, spr[h].y);
	  if (hardm == 0)
	    {  
	      spr[h].x += 1;
	      
	    }
	  else
	    {
	      int hardm = get_hard_play(h, spr[h].x+1, spr[h].y+1);
	      if (hardm == 0)
		{  
		  spr[h].x += 1;
		  spr[h].y += 1;
		}
	      else
		{
		  int hardm = get_hard_play(h, spr[h].x+1, spr[h].y+2);
		  if (hardm == 0)
		    {  
		      spr[h].x += 1;
		      spr[h].y += 2;
		    }
		  else
		    {
		      int hardm = get_hard_play(h, spr[h].x, spr[h].y-1);
		      if (hardm == 0)
			{  
			  spr[h].y -= 1;
			  
			}
		      else
			{
			  int hardm = get_hard_play(h, spr[h].x-1, spr[h].y-1);
			  if (hardm == 0)
			    {  
			      spr[h].x -= 1;
			      spr[h].y -= 1;
			    }  
			}
		    }
		}
	    }
	}
      
      if (spr[h].seq == spr[h].base_walk + 7)
	{
	  int hardm = get_hard_play(h, spr[h].x-1, spr[h].y);
	  if (hardm == 0)
	    {  
	      spr[h].x -= 1;
	    }
	  else
	    {
	      int hardm = get_hard_play(h, spr[h].x-1, spr[h].y+1);
	      if (hardm == 0)
		{  
		  spr[h].x -= 1;
		  spr[h].y += 1;
		}
	      else
		{
		  int hardm = get_hard_play(h, spr[h].x-1, spr[h].y+2);
		  if (hardm == 0)
		    {  
		      spr[h].x -= 1;
		      spr[h].y += 2;
		    }
		  else
		    {
		      int hardm = get_hard_play(h, spr[h].x, spr[h].y-1);
		      if (hardm == 0)
			{  				
			  spr[h].y -= 1;
			}
		      else
			{
			  int hardm = get_hard_play(h, spr[h].x+1, spr[h].y-1);
			  if (hardm == 0)
			    {  				
			      spr[h].x += 1;
			      spr[h].y -= 1;
			    }
			}
		    }
		}
	    }
	}
      
      if (spr[h].seq == spr[h].base_walk + 1)
	{
	  int hardm = get_hard_play(h, spr[h].x-1, spr[h].y);
	  if (hardm == 0)
	    {  
	      spr[h].x -= 1;
	    }
	  else
	    {
	      int hardm = get_hard_play(h, spr[h].x-1, spr[h].y-1);
	      if (hardm == 0)
		{  
		  spr[h].x -= 1;
		  spr[h].y -= 1;
		}
	      else
		{
		  int hardm = get_hard_play(h, spr[h].x-1, spr[h].y-2);
		  if (hardm == 0)
		    {  
		      spr[h].x -= 1;
		      spr[h].y -= 2;
		    }
		  else
		    {
		      int hardm = get_hard_play(h, spr[h].x, spr[h].y+1);
		      if (hardm == 0)
			{  
			  spr[h].y += 1;
			}
		      else
			{
			  int hardm = get_hard_play(h, spr[h].x+1, spr[h].y+1);
			  if (hardm == 0)
			    {  
			      spr[h].x += 1;
			      spr[h].y += 1;
			    } 
			}
		    }
		}
	    }
	}
		  
      if (spr[h].seq == spr[h].base_walk + 3)
	{
	  int hardm = get_hard_play(h, spr[h].x+1, spr[h].y);
	  if (hardm == 0)
	    {  
	      spr[h].x += 1;
	    }
	  else
	    {
	      int hardm = get_hard_play(h, spr[h].x+1, spr[h].y-1);
	      if (hardm == 0)
		{  
		  spr[h].x += 1;
		  spr[h].y -= 1;
		}
	      else
		{
		  int hardm = get_hard_play(h, spr[h].x+1, spr[h].y-2);
		  if (hardm == 0)
		    {  
		      spr[h].x += 1;
		      spr[h].y -= 2;
		    }
		  else
		    {
		      int hardm = get_hard_play(h, spr[h].x, spr[h].y+1);
		      if (hardm == 0)
			{  
			  spr[h].y += 1;
			}
		      else
			{
			  int hardm = get_hard_play(h, spr[h].x-1, spr[h].y+1);
			  if (hardm == 0)
			    {  
			      spr[h].x -= 1;
			      spr[h].y += 1;
			    }
			}
		    }
		}
	    }
	}
    }

 smoothend:
  ;
}

/**
 * Screen transition with scrolling effect.
 * Returns 0 when transition is finished.
 */
/*bool*/int transition()
{
  SDL_Rect src, dst;
  
  //we need to do our fancy screen transition
  int dumb = fps_final * 2;
  
  if (0) //DEBUG
    {
      // Make the transition last ~5 seconds
      if (move_screen == 4 || move_screen == 6)
	dumb = 600 / (fps_final * 5);
      else if (move_screen == 8 || move_screen == 2)
	dumb = 400 / (fps_final * 5);
    }
  
  move_counter += dumb;
  
  
  if (move_screen == 4)
    {
      if (move_counter > 598)
	move_counter = 598;
      
      src.x = 0;
      src.y = 0;
      src.w = 600 - move_counter;
      src.h = 400;
      dst.x = 20 + move_counter;
      dst.y = 0;
      SDL_BlitSurface(GFX_lpDDSTrick, &src, GFX_lpDDSBack, &dst);
      
      src.x = 600 - move_counter;
      src.y = 0;
      src.w = move_counter;
      src.h = 400;
      dst.x = 20;
      dst.y = 0;
      SDL_BlitSurface(GFX_lpDDSTrick2, &src, GFX_lpDDSBack, &dst);
      
      if (move_counter >= 595)
	{
	  transition_in_progress = 0;
	  move_screen = 0;
	  move_counter = 0;
	  //draw_map();
	  return /*false*/0;
	}
      
      return /*true*/1;
    }

  
  if (move_screen == 6)
    {
      if (move_counter > 598)
	move_counter = 598;
      
      src.x = move_counter;
      src.y = 0;
      src.w = 600 - move_counter;
      src.h = 400;
      dst.x = 20;
      dst.y = 0;
      SDL_BlitSurface(GFX_lpDDSTrick, &src, GFX_lpDDSBack, &dst);
		
      src.x = 0;
      src.y = 0;
      src.w = move_counter;
      src.h = 400;
      dst.x = 620 - move_counter;
      dst.y = 0;
      SDL_BlitSurface(GFX_lpDDSTrick2, &src, GFX_lpDDSBack, &dst);
      
      if (move_counter >= 595)
	{
	  transition_in_progress = 0;
	  move_screen = 0;
	  move_counter = 0;
	  //draw_map();
	  return /*false*/0;
	}
      
      return /*true*/1;
    }
  

  if (move_screen == 8)
    {
      if (move_counter > 398)
	move_counter = 398;
      
      src.x = 0;
      src.y = 0;
      src.w = 600;
      src.h = 400 - move_counter;
      dst.x = 20;
      dst.y = move_counter;
      SDL_BlitSurface(GFX_lpDDSTrick, &src, GFX_lpDDSBack, &dst);
      
      src.x = 0;
      src.y = 400 - move_counter;
      src.w = 600;
      src.h = move_counter;
      dst.x = 20;
      dst.y = 0;
      SDL_BlitSurface(GFX_lpDDSTrick2, &src, GFX_lpDDSBack, &dst);
      
      if (move_counter >= 398)
	{
	  transition_in_progress = 0;
	  move_screen = 0;
	  move_counter = 0;
	  //draw_map();
	  return /*false*/0;
	}
      
      return /*true*/1;
    }
  

  if (move_screen == 2)
    {
      if (move_counter > 398)
	move_counter = 398;
      
      src.x = 0;
      src.y = move_counter;
      src.w = 600;
      src.h = 400 - move_counter;
      dst.x = 20;
      dst.y = 0;
      SDL_BlitSurface(GFX_lpDDSTrick, &src, GFX_lpDDSBack, &dst);
      
      src.x = 0;
      src.y = 0;
      src.w = 600;
      src.h = move_counter;
      dst.x = 20;
      dst.y = 400 - move_counter;
      SDL_BlitSurface(GFX_lpDDSTrick2, &src, GFX_lpDDSBack, &dst);
      
      if (move_counter >= 398)
	{
	  transition_in_progress = 0;
	  move_screen = 0;
	  move_counter = 0;
	  //draw_map();
	  return /*false*/0;
	}
      
      return /*true*/1;
    }
  
  return /*false*/0;
}


/**
 * Trigger a warp (teleport)
 * block: the warp editor sprite
 */
int special_block(int block)
{
  if (pam.sprite[block].is_warp == 1)
    {
      //they touched a warp
      if (pam.sprite[block].sound == 0)
        SoundPlayEffect(7, 12000, 0, 0, 0);
      else
        SoundPlayEffect(pam.sprite[block].sound, 22050, 0, 0, 0);
      
      if (pam.sprite[block].parm_seq != 0)
        {
          // we'll also play an animation here
          int sprite = find_sprite(block);
          if (sprite > 0)
            {
              spr[sprite].seq = pam.sprite[block].parm_seq;
              process_warp = block;
            }
          return 1;
        }
      process_warp = block;
      return 1; // redraw screen with fade
    }
  return 0;
}
	
/* fade_down() - fade to black */
void CyclePalette()
{
  if (!truecolor)
    {
      SDL_Color palette[256];
      int kk;

      gfx_palette_get_phys(palette);

      for (kk = 1; kk < 256; kk++) /* skipping index 0 because it's
				      already (and always) black ;) */
	{
	  if (dversion >= 108)
	    {
	      /* Use time-based rather than absolute increments;
		 avoids incomplete fades on slow computers */
	      /* dt / 2 == dt * 256 / 512 == complete fade in 512ms */
	      int lValue = (thisTickCount - lastTickCount) / 2;
	      if (palette[kk].b != 0)
		{
		  if (palette[kk].b > lValue)
		    palette[kk].b -= lValue;
		  else
		    palette[kk].b = 0;
		}
	      if (palette[kk].g != 0)
		{
		  if (palette[kk].g > lValue)
		    palette[kk].g -= lValue;
		  else
		    palette[kk].g = 0;
		}
	      if (palette[kk].r != 0)
		{
		  if (palette[kk].r > lValue)
		    palette[kk].r -= lValue;
		  else
		    palette[kk].r = 0;
		}
	    }
	  else
	    {
	      if (palette[kk].b != 0)
		{
		  if (palette[kk].b > 10)
		    palette[kk].b -= 10;
		  else
		    palette[kk].b--;
		}
	      if (palette[kk].g != 0)
		{
		  if (palette[kk].g > 10)
		    palette[kk].g -= 10;
		  else
		    palette[kk].g--;
		}
	      if (palette[kk].r != 0)
		{
		  if (palette[kk].r > 10)
		    palette[kk].r -= 10;
		  else
		    palette[kk].r--;
		}
	    }
	}
  
      gfx_palette_set_phys(palette);
    }
  else
    {
      /* truecolor */
      if (truecolor_fade_lasttick == -1)
	{
	  truecolor_fade_lasttick = game_GetTicks();
	  //truecolor_fade_brightness -= 256*.3;
	}
      else
	{
	  int delta = game_GetTicks() - truecolor_fade_lasttick;
	  /* Complete fade in 400ms */
	  truecolor_fade_lasttick = game_GetTicks();
	  truecolor_fade_brightness -= delta * 256 / 400.0;
	}
      if (truecolor_fade_brightness <= 0)
	truecolor_fade_brightness = 0;
      
    }

  if (process_downcycle) 
    {
      if (thisTickCount > cycle_clock)
	{
	  process_downcycle = /*false*/0;
	  truecolor_fade_lasttick = -1;
				
	  if (cycle_script != 0)
	    {
	      int junk = cycle_script;
	      cycle_script = 0;	
	      run_script(junk);
	    }
	}
    }
}
	
/* fade_up() */	
void up_cycle(void)
{
  int donethistime = 1;

  if (!truecolor)
    {
      SDL_Color palette[256];
      int kk;

      gfx_palette_get_phys(palette);

      for (kk = 1; kk < 256; kk++)
	{
	  int tmp = -1;

	  tmp = palette[kk].b; // 'int' to avoid 'char' overflow
	  if (tmp != GFX_real_pal[kk].b)
	    {
	      donethistime = 0;
	      if (tmp > 246)
		tmp++;
	      else
		tmp += 10;
	    }
	  if (tmp > GFX_real_pal[kk].b)
	    tmp = GFX_real_pal[kk].b;
	  palette[kk].b = tmp;
      
	  tmp = palette[kk].g; // 'int' to avoid 'char' overflow
	  if (tmp != GFX_real_pal[kk].g)
	    {
	      donethistime = 0;
	      if (tmp > 246)
		tmp++;
	      else
		tmp += 10;
	    }
	  if (tmp > GFX_real_pal[kk].g)
	    tmp = GFX_real_pal[kk].g;
	  palette[kk].g = tmp;
      
	  tmp = palette[kk].r; // 'int' to avoid 'char' overflow
	  if (tmp != GFX_real_pal[kk].r)
	    {
	      donethistime = 0;
	      if (tmp > 246)
		tmp++;
	      else
		tmp += 10;
	    }
	  if (tmp > GFX_real_pal[kk].r)
	    tmp = GFX_real_pal[kk].r;
	  palette[kk].r = tmp;
	}
  
      gfx_palette_set_phys(palette);
    }
  else
    {
      /* truecolor */
      donethistime = 0;
      if (truecolor_fade_lasttick == -1)
	{
	  truecolor_fade_lasttick = game_GetTicks();
	  //truecolor_fade_brightness += 256*.3;
	}
      else
	{
	  int delta = game_GetTicks() - truecolor_fade_lasttick;
	  /* Complete fade in 400ms */
	  truecolor_fade_lasttick = game_GetTicks();
	  truecolor_fade_brightness += delta * 256 / 400.0;
	}
      if (truecolor_fade_brightness >= 256)
	{
	  truecolor_fade_brightness = 256;
	  donethistime = 1;
	}
    }
		
  if (process_upcycle)
    if (donethistime == 1)
      {
	process_upcycle = 0;
	truecolor_fade_lasttick = -1;
	
	if (cycle_script != 0)
	  {
	    int junk = cycle_script;
	    cycle_script = 0;	
	    run_script(junk);
	  }
      }
}


void draw_box(rect box, int color)
{
/*   DDBLTFX     ddbltfx; */
  
/*   ddbltfx.dwSize = sizeof(ddbltfx); */
/*   ddbltfx.dwFillColor = color; */
  
/*   ddrval = lpDDSBack->Blt(&box ,NULL, NULL, DDBLT_COLORFILL| DDBLT_WAIT, &ddbltfx); */
  // GFX
  {
    SDL_Rect dst;
    dst.x = box.left; dst.y = box.top;
    dst.w = box.right - box.left;
    dst.h = box.bottom - box.top;
    SDL_FillRect(GFX_lpDDSBack, &dst, color);
  }
}


/**
 * Check which sprites are affected by an attack from 'h', the
 * attacker.
 */
	void run_through_tag_list(int h, int strength)
	{
		rect box;
		int amount, amounty;
		int i;

		for (i = 1; i <= last_sprite_created; i++)
		{
			if (spr[i].active) if (i != h) if
				(! ( (spr[i].nohit == 1) && (spr[i].script == 0)) )
			{
				
				rect_copy(&box, &k[getpic(i)].hardbox);
				rect_offset(&box, spr[i].x, spr[i].y);
				
				//InflateRect(&box, 10,10);
				
				box.right += 5;
				box.left -= 5;
				box.top -= 5;
				box.bottom += 10;
				if (spr[h].range == 0)		
					amount = 28; else amount = spr[h].range;
				
				if (spr[h].range == 0)		
					
					amounty = 36; else amounty = (spr[h].range + (spr[h].range / 6));
				
				int range_amount = spr[h].range / 8;
				
				if (spr[h].dir == 6)
				{
					box.top -= 10;
					box.bottom += 10;
					if (spr[h].range != 0) box.top -= range_amount;
					if (spr[h].range != 0) box.bottom += range_amount;
					
					
					box.left -= amount;
				}
				
				if (spr[h].dir == 4)
				{
					box.right += amount;
					
					box.top -= 10;
					box.bottom += 10;
					if (spr[h].range != 0) box.top -= range_amount;
					if (spr[h].range != 0) box.bottom += range_amount;
					
				}
				
				
				if (spr[h].dir == 2)
				{
					box.right += 10;
					box.left -= 10;
					box.top -= amounty;
					
					if (spr[h].range != 0) box.right += range_amount;
					if (spr[h].range != 0) box.left -= range_amount;
					
				}
				
				if (spr[h].dir == 8)
				{
					box.right += 10;
					box.left -= 10;
					box.bottom += amounty;
					
					if (spr[h].range != 0) box.right += range_amount;
					if (spr[h].range != 0) box.right -= range_amount;
					
				}
				
				if (debug_mode) draw_box(box, 33);
				
				if (inside_box(spr[h].x, spr[h].y, box))
				{	
					// addition for fixing missle_source problems
					if (dversion >= 108)
					  *pmissle_source = h;

					if (spr[i].nohit == 1)
					{
						if (spr[i].script > 0)
						{
							//if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))
							*penemy_sprite = h;
							
							if (  (spr[i].base_attack != -1) || (spr[i].touch_damage > 0))
								spr[i].target = h;   
							
							if (locate(spr[i].script, "HIT"))
							{
								kill_returning_stuff(spr[i].script);
								run_script(spr[i].script);
							}
							
							
						}
						
						
					} else
					{
						//hit this personb/thing
						if (spr[h].attack_hit_sound == 0)
						{
							SoundPlayEffect( 9,22050, 0 ,0,0);
						} else
						{
							SoundPlayEffect( spr[h].attack_hit_sound,spr[h].attack_hit_sound_speed, 0 ,0,0);
						}
						if (  (spr[i].base_attack != -1) || (spr[i].touch_damage > 0))
							spr[i].target = h;   
						if (spr[h].strength == 0)
						{
							
						} else
						{
							if (  (spr[i].hitpoints > 0) || (i == 1) )
							{
								
								spr[i].last_hit = h; 
								if ( hurt_thing(i, (spr[h].strength / 2) + ((rand() % ((spr[h].strength+1) / 2))+1), 0) > 0)
									random_blood(spr[i].x, spr[i].y-40, i);
							}
							
						}
						if (spr[i].script > 0)
						{
							//if (  (spr[i].brain == 0) | (spr[i].brain == 5) | (spr[i].brain == 6) | (spr[i].brain == 7))
							spr[i].last_hit = h;    
							*penemy_sprite = h;
							if (  (spr[i].base_attack != -1) || (spr[i].touch_damage > 0))
								spr[i].target = h;   
							
							if (locate(spr[i].script, "HIT"))
							{
								kill_returning_stuff(spr[i].script);
								run_script(spr[i].script);
							}
							
						}
						
					}
					
				}
				
		}
		
	}
	
}



void run_through_tag_list_push(int h)
{
	rect box;
	int i;

	for (i = 1; i <= last_sprite_created; i++)
	{
		if (spr[i].active) if (i != h) if
			((spr[i].script != 0) )
		{
			
			rect_copy(&box, &k[getpic(i)].hardbox);
			rect_offset(&box, spr[i].x, spr[i].y);
			
			//InflateRect(&box, 10,10);
			
			box.right += 2;
			box.left -= 2;
			box.top -= 2;
			box.bottom += 2;
			//draw_box(box, 33);
			
			if (inside_box(spr[h].x, spr[h].y, box))
			{	
				if (locate(spr[i].script, "PUSH")) run_script(spr[i].script);
			}
			
		}
		
	}
	
}




void run_through_touch_damage_list(int h)
{
	rect box;
	int i;

	for (i = 1; i <= last_sprite_created; i++)
	{
		if (spr[i].active) if (i != h) if
			((spr[i].touch_damage != 0) )
		{
			
			if (spr[i].touch_damage != -1) if (spr[h].notouch) return;
			rect_copy(&box, &k[getpic(i)].hardbox);
			rect_offset(&box, spr[i].x, spr[i].y);
			
			//InflateRect(&box, 10,10);
			
			box.right += 2;
			box.left -= 2;
			box.top -= 2;
			box.bottom += 2;
			if (debug_mode)		
				draw_box(box, 33);
			
			
			if (inside_box(spr[h].x, spr[h].y, box))
			{	
				
				if ((spr[i].touch_damage == -1) && (spr[i].script != 0))
				{
					if (locate(spr[i].script, "TOUCH")) run_script(spr[i].script);
				} else
				{
					if (spr[i].touch_damage == -1)
					{
						log_error("Sprites touch damage is set to -1 but there is no script set!");
					} else
					{
						//lets hurt the guy
						
						spr[h].notouch = /*true*/1;
						spr[h].notouch_timer = thisTickCount+400;
						spr[h].last_hit = i;
						if (spr[i].script != 0)
							if (locate(spr[i].script, "TOUCH")) run_script(spr[i].script);
							if (hurt_thing(h, spr[i].touch_damage, 0) > 0)
								random_blood(spr[h].x, spr[h].y-40, h);
							
							
					}
					
				}
				
				
				
			}
			
			
		}
		
	}
	
}




void process_warp_man(void)
{
  int sprite = find_sprite(process_warp);
  
  /* warp sprite doesn't exist (e.g. merged background sprite), or
     warp anim is finished */
  /* Cf. http://www.dinknetwork.com/forum.cgi?MID=168476 */
  if (sprite == 0 || spr[sprite].seq == 0)
    {
      process_count++;
      CyclePalette();
      /* Wait 5 CyclePalette before blanking the screen and
	 warp. Truecolor more: the fade algorithm is a bit different
	 and requires a few more cycles to get the same effect; unlike
	 v1.08, we don't wait for a full fadedown, which is long and
	 can be not very smooth on old computers. */
      if ((!truecolor && process_count > 5)
	  || (truecolor && truecolor_fade_brightness <= 180))
	{
	  SDL_FillRect(GFX_lpDDSBack, NULL,
		       SDL_MapRGB(GFX_lpDDSBack->format, 0, 0, 0));
	  flip_it();
	  
	  process_count = 0;
	  int block = process_warp;
	  update_screen_time();
	  spr[1].x = pam.sprite[block].warp_x;
	  spr[1].y = pam.sprite[block].warp_y;
	  *pmap = pam.sprite[block].warp_map;	

	  // update map indicator
	  if (map.indoor[pam.sprite[block].warp_map] == 0)
	    play.last_map = pam.sprite[block].warp_map;
	  
	  game_load_map(map.loc[pam.sprite[block].warp_map]);
	  draw_map_game();
	  
	  process_upcycle = 1;
	  process_warp = 0;
	}
    }
  else /* warp anim didn't finish yet */
    {
      process_count = 0;		
    }
}

void one_time_brain(int h)
{
	
	//goes once then draws last frame to background
	
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	if (spr[h].follow > 0)
	{
		process_follow(h);
	}
	
	
	if (spr[h].seq == 0)
	{
	  draw_sprite_game(GFX_lpDDSTwo, h);
		spr[h].active = /*false*/0;			
		return;
	}
	
	changedir(spr[h].dir,h,-1);
	automove(h);
	
}

void one_time_brain_for_real(int h)
{
	
	if (spr[h].move_active) 
	{
		process_move(h);
	}
	
	
	if (spr[h].follow > 0)
	{
		process_follow(h);
	}
	
	
	if (spr[h].seq == 0)
	{
		
		spr[h].active = /*false*/0;			
		return;
	}
	if (spr[h].dir > 0)
	{
		changedir(spr[h].dir,h,-1);
		automove(h);
	}
}


void scale_brain(int h)
{
	
	if (spr[h].size == spr[h].brain_parm)
	{
		spr[h].active = /*false*/0;
		
		
		return;
	}
	
	int num = 5 * (base_timing / 4);
	
	
	
	if (spr[h].size > spr[h].brain_parm)
	{
		if (spr[h].size - num < spr[h].brain_parm) num = spr[h].size - spr[h].brain_parm;
		spr[h].size -= num;
	}
	
	if (spr[h].size < spr[h].brain_parm) 
	{
		if (spr[h].size + num > spr[h].brain_parm) num = spr[h].brain_parm - spr[h].size;   
		spr[h].size += num;
	}
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	
	
	if (spr[h].dir > 0)
	{
		changedir(spr[h].dir,h,-1);
		automove(h);
	}
	
	
}



void repeat_brain(int h)
{
	
	if (spr[h].move_active) 
	{
		process_move(h);
		//		return;
	}
	
	
	if (spr[h].seq_orig == 0) if (spr[h].sp_index != 0) 
	{
		spr[h].seq_orig = pam.sprite[spr[h].sp_index].seq;
		spr[h].frame = pam.sprite[spr[h].sp_index].frame;
		spr[h].wait = 0;
		
		//pam.sprite[spr[h].sp_index].frame;
		
	}
	   
	   if (spr[h].seq == 0) spr[h].seq = spr[h].seq_orig;
	   
}


void text_brain(int h)
{
	
	
	
	if (  (spr[h].damage == -1) && (spr[h].owner != 1000))
	{
		
		if (spr[spr[h].owner].active == /*false*/0)
		{
			//msg("Killing text brain %d, because owner %d is dead.",h, spr[h].owner);
			spr[h].active = /*false*/0;
			return;
		}
		
		//give this text the cords from it's owner sprite
		spr[h].x = spr[spr[h].owner].x - spr[h].strength;
		
		
		spr[h].y = spr[spr[h].owner].y - spr[h].defense;
		
		if (spr[h].x < 1) spr[h].x = 1;
		
		if (spr[h].y < 1) spr[h].y = 1;
		
		
	} else
	{
		//Msg("automoving %d.. ", h);
		
		if (spr[h].move_active) 
		{
			process_move(h);
			return;
		}
		
		
		automove(h);
	}
	
}


void button_brain(int h )
{
	rect box;
	if (spr[h].move_active) 
	{
		process_move(h);
		return;
	}
	
	
	if (spr[h].script == 0) return;
	
	rect_copy(&box, &k[getpic(h)].hardbox);
	rect_offset(&box, spr[h].x, spr[h].y);
	
	if (spr[h].brain_parm == 0)
	{
		if (inside_box(spr[1].x, spr[1].y, box))
		{	
			spr[h].brain_parm = 1;
			
			if (locate(spr[h].script, "BUTTONON")) 
			{
				run_script(spr[h].script);
				
				return;
			}
			
		}
		
	}
	else
	{
		if (!inside_box(spr[1].x, spr[1].y, box))
		{	
			spr[h].brain_parm = 0;
			
			if (locate(spr[h].script, "BUTTONOFF")) 
			{
				
				run_script(spr[h].script);
				return;
			}
			
		}
		
	}
	
	
}

void process_show_bmp( void )
{
  // We could disable this Blit (work is already done in show_bmp())
  // but we want to display the shiny mark on the map below. Besides,
  // after show_bmp(), other parts of the code drew sprites on
  // lpDDSBack, so we need to regenerate it anyway.
  SDL_BlitSurface(GFX_lpDDSTrick, NULL, GFX_lpDDSBack, NULL);
  
  if (showb.showdot)
    {
      //let's display a nice dot to mark where they are on the map
      int x = play.last_map - 1;
      int mseq = 165;
      
      showb.picframe++;
      if (showb.picframe > seq[mseq].len) showb.picframe = 1;
      int mframe = showb.picframe;
      
      SDL_Rect dst;
      // convert map# to a (x,y) position on a FreeDinkedit minimap
      dst.x = (x % 32) * 20;
      dst.y = (x / 32) * 20;
      SDL_BlitSurface(GFX_k[seq[mseq].frame[mframe]].k, NULL, GFX_lpDDSBack, &dst);
    }
  
  
  if ((sjoy.button[ACTION_ATTACK])
      || (sjoy.button[ACTION_TALK])
      || (sjoy.button[ACTION_MAGIC])
      || (sjoy.button[ACTION_INVENTORY])
      || (sjoy.button[ACTION_MENU])
      || (sjoy.button[ACTION_MAP]))
    {
      showb.active = /*false*/0;
      if (showb.script != 0)
	run_script(showb.script);
      showb.stime = thisTickCount+2000;
      but_timer = thisTickCount + 200;
      
      int sprite = say_text_xy("", 1, 440, 0);								
      spr[sprite].noclip = 1;
      
      
      // Return to canonical game palette
      gfx_palette_set_phys(GFX_real_pal);
      // The main flip_it() will be called, skip it - lpDDSBack is
      // not matching the palette anymore, it needs to be redrawn
      // first.
      abort_this_flip = /*true*/1;
    }
}

void drawscreenlock( void )
{
/*   HRESULT     ddrval; */
  
/*  loop: */
  //draw the screenlock icon
/*   ddrval = lpDDSBack->BltFast(0, 0, k[seq[423].frame[9]].k, */
/* 			      &k[seq[423].frame[9]].box  , DDBLTFAST_NOCOLORKEY  ); */
/*   if (ddrval == DDERR_WASSTILLDRAWING ) goto loop; */
  //if (ddrval != DD_OK) dderror(ddrval);
  // GFX
  gfx_blit_nocolorkey(GFX_k[seq[423].frame[9]].k, NULL, GFX_lpDDSBack, NULL);
  
/*  loop2: */
  //draw the screenlock icon
/*   ddrval = lpDDSBack->BltFast(620, 0, k[seq[423].frame[10]].k, */
/* 			      &k[seq[423].frame[10]].box  , DDBLTFAST_NOCOLORKEY  ); */
/*   if (ddrval == DDERR_WASSTILLDRAWING ) goto loop2; */
  // if (ddrval != DD_OK) dderror(ddrval);
  // GFX
  {
    SDL_Rect dst = {620, 0};
    gfx_blit_nocolorkey(GFX_k[seq[423].frame[10]].k, NULL, GFX_lpDDSBack, &dst);
  }
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

  if (SDL_GetModState()&KMOD_LALT) {
    freedink_input_global_shortcuts(ev);
  } else if (console_active) {
    dinkc_console_process_key(ev);
  } else {
    // update virtual keyboard
    // doesn't take above events into account
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
