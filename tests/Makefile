# BA tests makefile

TARGET = tests
LIBS =
CC = g++
CFLAGS = -O0 -g 

UTPP_LIB = /usr/local/lib/libUnitTest++.a

HEADERS_DIR = ../

TEST_SUBJECTS = ../libboxarchive.0.09.a
TEST_CODE     = ba.test.cpp

.PHONY: default all clean

default: $(TARGET)

all: default

%.o: %.cpp
	$(CC) -isystem $(HEADERS_DIR) $(CFLAGS) -o $@ -c $<

$(TARGET): $(TEST_CODE)
	$(CC) $(TEST_SUBJECTS) $(TEST_CODE) tests.cpp $(UTPP_LIB) $(CFLAGS) -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	-rm -f *.o *.a
	-rm -f $(TARGET)
