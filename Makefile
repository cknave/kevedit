# Makefile for KevEdit

# Choose your compiler
CC = i586-pc-msdosdjgpp-gcc
#CC = gcc

# Uncomment next line to optimize kevedit
# Uncomment second line to not optimize and include debugging information
OPTIMIZE = -s -O3 -fexpensive-optimizations -fomit-frame-pointer -finline-functions -funroll-loops -march=pentium
#OPTIMIZE = -g -Wall

# Set CGI to ON to enable GGI display
GGI =
# Set VCSA to ON to enable VCSA display
VCSA =
# Set DOS to ON to enable DOS display
DOS = ON

# Set up display settings
ifeq ($(GGI),ON)
  GGI = -DGGI -lggi
  GGIOBJ = display_ggi.o
endif

ifeq ($(VCSA),ON)
  VCSA = -DVCSA
  VCSAOBJ = display_vcsa.o
endif

ifeq ($(DOS),ON)
  DOS = -DDOS
	DOSOBJ = display_dos.o
endif

CFLAGS = $(OPTIMIZE) $(GGI) $(VCSA) $(DOS) -DVERSION=\"0.3.4\"

# No more modifications below this line
# -------------------------------------

CENTRALOBJS = main.o misc.o menu.o editbox.o screen.o
LIBRARYOBJS = libzzt.o svector.o files.o zzm.o zzl.o selection.o zlaunch.o helplist.o hypertxt.o
MISCOBJS    = patbuffer.o help.o infobox.o register.o
DRAWOBJS    = panel.o panel_f1.o panel_f2.o panel_f3.o panel_ed.o panel_hl.o scroll.o tbox.o cbox.o
DISPLAYOBJS = display.o $(GGIOBJ) $(VCSAOBJ) $(DOSOBJ)

OBJECTS = $(CENTRALOBJS) $(LIBRARYOBJS) $(MISCOBJS) $(DRAWOBJS) $(DISPLAYOBJS)

all: kevedit

clean:
	rm -f *.o kevedit

kevedit: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS)

# Central KevEditing routines
main.o: main.c kevedit.h misc.h menu.h editbox.h screen.h zzt.h patbuffer.h help.h register.h infobox.h display.h
	$(CC) -o $@ main.c $(CFLAGS) -c
misc.o: misc.c misc.h kevedit.h editbox.h screen.h svector.h hypertxt.h selection.h patbuffer.h display.h
	$(CC) -o $@ misc.c $(CFLAGS) -c
menu.o: menu.c menu.h kevedit.h screen.h editbox.h zzt.h svector.h files.h zzl.h hypertxt.h patbuffer.h display.h
	$(CC) -o $@ menu.c $(CFLAGS) -c
editbox.o: editbox.c editbox.h screen.h svector.h zzm.h colours.h register.h help.h scroll.h panel_ed.h display.h
	$(CC) -o $@ editbox.c $(CFLAGS) -c
screen.o: screen.c screen.h kevedit.h editbox.h zzt.h hypertxt.h zlaunch.h panel.h panel_f1.h panel_f2.h panel_f3.h scroll.h tbox.h cbox.h
	$(CC) -o $@ screen.c $(CFLAGS) -c

# libzzt
libzzt.o: libzzt.c zzt.h
	$(CC) -o $@ libzzt.c $(CFLAGS) -c

# Other libraries
svector.o: svector.c svector.h
	$(CC) -o $@ svector.c $(CFLAGS) -c
files.o: files.c files.h svector.h
	$(CC) -o $@ files.c $(CFLAGS) -c
zzm.o: zzm.c zzm.h svector.h editbox.h kevedit.h
	$(CC) -o $@ zzm.c $(CFLAGS) -c
zzl.o: zzl.c zzl.h svector.h editbox.h kevedit.h
	$(CC) -o $@ zzl.c $(CFLAGS) -c
selection.o: selection.c
	$(CC) -o $@ selection.c $(CFLAGS) -c
zlaunch.o: zlaunch.c zlaunch.h svector.h
	$(CC) -o $@ zlaunch.c $(CFLAGS) -c
helplist.o: helplist.c helplist.h svector.h
	$(CC) -o $@ helplist.c $(CFLAGS) -c
hypertxt.o: hypertxt.c hypertxt.h svector.h
	$(CC) -o $@ hypertxt.c $(CFLAGS) -c

# Misc
patbuffer.o: patbuffer.c kevedit.h zzt.h display.h
	$(CC) -o $@ patbuffer.c $(CFLAGS) -c
help.o: help.c help.h svector.h editbox.h hypertxt.h panel_hl.h helplist.h
	$(CC) -o $@ help.c $(CFLAGS) -c
infobox.o: infobox.c infobox.h zzt.h display.h
	$(CC) -o $@ infobox.c $(CFLAGS) -c
register.o: register.c register.h editbox.h
	$(CC) -o $@ register.c $(CFLAGS) -c

# Draw data structures
panel.o: panel.c panel.h
	$(CC) -o $@ panel.c $(CFLAGS) -c
panel_f1.o: panel_f1.c panel_f1.h
	$(CC) -o $@ panel_f1.c $(CFLAGS) -c
panel_f2.o: panel_f2.c panel_f2.h
	$(CC) -o $@ panel_f2.c $(CFLAGS) -c
panel_f3.o: panel_f3.c panel_f3.h
	$(CC) -o $@ panel_f3.c $(CFLAGS) -c
panel_ed.o: panel_ed.c panel_ed.h
	$(CC) -o $@ panel_ed.c $(CFLAGS) -c
panel_hl.o: panel_hl.c panel_hl.h
	$(CC) -o $@ panel_hl.c $(CFLAGS) -c
scroll.o: scroll.c scroll.h
	$(CC) -o $@ scroll.c $(CFLAGS) -c
tbox.o: tbox.c tbox.h
	$(CC) -o $@ tbox.c $(CFLAGS) -c
cbox.o: cbox.c cbox.h
	$(CC) -o $@ cbox.c $(CFLAGS) -c

# Display libraries
display.o: display.c display.h
	$(CC) -o $@ display.c $(CFLAGS) -c
display_ggi.o: display_ggi.c display.h display_ggi.h
	$(CC) -o $@ display_ggi.c $(CFLAGS) -c
display_vcsa.o: display_vcsa.c display.h display_vcsa.h
	$(CC) -o $@ display_vcsa.c $(CFLAGS) -c
display_dos.o: display_dos.c display.h display_dos.h
	$(CC) -o $@ display_dos.c $(CFLAGS) -c
