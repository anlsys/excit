#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "excit.h"
#include "excit_test.h"

excit_t create_test_range(ssize_t start, ssize_t stop, ssize_t step)
{
	excit_t it;

	it = excit_alloc_test(EXCIT_RANGE);
	assert(excit_range_init(it, start, stop, step) == ES);
	return it;
}

void test_alloc_init_repeat(int repeat, excit_t sit)
{
	excit_t it;
	ssize_t dim, expected_dim, size, expected_size;

	it = excit_alloc_test(EXCIT_REPEAT);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 0);

	assert(excit_repeat_init(it, excit_dup(sit), repeat) == ES);
	assert(excit_dimension(it, &dim) == ES);
	assert(excit_dimension(sit, &expected_dim) == ES);
	assert(dim == expected_dim);
	assert(excit_size(sit, &expected_size) == ES);
	expected_size *= repeat;
	assert(excit_size(it, &size) == ES);
	assert(size == expected_size);

	excit_free(it);
}

excit_t create_test_repeat(int repeat, excit_t sit)
{
	excit_t it;
	it = excit_alloc_test(EXCIT_REPEAT);
	assert(excit_repeat_init(it, excit_dup(sit), repeat) == ES);
	return it;
}

void test_next_repeat(int repeat, excit_t sit)
{
	excit_t it, new_sit;
	ssize_t *indexes1, *indexes2;
	ssize_t dim;

	it = create_test_repeat(repeat, sit);
	new_sit = excit_dup(sit);

	assert(excit_dimension(it, &dim) == ES);
	indexes1 = (ssize_t *)malloc(dim * sizeof(ssize_t));
	indexes2 = (ssize_t *)malloc(dim * sizeof(ssize_t));

	while (excit_next(new_sit, indexes1) == ES)
		for (int i = 0; i < repeat; i++) {
			assert(excit_next(it, indexes2) == ES);
			assert(memcmp(indexes1, indexes2, dim * sizeof(ssize_t)) == 0);
			
		}
	assert(excit_next(it, indexes2) == EXCIT_STOPIT);

	free(indexes1);
	free(indexes2);

	excit_free(it);
	excit_free(new_sit);
}

void test_repeat_iterator(int repeat, excit_t sit)
{
	test_alloc_init_repeat(repeat, sit);

	test_next_repeat(repeat, sit);

	int i = 0;
	while (synthetic_tests[i]) {
		excit_t it = create_test_repeat(repeat, sit);

		synthetic_tests[i](it);
		excit_free(it);
		i++;
	}
	
}

int main(int argc, char *argv[])
{
	excit_t it;

	it = create_test_range(0, 3, 1);
	test_repeat_iterator(3, it);
	return 0;
}
