#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "brain.h"
#include "live_sprites_manager.h"

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
