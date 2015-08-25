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
#include "IOGfxGLFuncs.h"
#include "SDL_image.h"
#include "freedink_xpm.h"
#include "log.h"

class TestIOGraphics : public CxxTest::TestSuite {
public:
	IOGfxDisplay* display;

	void setUp() {
		TS_ASSERT_EQUALS(SDL_InitSubSystem(SDL_INIT_VIDEO), 0);
		// SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
	}
	void tearDown() {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}

	void openDisplay(bool gl, bool truecolor, Uint32 flags) {
		if (gl)
			display = new IOGfxDisplayGL2(50, 50, truecolor, flags);
//		else
//			g = new IOGfxDisplaySW(800, 600, truecolor, flags);
		bool opened = display->open();
		TS_ASSERT_EQUALS(opened, true);
	}

	void test_splashScreen() {
		// A first inter-texture blit before anything else
		// Tests IOGfxDisplayGL2->androidWorkAround()
		openDisplay(true, true, SDL_WINDOW_HIDDEN); /* SDL_WINDOW_FULLSCREEN_DESKTOP); */

		SDL_Surface* surf;
		IOGfxSurface *backbuffer, *splash;

		Uint32 Rmask=0, Gmask=0, Bmask=0, Amask=0; int bpp=0;
		SDL_PixelFormatEnumToMasks(display->getFormat(), &bpp,
			&Rmask, &Gmask, &Bmask, &Amask);
		surf = SDL_CreateRGBSurface(0, 5, 5, bpp,
			Rmask, Gmask, Bmask, Amask);
		backbuffer = display->upload(surf);
		//g->flip(backbuffer);

		surf = SDL_CreateRGBSurface(0, 3, 3, 8,
			0, 0, 0, 0);
		Uint8* pixels = (Uint8*)surf->pixels;
		surf->format->palette->colors[0].r = 255;
		surf->format->palette->colors[0].g = 255;
		surf->format->palette->colors[0].b = 255;
		surf->format->palette->colors[1].r = 255;
		surf->format->palette->colors[1].g = 0;
		surf->format->palette->colors[1].b = 0;
		pixels[0] = 1;
		pixels[1] = 1;
		pixels[2] = 1;
		pixels[surf->pitch+0] = 1;
		pixels[surf->pitch+1] = 1;
		pixels[surf->pitch+2] = 1;
		splash = display->upload(surf);
		//g->flip(splash);

		SDL_Rect dstrect = {0, 0, -1, -1};
		// without workaround, the blit is ignored/delayed
		backbuffer->blit(splash, NULL, &dstrect);

		// Check the result:
		display->flip(backbuffer);

		SDL_Surface* screenshot = dynamic_cast<IOGfxDisplayGL2*>(display)->screenshot();
		Uint8 cr, cg, cb, ca;
		// Middle pixel of centered area is red
		SDL_GetRGBA(((Uint32*)screenshot->pixels)[display->w/4],
					screenshot->format,
					&cr, &cg, &cb, &ca);
		TS_ASSERT_EQUALS(cr, 255);
		TS_ASSERT_EQUALS(cb, 0);
		TS_ASSERT_EQUALS(cg, 0);
		TS_ASSERT_EQUALS(ca, 255);
		SDL_FreeSurface(screenshot);

		delete splash;
		delete backbuffer;
	}

	void testScreenshot() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);

		SDL_Surface* img;
		IOGfxSurface* surf;
		img = SDL_CreateRGBSurface(0, 3, 3, 8,
			0, 0, 0, 0);
		Uint8* pixels = (Uint8*)img->pixels;
		img->format->palette->colors[0].r = 255;
		img->format->palette->colors[0].g = 255;
		img->format->palette->colors[0].b = 255;
		img->format->palette->colors[1].r = 255;
		img->format->palette->colors[1].g = 0;
		img->format->palette->colors[1].b = 0;
		pixels[0] = 1;
		surf = display->upload(img);
		display->flip(surf);

		// Check that first pixel is red - and that pic is not vertically flipped
		SDL_Surface* screenshot = display->screenshot();
		Uint8 cr, cg, cb, ca;
		int x, y;

		x = 0, y = 0;
		display->surfToDisplayCoord(surf, x, y);
		SDL_GetRGBA(((Uint32*)screenshot->pixels)[x],
					screenshot->format,
					&cr, &cg, &cb, &ca);
		TS_ASSERT_EQUALS(cr, 255);
		TS_ASSERT_EQUALS(cb, 0);
		TS_ASSERT_EQUALS(cg, 0);
		TS_ASSERT_EQUALS(ca, 255);

		x = 1, y = 0;
		display->surfToDisplayCoord(surf, x, y);
		SDL_GetRGBA(((Uint32*)screenshot->pixels)[x],
					screenshot->format,
					&cr, &cg, &cb, &ca);
		TS_ASSERT_EQUALS(cr, 255);
		TS_ASSERT_EQUALS(cb, 255);
		TS_ASSERT_EQUALS(cg, 255);
		TS_ASSERT_EQUALS(ca, 255);

		SDL_SaveBMP(screenshot, "screenshot.bmp");
		SDL_FreeSurface(screenshot);
	}

	void test_surfToDisplayCoord() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);
		SDL_Surface* img;
		IOGfxSurface *surf;
		int x, y;

		img = SDL_CreateRGBSurface(0, 50, 50, 8, 0, 0, 0, 0);
		surf = display->upload(img);

		display->onSizeChange(50, 50);
		x = 0; y = 0;
		display->surfToDisplayCoord(surf, x, y);
		TS_ASSERT_EQUALS(x, 0);
		TS_ASSERT_EQUALS(y, 0);

		x = 11; y = 12;
		display->surfToDisplayCoord(surf, x, y);
		TS_ASSERT_EQUALS(x, 11);
		TS_ASSERT_EQUALS(y, 12);

		display->onSizeChange(800, 480);

		x = 0; y = 0;
		display->surfToDisplayCoord(surf, x, y);
		TS_ASSERT_EQUALS(x, 160);
		TS_ASSERT_EQUALS(y, 0);

		x = 11; y = 12;
		display->surfToDisplayCoord(surf, x, y);
		TS_ASSERT_EQUALS(x, 266);
		TS_ASSERT_EQUALS(y, 116);
	}

	void test_blit() {
		openDisplay(true, true, SDL_WINDOW_HIDDEN);

		SDL_Surface* surf;
		IOGfxSurface *tex1, *tex3;

		surf = IMG_Load("test.png");
		tex1 = display->upload(surf);
		display->flip(tex1);

		surf = IMG_ReadXPMFromArray(freedink_xpm);
		IOGfxSurface* tex2 = display->upload(surf);
		display->flip(tex2);

		surf = IMG_Load("test2.bmp");
		SDL_SetColorKey(surf, SDL_TRUE, 0);
		tex3 = display->upload(surf);



		SDL_Rect dstrect = {200, 200, -1, -1};
		tex1->blit(tex3, NULL, &dstrect);

		dstrect.x = 210; dstrect.y = 210;
		tex1->blit(tex3, NULL, &dstrect);

		dstrect.x = -1; dstrect.y = -1;
		tex1->blit(tex3, NULL, &dstrect);

		dstrect.x = 330; dstrect.y = 350;
		tex1->blit(tex3, NULL, &dstrect);

		dstrect.x = 330; dstrect.y = 50;
		tex1->blitNoColorKey(tex3, NULL, &dstrect);

		dstrect.x = 330; dstrect.y = 50;
		dstrect.w = 50; dstrect.h = 300;
		tex1->blitStretch(tex3, NULL, &dstrect);

		SDL_Rect srcrect = {55,6, 11,14};
		dstrect.x = 50; dstrect.y = 280;
		tex1->blit(tex3, &srcrect, &dstrect);
		display->flip(tex1);

		delete tex1;
		delete tex2;
		delete tex3;

		display->close();
	}
};
