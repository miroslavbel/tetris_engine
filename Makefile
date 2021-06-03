### This is Windows dependent Makefile

### ?= doesn't work for me and make don't see $(CC) at all
CC=clang
CFLAGS=-O3

HEADER_DIR=include/
SOURCE_DIR=src/
BUILD_DIR=build/

HEADER=include/engine.h
SOURCE=src/engine.c
OBJECT=build/engine.o
DOXYFILE=Doxyfile

clean-doc:
	if exist doc rmdir doc /s /q

clean-build:
	if exist build rmdir build /s /q

clean: clean-doc clean-build

# make html documentation using doxygen
install-html: $(HEADER) $(DOXYFILE)
	doxygen $(DOXYFILE)

#------ Generate documentation files in the given format.
# These targets should always exist, but any or all can be a no-op if the given
# output format cannot be generated.
dvi: ;
pdf: ;
ps: ;
html: install-html

build-obj: $(SOURCE) $(HEADER)
	if not exist build mkdir build
	$(CC) $(CFLAGS) -o $(OBJECT) -c $(SOURCE) -I$(HEADER_DIR)

install: build-obj doc

all: build-obj

.DEFAULT_GOAL := all
