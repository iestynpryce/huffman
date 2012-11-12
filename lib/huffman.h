#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include <assert.h>

#include "file_stat.h"

typedef struct {
	uint8_t	buf;
	int	        len;
} Buffer;

int huffman(f_stat *in, f_stat *out);

#endif
