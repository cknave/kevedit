SUBDIRS = docs libzzt2 synth glob

bin_PROGRAMS = kevedit

kevedit_SOURCES = main.c \
	kevedit.c misc.c dosemu.c menu.c editbox.c screen.c \
	svector.c files.c zzm.c zzl.c selection.c zlaunch.c helplist.c hypertxt.c gradient.c \
	patbuffer.c help.c dialog.c infobox.c paramed.c register.c zoopdraw.c \
	panel.c panel_f1.c panel_f2.c panel_f3.c panel_ed.c panel_hl.c panel_bi.c panel_wi.c panel_g1.c panel_g2.c panel_dd.c panel_fd.c panel_fn.c panel_bd.c panel_sd.c panel_ti.c scroll.c tbox.c cbox.c tdialog.c \
	display.c display.h \
	cbox.h colours.h dialog.h dosemu.h editbox.h files.h gradient.h help.h helplist.h hypertxt.h infobox.h kevedit.h menu.h misc.h panel_bd.h panel_bi.h panel_dd.h panel_ed.h panel_f1.h panel_f2.h panel_f3.h panel_fd.h panel_fn.h panel_g1.h panel_g2.h panel.h panel_hl.h panel_sd.h panel_ti.h panel_wi.h paramed.h patbuffer.h register.h screen.h scroll.h selection.h svector.h tbox.h tdialog.h zlaunch.h zoopdraw.h zzl.h zzm.h zzt.h

if SDL
kevedit_SOURCES += display_sdl.c display_sdl.h
endif

if DOS
kevedit_SOURCES += display_dos.c display_dos.h
endif

if HAVE_WINDRES
noinst_kevedit_SOURCES = icon.rc
endif

kevedit_LDADD = $(GLOBLIB) libzzt2/libzzt2.a synth/libsynth.a

kevedit_DEPENDENCIES = $(GLOBLIB) libzzt2/libzzt2.a synth/libsynth.a

INCLUDES = $(GLOBINC)

EXTRA_DIST = copying.txt soundfx.zzm

pkgdata_DATA = soundfx.zzm

.rc.o:
	$(WINDRES) -o $@ $<

icon.o: icon.rc

icon.rc:
	echo kevedit ICON \"$(srcdir)/kevedit.ico\" > icon.rc

