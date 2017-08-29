TARGET = tests
LIBS = -lcmocka
CC = gcc
CFLAGS = -g -O0 -Wall
WRAP = ba_get_metadata,ba_fsize
BALIB = ../libboxarchive.0.09.a

.PHONY: default all clean tests files

default: $(TARGET)
all: default

TESTFILES = ba_new.o ba_add_dir.o

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): files
	$(CC) -Wl,--wrap=$(WRAP) -o $(TARGET) tests.c $(TESTFILES) $(BALIB) $(LIBS)

files:
	$(CC) -c ba_add_dir.c
	$(CC) -c ba_new.c

clean:
	-rm -f *.o
	-rm -f $(TARGET)

# gcc -Wl,--wrap=fgets -L/usr/lib -o tests tests.c ../libboxarchive.0.09.a -lcmocka
