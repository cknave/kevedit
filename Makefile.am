SUBDIRS = libzzt2 glob

bin_PROGRAMS = kevedit

kevedit_SOURCES = main.c \
	kevedit.c misc.c dosemu.c menu.c editbox.c screen.c \
	svector.c files.c zzm.c zzl.c selection.c zlaunch.c helplist.c hypertxt.c gradient.c \
	patbuffer.c help.c dialog.c infobox.c paramed.c register.c zoopdraw.c \
	panel.c panel_f1.c panel_f2.c panel_f3.c panel_ed.c panel_hl.c panel_bi.c panel_wi.c panel_g1.c panel_g2.c panel_dd.c panel_fd.c panel_fn.c panel_bd.c panel_sd.c panel_ti.c scroll.c tbox.c cbox.c tdialog.c \
	display.c display_sdl.c
#	display.c display_dos.c

kevedit_LDADD = $(GLOBLIB) libzzt2/libzzt2.a synth/libsynth.a
kevedit_LDFLAGS = `sdl-config --libs`

DEFS = -DSDL -DCANGLOB -DkeveditVERSION=\"autotest\" -DDATAPATH=\".\"

AM_CFLAGS =	$(GLOBINC) `sdl-config --cflags`

EXTRA_DIST = README