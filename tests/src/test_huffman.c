/* Test huffman.c
 *
 * Unit tests for huffman.c
 */

#include "huffman.h"
#include "minunit.h"
#include "file_stat_error.h"

#include <stdio.h>

int tests_run = 0;

static char *test_symbol_cmp()
{
	mu_assert("symbol_cmp(NULL,NULL) != E_UNEXPECTED_NULL_POINTER",symbol_cmp(NULL,NULL)==E_UNEXPECTED_NULL_POINTER);

	return NULL;
}

char *all_tests()
{
	mu_run_test(test_symbol_cmp);

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
