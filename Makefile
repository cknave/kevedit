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

CFLAGS = $(OPTIMIZE) $(GGI) $(VCSA) $(DOS) -DVERSION=\"0.3.2\"

# No more modifications below this line
# -------------------------------------

OBJECTS = display.o main.o menu.o misc.o register.o patbuffer.o hypertxt.o zzm.o svector.o editbox.o panel.o panel_f1.o panel_f2.o panel_f3.o panel_ed.o screen.o scroll.o tbox.o cbox.o libzzt.o $(GGIOBJ) $(VCSAOBJ) $(DOSOBJ)

all: kevedit

clean:
	rm -f *.o kevedit

kevedit: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS)

display.o: display.c display.h
	$(CC) -o $@ display.c $(CFLAGS) -c
main.o: main.c display.h screen.h scroll.h kevedit.h zzt.h editbox.h register.h patbuffer.h misc.h menu.h
	$(CC) -o $@ main.c $(CFLAGS) -c
menu.o: menu.c
	$(CC) -o $@ menu.c $(CFLAGS) -c
misc.o: misc.c misc.h patbuffer.h
	$(CC) -o $@ misc.c $(CFLAGS) -c
screen.o: screen.c panel.h kevedit.h display.h zzt.h panel_f1.h panel_f2.h panel_f3.h tbox.h cbox.h editbox.h
	$(CC) -o $@ screen.c $(CFLAGS) -c
scroll.o: scroll.c scroll.h
	$(CC) -o $@ scroll.c $(CFLAGS) -c
libzzt.o: libzzt.c zzt.h
	$(CC) -o $@ libzzt.c $(CFLAGS) -c
svector.o: svector.c svector.h
	$(CC) -o $@ svector.c $(CFLAGS) -c
editbox.o: editbox.c editbox.h scroll.h colours.h svector.h panel_ed.h zzm.h register.h
	$(CC) -o $@ editbox.c $(CFLAGS) -c
zzm.o: zzm.c zzm.h svector.h editbox.h kevedit.h
	$(CC) -o $@ zzm.c $(CFLAGS) -c
register.o: register.c register.h editbox.h
	$(CC) -o $@ register.c $(CFLAGS) -c
patbuffer.o: patbuffer.c kevedit.h zzt.h display.h
	$(CC) -o $@ patbuffer.c $(CFLAGS) -c
hypertxt.o: hypertxt.c hypertxt.h svector.h
	$(CC) -o $@ hypertxt.c $(CFLAGS) -c

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
tbox.o: tbox.c tbox.h
	$(CC) -o $@ tbox.c $(CFLAGS) -c
cbox.o: cbox.c cbox.h
	$(CC) -o $@ cbox.c $(CFLAGS) -c

display_ggi.o: display_ggi.c display.h display_ggi.h
	$(CC) -o $@ display_ggi.c $(CFLAGS) -c
display_vcsa.o: display_vcsa.c display.h display_vcsa.h
	$(CC) -o $@ display_vcsa.c $(CFLAGS) -c
display_dos.o: display_dos.c display.h display_dos.h
	$(CC) -o $@ display_dos.c $(CFLAGS) -c
