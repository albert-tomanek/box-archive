TARGET = box
LIBS =
CC = gcc
CFLAGS = -g -O0 -Wall
EZXML = ezxml

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = main.o ezxml/ezxml.o box_archive.o filelist.o ints.o
HEADERS = main.h ezxml/ezxml.h box_archive.h filelist.h ints.h positions.h errors.h dbg.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
