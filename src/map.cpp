/**
 * Map - group of screen references (dink.dat)

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2005, 2007, 2008, 2009, 2010, 2012, 2014, 2015  Sylvain Beucler

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

#include "map.h"

#include "paths.h"
#include "io_util.h"

/* dink.dat */
struct map_info map;

char current_dat[50] = "dink.dat";
char current_map[50] = "map.dat";
char current_hard[50] = "hard.dat";

/**
 * Load dink.dat to specified memory buffer
 */
int map_new(char* path, struct map_info *mymap)
{
  FILE *f = NULL;

  f = paths_dmodfile_fopen(path, "rb");
  if (!f)
    return -1;

  /* Portably load struct map_info from disk */
  int i = 0;
  fseek(f, 20, SEEK_CUR); // unused 'name' field
  for (i = 0; i < 769; i++)
    mymap->loc[i]    = read_lsb_int(f);
  for (i = 0; i < 769; i++)
    mymap->music[i]  = read_lsb_int(f);
  for (i = 0; i < 769; i++)
    mymap->indoor[i] = read_lsb_int(f);
  fseek(f, 2240, SEEK_CUR); // unused space

  fclose(f);

  memset(&map.ts_loc_mem, 0, sizeof(map.ts_loc_mem));

  return 0;
}

/**
 * Load dink.dat, an offsets index to screens stored in map.dat, with
 * some metadata (midi #, indoor/outdoor)
 */
void map_load(void)
{
  map_new(current_dat, &map);
}


/**
 * Save dink.dat (index of map offsets + midi# + indoor/outdoor)
 * Only used by the editor
 */
void map_save(void)
{
  FILE *f = paths_dmodfile_fopen(current_dat, "wb");
  if (f == NULL)
    {
      perror("Cannot save dink.dat");
      return;
    }
  
  /* Portably dump struct map_info to disk */
  int i = 0;
  char name[20] = "Smallwood";
  fwrite(name, 20, 1, f);
  for (i = 0; i < 769; i++)
    write_lsb_int(map.loc[i],    f);
  for (i = 0; i < 769; i++)
    write_lsb_int(map.music[i],  f);
  for (i = 0; i < 769; i++)
    write_lsb_int(map.indoor[i], f);
  fseek(f, 2240, SEEK_CUR); // unused field

  fclose(f);
}
