/* Test huffman.c
 *
 * Unit tests for huffman.c
 */

#include "huffman.h"
#include "minunit.h"
#include "file_stat_error.h"
#include "huffman_errno.h"

#include <stdio.h>

int tests_run = 0;

static char *test_symbol_cmp()
{
	Symbol symbol1, symbol2;
	symbol1.weight   = 0;
	symbol2.weight   = 1;
	Symbol *s1_ptr = &symbol1;
	Symbol *s2_ptr = &symbol2;

	mu_assert("symbol1.weight != symbol1.weight", symbol_cmp(&s1_ptr,&s1_ptr) == 0);
	mu_assert("symbol1.weight == symbol2.weight", symbol_cmp(&s1_ptr,&s2_ptr) != 0);
	return NULL;
}

static char *test_unhuffman()
{
	mu_assert("unhuffman != HUFF_INVALIDARG", unhuffman(NULL,NULL) == HUFF_INVALIDARG);
	return NULL;
}

static char *test_huffman()
{
	mu_assert("huffman != HUFF_INVALIDARG", huffman(NULL,NULL) == HUFF_INVALIDARG);
	return NULL;
}

char *all_tests()
{
	mu_run_test(test_symbol_cmp);
	mu_run_test(test_unhuffman);
	mu_run_test(test_huffman);

	return NULL;
}

int main(int argc, char **argv)
{
	char *result = all_tests();
	if (result != 0)
	{
		printf("%s\n", result);
	}
	else
	{
		printf("%s: ALL TESTS PASSED\n",argv[0]);
	}
	printf("Tests run in %s: %d\n", argv[0],tests_run);

	return result != 0;
}
