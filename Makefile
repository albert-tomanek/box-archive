TARGET = box
LIBS =
CC = gcc
CFLAGS = -g -O0 -Wall -Wno-discarded-qualifiers -Wno-deprecated-declarations -Wno-unused-label
EZXML = ezxml

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = main.o ezxml/ezxml.o box_archive.o file.o entry.o entrylist.o filesystem.o dupcat.o byteorder.o ints.o
HEADERS = main.h ezxml/ezxml.h box_archive.h file.h entry.h entrylist.h filesystem.h dupcat.h byteorder.h ints.h positions.h errors.h dbg.h

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
