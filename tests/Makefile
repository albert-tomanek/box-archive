TARGET = tests
LIBS = -lcmocka
CC = gcc
CFLAGS = -g -O0 -Wall
WRAP = --wrap=ba_get_metadata,--wrap=ba_fsize
BALIB = ../libboxarchive.0.09.a

.PHONY: default all clean tests files

default: $(TARGET)
all: default

TESTFILES = ba_new.o ba_add_dir.o ba_add_file.c

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): files
	$(CC) -Wl,$(WRAP) -o $(TARGET) tests.c $(TESTFILES) $(BALIB) $(LIBS)

files:
	$(CC) $(CFLAGS) -c ba_new.c
	$(CC) $(CFLAGS) -c ba_add_dir.c
	$(CC) $(CFLAGS) -c ba_add_file.c

clean:
	-rm -f *.o
	-rm -f $(TARGET)

# gcc -Wl,--wrap=fgets -L/usr/lib -o tests tests.c ../libboxarchive.0.09.a -lcmocka
