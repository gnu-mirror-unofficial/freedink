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
#include "gfx.h"
#include "gfx_fonts.h"
SDL_Surface *GFX_backbuffer;

extern int add_text_sprite(char* text, int script, int sprite_owner, int mx, int my);

/* Mocks */
int last_text = 0;
int* plast_text = &last_text;
int dversion = 108;

static int g_colorIndex = 0;
static Uint8 g_r = 0;
static Uint8 g_g = 0;
static Uint8 g_b = 0;
void FONTS_SetTextColor(Uint8 r, Uint8 g, Uint8 b) {
  g_r = r;
  g_g = g;
  g_b = b;
}
void FONTS_SetTextColorIndex(int color) {
  g_colorIndex = color;
}
int print_text_wrap(char*, rect*, int, int, FONT_TYPE) { return 0; }
int gfx_fonts_init(void) { return 1; }
void gfx_fonts_init_colors() {}
void gfx_fonts_quit() {}

double truecolor_fade_brightness;
int truecolor = 1;
SDL_Surface* GFX_tmp2;
SDL_Color GFX_ref_pal[256];
extern int PlayMidi(char *sFileName) { return 1; }
extern SDL_Surface* load_bmp_from_fp(FILE* in) { return NULL; }


class TestText : public CxxTest::TestSuite {
public:
	void setUp() {
		live_sprites_manager_init();
		truecolor_fade_brightness = 256;
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
		TS_ASSERT_EQUALS(spr[3].kill_ttl, 2700);
		text_draw(3, 256);
		
		TS_ASSERT_EQUALS(say_text("Hello", 2, 0), 4);
		text_draw(4, 256);

		// Crash tests

		TS_ASSERT_EQUALS(say_text("Hello1000", 1000, 0), 5);
		text_draw(5, 256);

		TS_ASSERT_EQUALS(say_text("Hello1200", 1200, 0), 6);
		text_draw(6, 256);

		TS_ASSERT_EQUALS(say_text("Hello100000", 100000, 0), 7);
		text_draw(7, 256);
	}

	void test_fade() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(add_text_sprite("Fading", 0, 1, 0, 0), 2);
		text_draw(2, 256);
		TS_ASSERT_EQUALS(g_colorIndex, 14);
		TS_ASSERT_EQUALS(add_text_sprite("`2Fading", 0, 1, 0, 0), 3);
		text_draw(3, 256);
		TS_ASSERT_EQUALS(g_colorIndex, 2);
		
		text_draw(2, 0);
		TS_ASSERT_EQUALS(g_colorIndex, 15);
		text_draw(3, 0);
		TS_ASSERT_EQUALS(g_colorIndex, 15);
	}

	void test_damage_experience() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(add_sprite(100,100,8,0,0), 2);
		spr[2].damage = 10;
		
		spr[2].brain_parm = 1;
		text_draw(2, 256);
		TS_ASSERT_EQUALS(g_r, 255);
		TS_ASSERT_EQUALS(g_g, 255);
		TS_ASSERT_EQUALS(g_b, 255);

		spr[2].brain_parm = 5000;
		spr[2].damage = 10;
		text_draw(2, 256);
		TS_ASSERT_EQUALS(g_colorIndex, 14);
	}
};
