/**
 * Test suite for spr[] management

 * Copyright (C) 2015  Sylvain Beucler

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

#include "live_sprites_manager.h"

class TestLiveSpritesManager : public CxxTest::TestSuite {
public:
	void setUp() {
		live_sprites_manager_init();
	}
	void tearDown() {
	}

	void test_add_sprite() {
		TS_ASSERT_EQUALS(spr[1].active, 0);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(last_sprite_created, 1);
		TS_ASSERT_EQUALS(spr[1].active, 1);
	}
	void test_add_sprite_dump() {
		TS_ASSERT_EQUALS(spr[1].active, 0);
		TS_ASSERT_EQUALS(add_sprite_dumb(0, 0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(last_sprite_created, 1);
		TS_ASSERT_EQUALS(spr[1].active, 1);
	}
	void test_sprite_1_not_killed() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		
		TS_ASSERT_EQUALS(kill_last_sprite(), 0);
		TS_ASSERT_EQUALS(spr[1].active, 1);
		TS_ASSERT_EQUALS(last_sprite_created, 1);
	}
	void test_sprite_gt_1_killed() {
		TS_ASSERT_EQUALS(spr[2].active, 0);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		TS_ASSERT_EQUALS(last_sprite_created, 2);
		TS_ASSERT_EQUALS(spr[2].active, 1);
		
		TS_ASSERT_EQUALS(kill_last_sprite(), 1);
		TS_ASSERT_EQUALS(last_sprite_created, 1);
		TS_ASSERT_EQUALS(spr[2].active, 0);
	}
	void test_sprite_live_not_killed_dont_ask_me_why_though() {
		TS_ASSERT_EQUALS(spr[2].active, 0);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		TS_ASSERT_EQUALS(last_sprite_created, 2);
		TS_ASSERT_EQUALS(spr[2].active, 1);
		spr[2].live = 1;
		
		TS_ASSERT_EQUALS(kill_last_sprite(), 0);
		TS_ASSERT_EQUALS(last_sprite_created, 2);
		TS_ASSERT_EQUALS(spr[2].active, 1);
	}
	void test_sprite_live_can_make_last_sprite_created_inconsistent() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		TS_ASSERT_EQUALS(spr[2].active, 1);
		spr[2].live = 1;
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 3);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 4);
		
		TS_ASSERT_EQUALS(kill_last_sprite(), 1);
		TS_ASSERT_EQUALS(spr[4].active, 0);
		TS_ASSERT_EQUALS(last_sprite_created, 4);
	}
	void test_add_sprite_keeps_sprite_created_inconsistent() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		spr[2].live = 1;
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 3);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 4);
		
		TS_ASSERT_EQUALS(kill_last_sprite(), 1);
		TS_ASSERT_EQUALS(spr[4].active, 0);
		TS_ASSERT_EQUALS(last_sprite_created, 4);
		
		TS_ASSERT_EQUALS(kill_last_sprite(), 1);
		TS_ASSERT_EQUALS(spr[3].active, 0);
		TS_ASSERT_EQUALS(last_sprite_created, 4);
		
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 3);
		TS_ASSERT_EQUALS(spr[3].active, 1);
		TS_ASSERT_EQUALS(spr[4].active, 0);
		TS_ASSERT_EQUALS(last_sprite_created, 4);
	}
	void test_sprite_live_can_make_hole() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 3);
		
		spr[3].live = 1;
		TS_ASSERT_EQUALS(kill_last_sprite(), 1);
		TS_ASSERT_EQUALS(last_sprite_created, 3);
		TS_ASSERT_EQUALS(spr[1].active, 1);
		TS_ASSERT_EQUALS(spr[2].active, 0);
		TS_ASSERT_EQUALS(spr[3].active, 1);
	}
 
	void test_get_last_sprite() {
		get_last_sprite();
		TS_ASSERT_EQUALS(last_sprite_created, 0);

		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		get_last_sprite();
		TS_ASSERT_EQUALS(last_sprite_created, 1);

		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		get_last_sprite();
		TS_ASSERT_EQUALS(last_sprite_created, 2);

		TS_ASSERT_EQUALS(kill_last_sprite(), 1);
		get_last_sprite();
		TS_ASSERT_EQUALS(last_sprite_created, 1);

		TS_ASSERT_EQUALS(kill_last_sprite(), 0);
		get_last_sprite();
		TS_ASSERT_EQUALS(last_sprite_created, 1);
	}
	void test_get_last_sprite_makes_last_sprite_created_consistent() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		spr[2].live = 1;
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 3);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 4);
		TS_ASSERT_EQUALS(kill_last_sprite(), 1);
		TS_ASSERT_EQUALS(kill_last_sprite(), 1);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 3);
		TS_ASSERT_EQUALS(spr[3].active, 1);
		TS_ASSERT_EQUALS(spr[4].active, 0);
		TS_ASSERT_EQUALS(last_sprite_created, 4);

		get_last_sprite();
		TS_ASSERT_EQUALS(last_sprite_created, 3);
	}
};
