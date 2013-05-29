/* Huffman Encoder
 * 
 * Iestyn Pryce 2012
 */

#include "huffman.h"
#include "huffman_util.h"
#include "huffman_errno.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

/* Structure to build a linked list of bits */
typedef struct bits
{
	bool         bit;
        struct bits *next;
} Bits;

/* Structure to build a linked list of symbols and bit codes */
typedef struct code
{
	struct bits   *code;
	unsigned int   length;
	unsigned char  symbol;
	struct code   *next;
} Code;

typedef struct node 
{
	char value;
	struct node *left;
	struct node *right;
} Node;

typedef struct {
	uint8_t	buf;
	int	        len;
} Buffer;

HUFF_ERR  _output_byte(Node **ret_node, Buffer *b, Node *n, Node *top, int stop, FILE *output);
void _free_tree(Symbol *t);

/* Comparison function to be used by the C library qsort(...) function */
int _symbol_cmp (const void *s1, const void *s2)
{
	/* Validate the input */
	assert(s1 != NULL && s2 != NULL);

	const Symbol *_s1 = *(Symbol **)s1;
	const Symbol *_s2 = *(Symbol **)s2;
	return _s1->weight - _s2->weight;
}

/* Sort the linked list by swapping the values
 * Uses the stdlib qsort algorithm
 */
HUFF_ERR _sort_symbol_list(Symbol **start, Symbol* s)
{
	int i;
	int count = 0;
	
	*start = s;
	
	/* Count number of elements in the linked list */
	do
	{
		count++;
	} while ((s=s->next));
	
	/* Build an array of pointers to symbols*/
	Symbol **symbol_list = calloc(count,sizeof(Symbol*));
	s = *start;

	for (i=0; i<count; i++)
	{
		symbol_list[i] = s;
		s = s->next;
	}

	/* Sort using qsort */
	qsort(symbol_list, count, sizeof (Symbol*), (void*)_symbol_cmp);

	/* Rebuild the linked list */
	*start = symbol_list[0];
	s = *start;
	for (i=1; i<count; i++)
	{
		s->next = symbol_list[i];
		s = s->next;
	}
	s->next = NULL;

	free(symbol_list);

	return HUFF_SUCCESS;
}

/* Create a linked list of statistics for bytes in the input */
HUFF_ERR _build_statistics(Symbol **root, f_stat *fp)
{
	assert(fp != NULL);

	Symbol *start, *s;
	int c;

	start = calloc(1,sizeof(Symbol));
	if (start == NULL )
	{
		/* Out of memory */
		perror("Unable to allocate memory");
		return HUFF_NOMEM;
	}

	start->symbol = fgetc_stat(fp);
	start->weight++;

	/* Copy the starting position */
	s = start;

	while((c=fgetc_stat(fp))!=EOF)
	{
		while (s->symbol != c && s->next != NULL)
		{
			s = s->next;
		}
		if (s->symbol != c)
		{
			s->next = calloc(1,sizeof(Symbol));
			if (s->next == NULL)
			{
				/* Out of memory */
				perror("Unable to allocate memory");
				_free_tree(start);
				return HUFF_NOMEM;
			}
			s=s->next;
		}

		s->symbol = c;
		s->weight++;

		s=start;
	}
	
	return _sort_symbol_list(root,start);
}

HUFF_ERR _build_tree(Symbol ***leaves, Symbol *s)
{
	assert (s != NULL);

	Symbol *node = NULL;
	HUFF_ERR rc = HUFF_SUCCESS;

	unsigned int nleaves = UCHAR_MAX+1; /* 2**CHAR_BIT */
	int n = 0;
	
	/* Make sure that we have an extra null terminated byte at the end */
	*leaves = calloc(nleaves+1,sizeof(Symbol*));
	if (*leaves == NULL)
	{
		/* Out of memory */
		perror("Unable to allocate memory");
		return HUFF_NOMEM;
	}
	(*leaves)[nleaves] = NULL;

	/* Run if we only have a single symbol */
	if (s->next == NULL) {
		(*leaves)[n] = s;
	}

	while (s->next != NULL)
	{
		/* Pop the first two symbols in the list and point to *
		 * them on the left and right of the tree node        */
		node = calloc(1,sizeof(Symbol));
		if (node == NULL)
		{
			/* Out of memory */
			return HUFF_NOMEM;
		}
		node->left = s;

		if (s->left == NULL && s->right == NULL)
		{
			(*leaves)[n] = s;
			n++;
		}

		s = s->next;
		node->right = s;

		if (s->left == NULL && s->right == NULL)
		{
			(*leaves)[n] = s;
			n++;
		}

		/* Add the new node to the top of the list */
		node->next = s->next;

		/* Make the left/right nodes know who their parent node is */
		node->left->parent = node->right->parent = node;

		/* Give the nodes a binary code */
		node->left->code  = false; /* 0 */
		node->right->code = true;  /* 1 */

		/* Calculate the weight of the new node */
		node->weight = node->left->weight + node->right->weight;

		/* re-sort the linked list so the two least likely nodes are
 		 *  at the head */
		rc = _sort_symbol_list(&s,node);
		if (rc != HUFF_SUCCESS)
		{
			return rc;
		}
	}

	/* Return an array of pointers to leaf nodes */
	return rc;
}

/* Based on the leaves in the huffman encoding tree, return a linked list *
 * of codes, comprising of the symbol and the binary huffman encoding of  *
 * the symbol                                                             */
HUFF_ERR _get_codes(Code **codes, Symbol **leaves)
{
	assert(leaves != NULL);

	Code*c, *c_prev;
	Symbol *s;
	Bits   *b;
	int n = 0;

	*codes = c = c_prev = NULL;

	assert(leaves[n] != NULL);
	while (leaves[n] != NULL )
	{
		s = leaves[n];
		assert(s != NULL);

		c = calloc(1,sizeof(Code));
		if (c == NULL)
		{
			/* Out of memory */
			perror("Unable to allocate memory");
			return HUFF_NOMEM;
		}
		c->code = NULL;
		c->length = 0;
		c->symbol = s->symbol;

		do 
		{
			b = calloc(1,sizeof(Bits));
			if (b == NULL)
			{
				/* Out of memory */
				perror("Unable to allocate memory");
				return HUFF_NOMEM;
			}
			b->bit = s->code;
			b->next = c->code;
			c->code = b;
			c->length++;
			if (s->parent != NULL)
			{
				s = s->parent;
			}
		} while (s->parent != NULL);

		if (*codes == NULL)
		{
			*codes = c;
		}
		else
		{
			c_prev->next = c;
		}

		c_prev = c;
		n++;
	}
	assert(*codes != NULL);

	return HUFF_SUCCESS;
}

HUFF_ERR _get_char_code(Code **code, unsigned char c, Code *code_book)
{
	assert(code_book != NULL);

	while (code_book != NULL && code_book->symbol != c)
	{
		code_book = code_book->next;
	}

	*code = code_book;
	return HUFF_SUCCESS;
}

/* Return the bit value of the current bit defined in the buffer */
bool _get_bit(Buffer *b)
{
	assert(b != NULL);
	uint8_t i = 0x01;

	/* Return the bit that the buffer is currently pointing to */
	i <<= (CHAR_BIT*sizeof(b->buf) - b->len - 1);
	i &=  b->buf;
	i >>= (CHAR_BIT*sizeof(b->buf) - b->len - 1);

	return (i == 1) ? true : false; 
}


/* Read the file in again, using the codebook generated to output the compressed
 * symbols
 */
HUFF_ERR _compress_file(Code *codes, f_stat *in_fp, f_stat *out_fp) 
{
	assert(codes != NULL);
	assert(in_fp != NULL);
	assert(out_fp != NULL);

	Code *codes_start = codes;
	Code *code;

	HUFF_ERR rc = HUFF_SUCCESS;

	rewind_stat(in_fp);

	int c;
	uint8_t buf = 0;
	uint8_t footer = 0x01;
	int bit_count = 0;

	/* Read every byte in the file */
	while ((c=fgetc_stat(in_fp)) != EOF)
	{
		rc = _get_char_code(&code,c,codes_start);
		if (rc != HUFF_SUCCESS)
		{
			return rc;
		}

		assert(code != NULL);
		Bits *b = code->code;

		while (b != NULL) 
		{
			/* CHAR_BIT - bit_count -1 as we want to shift by a  *
 			 * maximum of 7 bits to the right and a minimum of 0 */
			buf |= (b->bit << (CHAR_BIT - bit_count -1));
			bit_count++;
			if ( bit_count >= CHAR_BIT ) 
			{
				rc = fputc_stat(buf,out_fp);
				buf = 0;
				bit_count = 0;
			}
			b = b->next;
		}

		if (bit_count == CHAR_BIT)
		{
			rc = fputc_stat(buf,out_fp);
			fflush_stat(out_fp);
			buf = 0;
			bit_count = 0;
		}			
	}

	if (bit_count > 0)
	{
		fputc_stat(buf,out_fp);
		/* The footer is the last byte of data, it has a bit set *
                 * to indicate the last bit of data in the previous byte */
		footer = 0x01 << (CHAR_BIT - bit_count);
	}

	/* Write out the footer */
	rc = fputc_stat(footer,out_fp);
	if (rc != HUFF_SUCCESS)
	{
		return rc;
	}

	fflush_stat(out_fp);
	if (rc != HUFF_SUCCESS)
	{
		return rc;
	}

	/* Make sure that codes is pointing at the first in the linked list */
	codes = codes_start;

	return rc;
}

HUFF_ERR _encode_node(Symbol *s, f_stat *fp, Buffer *b) {

	size_t bufsize = CHAR_BIT*sizeof(b->buf);

	HUFF_ERR rc = HUFF_SUCCESS;

	/* We write out the symbol marker followed by the symbol*/
	if (s->left == NULL || s->right == NULL) 
	{
		/* Write out bit=1 */
		b->buf |= (1 << (bufsize - b->len - 1));
		b->len++;
		if (b->len == bufsize) 
		{
			fputc_stat(b->buf,fp);
			b->buf = 0;
			b->len = 0;
		}
		/* Write out symbol */
		if (b->len == 0) 
		{
			b->buf = s->symbol;
			fputc_stat(b->buf,fp);
			b->buf = 0;
			b->len = 0;
		} 
		else 
		{
			b->buf |= ( s->symbol >> b->len);
			fputc_stat(b->buf,fp);
			/*Note we've not reset the b->len variable! We 	*
 			 * still want the value.			*/
			b->buf = 0; 

			b->buf |= ( s->symbol << (bufsize - b->len) );
			b->len = b->len;
		}
	} else {
	/* We write out the branch marker, bit = 0 */
		b->buf |= (0 << (bufsize - b->len - 1));
		b->len++;
		if (b->len == bufsize) {
			fputc_stat(b->buf,fp);
			b->buf = 0;
			b->len = 0;
		}
		/* Go down both branches */
		_encode_node(s->left, fp, b);
		_encode_node(s->right,fp, b);
	}

	return rc;
}

/* Write the Huffman tree as a series of bits */
HUFF_ERR _write_tree(Symbol *s, f_stat *fp) {
	assert(s != NULL);
	assert(fp != NULL);

	Buffer b;
	int rc = 0;

	b.buf = 0;
	b.len = 0;

	rc = _encode_node(s,fp,&b);
	if (rc != HUFF_SUCCESS)
	{
		return rc;
	}

	/* Ensure that we clear any trailing bits in the buffer */
	if (b.len != 0) {
		rc = fputc_stat(b.buf,fp);
	}
	return rc;
}

/* Write out the 'magic number' in the first 4 bytes so we can identify the *
 * compressed file has having been written by this program.                 */
HUFF_ERR _write_header(f_stat *fp) {
	assert(fp != NULL);
	const char buf[] = "HUFF";
	if (fwrite_stat(buf,4,sizeof(char),fp) <= 0)
	{
		return HUFF_WRITEFAIL;
	}
	return HUFF_SUCCESS;
}

/* Check that this is file has the correct 'magic number' in the header	*
 * so that we identify it as a file compressed by the huffman encoder   */
HUFF_ERR _check_header(FILE *fp)
{
	assert(fp != NULL);

	char *c = calloc(5,sizeof(char));
	if (c == NULL)
	{
		/* Out of memory */
		perror("Unable to allocate memory");
		return HUFF_NOMEM;
	}
	assert(c != NULL);
	
	/* Define the expected header */
	const char header[] = "HUFF";
	fread(c,4,sizeof(char),fp);
	c[4] = '\0';

	int rc =  strcmp(c,header);
	free(c);

	if (rc != HUFF_SUCCESS)
	{
		rc = HUFF_INVALIDHEADER;
	}

	return rc;
}

/* Free memory in a symbol tree */
void _free_tree(Symbol *t)
{
	assert(t != NULL);

	if (t->left != NULL) {
		_free_tree(t->left);
	}
	if (t->right != NULL) {
		_free_tree(t->right);
	}

	free(t);
}

/* Free Node structure memory */
void _free_node(Node *n)
{
	assert(n != NULL);

	if (n->left != NULL)
	{
		_free_node(n->left);
	}
	if (n->right != NULL)
	{
		_free_node(n->right);
	}
	free(n);
}

/* Given the huffman tree, decompress the file to the output */
HUFF_ERR _output_message(Node *n, FILE *input, FILE *output)
{
	/* Define assumptions with assert */
	assert(n != NULL);
	assert(input != NULL);
	assert(output != NULL);

	Buffer b;
	uint8_t last;
	uint8_t next;
	HUFF_ERR rc = HUFF_SUCCESS;
	
	Node *top = n;

	fread(&b.buf,1,1,input);
	fread(&next,1,1,input);

	while (fread(&last,1,1,input))
	{
		b.len = 0;
		rc = _output_byte(&n,&b,n,top,CHAR_BIT*sizeof(b.buf),output);
		if (rc != HUFF_SUCCESS)
		{
			return rc;
		}
		b.buf = next;
		next  = last;
	}

	/* print out the data in the last but one byte using the last byte  *
         * footer data.							    */
	b.len = 0;
	int stop = 1;
	while ( (next <<= 1) > 0 )
	{ 
		stop++;
	}

	/* The stopping bit _cannot be longer than the number of bits in a *
 	 * a char.							   */
	assert(stop <= CHAR_BIT);

	rc = _output_byte(&n,&b,n,top,stop,output);

	/* make sure n is back at the top of the tree */
	n = top;
	return rc;
}

/* Output byte in buffer */
HUFF_ERR  _output_byte(Node **ret_node,Buffer *b, Node *n, Node *top, int stop, FILE *output)
{
        bool bit;

        while (b->len < stop)
	{
                bit = _get_bit(b);
		if (n->right == NULL && n->left == NULL)
		{
			fwrite(&n->value,1,sizeof(n->value),output);
			n=top;
		} 
		else
		{
                	n = (bit == false) ? n->left : n->right;
			assert(n != NULL);

                	if (n->right == NULL && n->left == NULL)
			{
                        	fwrite(&n->value,1,sizeof(n->value),output);
                        	n=top;
                	}
		}
                b->len++;
        }
	*ret_node = n;
        return HUFF_SUCCESS;
}

/* Return entire character from file, starting at current buffer location*/
HUFF_ERR _get_val(uint8_t *c,Buffer *b, FILE *fp)
{
	assert(b != NULL);
	assert(fp != NULL);

	*c = 0;

	*c |= (b->buf << b->len );
	/* get the next byte */
	fread(&b->buf,1,sizeof(b->buf),fp);

	*c |= (b->buf >> (CHAR_BIT*sizeof(b->buf) - b->len));
	/* note that b->len remains the same as we've moved a whole byte over */

	return HUFF_SUCCESS;
}

HUFF_ERR _get_node(Node **n, Buffer *b, FILE *fp)
{
	/* Assume no input in NULL */
	assert(b != NULL);
	assert(fp != NULL);

	bool bit = _get_bit(b);
	HUFF_ERR rc = HUFF_SUCCESS;

	/* If we've not reached the end of the byte, iterate the length *
 	 * counter, else get the next byte from the file		*/
	if (b->len < (CHAR_BIT*sizeof(b->buf) - 1))
	{
		b->len++;
	} 
	else
	{
		fread(&b->buf,1,1,fp);
		b->len = 0;
	}
	
	*n = calloc(1,sizeof(Node));
	if (n == NULL)
	{
		/* Out of memory*/
		return HUFF_NOMEM;
	}
	assert(n != NULL);

	if (bit == true)
	{

		rc = _get_val((uint8_t*) &((*n)->value),b,fp);
		if (rc != HUFF_SUCCESS)
		{
			return rc;
		}
		(*n)->left  = NULL;
		(*n)->right = NULL;
	}
	else
	{
		Node *left, *right;
		rc  = _get_node(&left,b,fp);
		if (rc != HUFF_SUCCESS)
		{
			return rc;
		}
		rc  = _get_node(&right,b,fp);
		if (rc != HUFF_SUCCESS)
		{
			return rc;
		}
		if ( left == NULL || right == NULL) {
			/* Error has occurred */
			return HUFF_FAILURE;
		}

		(*n)->value = 0;
		(*n)->left  = left;
		(*n)->right = right;
	}
	return rc;
}

HUFF_ERR get_tree(Node **n, FILE *fp)
{
	assert(fp != NULL);

	Buffer b;
	b.buf = 0;
	b.len = 0;

	/* read in the first byte */
	fread(&b.buf,1,1,fp);
	/* get the node tree */
	return _get_node(n,&b,fp);
}


/* Return the root node in a Symbol tree */
HUFF_ERR _get_root(Symbol **tree, Symbol *t) {
	assert(t != NULL);

	*tree = t;
	while ((*tree)->parent != NULL) 
	{ 
		*tree=(*tree)->parent; 
	}
	return HUFF_SUCCESS;
}

/* Performs huffman encoding on an input file stream, and outputs the  *
 * compressed file to the output file stream                           */
HUFF_ERR huffman(f_stat *in, f_stat *out)
{
	Symbol *tree;
	Symbol **leaves;
	Code   *codes, *c;
	Bits   *bits,  *b;

	int rc = HUFF_SUCCESS;
	int ecode = 0;

	/* Validate the inputs */
	if (in == NULL || out == NULL)
	{
		return HUFF_INVALIDARG;
	}

	/* Collect statistics for the 8bit characters in the file */
	rc = _build_statistics(&tree,in);
	if (rc != HUFF_SUCCESS)
	{
		return rc;
	}
	rc = _build_tree(&leaves,tree);
	if (rc != HUFF_SUCCESS)
	{
		/* Free the tree */
		_free_tree(tree);
		return rc;
	}

	rc = _get_codes(&codes,leaves);
	if (codes == NULL || rc != HUFF_SUCCESS)
	{
		/* Free tree */
		_free_tree(tree);
		/* Free leaves */
		free(leaves);
		/* Return failure */
		rc = HUFF_FAILURE;
		return rc;
	}

#ifdef DEBUG
	print_codes_from_tree(leaves);
#endif /* DEBUG */

	/* Assure that we're is pointing at the top of the tree */
	Symbol *tmp = NULL;
	rc = _get_root(&tmp,tree);
	assert(tmp != NULL);
	tree = tmp;

	ecode = _write_header(out);
	if (ecode != HUFF_SUCCESS)
	{
		rc = HUFF_FAILURE;
	}

	ecode = _write_tree(tree,out);
	if (ecode != 0)
	{
		rc = HUFF_FAILURE;
	}
	
	if (_compress_file(codes,in,out) != HUFF_SUCCESS)
	{
		rc = HUFF_FAILURE;
	}

	/* Start freeing allocated memory */
	rc = _get_root(&tmp,tree);
	tree = tmp;
	_free_tree(tree);
	free(leaves);

	while(codes != NULL)
	{
		c = codes;
		codes = codes->next;
		/* Free the individually malloc'd `bits' */
		bits = c->code;
		while (bits != NULL)
		{
			b = bits;
			bits = bits->next;
			free(b);
		}
		free(c);
	}
	/* End freeing of allocated memory */
	
	return rc;
}

/* Perform a decompression on the huffman encoded `in' file. */
HUFF_ERR unhuffman(f_stat *in, f_stat *out)
{
	Node *n;
	HUFF_ERR rc = HUFF_SUCCESS;

	/* Validate the inputs are not null */
	if (in == NULL || out == NULL)
	{
		return HUFF_INVALIDARG;
	}

	/* Validate that the input file was encoded by this huffman encoder */
	if (_check_header(in->file) != 0)
	{
		fprintf(stderr,"File not encoded by huffman\n");
		return HUFF_FAILURE;
	}

	rc = get_tree(&n,in->file);
	if (rc == HUFF_SUCCESS)
	{
		rc = _output_message(n,in->file,out->file);
	}

	/* Free node tree */
	_free_node(n);

	return rc;
}

