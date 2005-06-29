# Makefile for KevEdit DOSBox support

AUTOMAKE_OPTIONS = 1.7

pkgdata_DATA = kevedos.cfg kevedos.iso

EXTRA_DIST = README

MOSTLYCLEANFILES = kevedos.iso

kevedos.iso: image $(wildcard image/*)
	mkisofs -o $@ image
