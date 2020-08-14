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
#include "excit.h"
#include "excit_test.h"

void test_alloc_init_range(ssize_t start, ssize_t stop, ssize_t step)
{
	excit_t it;
	ssize_t dim;

	it = excit_alloc_test(EXCIT_RANGE);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 0);

	assert(excit_range_init(it, start, stop, step) == ES);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 1);
	excit_free(it);
}

excit_t create_test_range(ssize_t start, ssize_t stop, ssize_t step)
{
	excit_t it;

	it = excit_alloc_test(EXCIT_RANGE);
	assert(excit_range_init(it, start, stop, step) == ES);
	return it;
}

void test_next_range(ssize_t start, ssize_t stop, ssize_t step)
{
	excit_t it;
	ssize_t indexes[1];

	it = create_test_range(start, stop, step);

	assert(step != 0);

	if (step < 0)
		for (int i = start; i >= stop; i += step) {
			assert(excit_next(it, indexes) == ES);
			assert(indexes[0] == i);
	} else
		for (int i = start; i <= stop; i += step) {
			assert(excit_next(it, indexes) == ES);
			assert(indexes[0] == i);
		}
	assert(excit_next(it, indexes) == EXCIT_STOPIT);
	excit_free(it);
}

void test_range_iterator(ssize_t start, ssize_t stop, ssize_t step)
{
	test_alloc_init_range(start, stop, step);

	test_next_range(start, stop, step);

	int i = 0;

	while (synthetic_tests[i]) {
		excit_t it = create_test_range(start, stop, step);

		synthetic_tests[i] (it);
		excit_free(it);
		i++;
	}
}

int main()
{
	test_range_iterator(4, 12, 3);
	test_range_iterator(0, 3, 1);
	test_range_iterator(0, 6, 2);
	test_range_iterator(-15, 14, 2);
	test_range_iterator(3, 0, -1);
	test_range_iterator(6, 0, -2);
	test_range_iterator(15, -14, -2);
	return 0;
}
