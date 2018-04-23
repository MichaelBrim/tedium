CC = gcc
CFLAGS = -g

CXX = g++
CXXFLAGS = -g

TEST_PROGRAMS = test testpp

.PHONY: all clean

all: $(TEST_PROGRAMS)

test: ini.o configurator.o test.o
	$(CC) -o $@ $^

testpp: ini.o configurator.o testpp.o
	$(CXX) -o $@ $^

clean:
	$(RM) *.o $(TEST_PROGRAMS)

