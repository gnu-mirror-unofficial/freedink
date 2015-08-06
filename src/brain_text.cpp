#include "brain.h"
#include "live_sprites_manager.h"

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
