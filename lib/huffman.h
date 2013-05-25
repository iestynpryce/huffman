#ifndef HUFFMAN_H
#define HUFFMAN_H


#include "file_stat.h"

/* Tree node structure */
typedef struct symbol
{
	struct symbol *next;
	struct symbol *parent;
	struct symbol *left;
	struct symbol *right;
	unsigned char  symbol;
	long int       weight;
	bool           code;
} Symbol;

/* Huffman encodes the input, `in' and outputs to `out' */
int huffman(f_stat *in, f_stat *out);

/* Huffman decodes the input, `in' and outputs to `out' */
int unhuffman(f_stat *in, f_stat *out);

#endif /* HUFFMAN_H */
