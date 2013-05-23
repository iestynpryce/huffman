#include "file_stat.h"
#include "file_stat_error.h"
#include "minunit.h"

#include <stdio.h>

int tests_run = 0;

static char *test_fwrite_stat()
{
	/* Setup for test */
	size_t size = 0;
	size_t count = 0;
	void *ptr = NULL;
	f_stat *stream = NULL;

	mu_assert("null inputs != rc = E_UNEXPECTED_NULL_POINTER",fwrite_stat(ptr,size,count,stream) == E_UNEXPECTED_NULL_POINTER);
	return NULL;
}

static char *test_fputc_stat()
{
	f_stat *stream = NULL;
	int c = 'c';
	mu_assert("fputc_stat(,null) != E_UNEXPECTED_NULL_POINTER",fputc_stat(c,stream)==E_UNEXPECTED_NULL_POINTER);
	return NULL;
}

static char *test_fgetc_stat()
{
	mu_assert("fgetc_stat(NULL) != E_UNEXPECTED_NULL_POINTER",fgetc_stat(NULL)==E_UNEXPECTED_NULL_POINTER);
	return NULL;
}

static char *test_rewind_stat()
{
	mu_assert("rewind_stat(NULL) != E_UNEXPECTED_NULL_POINTER",rewind_stat(NULL)==E_UNEXPECTED_NULL_POINTER);
	return NULL;
}

static char *test_fflush_stat()
{
	mu_assert("fflush_stat(NULL) != E_UNEXPECTED_NULL_POINTER",fflush_stat(NULL)==E_UNEXPECTED_NULL_POINTER);
	return NULL;
}

static char *test_fclose_stat()
{
	mu_assert("fclose_stat(NULL) != E_UNEXPECTED_NULL_POINTER",fclose_stat(NULL)==E_UNEXPECTED_NULL_POINTER);
	return NULL;
}

static char *all_tests()
{
	mu_run_test(test_fwrite_stat);
	mu_run_test(test_fputc_stat);
	mu_run_test(test_fgetc_stat);
	mu_run_test(test_rewind_stat);
	mu_run_test(test_fflush_stat);
	mu_run_test(test_fclose_stat);

	return NULL;
}

int main(int argc, char **argv) {
	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run in %s: %d\n", argv[0],tests_run);

	return result != 0;
}
