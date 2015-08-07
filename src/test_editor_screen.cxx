/**
 * FreeDink test suite - Screen

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

#include "editor_screen.h"
#include "live_screen.h"
#include "live_sprites_manager.h"

class TestScreen : public CxxTest::TestSuite {
public:
  void setUp() {
  }
  void tearDown() {
  }
  
  void test_new() {
    live_screen_init();
    TS_ASSERT_EQUALS(spr[0].x, 0);
  }
};
