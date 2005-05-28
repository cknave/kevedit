# Makefile for textmode display library

AUTOMAKE_OPTIONS = 1.7.4

noinst_LIBRARIES = libdisplay.a

libdisplay_a_SOURCES = display.c display.h display_stdio.c display_stdio.h \
	textblock.c textblock.h \
	keys.h colours.h

OPTIONAL_SRC = display_sdl.c display_sdl.h display_dos.c display_dos.h
libdisplay_a_LIBADD = @display_objects@
libdisplay_a_DEPENDENCIES = @display_objects@
EXTRA_libdisplay_a_SOURCE = $(OPTIONAL_SRC)
EXTRA_DIST = $(OPTIONAL_SRC)

# Test program
noinst_PROGRAMS = test
test_SOURCES = test.c
test_LDADD = libdisplay.a
