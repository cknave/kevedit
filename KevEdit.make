# Makefile rules for building KevEdit in any environment

# Version & optimization information
include kevedit.version
include kevedit.optimize

# Set up display settings
ifeq ($(SDL),ON)
  SDL = -DSDL `sdl-config --cflags`
  SDLOBJ = display_sdl.o
  LDFLAGS += `sdl-config --libs`
endif

ifeq ($(VCSA),ON)
  VCSA = -DVCSA
  VCSAOBJ = display_vcsa.o
endif

ifeq ($(DOS),ON)
  DOS = -DDOS
	DOSOBJ = display_dos.o
endif

# Link in libglob
ifeq ($(BUILDGLOB),ON)
	CFLAGS += -DCANGLOB -Iglob
	GLOBLIB = glob/libglob.a
endif

PATHS = -DDATAPATH=\"$(datadir)\" -DBINPATH=\"$(bindir)\"

CFLAGS += $(OPTIMIZE) $(SDL) $(VCSA) $(DOS) $(VERSIONFLAG) $(PATHS)

# Objects

CENTRALOBJS = kevedit.o misc.o dosemu.o menu.o editbox.o screen.o
LIBRARYOBJS = libzzt2/libzzt2.a synth/synth.a svector.o files.o zzm.o zzl.o selection.o zlaunch.o helplist.o hypertxt.o gradient.o
MISCOBJS    = patbuffer.o help.o dialog.o infobox.o paramed.o register.o
DRAWOBJS    = panel.o panel_f1.o panel_f2.o panel_f3.o panel_ed.o panel_hl.o panel_bi.o panel_wi.o panel_g1.o panel_g2.o panel_dd.o panel_fd.o panel_fn.o panel_bd.o panel_sd.o panel_ti.o scroll.o tbox.o cbox.o tdialog.o
DISPLAYOBJS = display.o $(SDLOBJ) $(VCSAOBJ) $(DOSOBJ)

OBJECTS = $(CENTRALOBJS) $(LIBRARYOBJS) $(MISCOBJS) $(DRAWOBJS) $(DISPLAYOBJS) $(GLOBLIB) $(PLATFORMOBJS)

# Documents

DOCS = README AUTHORS TODO COPYING ChangeLog copying.txt windows.txt

.SUFFIXES: .o .c .h .rc

.c.o:
	$(CC) -o $@ $< $(CFLAGS) -c

.rc.o:
	windres -o $@ $<

# Targets
all: kevedit kevedit.zml

kevedit: $(OBJECTS) main.o
	$(CC) -o $@ $(OBJECTS) main.o $(CFLAGS) $(LDFLAGS)

kevedit.zml: docs/*.hlp permissions
	cd docs; ./makehelp.sh; cd ..

install: all
#TODO: use $(INSTALL) instead of mkdir
	mkdir -p $(bindir)
	mkdir -p $(datadir)
	mkdir -p $(docdir)
	$(INSTALL) $(BINARY) $(bindir)
	$(INSTALL) kevedit.zml $(datadir)
	$(INSTALL) $(DOCS) $(docdir)

uninstall:
	rm -f $(bindir)/$(BINARY)
	rm -f -R $(datadir)
	rm -f -R $(docdir)

clean:
	rm -f *.o kevedit kevedit.exe kevedit.zml synth/*.o
	make -C libzzt2 clean
	make -C synth clean
	make -C glob clean

# Sometimes permissions are not stored correctly
permissions:
	chmod a+x docs/makehelp.sh

# Libraries
libzzt2/libzzt2.a: libzzt2/*.c libzzt2/*.h
	make libzzt2.a -C libzzt2 -f $(MAKEFILE_NAME)

synth/synth.a: synth/*.c synth/*.h
	make synth.a   -C synth   -f $(MAKEFILE_NAME)

# Build libglob
ifeq ($(BUILDGLOB),ON)
glob/libglob.a: glob/*.c glob/*.h
	make -C glob
endif

# Dependancies

main.o: main.c kevedit.h libzzt2/zzt.h help.h register.h files.h display.h colours.h patbuffer.h

# Central KevEditing routines
kevedit.o: kevedit.c kevedit.h misc.h menu.h editbox.h screen.h libzzt2/zzt.h patbuffer.h help.h register.h infobox.h display.h
misc.o: misc.c misc.h kevedit.h editbox.h screen.h svector.h hypertxt.h selection.h gradient.h patbuffer.h display.h dosemu.h
dosemu.o: dosemu.c dosemu.h
menu.o: menu.c menu.h kevedit.h screen.h editbox.h libzzt2/zzt.h svector.h files.h zzl.h hypertxt.h patbuffer.h display.h
editbox.o: editbox.c editbox.h screen.h svector.h zzm.h colours.h register.h help.h scroll.h panel_ed.h display.h
screen.o: screen.c screen.h kevedit.h editbox.h libzzt2/zzt.h hypertxt.h zlaunch.h panel.h panel_f1.h panel_f2.h panel_f3.h scroll.h tbox.h cbox.h

# libzzt
libzzt.o: libzzt.c libzzt2/zzt.h

# Other libraries
svector.o: svector.c svector.h
files.o: files.c files.h svector.h
zzm.o: zzm.c zzm.h svector.h editbox.h kevedit.h
zzl.o: zzl.c zzl.h svector.h editbox.h kevedit.h
selection.o: selection.c
zlaunch.o: zlaunch.c zlaunch.h svector.h
helplist.o: helplist.c helplist.h svector.h
hypertxt.o: hypertxt.c hypertxt.h svector.h
gradient.o: gradient.c gradient.h
synth/notes.o: synth/notes.c synth/notes.h

# Misc
patbuffer.o: patbuffer.c kevedit.h libzzt2/zzt.h display.h
help.o: help.c help.h svector.h editbox.h hypertxt.h panel_hl.h helplist.h
dialog.o: dialog.c display.h svector.h
infobox.o: infobox.c infobox.h libzzt2/zzt.h display.h
paramed.o: paramed.c paramed.h libzzt2/zzt.h display.h dialog.h
register.o: register.c register.h editbox.h

# Draw data structures
panel.o: panel.c panel.h
panel_f1.o: panel_f1.c panel_f1.h # F1 panel
panel_f2.o: panel_f2.c panel_f2.h # F2 panel
panel_f3.o: panel_f3.c panel_f3.h # F3 panel
panel_ed.o: panel_ed.c panel_ed.h # Editbox panel
panel_hl.o: panel_hl.c panel_hl.h # Help panel
panel_bi.o: panel_bi.c panel_bi.h # Board Info panel
panel_wi.o: panel_wi.c panel_wi.h # World Info panel
panel_g1.o: panel_g1.c panel_g1.h # Gradient tool panel 1
panel_g2.o: panel_g2.c panel_g2.h # Gradient tool panel 2
panel_dd.o: panel_dd.c panel_dd.h # Directory Dialog
panel_fd.o: panel_fd.c panel_fd.h # File Dialog
panel_fn.o: panel_fn.c panel_fn.h # File Name Dialog
panel_bd.o: panel_bd.c panel_bd.h # Board Dialog
panel_sd.o: panel_sd.c panel_sd.h # Stats Dialog
panel_ti.o: panel_ti.c panel_ti.h # Tile Info
scroll.o: scroll.c scroll.h
tbox.o: tbox.c tbox.h
cbox.o: cbox.c cbox.h
tdialog.o: tdialog.c tdialog.h

# Display libraries
display.o: display.c display.h
display_ggi.o: display_ggi.c display.h display_ggi.h
display_vcsa.o: display_vcsa.c display.h display_vcsa.h
display_dos.o: display_dos.c display.h display_dos.h

# Windows resources
resources.o: resources.rc
