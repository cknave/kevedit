# Makefile for KevEdit text editor

AUTOMAKE_OPTIONS = 1.6.3

noinst_LIBRARIES = libtexteditor.a

libtexteditor_a_SOURCES = \
	lineeditor.c lineeditor.h \
	display.c editbox.c register.c select.c zoopdraw.c zzl.c zzm.c \
	display.h editbox.h register.h select.h zoopdraw.h zzl.h zzm.h \
	texteditor.c texteditor.h

if MACOS
macos_SOURCES = $(top_srcdir)/src/kevedit/macos.m
macos_frameworks = -framework Cocoa
endif

noinst_PROGRAMS = test
test_SOURCES = test.c $(macos_SOURCES)
test_LDADD = libtexteditor.a ../display/libdisplay.a ../libzzt2/libzzt2.a
test_LDFLAGS = $(macos_frameworks)
