# BA tests makefile

TARGET = tests
LIBS =
CCPP = g++
CC   = gcc
CFLAGS = -O0 -g -Wno-write-strings

UTPP_LIB = /usr/local/lib/libUnitTest++.a

HEADERS_DIR = ../

TEST_SUBJECTS = ../libboxarchive.0.09.a
TEST_CODE     = test_functs.o ba.test.o

.PHONY: default all clean

default: $(TARGET)

all: default

%.o: %.cpp
	$(CCPP) -isystem $(HEADERS_DIR) $(CFLAGS) -o $@ -c $<

%.o: %.c
	$(CC) -isystem $(HEADERS_DIR) $(CFLAGS) -o $@ -c $<

$(TARGET): $(TEST_CODE)
	$(CCPP) $(TEST_CODE) $(TEST_SUBJECTS) tests.cpp $(UTPP_LIB) $(CFLAGS) -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	-rm -f *.o *.a
	-rm -f $(TARGET)
