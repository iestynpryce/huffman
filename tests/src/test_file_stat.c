#include "file_stat.h"
#include "minunit.h"

#include <stdio.h>

int tests_run = 0;

static char *test_fwrite_stat()
{
	mu_assert("message",1);
	return NULL;
}

static char *test_fputc_stat()
{
	mu_assert("message",1);
	return NULL;
}

static char *test_fgetc_stat()
{
	mu_assert("message",1);
	return NULL;
}

static char *test_rewind_stat()
{
	mu_assert("message",1);
	return NULL;
}

static char *test_fflush_stat()
{
	mu_assert("message",1);
	return NULL;
}

static char *test_fclose_stat()
{
	mu_assert("message",1);
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
