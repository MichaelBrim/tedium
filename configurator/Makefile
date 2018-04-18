CC = gcc
CFLAGS = -g

.PHONY: all clean

all: test

test: ini.o configurator.o test.o
	$(CC) -o $@ $^

clean:
	$(RM) *.o test

