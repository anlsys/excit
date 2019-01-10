#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "excit.h"
#include "excit_test.h"

static excit_t create_test_tleaf(const ssize_t depth,
				 const ssize_t *arities,
				 const enum tleaf_it_policy_e policy,
				 const ssize_t *user_policy)
{
	int err = EXCIT_SUCCESS;
	excit_t it;

	it = excit_alloc_test(EXCIT_TLEAF);
	assert(it != NULL);

	err = excit_tleaf_init(it, depth + 1, arities, policy, user_policy);
	assert(err == EXCIT_SUCCESS);

	ssize_t i, size = 1, it_size;

	for (i = 0; i < depth; i++)
		size *= arities[i];
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

static void tleaf_test_round_robin_split(excit_t tleaf,
					 const ssize_t depth,
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
		excit_t it = split[i*ncut/size];

		assert(excit_next(it, &value) == EXCIT_SUCCESS);
		assert(value == i);
	}

	for (i = 0; i < ncut; i++)
		excit_free(split[i]);
	free(split);
	free(cut_sizes);
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

void run_tests(const ssize_t depth, const ssize_t *arities)
{
	excit_t rrobin =
	    create_test_tleaf(depth, arities, TLEAF_POLICY_ROUND_ROBIN, NULL);

	assert(rrobin != NULL);
	tleaf_test_round_robin_split(rrobin, depth, arities);
	tleaf_test_round_robin_policy(rrobin);
	excit_free(rrobin);

	excit_t scatter =
	    create_test_tleaf(depth, arities, TLEAF_POLICY_SCATTER, NULL);

	assert(scatter != NULL);
	tleaf_test_scatter_policy_no_split(scatter, depth, arities);
	excit_free(scatter);

	int i = 0;

	while (synthetic_tests[i]) {
		excit_t it =
		    create_test_tleaf(depth, arities, TLEAF_POLICY_ROUND_ROBIN,
				      NULL);

		synthetic_tests[i] (it);
		excit_free(it);
		i++;
	}
}

int main(int argc, char *argv[])
{
	ssize_t depth = 4;
	const ssize_t arities_0[4] = { 4, 8, 2, 4 };

	run_tests(depth, arities_0);

	depth = 8;
	const ssize_t arities_1[8] = { 4, 6, 2, 4, 3, 6, 2, 9 };

	run_tests(depth, arities_1);

	return 0;
}
