# Makefile for KevEdit

AUTOMAKE_OPTIONS = 1.6.3

SUBDIRS = docs dosbox glob src

EXTRA_DIST = README.md bootstrap.sh default.zln inst/.dockerignore inst/build.py inst/Dockerfile.appimage \
	inst/Dockerfile.dos inst/Dockerfile.macos inst/Dockerfile.macos_sdk_extractor inst/Dockerfile.source \
	inst/Dockerfile.windows inst/icon16.png inst/icon24.png inst/icon32.png inst/icon48.png inst/icon128.png \
	inst/icon256.png inst/icon512.png inst/icon.svg inst/platform legal.md soundfx.zzm \
	src/themes/rusty/c2bin.py

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
		rm -f $(DESTDIR)$(datadir)/icons/hicolor/$${size}x$${size}/apps/kevedit.png; \
	done

install-data-local: install-icons
uninstall-local: uninstall-icons
#endif
