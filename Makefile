# Makefile for KevEdit

CC = i586-pc-msdosdjgpp-gcc

GGI =
VCSA =
DOS =

# Comment the next 2 lines to disable GGI display
#GGI = -DGGI -lggi
#GGIOBJ = display_ggi.o
# Comment the next 2 lines to disable Linux console display
#VCSA = -DVCSA
#VCSAOBJ = display_vcsa.o
# Comment the next 2 lines to disable DOS display
DOS = -DDOS
DOSOBJ = display_dos.o

CFLAGS = -s -O6 $(GGI) $(VCSA) $(DOS) -DVERSION=\"0.2.1\"

# No more modifications below this line
# -------------------------------------

all: kevedit

clean:
	rm -f *.o kevedit

kevedit: display.o main.o panel.o panel_f1.o panel_f2.o panel_f3.o screen.o scroll.o tbox.o cbox.o libzzt.o $(GGIOBJ) $(VCSAOBJ) $(DOSOBJ)
	$(CC) -o $@ display.o main.o panel.o panel_f1.o panel_f2.o panel_f3.o screen.o scroll.o tbox.o cbox.o libzzt.o $(GGIOBJ) $(VCSAOBJ) $(DOSOBJ) $(CFLAGS)

display.o: display.c display.h
	$(CC) -o $@ display.c $(CFLAGS) -c
main.o: main.c display.h screen.h scroll.h kevedit.h zzt.h
	$(CC) -o $@ main.c $(CFLAGS) -c
screen.o: screen.c panel.h kevedit.h display.h zzt.h panel_f1.h panel_f2.h panel_f3.h tbox.h cbox.h
	$(CC) -o $@ screen.c $(CFLAGS) -c
scroll.o: scroll.c scroll.h
	$(CC) -o $@ scroll.c $(CFLAGS) -c
libzzt.o: libzzt.c zzt.h
	$(CC) -o $@ libzzt.c $(CFLAGS) -c

panel.o: panel.c panel.h
	$(CC) -o $@ panel.c $(CFLAGS) -c
panel_f1.o: panel_f1.c panel_f1.h
	$(CC) -o $@ panel_f1.c $(CFLAGS) -c
panel_f2.o: panel_f2.c panel_f2.h
	$(CC) -o $@ panel_f2.c $(CFLAGS) -c
panel_f3.o: panel_f3.c panel_f3.h
	$(CC) -o $@ panel_f3.c $(CFLAGS) -c
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
