/* Huffman Encoder
 * 
 * Iestyn Pryce 2012
 */

#include "huffman.h"
#include "file_stat.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

/* Tree node structure */
typedef struct symbol
{
	struct symbol *next;
	struct symbol *parent;
	struct symbol *left;
	struct symbol *right;
	unsigned char  symbol;
	uint8_t        weight;
	bool           code;
} Symbol;

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

Node  *output_byte(Buffer *b, Node *n, Node *top, int stop, FILE *output);

void print_ll(Symbol *s)
{
	assert(s != NULL);

	Symbol *p = s;

	while (p != NULL)
	{
		printf("%c(%#x) %d",p->symbol,p->symbol,p->weight);
		if (p->left != NULL) printf("\tl: %d",p->left->code);
		if (p->right != NULL) printf("\tr: %d",p->right->code);
		printf("\n");
		p=p->next;
	}
}

/* Sort the linked list by swapping the values
 * Uses the bubble sort algorithm
 */
Symbol *sort_ll(Symbol* s)
{
	assert(s != NULL);

	Symbol *top, *top_prev;
	Symbol *small, *small_prev;

	Symbol *start, *tmp, *prev;

	top = start = s;

	small = top_prev = prev = NULL;

	while (top != NULL)
	{
		s=top;
		/* Try and find a node smaller than the node at the top */ 
		while (s != NULL)
		{
			if ( ( (top->weight > s->weight) 
				&& (small == NULL || small->weight > s->weight)
			     )
			   ||
			     ( (top->weight == s->weight) 
				  && (top->symbol > s->symbol)
			     )
			   ) 
			{
				small = s;
				small_prev = prev;
			}
			prev=s;
			s=s->next;
		}

		if (small != NULL)
		{
			/* Make sure the node before the top node is now *
			 * pointing at the correct new node in the 'top'.*
			 * When this is the first node in the list make  *
			 * sure the starting pointer points to the new   *
			 * node.					 */
			if (top_prev != NULL)
			{
				top_prev->next = small;
			}
			else
			{
				start = small;
			}		

			/* Assure that the node before the smallest value *
			 * changes it's next value to the node at the top *
			 * of the tree. 				  */
			small_prev->next = top;

			/* Swap both pointers around */
			tmp = small->next;
			small->next = top->next;
			top->next = tmp;

			/* Reset the top and utility pointers */
			top_prev = prev = small;
			top = small->next;
			small = NULL;
		} 
		else
		{ 
			top_prev = prev = top;
			top = top->next;
		}

	}
	return start;	
}

/* Create a linked list of statistics for bytes in the input */
Symbol *build_statistics( f_stat *fp)
{
	assert(fp != NULL);

	Symbol *start, *s;
	int c;

	start = calloc(1,sizeof(Symbol));
	if (start == NULL )
	{
		fprintf(stderr,"Out of memory\n");
		exit(EXIT_FAILURE);
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
				fprintf(stderr,"Out of memory\n");
				exit(EXIT_FAILURE);
			}
			s=s->next;
		}

		s->symbol = c;
		s->weight++;

		s=start;
	}

	start = sort_ll(start);

	return start;
}

Symbol** build_tree(Symbol *s)
{
	assert (s != NULL);

	Symbol *node = NULL;
	Symbol **leaves;
	unsigned int nleaves = UCHAR_MAX+1; /* 2**CHAR_BIT */
	int n = 0;
	
	/* Make sure that we have an extra null terminated byte at the end */
	leaves = calloc(nleaves+1,sizeof(Symbol*));
	if (leaves == NULL)
	{
		fprintf(stderr,"Out of memory\n");
		exit(EXIT_FAILURE);
	}
	leaves[nleaves] = NULL;

	/* Run if we only have a single symbol */
	if (s->next == NULL) {
		leaves[n] = s;
	}

	while (s->next != NULL)
	{
		/* Pop the first two symbols in the list and point to *
		 * them on the left and right of the tree node        */
		node = calloc(1,sizeof(Symbol));
		if (node == NULL)
		{
			fprintf(stderr,"Out of memory\n");
			exit(EXIT_FAILURE);
		}
		node->left = s;

		if (s->left == NULL && s->right == NULL)
		{
			leaves[n] = s;
			n++;
		}

		s = s->next;
		node->right = s;

		if (s->left == NULL && s->right == NULL)
		{
			leaves[n] = s;
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
		s = sort_ll(node);
	}

	/* Return an array of pointers to leaf nodes */
	return leaves;
}

/* Used in debugging to print symbols with their bit codes */
void print_codes_from_tree(Symbol **leaves)
{
	assert(leaves != NULL);

	int n = 0;
	Symbol *s;
	while (leaves[n] != NULL)
	{
		s = leaves[n];
		assert(s != NULL);
		printf("%#x|%d|\t",s->symbol,s->weight);
		do 
		{
			printf("%d ",s->code);
			s=s->parent;
		} while (s->parent != NULL);
		printf("\n");
		n++;
	}
}

/* Based on the leaves in the huffman encoding tree, return a linked list *
 * of codes, comprising of the symbol and the binary huffman encoding of  *
 * the symbol                                                             */
Code *get_codes(Symbol **leaves)
{
	assert(leaves != NULL);

	Code *codes, *c, *c_prev;
	Symbol *s;
	Bits   *b;
	int n = 0;

	c = codes = c_prev = NULL;

	assert(leaves[n] != NULL);
	while (leaves[n] != NULL )
	{
		s = leaves[n];
		assert(s != NULL);

		c = calloc(1,sizeof(Code));
		if (c == NULL)
		{
			fprintf(stderr,"Out of memory\n");
			exit(EXIT_FAILURE);
		}
		c->code = NULL;
		c->length = 0;
		c->symbol = s->symbol;

		do 
		{
			b = calloc(1,sizeof(Bits));
			if (b == NULL)
			{
				fprintf(stderr,"Out of memory\n");
				exit(EXIT_FAILURE);
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

		if (codes == NULL)
		{
			codes = c;
		}
		else
		{
			c_prev->next = c;
		}

		c_prev = c;
		n++;
	}
	assert(codes != NULL);

	return codes;
}

Code *get_char_code(unsigned char c, Code *codes)
{
	assert(codes != NULL);

	while (codes != NULL && codes->symbol != c)
	{
		codes = codes->next;
	}

	return codes;
}

/* Return the bit value of the current bit defined in the buffer */
bool get_bit(Buffer *b)
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
int compress_file(Code *codes, f_stat *in_fp, f_stat *out_fp) 
{
	assert(codes != NULL);
	assert(in_fp != NULL);
	assert(out_fp != NULL);

	Code *codes_start = codes;
	Code *code;

	rewind_stat(in_fp);

	int c;
	uint8_t buf = 0;
	uint8_t footer = 0x01;
	int bit_count = 0;

	/* Read every byte in the file */
	while ((c=fgetc_stat(in_fp)) != EOF)
	{
		code = get_char_code(c,codes_start);
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
				fputc_stat(buf,out_fp);
				buf = 0;
				bit_count = 0;
			}
			b = b->next;
		}

		if (bit_count == CHAR_BIT)
		{
			fputc_stat(buf,out_fp);
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
	fputc_stat(footer,out_fp);
	fflush_stat(out_fp);

	/* Make sure that codes is pointing at the first in the linked list */
	codes = codes_start;

	return EXIT_SUCCESS;
}

void encode_node(Symbol *s, f_stat *fp, Buffer *b) {

	size_t bufsize = CHAR_BIT*sizeof(b->buf);

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
		encode_node(s->left, fp, b);
		encode_node(s->right,fp, b);
	}
}

/* Write the Huffman tree as a series of bits */
void write_tree(Symbol *s, f_stat *fp) {
	assert(s != NULL);
	assert(fp != NULL);

	Buffer b;

	b.buf = 0;
	b.len = 0;

	encode_node(s,fp,&b);

	/* Ensure that we clear any trailing bits in the buffer */
	if (b.len != 0) {
		fputc_stat(b.buf,fp);
	}
}

/* Write out the 'magic number' in the first 4 bytes so we can identify the *
 * compressed file has having been written by this program.                 */
void write_header(f_stat *fp) {
	assert(fp != NULL);
	const char buf[] = "HUFF";
	fwrite_stat(buf,4,sizeof(char),fp);
}

/* Check that this is file has the correct 'magic number' in the header	*
 * so that we identify it as a file compressed by the huffman encoder   */
int check_header(FILE *fp)
{
	assert(fp != NULL);

	char *c = calloc(5,sizeof(char));
	if (c == NULL)
	{
		fprintf(stderr,"Out of memory\n");
		exit(EXIT_FAILURE);
	}
	assert(c != NULL);
	
	/* Define the expected header */
	const char header[] = "HUFF";
	fread(c,4,sizeof(char),fp);
	c[4] = '\0';

	int r =  strcmp(c,header);
	free(c);

	return r;
}

/* Free memory in a symbol tree */
void free_tree(Symbol *t) {
	assert(t != NULL);

	if (t->left != NULL) {
		free_tree(t->left);
	}
	if (t->right != NULL) {
		free_tree(t->right);
	}

	free(t);
}

/* Free Node structure memory */
void free_node(Node *n)
{
	assert(n != NULL);

	if (n->left != NULL)
	{
		free_node(n->left);
	}
	if (n->right != NULL)
	{
		free_node(n->right);
	}
	free(n);
}

/* Given the huffman tree, decompress the file to the output */
void output_message(Node *n, FILE *input, FILE *output)
{
	assert(n != NULL);
	assert(input != NULL);
	assert(output != NULL);

	Buffer b;
	uint8_t last;
	uint8_t next;
	
	Node *top = n;

	fread(&b.buf,1,1,input);
	fread(&next,1,1,input);

	while (fread(&last,1,1,input))
	{
		b.len = 0;
		n = output_byte(&b,n,top,CHAR_BIT*sizeof(b.buf),output);
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

	n = output_byte(&b,n,top,stop,output);

	/* make sure n is back at the top of the tree */
	n = top;
}

/* Output byte in buffer */
Node  *output_byte(Buffer *b, Node *n, Node *top, int stop, FILE *output)
{
        bool bit;

        while (b->len < stop)
	{
                bit = get_bit(b);
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
        return n;
}

uint8_t get_val(Buffer *b, FILE *fp)
{
	assert(b != NULL);
	assert(fp != NULL);

	uint8_t c = 0;

	c |= (b->buf << b->len );
	/* get the next byte */
	fread(&b->buf,1,sizeof(b->buf),fp);

	c |= (b->buf >> (CHAR_BIT*sizeof(b->buf) - b->len));
	/* note that b->len remains the same as we've moved a whole byte over */

	return c;
}

/* Usage information */
Node *get_node(Buffer *b, FILE *fp)
{
	assert(b != NULL);
	assert(fp != NULL);

	bool bit = get_bit(b);

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
	
	Node *n = calloc(1,sizeof(Node));
	if (n == NULL)
	{
		fprintf(stderr,"Out of memory\n");
		exit(EXIT_FAILURE);
	}
	assert(n != NULL);

	if (bit == true)
	{
		n->value = get_val(b,fp);
		n->left  = NULL;
		n->right = NULL;
	}
	else
	{
		Node *left  = get_node(b,fp);
		Node *right = get_node(b,fp);

		n->value = 0;
		n->left  = left;
		n->right = right;
	}
	return n;
}

Node *get_tree(FILE *fp)
{
	assert(fp != NULL);

	Buffer b;
	b.buf = 0;
	b.len = 0;

	/* read in the first byte */
	fread(&b.buf,1,1,fp);
	/* get the node tree */
	return get_node(&b,fp);
}


/* Return the root node in a Symbol tree */
Symbol *get_root(Symbol *t) {
	assert(t != NULL);

	Symbol *tree;
	tree = t;
	while (tree->parent != NULL) { 
		tree=tree->parent; 
	}
	return tree;
}

/* Performs huffman encoding on an input file stream, and outputs the  *
 * compressed file to the output file stream                           */
int huffman(f_stat *in, f_stat *out)
{
	Symbol *tree;
	Symbol **leaves;
	Code   *codes, *c;
	Bits   *bits,  *b;

	int rc = EXIT_SUCCESS;

	/* Collect statistics for the 8bit characters in the file */
	tree = build_statistics(in);

	leaves = build_tree(tree);

	codes = get_codes(leaves);

#ifdef DEBUG
	print_codes_from_tree(leaves);
#endif /* DEBUG */

	/* Assure that we're is pointing at the top of the tree */
	tree = get_root(tree);

	write_header(out);
	write_tree(tree,out);
	if (compress_file(codes,in,out) != EXIT_SUCCESS)
	{
		fprintf(stderr,"Error compressing file\n");
		rc = EXIT_FAILURE;
	}

	/* Start freeing allocated memory */
	tree = get_root(tree);
	free_tree(tree);
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
int unhuffman(f_stat *in, f_stat *out)
{
	Node *n;

	if (check_header(in->file) != 0)
	{
		fprintf(stderr,"File not encoded by huffman\n");
		return EXIT_FAILURE;
	}

	n = get_tree(in->file);

	output_message(n,in->file,out->file);

	/* Free node tree */
	free_node(n);

	return EXIT_SUCCESS;
}

