#ifndef HUFFMAN_H
#define HUFFMAN_H


#include "file_stat.h"

/* Huffman encodes the input, `in' and outputs to `out' */
int huffman(f_stat *in, f_stat *out);

/* Huffman decodes the input, `in' and outputs to `out' */
int unhuffman(f_stat *in, f_stat *out);

#endif /* HUFFMAN_H */
