# Makefile for KevEdit program

AUTOMAKE_OPTIONS = 1.6.3

AM_CPPFLAGS = -DDATAPATH=\"$(pkgdatadir)\"

bin_PROGRAMS = kevedit

if WINDOWS
windres_SOURCES = $(top_builddir)/inst/platform/windows/kevedit.rc
endif

if MACOS
macos_SOURCES = macos.m
macos_frameworks = -framework Cocoa
endif
EXTRA_DIST = macos.h

kevedit_SOURCES = \
	kevedit.c main.c menu.c misc.c patbuffer.c screen.c \
	kevedit.h menu.h misc.h patbuffer.h screen.h $(windres_SOURCES) \
	../mkdtemp/mkdtemp.c $(macos_SOURCES)

kevedit_LDADD = ../zlaunch/libzlaunch.a ../texteditor/libtexteditor.a ../dialogs/libdialogs.a ../help/libhelp.a ../themes/rusty/librusty.a ../synth/libsynth.a ../display/libdisplay.a ../structures/libstructures.a ../libzzt2/libzzt2.a $(GLOBLIB)
kevedit_LDFLAGS = $(macos_frameworks)

.rc.o:
	$(WINDRES) -o $@ $<

resources.o: $(windres_SOURCES)
