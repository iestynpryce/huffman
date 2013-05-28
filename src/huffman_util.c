/* Implements functions declared in huffman_util.h
 * These are functions not essential to huffman encoding however they can
 * be useful in understanding its inner workings, and for debugging.
 *
 * Iestyn Pryce 2012/2013
 */

#include "huffman.h"

#include <assert.h>
#include <stdio.h>

/* Used in debugging to symbols with their bit codes */
void print_codes_from_tree(Symbol **leaves)
{
	assert(leaves != NULL);

	int n = 0;
	Symbol *s;
	while (leaves[n] != NULL)
	{
		s = leaves[n];
		assert(s != NULL);
		printf("%#x|%ld|\t",s->symbol,s->weight);
		do 
		{
			printf("%d ",s->code);
			s=s->parent;
		} while (s->parent != NULL);
		printf("\n");
		n++;
	}
}

/* Prints the symbol and its code as stored in the symbol linked list */
void print_ll(Symbol *s)
{
	assert(s != NULL);

	Symbol *p = s;

	while (p != NULL)
	{
		printf("%c(%#x) %ld",p->symbol,p->symbol,p->weight);
		if (p->left != NULL) printf("\tl: %d",p->left->code);
		if (p->right != NULL) printf("\tr: %d",p->right->code);
		printf("\n");
		p=p->next;
	}
}


