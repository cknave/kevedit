# Makefile for KevEdit in Linux
MAKEFILE_NAME = Makefile

# Compile using GCC
CC = gcc

# Set SDL to ON to enable SDL display
SDL = ON
# Set VCSA to ON to enable VCSA display
VCSA =
# Set DOS to ON to enable DOS display
DOS =

# Installation directories
root =
prefix = $(root)/usr/local
bindir = $(prefix)/bin
datadir = $(prefix)/share/kevedit
docdir = $(prefix)/doc/kevedit-$(VERSION)

# Target binary
BINARY = kevedit

# Install program
INSTALL = /usr/bin/install -c

# Compile and link flags
CFLAGS = -DCANGLOB
LDFLAGS =

# Makefile rules
include KevEdit.make
