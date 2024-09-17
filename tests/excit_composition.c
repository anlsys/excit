/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
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

void test_alloc_init_composition(excit_t source, excit_t indexer)
{
	excit_t it;
	ssize_t dim, expected_dim, size, expected_size;

	it = excit_alloc_test(EXCIT_COMPOSITION);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 0);

	assert(excit_composition_init(it, excit_dup(source), excit_dup(indexer)) ==
	       ES);
	assert(excit_dimension(it, &dim) == ES);
	assert(excit_dimension(source, &expected_dim) == ES);
	assert(dim == expected_dim);
	assert(excit_size(it, &size) == ES);
	assert(excit_size(indexer, &expected_size) == ES);
	assert(size == expected_size);

	excit_free(it);
}

excit_t create_test_composition(excit_t source, excit_t indexer)
{
	excit_t it;

	it = excit_alloc_test(EXCIT_COMPOSITION);
	assert(excit_composition_init(it, excit_dup(source), excit_dup(indexer)) ==
	       ES);
	return it;
}

void test_next_composition(excit_t source, excit_t indexer)
{
	excit_t it = create_test_composition(source, indexer);

	excit_t iit = excit_dup(indexer);

	ssize_t *indexes1, *indexes2;
	ssize_t index;
	ssize_t dim;

	assert(excit_dimension(source, &dim) == ES);
	indexes1 = (ssize_t *) malloc(dim * sizeof(ssize_t));
	indexes2 = (ssize_t *) malloc(dim * sizeof(ssize_t));

	while (excit_next(iit, &index) == ES) {
		assert(excit_nth(source, index, indexes2) == ES);
		assert(excit_next(it, indexes1) == ES);
		assert(memcmp(indexes1, indexes2, dim * sizeof(ssize_t)) == 0);
	}
	assert(excit_next(it, indexes1) == EXCIT_STOPIT);

	free(indexes1);
	free(indexes2);
	excit_free(it);
	excit_free(iit);
}

void test_composition_iterator(excit_t source, excit_t indexer)
{
	test_alloc_init_composition(source, indexer);

	test_next_composition(source, indexer);

	int i = 0;

	while (synthetic_tests[i]) {
		excit_t it = create_test_composition(source, indexer);

		synthetic_tests[i] (it);
		excit_free(it);
		i++;
	}
}

int main(void)
{
	excit_t source, indexer, source2, indexer2;

	source = create_test_range(-15, 14, 2);
	indexer = create_test_range(4, 12, 3);
	test_composition_iterator(source, indexer);

	source2 = excit_alloc_test(EXCIT_PRODUCT);
	assert(excit_product_add(source2, create_test_range(-15, 14, 2)) == ES);
	assert(excit_product_add(source2, create_test_range(4, 12, 3)) == ES);
	indexer2 = create_test_range(4, 35, 2);
	test_composition_iterator(source2, indexer2);

	excit_free(source);
	excit_free(source2);
	excit_free(indexer);
	excit_free(indexer2);
}
