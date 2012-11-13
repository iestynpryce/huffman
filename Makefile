# Set compiler and its options
CC=gcc
CFLAGS=-g -Wall -Werror --std=c99 -O3
DEBUG=-DDEBUG
LDFLAGS=-I lib/

all: huff unhuff

# Build the encoder
huff: src/huffman.c lib/huffman.h file_stat.o
	$(CC) $(CFLAGS) $(LDFLAGS) src/huffman.c file_stat.o -o huffman

# Build the decoder
unhuff: src/huffman.c lib/huffman.h file_stat.o
	$(CC) $(CFLAGS) $(LDFLAGS) -DUNHUFFMAN src/huffman.c file_stat.o -o unhuffman

file_stat.o: lib/file_stat.h src/file_stat.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c src/file_stat.c

# Include debug flag in compilation
debug:  src/huffman.c lib/huffman.h file_stat.o
	$(CC) $(CFLAGS) $(DEBUG) $(LDFLAGS) src/huffman.c file_stat.o -o huffman
	$(CC) $(CFLAGS) $(DEBUG) $(LDFLAGS) -DUNHUFFMAN src/huffman.c file_stat.o -o unhuffman

# Run the regression tests
tests: huff unhuff
	./tests/run_tests.sh

# Build binary output tool
bd: tools/bd.c
	$(CC) $(CFLAGS) $(LDFLAGS) tools/bd.c -o bd

clean:
	rm -rf huffman unhuffman bd *.o
