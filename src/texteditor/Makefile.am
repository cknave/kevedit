# Makefile for KevEdit text editor

AUTOMAKE_OPTIONS = 1.7.4

noinst_LIBRARIES = libtexteditor.a

libtexteditor_a_SOURCES = \
	lineeditor.c lineeditor.h \
	display.c editbox.c register.c select.c zoopdraw.c zzl.c zzm.c \
	display.h editbox.h register.h select.h zoopdraw.h zzl.h zzm.h \
	texteditor.c texteditor.h


noinst_PROGRAMS = test
test_SOURCES = test.c
test_LDADD = libtexteditor.a ../display/libdisplay.a ../libzzt2/libzzt2.a

