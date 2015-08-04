AM_CXXFLAGS = -Wno-write-strings -Wall
# Cf. (autoconf.info)Installation Directory Variables:
# Don't use DATADIR, it conflicts with something in MinGW
AM_CPPFLAGS = -DBUILD_DATA_DIR=\"$(datadir)\"

# Gnulib:
# includes generated files from top_builddir (not only top_srcdir)
AM_CPPFLAGS += -I$(top_srcdir)/gnulib/lib -I$(top_builddir)/gnulib/lib
LDADD = $(top_builddir)/gnulib/lib/libgnu.a

# gettext:
# Relative locale dir for woe/android
if WOE
AM_CPPFLAGS += -DLOCALEDIR=\"freedink/locale\"
else
if ANDROID
AM_CPPFLAGS += -DLOCALEDIR=\"freedink/locale\"
else
AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\"
endif
endif
LDADD += @LIBINTL@
AM_CPPFLAGS += -I$(top_srcdir)/intl -I$(top_builddir)/intl

LIBS += -lm

COMMON_OBJS= app.cpp app.h bgm.cpp bgm.h dinkc_sp_custom.cpp		\
	dinkc_sp_custom.h dinkini.cpp dinkini.h dinkvar.cpp dinkvar.h	\
	fastfile.cpp fastfile.h str_util.cpp str_util.h io_util.cpp	\
	io_util.h gfx.cpp gfx.h gfx_fade.cpp gfx_fade.h gfx_tiles.cpp	\
	gfx_tiles.h gfx_palette.cpp gfx_palette.h gfx_fonts.cpp		\
	gfx_fonts.h rect.cpp rect.h resources.cpp resources.h		\
	input.cpp input.h freedink_xpm.cpp freedink_xpm.h map.cpp	\
	map.h paths.cpp paths.h log.cpp log.h gfx_sprites.cpp		\
	gfx_sprites.h vgasys_fon.cpp vgasys_fon.h msgbox.cpp msgbox.h	\
	screen.cpp screen.h sfx.cpp sfx.h

if ANDROID
  # Build .so JNI libs rather than executables
  # Define here rather than configure.ac, otherwise config.h gets confused
  AM_CFLAGS = -fPIC
  AM_LDFLAGS = -shared
  LIBS += -llog
  COMMON_OBJS += SDL_android_main.cpp
endif

bin_PROGRAMS = freedink freedinkedit
freedink_SOURCES = $(COMMON_OBJS) freedink.cpp freedink.h		\
	freedink_main.cpp update_frame.cpp update_frame.h		\
	dinkc_bindings.cpp dinkc_bindings.h dinkc.cpp dinkc.h		\
	dinkc_console.cpp dinkc_console.h game_engine.cpp		\
	game_engine.h i18n.cpp i18n.h inventory.cpp inventory.h		\
	meminfo.cpp meminfo.h savegame.cpp savegame.h status.cpp	\
	status.h talk.cpp talk.h text.cpp text.h
freedinkedit_SOURCES = $(COMMON_OBJS) freedinkedit.cpp
nodist_freedink_SOURCES = $(WOERES)
nodist_freedinkedit_SOURCES = $(WOERES)

EXTRA_DIST = media/freedink.ico media/system.sfd media/vgasys.fon	\
	media/README

resdir=$(top_srcdir)/share/freedink
install-exec-hook:
if HAVE_UPX
# UPX compression
# Not using --best because it takes much more time and the result is
# only a little bit smaller
# Needs to be done before bundling resources
	-$(UPX) "$(DESTDIR)$(bindir)/freedink$(EXEEXT)"
	-$(UPX) "$(DESTDIR)$(bindir)/freedinkedit$(EXEEXT)"
endif
# Add 'dink' and 'dinkedit' aliases, to please 3rd-party tools
	$(LN_S) -f "freedink$(EXEEXT)" "$(DESTDIR)$(bindir)/dink$(EXEEXT)"
	$(LN_S) -f "freedinkedit$(EXEEXT)" "$(DESTDIR)$(bindir)/dinkedit$(EXEEXT)"

uninstall-hook:
	rm -f "$(DESTDIR)$(bindir)/dink$(EXEEXT)"
	rm -f "$(DESTDIR)$(bindir)/dinkedit$(EXEEXT)"


# help2man-generated manpages
# Inspiration from (automake-1.10.info.gz)distcleancheck
dist_man_MANS = freedink.6 freedinkedit.6 dink.6 dinkedit.6
freedink.6: app.cpp $(top_srcdir)/configure.ac Makefile.am
	$(MAKE) $(AM_MAKEFLAGS) freedink$(EXEEXT)
	-$(HELP2MAN) -s6 -n "adventure and role-playing game (engine)"	\
	  --no-info --output=freedink.6 ./freedink$(EXEEXT)
freedinkedit.6: app.cpp $(top_srcdir)/configure.ac Makefile.am
	$(MAKE) $(AM_MAKEFLAGS) freedinkedit$(EXEEXT)
	-$(HELP2MAN) -s6 -n "adventure and role-playing game (editor)"	\
	  --no-info --output=freedinkedit.6 ./freedinkedit$(EXEEXT)
distclean-local:
	rm -f freedink.6 freedinkedit.6


# Woe icon and app infos
# Also check http://fragglet.livejournal.com/4448.html
.rc.o:
	$(WINDRES) -i $< -o $@

# How do we handle resources embedded in the executable binary?
if HAVE_WINDRES
WOERES=woeres.rc
else
WOERES=
endif


if HAVE_TESTS
check_PROGRAMS = test_str_util test_io_util test_dinkc test_integration
.cxx.cpp:
	$(CXXTESTGEN) --error-printer --include test_sdl_workaround.h -o $@ $<
EXTRA_DIST += test_sdl_workaround.h
test_integration_SOURCES = $(COMMON_OBJS) dinkc_bindings.cpp		\
	dinkc_bindings.h dinkc.cpp dinkc.h dinkc_console.cpp		\
	dinkc_console.h freedink.cpp game_engine.cpp game_engine.h	\
	i18n.cpp i18n.h inventory.cpp inventory.h meminfo.cpp		\
	meminfo.h savegame.cpp savegame.h status.cpp status.h		\
	talk.cpp talk.h text.cpp text.h test_integration.cpp
test_str_util_SOURCES = str_util.cpp test_str_util.cpp
test_io_util_SOURCES = io_util.cpp test_io_util.cpp
test_dinkc_SOURCES = dinkc.cpp str_util.cpp paths.cpp io_util.cpp	\
	msgbox.cpp i18n.cpp test_dinkc.cpp
# Tests to run
TESTS = $(check_PROGRAMS)
endif
