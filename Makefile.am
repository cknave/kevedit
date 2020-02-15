# Makefile for KevEdit

AUTOMAKE_OPTIONS = 1.6.3

SUBDIRS = docs dosbox glob src

EXTRA_DIST = copying.txt soundfx.zzm bootstrap.sh
pkgdata_DATA = soundfx.zzm

#if FREEDESKTOP
desktopdir = $(datadir)/applications
desktop_DATA = inst/platform/linux/kevedit.desktop

ICON_SIZES = 16 32 128 256 512

install-icons:
	for size in $(ICON_SIZES); do \
		$(INSTALL_DATA) -D $(srcdir)/inst/icon$${size}.png $(DESTDIR)$(datadir)/icons/hicolor/$${size}x$${size}/apps/kevedit.png; \
	done

uninstall-icons:
	for size in $(ICON_SIZES); do \
		rm -f $(datadir)/icons/hicolor/$${size}x$${size}/apps/kevedit.png; \
	done

install-data-local: install-icons
uninstall-local: uninstall-icons
#endif
