# Makefile for glob in Windows
CC = i686-pc-cygwin-gcc -mno-cygwin
AR = i686-pc-cygwin-ar cqs
# crus?

# Grab those optimizations
include glob.optimize

CFLAGS = -DHAVE_CONFIG_H -DWINDOWS32 -I. $(OPTIMIZE)
OBJS = glob.o fnmatch.o

.SUFFIXES: .o .c .h

.c.o:
	$(CC) -o $@ $< $(CFLAGS) -c

# The glob library
libglob.a: $(OBJS)
	$(AR) $@ $(OBJS)

clean:
	rm -f *.o *.a

#Dependancies
glob.c: glob.h config.h
fnmatch.c: fnmatch.h

