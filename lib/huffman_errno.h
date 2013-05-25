/* Error codes and handling for huffman.h
 */

#ifndef __HUFFMAN_ERRNO_H__
#define __HUFFMAN_ERRNO_H__

typedef enum {
	HUFF_SUCCESS 	= 0, 	/* Success */
	HUFF_FAILURE 	= -1, 	/* Failure */
	HUFF_NOMEM   	= 1, 	/* Out of memory */
	HUFF_INVALIDARG = 2, 	/* Invalid function argument */
} HUFF_ERR;

#endif /* __HUFFMAN_ERRNO_H__ */
