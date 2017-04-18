# http://stackoverflow.com/questions/2734719/how-to-compile-a-static-library-in-linux

TARGET = box
LIBVER = 0.09
LIBNAME = libboxarchive.$(LIBVER).a
CC = gcc
CFLAGS = -g -Wall -Wno-discarded-qualifiers -Wno-deprecated-declarations -Wno-unused-label

.PHONY: default all clean lib

default: $(TARGET)
all: default

LIBFILES = box_archive.o file.o entry.o entrylist.o filesystem.o dupcat.o byteorder.o
MAIN_SOURCE = main.c
HEADERS = ezxml/ezxml.h box_archive.h file.h entry.h entrylist.h filesystem.h dupcat.h byteorder.h positions.h errors.h dbg.h

EZXML_DIR = ezxml
EZXML_LIB = $(EZXML_DIR)/libezxml.a
EZXML_OBJ = ezxml.o

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

lib: $(LIBFILES)
	make -C $(EZXML_DIR)
	ar -x $(EZXML_LIB)
	ar -rcs $(LIBNAME) $(EZXML_OBJ) $(LIBFILES)

$(TARGET): $(OBJECTS) lib			# These arguments (eg $(OBJECTS)) are the things that need to be done before the following code can be done
	$(CC) $(CFLAGS) $(MAIN_SOURCE) $(LIBNAME) -Wall $(LIBS) -o $@

install: 
	cp $(TARGET) /usr/bin
	cp $(LIBNAME) /usr/lib
	
	mkdir /usr/include/box_archive
	cp box_archive.h /usr/include/box_archive
	cp entry.h       /usr/include/box_archive
	cp file.h        /usr/include/box_archive
	cp types.h       /usr/include/box_archive
	
	mkdir /usr/include/box_archive/ezxml
	cp ezxml/ezxml.h /usr/include/box_archive/ezxml

uninstall:
	rm /usr/bin/$(TARGET)
	rm /usr/lib/$(LIBNAME)
	
	rm -r /usr/include/box_archive

clean:
	make -C $(EZXML_DIR) clean
	-rm -f *.o
	-rm -f $(LIBNAME)
	-rm -f $(TARGET)
