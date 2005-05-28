# -*-Makefile-*-, or close enough

#AUTOMAKE_OPTIONS =	1.6 foreign

noinst_LIBRARIES = libsynth.a

libsynth_a_SOURCES = notes.c notes.h synth.h zzm.c zzm.h

OPTIONAL_SRC = sdl_synth.c sdl_synth.h pcspeaker.c pcspeaker.h
libsynth_a_LIBADD = @synth_objects@
libsynth_a_DEPENDENCIES = @synth_objects@
EXTRA_libsynth_a_SOURCE = $(OPTIONAL_SRC)
EXTRA_DIST = $(OPTIONAL_SRC)
