SUBDIRS = libzzt2 synth glob

bin_PROGRAMS = kevedit

kevedit_SOURCES = main.c \
	kevedit.c misc.c dosemu.c menu.c editbox.c screen.c \
	svector.c files.c zzm.c zzl.c selection.c zlaunch.c helplist.c hypertxt.c gradient.c \
	patbuffer.c help.c dialog.c infobox.c paramed.c register.c zoopdraw.c \
	panel.c panel_f1.c panel_f2.c panel_f3.c panel_ed.c panel_hl.c panel_bi.c panel_wi.c panel_g1.c panel_g2.c panel_dd.c panel_fd.c panel_fn.c panel_bd.c panel_sd.c panel_ti.c scroll.c tbox.c cbox.c tdialog.c \
	display.c display.h

if SDL
kevedit_SOURCES += display_sdl.c display_sdl.h
endif

if DOS
kevedit_SOURCES += display_dos.c display_dos.h
endif

if HAVE_WINDRES
kevedit_SOURCES += icon.rc
endif

kevedit_LDADD = $(GLOBLIB) libzzt2/libzzt2.a synth/libsynth.a

kevedit_DEPENDENCIES = $(GLOBLIB) libzzt2/libzzt2.a synth/libsynth.a

INCLUDES = $(GLOBINC)

EXTRA_DIST = docs

pkgdata_DATA = kevedit.zml

.rc.o:
	$(WINDRES) -o $@ $<

icon.o: icon.rc

icon.rc:
	echo kevedit ICON \"$(srcdir)/kevedit.ico\" > icon.rc

kevedit.zml: $(srcdir)/docs/*.hlp
	chmod a+x $(srcdir)/docs/makehelp.sh
	$(srcdir)/docs/makehelp.sh $(srcdir)/docs .

