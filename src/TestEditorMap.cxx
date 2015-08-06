/**
 * FreeDink test suite - Map

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

#include "EditorMap.h"
#include "paths.h"

class TestEditorMap : public CxxTest::TestSuite {
public:
	void setUp() {}
	void tearDown() {}
	
	void test_new() {
		ts_paths_init();
		TS_ASSERT_EQUALS(current_dat, "dink.dat");
		map_load();
		TS_ASSERT_EQUALS(g_map.loc[1], 0);
		
		EditorMap testmap;
		TS_ASSERT_EQUALS(map_new("", &testmap), -1);
		testmap.load();
	}
};
