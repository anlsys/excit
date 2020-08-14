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

void test_alloc_init_loop(int loop, excit_t sit)
{
	excit_t it;
	ssize_t dim, expected_dim, size, expected_size;

	it = excit_alloc_test(EXCIT_LOOP);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 0);

	assert(excit_loop_init(it, excit_dup(sit), loop) == ES);
	assert(excit_dimension(it, &dim) == ES);
	assert(excit_dimension(sit, &expected_dim) == ES);
	assert(dim == expected_dim);
	assert(excit_size(sit, &expected_size) == ES);
	expected_size *= loop;
	assert(excit_size(it, &size) == ES);
	assert(size == expected_size);

	excit_free(it);
}

excit_t create_test_loop(int loop, excit_t sit)
{
	excit_t it;

	it = excit_alloc_test(EXCIT_LOOP);
	assert(excit_loop_init(it, excit_dup(sit), loop) == ES);
	return it;
}

void test_next_loop(int loop, excit_t sit)
{
	excit_t it, new_sit;
	ssize_t *indexes1, *indexes2;
	ssize_t dim;

	it = create_test_loop(loop, sit);
	new_sit = excit_dup(sit);

	assert(excit_dimension(it, &dim) == ES);
	indexes1 = (ssize_t *) malloc(dim * sizeof(ssize_t));
	indexes2 = (ssize_t *) malloc(dim * sizeof(ssize_t));

	for (int i = 0; i < loop; i++) {
		while (excit_next(new_sit, indexes1) == ES) {
			assert(excit_next(it, indexes2) == ES);
			assert(memcmp(indexes1, indexes2, dim * sizeof(ssize_t))
			       == 0);
		}
		assert(excit_rewind(new_sit) == ES);
	}
	assert(excit_next(it, indexes2) == EXCIT_STOPIT);

	free(indexes1);
	free(indexes2);

	excit_free(it);
	excit_free(new_sit);
}

void test_loop_iterator(int loop, excit_t sit)
{
	test_alloc_init_loop(loop, sit);

	test_next_loop(loop, sit);

	int i = 0;

	while (synthetic_tests[i]) {
		excit_t it = create_test_loop(loop, sit);

		synthetic_tests[i] (it);
		excit_free(it);
		i++;
	}
}

int main()
{
	excit_t it1, it2, it3;

	it1 = create_test_range(0, 3, 1);
	test_loop_iterator(3, it1);
	it2 = create_test_range(-15, 14, 2);
	test_loop_iterator(3, it2);

	it3 = excit_alloc_test(EXCIT_PRODUCT);
	assert(excit_product_add_copy(it3, it1) == ES);
	assert(excit_product_add_copy(it3, it2) == ES);
	test_loop_iterator(4, it3);

	excit_free(it1);
	excit_free(it2);
	excit_free(it3);
	return 0;
}
