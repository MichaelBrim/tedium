CC = gcc
CFLAGS = -g

CXX = g++
CXXFLAGS = -g

TEST_PROGRAMS = test testpp

CONFIGURATOR_OBJS = ini.o tinyexpr.o configurator.o

.PHONY: all clean

all: $(TEST_PROGRAMS)

test: $(CONFIGURATOR_OBJS) test.o
	$(CC) -o $@ $^

testpp: $(CONFIGURATOR_OBJS) testpp.o
	$(CXX) -o $@ $^

clean:
	$(RM) *.o $(TEST_PROGRAMS)

