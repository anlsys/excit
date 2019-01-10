#include "excit_test.h"
#include "excit.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

excit_t excit_alloc_test(enum excit_type_e type) {
	excit_t it;
	enum excit_type_e newtype;

	it = excit_alloc(type);
	assert(it != NULL);
	assert(excit_type(it, &newtype) == ES);
	assert(type == newtype);
	return it;
}

static inline excit_t excit_dup_test(const excit_t it) {
	excit_t newit = excit_dup(it);
	assert(newit != NULL);
	return newit;
}

static inline void excit_dimension_test(excit_t it, ssize_t *dim) {
	assert( excit_dimension(it, dim) == ES );
}

static inline void deplete_iterator(excit_t it) {
	while(excit_next(it, NULL) == ES)
		;
}

static void test_iterator_result_equal(excit_t it1, excit_t it2) {
	ssize_t dim1, dim2;
	excit_dimension_test(it1, &dim1);
	excit_dimension_test(it2, &dim2);
	assert(dim1 == dim2);

	ssize_t *indexes1, *indexes2;
	ssize_t buff_dim = dim1 * sizeof(ssize_t);

	indexes1 = (ssize_t *)malloc(buff_dim);
	indexes2 = (ssize_t *)malloc(buff_dim);

	while (excit_next(it1, indexes1) == ES) {
		assert(excit_next(it2, indexes2) == ES);
		assert(memcmp(indexes1, indexes2, buff_dim) == 0);
	}
        assert( excit_next(it2, indexes2) == EXCIT_STOPIT );
	
	free(indexes1);
	free(indexes2);
}

void test_dup(excit_t it1) {
	excit_t it2, it3;

        it2 = excit_dup_test(it1);
	it3 = excit_dup_test(it1);

	test_iterator_result_equal(it1, it2);
        excit_free(it2);

        /* Check that the state is correctly copied */
        assert(excit_next(it3, NULL) == ES);
	it2 = excit_dup(it3);
	test_iterator_result_equal(it3, it2);
	excit_free(it2);
	excit_free(it3);
}

void test_rewind(excit_t it1) {
	excit_t it2, it3;

        it2 = excit_dup_test(it1);
	it3 = excit_dup_test(it1);

	assert(excit_next(it1, NULL) == ES);
	assert(excit_rewind(it1) == ES);
	test_iterator_result_equal(it1, it2);

	deplete_iterator(it1);
	assert(excit_rewind(it1) == ES);
	test_iterator_result_equal(it1, it3);

	excit_free(it2);
	excit_free(it3);
}

void test_peek(excit_t it1) {
	excit_t it2;
	it2 = excit_dup_test(it1);

	ssize_t dim1;
	excit_dimension_test(it1, &dim1);

	ssize_t *indexes1, *indexes2;
	ssize_t buff_dim = dim1 * sizeof(ssize_t);

	indexes1 = (ssize_t *)malloc(buff_dim);
	indexes2 = (ssize_t *)malloc(buff_dim);

	while (excit_next(it1, indexes1) == ES) {
		assert(excit_peek(it2, indexes2) == ES);
		assert(memcmp(indexes1, indexes2, buff_dim) == 0);
		assert(excit_next(it2, indexes2) == ES);
		assert(memcmp(indexes1, indexes2, buff_dim) == 0);
	}
	assert(excit_peek(it2, indexes2) == EXCIT_STOPIT);
        assert(excit_next(it2, indexes2) == EXCIT_STOPIT);
	
	free(indexes1);
	free(indexes2);
	excit_free(it2);
}

void test_size(excit_t it) {
	ssize_t size;
	ssize_t count = 0;
	int err;

	err = excit_size(it, &size);
	if (err == -EXCIT_ENOTSUP)
		return;
	assert(err == ES);

	while (excit_next(it, NULL) == ES) {
		count++;
	}
	assert( size == count);
}

void test_cyclic_next(excit_t it1) {
	excit_t it2, it3;
	int looped = 0;

        it2 = excit_dup_test(it1);
	it3 = excit_dup_test(it1);
	
	ssize_t dim1;
	excit_dimension_test(it1, &dim1);

	ssize_t *indexes1, *indexes2;
	ssize_t buff_dim = dim1 * sizeof(ssize_t);

	indexes1 = (ssize_t *)malloc(buff_dim);
	indexes2 = (ssize_t *)malloc(buff_dim);

	while (excit_next(it1, indexes1) == ES) {
		assert(looped == 0);
		assert(excit_cyclic_next(it2, indexes2, &looped) == ES);
		assert(memcmp(indexes1, indexes2, buff_dim) == 0);
	}
	assert(looped == 1);
	test_iterator_result_equal(it2, it3);

	free(indexes1);
	free(indexes2);

	excit_free(it2);
	excit_free(it3);
}

void test_skip(excit_t it1) {
	excit_t it2;

        it2 = excit_dup_test(it1);

	while (excit_next(it1, NULL) == ES) {
		assert( excit_skip(it2) == ES);
	}
	assert( excit_skip(it2) == EXCIT_STOPIT );

	excit_free(it2);
}

void test_pos(excit_t it) {
	ssize_t rank, expected_rank;

	if (excit_pos(it, &rank) == -EXCIT_ENOTSUP)
		return;

	expected_rank = 0;
	while (excit_peek(it, NULL) == ES) {
		assert(excit_pos(it, &rank) == ES);
		assert(expected_rank == rank);
		assert(excit_next(it, NULL) == ES);
		expected_rank++;
	}
	assert(excit_pos(it, &rank) == EXCIT_STOPIT);
}

void test_nth(excit_t it1) {
	excit_t it2;
	ssize_t rank = -1;

	if (excit_nth(it1, 0, NULL) == -EXCIT_ENOTSUP)
		return;

        it2 = excit_dup_test(it1);
	ssize_t dim1;
	excit_dimension_test(it1, &dim1);

	ssize_t *indexes1, *indexes2;
	ssize_t buff_dim = dim1 * sizeof(ssize_t);

	indexes1 = (ssize_t *)malloc(buff_dim);
	indexes2 = (ssize_t *)malloc(buff_dim);

	assert(excit_nth(it2, rank, indexes2) == -EXCIT_EDOM);
	rank++;
	while (excit_next(it1, indexes1) == ES) {
		assert(excit_nth(it2, rank, indexes2) == ES);
		assert(memcmp(indexes1, indexes2, buff_dim) == 0);
		rank++;
	}
	assert(excit_nth(it2, rank, indexes2) == -EXCIT_EDOM);

	free(indexes1);
	free(indexes2);

	excit_free(it2);
}

void test_rank(excit_t it1) {
	excit_t it2;
	ssize_t rank, expected_rank;

        it2 = excit_dup_test(it1);
	ssize_t dim1;
	excit_dimension_test(it1, &dim1);

	ssize_t *indexes1;
	ssize_t buff_dim = dim1 * sizeof(ssize_t);

	indexes1 = (ssize_t *)malloc(buff_dim);

	assert(excit_peek(it1, indexes1) == ES);
	if (excit_rank(it2, indexes1, &rank) == -EXCIT_ENOTSUP)
		goto error;

	expected_rank = 0;
	while(excit_next(it1, indexes1) == ES) {
		assert(excit_rank(it2, indexes1, &rank) == ES);
		assert(expected_rank == rank);
		expected_rank++;
	}

	for(int i = 0; i < dim1; i++) {
		indexes1[i] = 0xDEADBEEFDEADBEEF;
	}
	assert(excit_rank(it2, indexes1, &rank) == -EXCIT_EINVAL);

error:
	free(indexes1);

	excit_free(it2);
	
}

void test_split(excit_t it) {
	int num_split = 3;
	excit_t its[num_split];
	ssize_t dim;

	assert(excit_split(it, 0, NULL) == -EXCIT_EDOM);
	assert(excit_split(it, -1, NULL) == -EXCIT_EDOM);
	assert(excit_split(it, 0xDEADBEEF, NULL) == -EXCIT_EDOM);

	excit_dimension_test(it, &dim);

	ssize_t *indexes1, *indexes2;
	ssize_t buff_dim = dim * sizeof(ssize_t);

	indexes1 = (ssize_t *)malloc(buff_dim);
	indexes2 = (ssize_t *)malloc(buff_dim);

	assert(excit_split(it, num_split, its) == ES);
	for(int i = 0; i < num_split; i++) {
		while(excit_next(its[i], indexes2) == ES) {
			assert(excit_next(it, indexes1) == ES);
			assert(memcmp(indexes1, indexes2, buff_dim) == 0);
		}
		excit_free(its[i]);
	}
	assert(excit_next(it, indexes1) == EXCIT_STOPIT);

	free(indexes1);
	free(indexes2);
}

void (*synthetic_tests[])(excit_t) = {
	&test_skip,
	&test_size,
	&test_dup,
	&test_peek,
	&test_rewind,
	&test_cyclic_next,
	&test_pos,
	&test_nth,
	&test_rank,
	&test_split,
	NULL
};
