TARGET = box
LIBS =
CC = gcc
CFLAGS = -g -O0 -Wall -Wno-discarded-qualifiers
EZXML = ezxml

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = main.o ezxml/ezxml.o box_archive.o filelist.o file.o ints.o
HEADERS = main.h ezxml/ezxml.h box_archive.h filelist.h file.c ints.h positions.h errors.h dbg.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
