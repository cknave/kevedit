# Makefile for structures library

AUTOMAKE_OPTIONS = 1.7.4

noinst_LIBRARIES = libstructures.a

libstructures_a_SOURCES = \
	gradient.c linkedlist.c selection.c svector.c \
	gradient.h linkedlist.h selection.h svector.h

# Test program
noinst_PROGRAMS = test
test_SOURCES = test.c
test_LDADD = libstructures.a
