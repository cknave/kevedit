# Makefile rules for libzzt2

include libzzt2.optimize

CFLAGS = $(OPTIMIZE) $(DOS)

# Objects

OBJS = board.o file.o tiles.o params.o world.o strtools.o zztoop.o

.c.o:
	$(CC) -o $@ $< $(CFLAGS) -c

# Targets

all: libzzt2.a test

libzzt2.a: $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

test: libzzt2.a test.c
	$(CC) -o $@ test.c libzzt2.a $(CFLAGS) -g

clean:
	$(RM) *.o libzzt2.a test test.exe

# Deps
board.o: board.c zzt.h
file.o: file.c zzt.h
tiles.o: tiles.c zzt.h
world.o: world.c zzt.h
params.o: params.c zzt.h

strtools.o: strtools.c strtools.h
zztoop.o: zztoop.c zztoop.h strtools.h
