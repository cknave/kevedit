# Makefile for textmode display library

AUTOMAKE_OPTIONS = 1.6.3

noinst_LIBRARIES = libdisplay.a

libdisplay_a_SOURCES = charset.c charset.h display.c display.h \
	display_stdio.c display_stdio.h textblock.c textblock.h keys.h colours.h \
	palette.c palette.h

# Test program
noinst_PROGRAMS = test
test_SOURCES = test.c
test_LDADD = libdisplay.a

if DOS
libdisplay_a_SOURCES += display_dos.c display_dos.h
endif

if SDL
libdisplay_a_SOURCES += display_sdl.c display_sdl.h unicode.c unicode.h
endif

if MACOS
libdisplay_a_SOURCES += $(top_srcdir)/src/kevedit/macos.m
test_SOURCES += $(top_srcdir)/src/kevedit/macos.m
test_LDFLAGS = -framework Cocoa
endif

EXTRA_DIST = default.chr ega.pal

