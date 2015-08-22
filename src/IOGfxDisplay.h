#ifndef IOGFXDISPLAY_H
#define IOGFXDISPLAY_H

class IOGfxDisplay {
public:
	virtual ~IOGfxDisplay();
	virtual bool open() = 0;

	virtual bool createWindow() = 0;
	virtual void logWindowInfo() = 0;

	virtual void clearWindow() = 0;
};

extern void gfx_toggle_fullscreen();
extern void flip_it(void);

#endif
