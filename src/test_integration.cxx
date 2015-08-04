/**
 * FreeDink test suite

 * Copyright (C) 2005, 2014, 2015  Sylvain Beucler

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

/**
 * Currently the testsuite is grossly in the same directory as the
 * code.  The reason is that there's no existing test suite, and that
 * we need to write tests with minimal changes to the code to assess
 * how the code _currently_ works.  In a second step we'll modularize
 * FreeDink more so that tests can target independent units of code -
 * and check if we broke anything in the process with the test suite
 * :)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cxxtest/TestSuite.h>

#include "dinkc.h"
#include "dinkc_bindings.h"
#include "game_engine.h"
#include "map.h"
#include "screen.h"
#include "dinkvar.h"
#include "freedink.h"
#include "paths.h"

/* test headers */
void dc_fade_down(int script, int* yield, int* preturnint);
void dc_fade_up(int script, int* yield, int* preturnint);

class TestIntegration : public CxxTest::TestSuite {
public:

  /* fade_up() is prioritary over fade_down(),
     post-fade callback script is still overwritten
     
     - broken during 108 merge of truecolor fade
     
     - half-fixed following:
     http://lists.gnu.org/archive/html/bug-freedink/2009-08/msg00042.html
     6b6bef615c4aae45206fa98bb8b4bfea96eb0f3b
     "Give priority to fade_up() over fade_down() - fix SoB intro in truecolor mode"
     -> in Pilgrim Quest, not Stone of Balance
     
     - 2nd half-fixed following:
     http://www.dinknetwork.com/forum.cgi?MID=189461#189461 "FreeDink-specific"
     http://www.dinknetwork.com/forum.cgi?MID=107994
     a54fb13973e6b733e594766f981b724436218061
  */
  void test_dinkc_concurrent_fades() {
    int yield, returnint;
    int script_id1 = ts_script_init("fade1", strdup(""));
    int script_id2 = ts_script_init("fade2", strdup(""));
    
    dinkc_init();
    process_upcycle = process_downcycle = 0;
    dc_fade_down(script_id1, &yield, &returnint);
    dc_fade_up(script_id2, &yield, &returnint);
    // callback set to last script that fade_xxx()'d
    TS_ASSERT_EQUALS(cycle_script, script_id2);
    // on fade_up/fade_down conflict, cancel fade effect
    TS_ASSERT_EQUALS(process_downcycle, 0);
    // TODO: in 108, flip_it would basically cancel the fade effect
    // FreeDink just forces up_cycle=1 and let it finish
    //TS_ASSERT_EQUALS(process_upcycle, 0);
    
    process_upcycle = process_downcycle = 0;
    dc_fade_up(script_id1, &yield, &returnint);
    dc_fade_down(script_id2, &yield, &returnint);
    // callback set to last script that fade_xxx()'d
    TS_ASSERT_EQUALS(cycle_script, script_id2);
    // on fade_up/fade_down conflict, cancel fade effect
    TS_ASSERT_EQUALS(process_downcycle, 0);
    // TODO: in 108, flip_it would basically cancel the fade effect
    // FreeDink just forces up_cycle=1 and let it finish
    //TS_ASSERT_EQUALS(process_upcycle, 0);
    
    kill_script(script_id1);
    kill_script(script_id2);
  }

  // See also http://www.dinknetwork.com/forum.cgi?MID=186069#186263
  void test_integration_player_position_is_updated_after_screen_is_loaded() {
    //SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    ts_paths_init();
    memset(&map.ts_loc_mem, 0, sizeof(map.ts_loc_mem));
    
    dinkc_init();
    make_int("&tsx", 123451, DINKC_GLOBAL_SCOPE, 0);
    make_int("&tsy", 123452, DINKC_GLOBAL_SCOPE, 0);
    int tsx_id = lookup_var("&tsx", DINKC_GLOBAL_SCOPE);
    int tsy_id = lookup_var("&tsy", DINKC_GLOBAL_SCOPE);
    const char* screen_main_code =
      "void main(void) {\n"
      "  &tsx = sp_x(1, -1);\n"
      "  &tsy = sp_y(1, -1);\n"
      "}";
    int screen_script_id = ts_script_init("ts_pos_check", strdup(screen_main_code));
    rinfo[screen_script_id]->sprite = 1000; // don't kill me
    
    int player_map = 33;
    pplayer_map = &player_map;
    int vision = 0;
    pvision = &vision;
    
    // Create 5 connected screens
    struct screen s;
    s.ts_script_id = screen_script_id;
    map.loc[33] = 1; map.ts_loc_mem[33] = &s;
    map.loc[32] = 1; map.ts_loc_mem[32] = &s;
    map.loc[34] = 1; map.ts_loc_mem[34] = &s;
    map.loc[1]  = 1; map.ts_loc_mem[1]  = &s;
    map.loc[65] = 1; map.ts_loc_mem[65] = &s;
    
    screenlock = 0;
    walk_off_screen = 0;
    spr[1].active = 1;
    
    *pplayer_map = 33;
    spr[1].x = -1;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(play.var[tsx_id].var, -1);
    TS_ASSERT_EQUALS(spr[1].x, 619);
    
    *pplayer_map = 33;
    spr[1].y = -1;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(play.var[tsy_id].var, -1);
    TS_ASSERT_EQUALS(spr[1].y, 399);
    
    *pplayer_map = 33;
    spr[1].x = 620;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(play.var[tsx_id].var, 620);
    TS_ASSERT_EQUALS(spr[1].x, 20);
    
    *pplayer_map = 33;
    spr[1].y = 401;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(play.var[tsy_id].var, 401);
    TS_ASSERT_EQUALS(spr[1].y, 0);
    
    
    walk_off_screen = 1;
    
    spr[1].x = -1;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].x, -1);
    
    spr[1].y = -1;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].y, -1);
    
    spr[1].x = 700;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].x, 700);
    
    spr[1].y = 500;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].y, 500);
    
    walk_off_screen = 0;
    
    
    screenlock = 1;
    
    spr[1].x = 19;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].x, 20);
    
    spr[1].x = 20;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].x, 20);
    
    spr[1].y = -1;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].y, 0);
    
    spr[1].y = 0;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].y, 0);
    
    spr[1].x = 620;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].x, 619);
    
    spr[1].x = 619;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].x, 619);
    
    spr[1].y = 400;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].y, 399);
    
    spr[1].y = 399;
    did_player_cross_screen();
    TS_ASSERT_EQUALS(spr[1].y, 399);
    
    screenlock = 0;
    
    
    paths_quit();
  }
};
