/**
 * Test text sprites generation

 * Copyright (C) 2015  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public
 * License along with GNU FreeDink.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cxxtest/TestSuite.h>

#include "gfx_fonts.h"
#include "IOGfxDisplay.h"
IOGfxSurface *IOGFX_backbuffer = NULL;
IOGfxDisplay* g_display = NULL;
#include "test_gfx_fonts_libe.h"
#include <iostream>
using namespace std;

char* vgasys_fon;
int truecolor;
extern TTF_Font *dialog_font;

class TestGfxFonts : public CxxTest::TestSuite {
public:
	void setUp() {
		TTF_Init();
		SDL_RWops* rw = SDL_RWFromMem(libe_ttf, libe_ttf_len);
		dialog_font = TTF_OpenFontRW(rw, 1, FONT_SIZE);
		setup_font(dialog_font);
	}
	void tearDown() {
	}
	
	void test_text() {

	}
};
