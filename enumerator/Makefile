CC = gcc
CFLAGS = -g

CXX = g++
CXXFLAGS = -g

TEST_PROGRAMS = test testpp

.PHONY: all clean

all: $(TEST_PROGRAMS)

test: enumerator.o test.o
	$(CC) -o $@ $^

testpp: enumerator.o testpp.o
	$(CXX) -o $@ $^

clean:
	$(RM) *.o $(TEST_PROGRAMS)

