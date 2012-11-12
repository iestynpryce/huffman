# Set compiler and its options
CC=gcc
CFLAGS=-g -Wall -Werror --std=c99 -O3
DEBUG=-DDEBUG
LDFLAGS=-I lib/

all: huff unhuff

# Build the encoder
huff: src/huffman.c lib/huffman.h src/file_stat.c lib/file_stat.h
	$(CC) $(CFLAGS) $(LDFLAGS) src/huffman.c src/file_stat.c -o huffman

# Build the decoder
unhuff: src/unhuffman.c lib/huffman.h
	$(CC) $(CFLAGS) $(LDFLAGS) src/unhuffman.c -o unhuffman

# Include debug flag in compilation
debug:  src/huffman.c src/unhuffman.c lib/huffman.h 
	$(CC) $(CFLAGS) $(DEBUG) $(LDFLAGS) src/huffman.c src/file_stat.c -o huffman
	$(CC) $(CFLAGS) $(DEBUG) $(LDFLAGS) src/unhuffman.c -o unhuffman

# Run the regression tests
tests: huff unhuff
	./tests/run_tests.sh

# Build binary output tool
bd: tools/bd.c
	$(CC) $(CFLAGS) $(LDFLAGS) tools/bd.c -o bd

clean:
	rm -rf huffman unhuffman bd
