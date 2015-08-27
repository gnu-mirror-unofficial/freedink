/**
 * Test OpenGL screen

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

#include "IOGfxDisplaySW.h"
#include "IOGfxDisplayGL2.h"
#include "IOGfxSurfaceSW.h"
#include "SDL_image.h"
#include "freedink_xpm.h"
#include "log.h"
#include "ImageLoader.h" /* GFX_ref_pal */ // TODO: break dep
#include "gfx_palette.h"

/**
 * Test graphics output
 * Shortcomings:
 * - Buggy driver init
 * - SDL_Renderer drops all ops when window is hidden; use a visible window
 * - Android display is 16-bit, so use fuzzy color comparisons
 * - Stretched display (fullscreen and Android) use linear interpolation, so use fuzzy pixel positions
 * - No OpenGL ES 2.0 API to retrieve texture contents, only retrieve main buffer
 */
class TestIOGfxDisplay : public CxxTest::TestSuite {
public:
	IOGfxDisplay* display;

	void setUp() {
		TS_ASSERT_EQUALS(SDL_InitSubSystem(SDL_INIT_VIDEO), 0);
		// SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

		// init palette; hopefully can be removed if we get rid of blitFormat
		for (int i = 0; i < 256; i++) {
			GFX_ref_pal[i].r = i;
			GFX_ref_pal[i].g = i;
			GFX_ref_pal[i].b = i;
			GFX_ref_pal[i].a = 255;
		}
		// Use a color that fail tests if RGBA->ABGR-reversed (non-symmetric)
		// Fully saturated green to avoid fuzzy color comparisons
		GFX_ref_pal[1].r = 255;
		GFX_ref_pal[1].g = 255;
		GFX_ref_pal[1].b = 0;
		GFX_ref_pal[1].a = 255;
		GFX_ref_pal[2].r = 0;
		GFX_ref_pal[2].g = 0;
		GFX_ref_pal[2].b = 255;
		GFX_ref_pal[2].a = 255;
		gfx_palette_set_phys(GFX_ref_pal);
	}
	void tearDown() {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}

	void openDisplay(bool gl, bool truecolor, Uint32 flags) {
		log_info("* Requesting %s %s", gl?"GL2":"SW", truecolor?"truecolor":"");
		if (gl)
			display = new IOGfxDisplayGL2(50, 50, truecolor, flags);
		else
			display = new IOGfxDisplaySW(50, 50, truecolor, flags);
		bool opened = display->open();
		TS_ASSERT_EQUALS(opened, true);
	}
	void closeDisplay() {
		display->close();
	}

	void ctestSplash() {
		// A first inter-texture blit before anything else
		// Tests IOGfxDisplayGL2->androidWorkAround()
		SDL_Surface* image;
		IOGfxSurface *backbuffer, *splash;

		backbuffer = display->alloc(50, 50);
		//g->flip(backbuffer); // not a single flip

		image = SDL_CreateRGBSurface(0, 40, 40, 8, 0, 0, 0, 0);
		Uint8* pixels = (Uint8*)image->pixels;
		SDL_SetPaletteColors(image->format->palette, GFX_ref_pal, 0, 256);
		pixels[0] = 1;
		pixels[1] = 1;
		pixels[2] = 1;
		pixels[3] = 1;
		pixels[image->pitch+0] = 1;
		pixels[image->pitch+1] = 1;
		pixels[image->pitch+2] = 1;
		splash = display->upload(image);
		//g->flip(splash); // not a single flip

		SDL_Rect dstrect = {0, 0, -1, -1};
		// without workaround, the blit is ignored/delayed
		backbuffer->blit(splash, NULL, &dstrect);

		// Check the result:
		display->flip(backbuffer);
		log_info("size: %d %d", display->w, display->h);
		log_info("sizebb: %d %d", backbuffer->w, backbuffer->h);

		SDL_Surface* screenshot = display->screenshot();
		TS_ASSERT(screenshot != NULL);
		if (screenshot == NULL)
			return;
		Uint8 cr, cg, cb, ca;
		int x = 2, y = 1;
		display->surfToDisplayCoords(backbuffer, x, y);
		SDL_GetRGBA(((Uint32*)screenshot->pixels)[x+y*screenshot->w],
					screenshot->format,
					&cr, &cg, &cb, &ca);
		TS_ASSERT_EQUALS(cr, 255);
		TS_ASSERT_EQUALS(cg, 255);
		TS_ASSERT_EQUALS(cb, 0);
		TS_ASSERT_EQUALS(ca, 255);
		//SDL_SaveBMP(screenshot, "1testSplash.bmp");
		SDL_FreeSurface(screenshot);

		delete splash;
		delete backbuffer;
	}
	void test01SplashGL2Truecolor() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);
		ctestSplash();
		closeDisplay();
	}
	void testSplashSWTruecolor() {
		openDisplay(false, true, 0); // can't render offscreen >(
		ctestSplash();
		closeDisplay();
	}
	void testSplashGL2() {
		openDisplay(false, false, SDL_WINDOW_HIDDEN);
		//ctestSplash(); // TODO
		closeDisplay();
	}
	void testSplashSW() {
		openDisplay(false, false, 0);
		ctestSplash();
		closeDisplay();
	}
	void debug2SplashSWTruecolor() {
		openDisplay(false, true, 0);
		ctestSplash();

		// wait for us to see
		SDL_Delay(2000);
		closeDisplay();
	}


	void ctest_alloc() {
		IOGfxSurface* surf = display->alloc(300, 300);
		TS_ASSERT(surf != NULL);
		TS_ASSERT_EQUALS(surf->w, 300);
		TS_ASSERT_EQUALS(surf->h, 300);
	}
	void test_allocGL2() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);
		ctest_alloc();
		closeDisplay();
	}
	void test_allocSW() {
		openDisplay(false, true, SDL_WINDOW_HIDDEN);
		ctest_alloc();
		closeDisplay();
	}



	void ctest_screenshot() {
		SDL_Surface* img;
		IOGfxSurface *backbuffer, *surf;

		backbuffer = display->alloc(50, 50);

		img = SDL_CreateRGBSurface(0, 40, 40, 8, 0, 0, 0, 0);
		Uint8* pixels = (Uint8*)img->pixels;
		SDL_SetPaletteColors(img->format->palette, GFX_ref_pal, 0, 256);
		pixels[0] = 255;
		pixels[1] = 1;
		surf = display->upload(img);
		backbuffer->blit(surf, NULL, NULL);
		display->flipRaw(backbuffer);

		// Check that pic is not vertically flipped
		SDL_Surface* screenshot = display->screenshot();
		TS_ASSERT(screenshot != NULL);
		if (screenshot == NULL)
			return;
		Uint8 cr, cg, cb, ca;

		SDL_GetRGBA(((Uint32*)screenshot->pixels)[0],
					screenshot->format,
					&cr, &cg, &cb, &ca);
		TS_ASSERT_EQUALS(cr, 255);
		TS_ASSERT_EQUALS(cb, 255);
		TS_ASSERT_EQUALS(cg, 255);
		TS_ASSERT_EQUALS(ca, 255);

		SDL_GetRGBA(((Uint32*)screenshot->pixels)[1],
					screenshot->format,
					&cr, &cg, &cb, &ca);
		TS_ASSERT_EQUALS(cr, 255);
		TS_ASSERT_EQUALS(cg, 255);
		TS_ASSERT_EQUALS(cb, 0);
		TS_ASSERT_EQUALS(ca, 255);

		//SDL_SaveBMP(screenshot, "screenshot.bmp");
		SDL_FreeSurface(screenshot);
	}
	void test_screenshotGL2() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);
		ctest_screenshot();
		closeDisplay();
	}
	void test_screenshotSW() {
		openDisplay(false, true, 0);
		ctest_screenshot();
		closeDisplay();
	}



	void test_surfToDisplayCoords() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);
		SDL_Surface* img;
		IOGfxSurface *surf;
		int x, y;

		img = SDL_CreateRGBSurface(0, 50, 50, 8, 0, 0, 0, 0);
		surf = display->upload(img);

		x = 0; y = 0;
		display->surfToDisplayCoords(surf, x, y);
		TS_ASSERT_EQUALS(x, 0);
		TS_ASSERT_EQUALS(y, 0);

		x = 11; y = 12;
		display->surfToDisplayCoords(surf, x, y);
		TS_ASSERT_EQUALS(x, 11);
		TS_ASSERT_EQUALS(y, 12);

		display->onSizeChange(800, 480);

		x = 0; y = 0;
		display->surfToDisplayCoords(surf, x, y);
		TS_ASSERT_EQUALS(x, 164);
		TS_ASSERT_EQUALS(y, 4);

		x = 11; y = 12;
		display->surfToDisplayCoords(surf, x, y);
		TS_ASSERT_EQUALS(x, 270);
		TS_ASSERT_EQUALS(y, 120);
	}


	void ctest_blitCheck(int x, int y, Uint8 r, Uint8 g, Uint8 b) {
		Uint8 cr, cg, cb, ca;
		SDL_Surface* screenshot = display->screenshot();
		SDL_GetRGBA(((Uint32*)screenshot->pixels)[x + y*screenshot->w],
				screenshot->format,
				&cr, &cg, &cb, &ca);
		TS_ASSERT_EQUALS(cr, r);
		TS_ASSERT_EQUALS(cg, g);
		TS_ASSERT_EQUALS(cb, b);
		SDL_SaveBMP(screenshot, "screenshot.bmp");
	}
	void ctest_blit() {
		SDL_Surface* img;
		IOGfxSurface *backbuffer, *surf;

		backbuffer = display->alloc(50, 50);

		img = SDL_CreateRGBSurface(0, 5, 5, 8, 0, 0, 0, 0);
		Uint8* pixels = (Uint8*)img->pixels;
		SDL_SetPaletteColors(img->format->palette, GFX_ref_pal, 0, 256);
		SDL_SetColorKey(img, SDL_TRUE, 0);
		pixels[0] = 255;
		pixels[1] = 1;
		pixels[img->pitch] = 255;
		pixels[img->pitch+1] = 1;
		surf = display->upload(img);

		SDL_Rect dstrect = {0, 0, -1, -1};

		dstrect.x = 0; dstrect.y = 0;
		backbuffer->blit(surf, NULL, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 255,255,255);
		ctest_blitCheck(1,0, 255,255,0);

		dstrect.x = 20; dstrect.y = 20;
		backbuffer->blit(surf, NULL, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(20,20, 255,255,255);
		ctest_blitCheck(21,20, 255,255,0);

		dstrect.x = -1; dstrect.y = -1;
		backbuffer->blit(surf, NULL, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 255,255,0);

		dstrect.x = 49; dstrect.y = 49;
		backbuffer->blit(surf, NULL, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(49,49, 255,255,255);


		dstrect.x = 0; dstrect.y = 0;
		backbuffer->blit(surf, NULL, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 255,255,255);
		dstrect.x = -2; dstrect.y = -2;
		backbuffer->blit(surf, NULL, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 255,255,255);
		dstrect.x = -2; dstrect.y = -2;
		backbuffer->blitNoColorKey(surf, NULL, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 0,0,0);

		dstrect.x = 0; dstrect.y = 0;
		dstrect.w = 10; dstrect.h = 20;
		backbuffer->blitStretch(surf, NULL, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,7, 255,255,255);
		ctest_blitCheck(3,7, 255,255,0);

		SDL_Rect srcrect = {1,1, 1,1};
		dstrect.x = 0; dstrect.y = 0;
		backbuffer->blit(surf, &srcrect, &dstrect);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 255,255,0);

		delete backbuffer;
		delete surf;

		display->close();
	}
	void test_blitGL2Truecolor() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);
		ctest_blit();
		closeDisplay();
	}
	void test_blitSWTruecolor() {
		openDisplay(false, true, 0);
		ctest_blit();
		closeDisplay();
	}
	void test_blitGL2() {
		openDisplay(true, false, SDL_WINDOW_HIDDEN);
		//ctest_blit(); // TODO
		closeDisplay();
	}
	void test_blitSW() {
		openDisplay(false, false, 0);
		ctest_blit();
		closeDisplay();
	}



	void ctest_fillRect() {
		IOGfxSurface *backbuffer;

		backbuffer = display->alloc(50, 50);

		SDL_Rect dstrect = {-1, -1, -1, -1};

		backbuffer->fillRect(NULL, 255, 255, 0);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 255,255,0);

		dstrect.x = 5;  dstrect.y = 5;
		dstrect.w = 20; dstrect.h = 10;
		backbuffer->fillRect(&dstrect, 0, 0, 255);
		display->flipRaw(backbuffer);
		ctest_blitCheck(4,4, 255,255,0);
		ctest_blitCheck(5,5, 0,0,255);
		ctest_blitCheck(24,14, 0,0,255);
		ctest_blitCheck(25,14, 255,255,0);
		ctest_blitCheck(24,15, 255,255,0);

		display->close();
	}
	void test_fillRectGL2Truecolor() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);
		ctest_fillRect();
		closeDisplay();
	}
	void test_fillRectSWTruecolor() {
		openDisplay(false, true, 0);
		ctest_fillRect();
		closeDisplay();
	}
	void test_fillRectGL2() {
		openDisplay(true, false, SDL_WINDOW_HIDDEN);
		//ctest_fillRect(); // TODO
		closeDisplay();
	}
	void test_fillRectSW() {
		openDisplay(false, false, 0);
		ctest_fillRect();
		closeDisplay();
	}

	void ctest_fill_screen() {
		IOGfxSurface *backbuffer;

		backbuffer = display->alloc(50, 50);

		backbuffer->fill_screen(0, GFX_ref_pal);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 0,0,0);

		backbuffer->fill_screen(1, GFX_ref_pal);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 255,255,0);

		backbuffer->fill_screen(2, GFX_ref_pal);
		display->flipRaw(backbuffer);
		ctest_blitCheck(0,0, 0,0,255);

		display->close();
	}
	void test_fill_screenGL2Truecolor() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);
		ctest_fill_screen();
		closeDisplay();
	}
	void test_fill_screenSWTruecolor() {
		openDisplay(false, true, 0);
		ctest_fill_screen();
		closeDisplay();
	}
	void test_fill_screenGL2() {
		openDisplay(true, false, SDL_WINDOW_HIDDEN);
		//ctest_fill_screen(); // TODO
		closeDisplay();
	}
	void test_fill_screenSW() {
		openDisplay(false, false, 0);
		ctest_fill_screen();
		closeDisplay();
	}
};
