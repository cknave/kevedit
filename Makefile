# Makefile for KevEdit
MAKEFILE_NAME = Makefile

# Choose your compiler
#CC = i586-pc-msdosdjgpp-gcc
CC = gcc

# Uncomment next line to optimize kevedit
# Uncomment second line to not optimize and include debugging information
#OPTIMIZE = -s -O3 -fexpensive-optimizations -fomit-frame-pointer -finline-functions -funroll-loops -march=pentium
OPTIMIZE = -g -Wall

# Set SDL to ON to enable SDL display
SDL = ON
# Set VCSA to ON to enable VCSA display
VCSA =
# Set DOS to ON to enable DOS display
DOS =

include KevEdit.version
CFLAGS = $(OPTIMIZE) $(SDL) $(VCSA) $(DOS) $(KEVEDIT_VERSION)
LDFLAGS =

# No more modifications below this line
# -------------------------------------

include KevEdit.make
