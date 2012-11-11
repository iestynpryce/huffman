/* unhuffman - a program to decode a huffman encoded file 	*
 * 								*
 * Iestyn Pryce 2012						*/

#include <string.h>
#include "huffman.h"

typedef struct node 
{
	char value;
	struct node *left;
	struct node *right;
} Node;

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

/* Perform a decompression on the huffman encoded `in' file. */
int unhuffman(FILE *in, FILE *out)
{
	Node *n;

	if (check_header(in) != 0)
	{
		fprintf(stderr,"File not encoded by huffman\n");
		return EXIT_FAILURE;
	}

	n = get_tree(in);

	output_message(n,in,out);

	/* Free node tree */
	free_node(n);

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	FILE *fp, *fpo;

	/* Parse command line arguments */
	if (argc > 1)
	{
		fp = fopen(argv[1],"rb");
		if (fp == NULL)
		{
			fprintf(stderr,"Could not open file: %s\n",argv[1]);
			exit(1);
		}
	}
	else
	{
		fp = stdin;
	}

	fpo = stdout;

	/* Decompress input */
	int rc = unhuffman(fp,fpo);

	return rc;
}

