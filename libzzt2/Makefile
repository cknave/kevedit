# Makefile for libzzt2

# Choose your compiler & archiver
#CC = i586-pc-msdosdjgpp-gcc
#AR = i586-pc-msdosdjgpp-ar cqs
CC = gcc
AR = ar cqs

RM = rm -f

# First line for optimization
# Second line for debug
OPTIMIZE = -s -O3 -fexpensive-optimizations -fomit-frame-pointer -finline-functions -funroll-loops -march=pentium
#OPTIMIZE = -g -Wall

# Uncomment when compiling from DOS
#DOS = -DDOS

CFLAGS = $(OPTIMIZE) $(DOS)

# No more modifications below this line
# -------------------------------------

OBJS = board.o file.o tiles.o params.o world.o

all: libzzt2.a test

libzzt2.a: $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

test: libzzt2.a test.c
	$(CC) -o $@ test.c libzzt2.a $(CFLAGS) -g

.c.o:
	$(CC) -o $@ $< $(CFLAGS) -c

clean:
	$(RM) *.o libzzt2.a

# Deps
board.o: board.c zzt.h
file.o: file.c zzt.h
tiles.o: tiles.c zzt.h
world.o: world.c zzt.h
params.o: params.c zzt.h
