# -*-Makefile-*-, or close enough

#AUTOMAKE_OPTIONS =	1.6 foreign

noinst_LIBRARIES = libsynth.a

libsynth_a_SOURCES = notes.c notes.h synth.h zzm.c zzm.h

EXTRA_libsynth_a_SOURCES = sdl_synth.c sdl_synth.h pcspeaker.c pcspeaker.h

libsynth_a_LIBADD = $(SYNTH_SDL) $(SYNTH_PCSPEAKER)
libsynth_a_DEPENDENCIES = $(SYNTH_SDL) $(SYNTH_PCSPEAKER)

