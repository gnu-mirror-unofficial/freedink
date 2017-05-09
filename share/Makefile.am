SUBDIRS = freedink

EXTRA_DIST = pixmaps/freedink.png applications/freedink.desktop	\
	applications/freedinkedit.desktop README README.fonts	\
	metainfo/freedink.appdata.xml


# freedesktop.org menu entry
install-data-local:
	$(MKDIR_P) "$(DESTDIR)$(datarootdir)/pixmaps"
	$(MKDIR_P) "$(DESTDIR)$(datarootdir)/applications"
	$(MKDIR_P) "$(DESTDIR)$(datarootdir)/metainfo"
	$(INSTALL_DATA) "$(srcdir)/pixmaps/freedink.png" "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL_DATA) "$(srcdir)/applications/freedink.desktop" "$(DESTDIR)$(datarootdir)/applications/"
	$(INSTALL_DATA) "$(srcdir)/applications/freedinkedit.desktop" "$(DESTDIR)$(datarootdir)/applications/"
	$(INSTALL_DATA) "$(srcdir)/metainfo/freedink.appdata.xml" "$(DESTDIR)$(datarootdir)/metainfo/freedink.appdata.xml"

uninstall-local:
	rm -f "$(DESTDIR)$(datarootdir)/pixmaps/freedink.png"
	rm -f "$(DESTDIR)$(datarootdir)/applications/freedink.desktop"
	rm -f "$(DESTDIR)$(datarootdir)/applications/freedinkedit.desktop"
	rm -f "$(DESTDIR)$(datarootdir)/metainfo/freedink.appdata.xml"
