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

#include "IOGfxDisplayGL2.h"
#include "IOGfxGLFuncs.h"
#include "SDL_image.h"
#include "freedink_xpm.h"

class TestIOGraphics : public CxxTest::TestSuite {
public:
	void setUp() {
		TS_ASSERT_EQUALS(SDL_InitSubSystem(SDL_INIT_VIDEO), 0);
	}
	void tearDown() {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}

	void test_graphics() {
		//IOGfxDisplay* g = new IOGfxDisplayGL2(800, 600, true, SDL_WINDOW_HIDDEN);
		IOGfxDisplay* g = new IOGfxDisplayGL2(800, 600, true, 0);
		bool opened = g->open();
		TS_ASSERT_EQUALS(opened, true);
		if (!opened)
			return;
		g->onSizeChange(800, 600);

		g->clear();
		SDL_GL_SwapWindow(g->window);

		SDL_Surface* screenshot = dynamic_cast<IOGfxDisplayGL2*>(g)->screenshot();
		Uint8 cr, cg, cb, ca;
		SDL_GetRGBA(((Uint32*)screenshot->pixels)[0],
					screenshot->format,
					&cr, &cg, &cb, &ca);
		TS_ASSERT_EQUALS(cr, 0);
		TS_ASSERT_EQUALS(cb, 0);
		TS_ASSERT_EQUALS(cg, 0);
		TS_ASSERT_EQUALS(ca, 255);
		SDL_FreeSurface(screenshot);


		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
		SDL_Surface* surf;

		surf = IMG_Load("test.png");
		IOGfxSurface* tex1 = g->upload(surf);
		g->flip(tex1);

		surf = IMG_ReadXPMFromArray(freedink_xpm);
		IOGfxSurface* tex2 = g->upload(surf);
		g->flip(tex2);

		surf = IMG_Load("test2.bmp");
		IOGfxSurface* tex3 = g->upload(surf);
		//g->flip(tex3);
		SDL_Rect dstrect = {200, 200, -1, -1};
		tex1->blit(tex3, NULL, &dstrect);
		dstrect.x = 210; dstrect.y = 210;
		tex1->blit(tex3, NULL, &dstrect);
		dstrect.x = -30; dstrect.y = -30;
		tex1->blit(tex3, NULL, &dstrect);
		g->flip(tex1);

		SDL_Delay(2000);

		delete tex1;
		delete tex2;
		delete tex3;
		g->close();
	}
};
