# -*-Makefile-*-, or close enough

#AUTOMAKE_OPTIONS =	1.6.3 foreign

noinst_LIBRARIES = libsynth.a

libsynth_a_SOURCES = notes.c notes.h synth.h zzm.c zzm.h

if SDL
libsynth_a_SOURCES += sdl_synth.c sdl_synth.h
endif

if DOS
libsynth_a_SOURCES += pcspeaker.c pcspeaker.h
endif

EXTRA_DIST = $(OPTIONAL_SRC) play.c
