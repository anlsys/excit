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

static excit_t create_test_tleaf(const ssize_t depth,
				 const ssize_t *arities,
				 excit_t *indexes,
				 const enum tleaf_it_policy_e policy,
				 const ssize_t *user_policy)
{
	int err = EXCIT_SUCCESS;
	excit_t it;

	it = excit_alloc_test(EXCIT_TLEAF);
	assert(it != NULL);

	err =
	    excit_tleaf_init(it, depth + 1, arities, indexes, policy,
			     user_policy);
	assert(err == EXCIT_SUCCESS);

	ssize_t i, size = 1, it_size, arity;

	for (i = 0; i < depth; i++) {
		if (indexes && indexes[i])
			assert(excit_size(indexes[i], &arity) == EXCIT_SUCCESS);
		else
			arity = arities[i];
		size *= arity;
	}
	assert(excit_size(it, &it_size) == EXCIT_SUCCESS);
	assert(it_size == size);

	return it;
}

static void tleaf_test_round_robin_policy(excit_t tleaf)
{
	ssize_t i, value, size;

	assert(excit_size(tleaf, &size) == EXCIT_SUCCESS);
	assert(excit_rewind(tleaf) == EXCIT_SUCCESS);
	for (i = 0; i < size; i++) {
		assert(excit_next(tleaf, &value) == EXCIT_SUCCESS);
		assert(value == i);
	}
	assert(excit_next(tleaf, &value) == EXCIT_STOPIT);
}

static void tleaf_test_indexed_round_robin_policy(excit_t tleaf,
						  const ssize_t depth,
						  const ssize_t *arities,
						  excit_t *_indexes)
{
	ssize_t i, j, value, indexed_value, indexed_mul, size, arity;
	ssize_t *values = malloc(depth * sizeof(*values));
	excit_t check = excit_alloc(EXCIT_PRODUCT);

	assert(values != NULL);
	assert(check != NULL);
	for (i = 0; i < depth; i++) {
		excit_t range = excit_alloc(EXCIT_RANGE);

		assert(range != NULL);
		assert(excit_range_init(range, 0, arities[i] - 1, 1) ==
		       EXCIT_SUCCESS);

		if (_indexes[i] != NULL) {
			excit_t comp = excit_alloc(EXCIT_COMPOSITION);

			assert(comp != NULL);

			excit_t index = excit_dup(_indexes[i]);

			assert(index != NULL);
			assert(excit_rewind(index) == EXCIT_SUCCESS);
			assert(excit_composition_init(comp, range, index) ==
			       EXCIT_SUCCESS);
			assert(excit_product_add(check, comp) == EXCIT_SUCCESS);
		} else {
			assert(excit_product_add(check, range) ==
			       EXCIT_SUCCESS);
		}
	}

	assert(excit_size(tleaf, &size) == EXCIT_SUCCESS);
	assert(excit_rewind(tleaf) == EXCIT_SUCCESS);
	assert(excit_rewind(check) == EXCIT_SUCCESS);

	for (i = 0; i < size; i++) {
		assert(excit_next(tleaf, &value) == EXCIT_SUCCESS);
		assert(excit_next(check, values) == EXCIT_SUCCESS);
		indexed_value = 0;
		indexed_mul = 1;
		for (j = 0; j < depth; j++) {
			arity = arities[depth - j - 1];
			indexed_value += indexed_mul * values[depth - j - 1];
			indexed_mul *= arity;
		}
		assert(value == indexed_value);
	}

	excit_free(check);
	free(values);
}

static void tleaf_test_scatter_policy_no_split(excit_t tleaf,
					       const ssize_t depth,
					       const ssize_t *arities)
{
	ssize_t i, j, r, n, c, value, val, size;

	assert(excit_size(tleaf, &size) == EXCIT_SUCCESS);
	assert(excit_rewind(tleaf) == EXCIT_SUCCESS);
	for (i = 0; i < size; i++) {
		c = i;
		n = size;
		val = 0;
		assert(excit_next(tleaf, &value) == EXCIT_SUCCESS);
		for (j = 0; j < depth; j++) {
			r = c % arities[j];
			n = n / arities[j];
			c = c / arities[j];
			val += n * r;
		}
		assert(value == val);
	}
	assert(excit_next(tleaf, &value) == EXCIT_STOPIT);
}

static void tleaf_test_round_robin_split(excit_t tleaf,
					 const ssize_t *arities)
{
	ssize_t i, value, size, cut_size;
	ssize_t ncut = arities[0];

	assert(excit_size(tleaf, &size) == EXCIT_SUCCESS);
	cut_size = size / ncut;

	excit_t *split = malloc(sizeof(*split) * ncut);

	assert(split != NULL);

	int err = tleaf_it_split(tleaf, 0, ncut, split);

	assert(err == EXCIT_SUCCESS);

	ssize_t *cut_sizes = malloc(sizeof(*cut_sizes) * ncut);

	assert(cut_sizes != NULL);

	for (i = 0; i < ncut; i++) {
		assert(excit_size(split[i], cut_sizes + i) == EXCIT_SUCCESS);
		assert(cut_sizes[i] == cut_size);
	}

	for (i = 0; i < size; i++) {
		excit_t it = split[i * ncut / size];

		assert(excit_next(it, &value) == EXCIT_SUCCESS);
		assert(value == i);
	}

	for (i = 0; i < ncut; i++)
		excit_free(split[i]);
	free(split);
	free(cut_sizes);
}

void run_tests(const ssize_t depth, const ssize_t *arities)
{
	/* Test of round robin policy */
	excit_t rrobin =
	    create_test_tleaf(depth, arities, NULL, TLEAF_POLICY_ROUND_ROBIN,
			      NULL);

	assert(rrobin != NULL);
	tleaf_test_round_robin_policy(rrobin);

	assert(excit_rewind(rrobin) == EXCIT_SUCCESS);

	/* Test of split operation on round robin policy */
	tleaf_test_round_robin_split(rrobin, arities);

	excit_free(rrobin);

	/* Test of indexing on a round robin policy */
	ssize_t i;
	excit_t *indexes = malloc(depth * sizeof(*indexes));

	assert(indexes != NULL);
	for (i = 0; i < depth; i++) {
		if (arities[i] > 2) {
			indexes[i] = excit_alloc(EXCIT_RANGE);
			assert(indexes[i] != NULL);
			assert(excit_range_init
			       (indexes[i], 0, arities[i] / 2 - 1,
				1) == EXCIT_SUCCESS);
		} else {
			indexes[i] = NULL;
		}
	}

	excit_t indexed_rrobin =
	    create_test_tleaf(depth, arities, indexes, TLEAF_POLICY_ROUND_ROBIN,
			      NULL);
	assert(indexed_rrobin != NULL);
	tleaf_test_indexed_round_robin_policy(indexed_rrobin, depth, arities,
					      indexes);
	excit_free(indexed_rrobin);

	for (i = 0; i < depth; i++)
		excit_free(indexes[i]);
	free(indexes);

	/* Test of scatter policy */
	excit_t scatter =
	    create_test_tleaf(depth, arities, NULL, TLEAF_POLICY_SCATTER, NULL);

	assert(scatter != NULL);
	tleaf_test_scatter_policy_no_split(scatter, depth, arities);
	excit_free(scatter);

	/* Generic iterator tests */
	i = 0;
	while (synthetic_tests[i]) {
		excit_t it =
		    create_test_tleaf(depth, arities, NULL,
				      TLEAF_POLICY_ROUND_ROBIN,
				      NULL);

		synthetic_tests[i] (it);
		excit_free(it);
		i++;
	}
}

int main(void)
{
	ssize_t depth = 4;
	const ssize_t arities_0[4] = { 4, 8, 2, 4 };

	run_tests(depth, arities_0);

	depth = 8;
	const ssize_t arities_1[8] = { 4, 6, 2, 4, 3, 6, 2, 9 };

	run_tests(depth, arities_1);

	return 0;
}
