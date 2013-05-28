/* Utility functions related to the huffman encoder.
 * Iestyn Pryce 2012/2013
 */

#ifndef _HUFFMAN_UTIL_H_
#define _HUFFMAN_UTIL_H_

#include "huffman.h"

/* Print out a linked list of symbols */
void print_ll(Symbol *s);

/* Print all the huffman codes from a tree */
void print_codes_from_tree(Symbol **leaves);

#endif /*_HUFFMAN_UTIL_H_ */
