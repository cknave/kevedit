# Makefile for KevEdit program

AUTOMAKE_OPTIONS = 1.7.4

AM_CPPFLAGS = -DDATAPATH=\"$(pkgdatadir)\"

bin_PROGRAMS = kevedit

kevedit_SOURCES = \
	kevedit.c main.c menu.c misc.c patbuffer.c screen.c \
	kevedit.h menu.h misc.h patbuffer.h screen.h

if HAVE_WINDRES
kevedit_SOURCES += resources.rc
MOSTLYCLEANFILES = resources.rc
endif

kevedit_LDADD = ../zlaunch/libzlaunch.a ../texteditor/libtexteditor.a ../dialogs/libdialogs.a ../help/libhelp.a ../themes/rusty/librusty.a ../synth/libsynth.a ../display/libdisplay.a ../structures/libstructures.a ../libzzt2/libzzt2.a $(GLOBLIB)

.rc.o:
	$(WINDRES) -o $@ $<

resources.o: resources.rc

resources.rc:
	echo kevedit ICON \"$(top_srcdir)/kevedit.ico\" > $@

