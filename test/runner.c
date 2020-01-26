#include <stdio.h>

#include "test-lib.h"
#include "test-suite.h"

static struct suite_test tests[] = {
		{ "tetris-well", tetris_well_test },
		{ NULL, NULL }
};

int main(void)
{
	int verbose = 1, immediate = 0;

	char *env = getenv("TETRIS_TEST_VERBOSE");
	if (env && strcmp(env, "") != 0 && strcmp(env, "0") != 0)
		verbose = 1;

	env = getenv("TETRIS_TEST_IMMEDIATE");
	if (env && strcmp(env, "") != 0 && strcmp(env, "0") != 0)
		immediate = 1;

	return execute_suite(tests, verbose, immediate);
}
