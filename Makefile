# Set compiler and its options
CC=gcc
CFLAGS=-g -Wall -Werror --std=c99 -O3 -D_POSIX_C_SOURCE=200112L
DEBUG=-DDEBUG
LDFLAGS=-I lib/

all: cli

cli: src/huffman-cli.c huffman.o file_stat.o
	$(CC) $(CFLAGS) $(LDFLAGS) src/huffman-cli.c huffman.o file_stat.o -o huffman
	$(CC) $(CFLAGS) $(LDFLAGS) -DUNHUFFMAN src/huffman-cli.c huffman.o file_stat.o -o unhuffman

# Build the encoder
huffman.o: src/huffman.c lib/huffman.h 
	$(CC) $(CFLAGS) $(LDFLAGS) -c src/huffman.c 

file_stat.o: lib/file_stat.h lib/file_stat_error.h src/file_stat.c src/file_stat_error.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c src/file_stat.c

# Include debug flag in compilation
debug:  src/huffman.c lib/huffman.h file_stat.o
	$(CC) $(CFLAGS) $(DEBUG) $(LDFLAGS) src/huffman-cli.c src/huffman.c file_stat.o -o huffman
	$(CC) $(CFLAGS) $(DEBUG) $(LDFLAGS) -DUNHUFFMAN src/huffman-cli.c src/huffman.c file_stat.o -o unhuffman

# Build the unit tests
unittest: tests/src/test_file_stat.c tests/src/test_huffman.c tests/src/minunit.h file_stat.o huffman.o
	$(CC) $(CDFLAGS) $(DEBUG) $(LDFLAGS) tests/src/test_file_stat.c file_stat.o -o tests/c_test_file_stat
	$(CC) $(CDFLAGS) $(DEBUG) $(LDFLAGS) tests/src/test_huffman.c huffman.o file_stat.o -o tests/c_test_huffman

# Run the regression tests
tests: cli unittest
	./tests/run_tests.sh
	./tests/c_test_file_stat
	./tests/c_test_huffman

# Build binary output tool
bd: tools/bd.c
	$(CC) $(CFLAGS) $(LDFLAGS) tools/bd.c -o bd

clean:
	rm -rf huffman unhuffman bd *.o tests/c_test*
