# Makefile for textmode display library

AUTOMAKE_OPTIONS = 1.7.4

noinst_LIBRARIES = libdisplay.a

libdisplay_a_SOURCES = display.c display.h display_stdio.c display_stdio.h \
	keys.h colours.h

if SDL
libdisplay_a_SOURCES += display_sdl.c display_sdl.h
endif

if DOS
libdisplay_a_SOURCES += display_dos.c display_dos.h
endif

# Test program
noinst_PROGRAMS = test
test_SOURCES = test.c
test_LDADD = libdisplay.a
