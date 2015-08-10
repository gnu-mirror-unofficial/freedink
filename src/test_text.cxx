/**
 * Say my name 10x without failing

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

#include "text.h"
#include "live_sprites_manager.h"
#include "gfx_fonts.h"

extern int add_text_sprite(char* text, int script, int sprite_owner, int mx, int my);

/* Mocks */
int last_text = 0;
int* plast_text = &last_text;
int dversion = 108;

void FONTS_SetTextColor(unsigned char, unsigned char, unsigned char) {}
void FONTS_SetTextColorIndex(int) {}
int print_text_wrap(char*, rect*, int, int, FONT_TYPE) { return 0; }
int gfx_fonts_init(void) { return 1; }
void gfx_fonts_init_colors() {}
void gfx_fonts_quit() {}

double truecolor_fade_brightness = 256;
int truecolor = 1;
SDL_Surface* GFX_lpDDSTrick2;
SDL_Color GFX_real_pal[256];
extern int PlayMidi(char *sFileName) { return 1; }
extern SDL_Surface* load_bmp_from_fp(FILE* in) { return NULL; }


class TestText : public CxxTest::TestSuite {
public:
	void setUp() {
		live_sprites_manager_init();
	}
	void tearDown() {
	}
	
	void test_text() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		spr[2].x = 500;

		TS_ASSERT_EQUALS(add_text_sprite("Hi", 123, 2, 0, 0), 3);
		TS_ASSERT_EQUALS(spr[3].text, "Hi");
		TS_ASSERT_EQUALS(spr[3].owner, 2);
		TS_ASSERT_EQUALS(spr[3].script, 123);
		TS_ASSERT_EQUALS(spr[3].damage, -1);
		TS_ASSERT_EQUALS(spr[3].kill, 2700);
		text_draw(3);
		
		TS_ASSERT_EQUALS(say_text("Hello", 2, 0), 4);
		text_draw(4);

		// Crash tests

		TS_ASSERT_EQUALS(say_text("Hello1000", 1000, 0), 5);
		text_draw(5);

		TS_ASSERT_EQUALS(say_text("Hello1200", 1200, 0), 6);
		text_draw(6);

		TS_ASSERT_EQUALS(say_text("Hello100000", 100000, 0), 7);
		text_draw(7);
	}
};
