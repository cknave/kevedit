# Makefile rules for libzzt2

include synth.optimize

CFLAGS = $(OPTIMIZE) $(DOS) $(SDL)

ifdef SDL
  SDL = -DSDL `sdl-config --cflags`
  LDFLAGS += `sdl-config --libs`

	SDLOBJ = sdl_synth.o
endif

ifdef DOS
	DOS = -DDOS

	DOSOBJ = pcspeaker.o
endif

# Objects
OBJS = notes.o zzm.o $(SDLOBJ) $(DOSOBJ)

.c.o:
	$(CC) -o $@ $< $(CFLAGS) -c

# Targets

all: synth.a play

synth.a: $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

play: synth.a play.c
	$(CC) -o $@ play.c synth.a $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) *.o synth.a play play.exe

# Deps
notes.o: notes.h
zzm.o: zzm.h notes.h

sdl_synth.o: sdl_synth.h notes.h
pcspeaker.o: pcspeaker.h notes.h
