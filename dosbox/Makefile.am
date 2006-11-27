# Makefile for KevEdit DOSBox support

AUTOMAKE_OPTIONS = 1.6.3

if WITH_DOSBOX
isofile = kevedos.iso
endif

pkgdata_DATA = kevedos.cfg $(isofile)

EXTRA_DIST = README

MOSTLYCLEANFILES = kevedos.iso

kevedos.iso: image $(wildcard image/*)
	@MKISOFS@ -o $@ image
