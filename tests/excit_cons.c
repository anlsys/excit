/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
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

void test_alloc_init_cons(int window, excit_t sit)
{
	excit_t it;
	ssize_t dim, expected_dim, size, expected_size;

	it = excit_alloc_test(EXCIT_CONS);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 0);

	assert(excit_cons_init(it, excit_dup(sit), window) == ES);
	assert(excit_dimension(it, &dim) == ES);
	assert(excit_dimension(sit, &expected_dim) == ES);
	expected_dim *= window;
	assert(dim == expected_dim);
	assert(excit_size(sit, &expected_size) == ES);
	expected_size -= (window - 1);
	assert(excit_size(it, &size) == ES);
	assert(size == expected_size);

	excit_free(it);
}

excit_t create_test_cons(int window, excit_t sit)
{
	excit_t it;

	it = excit_alloc_test(EXCIT_CONS);
	assert(excit_cons_init(it, excit_dup(sit), window) == ES);
	return it;
}

void test_next_cons(int window, excit_t sit)
{
	excit_t it, new_sit[window];
	ssize_t *indexes1, *indexes2;
	ssize_t dim, sdim;

	it = create_test_cons(window, sit);
	for (int i = 0; i < window; i++) {
		new_sit[i] = excit_dup(sit);
		assert(new_sit[i] != NULL);
		for (int j = 0; j < i; j++)
			assert(excit_skip(new_sit[i]) == ES);
	}

	assert(excit_dimension(it, &dim) == ES);
	assert(excit_dimension(sit, &sdim) == ES);
	indexes1 = (ssize_t *) malloc(dim * sizeof(ssize_t));
	indexes2 = (ssize_t *) malloc(dim * sizeof(ssize_t));

	while (excit_next(new_sit[window - 1], indexes2 + (window - 1) * sdim)
	       == ES) {
		for (int i = 0; i < window - 1; i++) {
			assert(excit_next(new_sit[i], indexes2 + i * sdim) ==
			       ES);
		}
		assert(excit_next(it, indexes1) == ES);
		assert(memcmp(indexes1, indexes2, dim * sizeof(ssize_t)) == 0);
	}
	assert(excit_next(it, indexes1) == EXCIT_STOPIT);

	free(indexes1);
	free(indexes2);

	excit_free(it);
	for (int i = 0; i < window; i++)
		excit_free(new_sit[i]);
}

void test_cons_iterator(int window, excit_t sit)
{
	test_alloc_init_cons(window, sit);

	test_next_cons(window, sit);

	int i = 0;

	while (synthetic_tests[i]) {
		excit_t it = create_test_cons(window, sit);

		synthetic_tests[i] (it);
		excit_free(it);
		i++;
	}
}

int main(void)
{
	excit_t it1, it2, it3;

	it1 = create_test_range(0, 9, 1);
	test_cons_iterator(3, it1);
	it2 = create_test_range(-15, 14, 2);
	test_cons_iterator(3, it2);

	it3 = excit_alloc_test(EXCIT_PRODUCT);
	assert(excit_product_add_copy(it3, it1) == ES);
	assert(excit_product_add_copy(it3, it2) == ES);
	test_cons_iterator(4, it3);

	excit_free(it1);
	excit_free(it2);
	excit_free(it3);

	return 0;
}
