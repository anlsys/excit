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

void test_alloc_init_prod(int count, excit_t *its)
{
	excit_t it;
	ssize_t dim, expected_dim, c, size, expected_size;

	it = excit_alloc_test(EXCIT_PRODUCT);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 0);
	assert(excit_product_count(it, &c) == ES);
	assert(c == 0);
	assert(excit_size(it, &size) == ES);
	assert(size == 0);

	expected_dim = 0;
	expected_size = 1;
	for (int i = 0; i < count; i++) {
		assert(excit_dimension(its[i], &dim) == ES);
		expected_dim += dim;
		assert(excit_size(its[i], &size) == ES);
		expected_size *= size;
		assert(excit_product_add_copy(it, its[i]) == ES);
		assert(excit_product_count(it, &c) == ES);
		assert(c == i + 1);
		assert(excit_dimension(it, &dim) == ES);
		assert(expected_dim == dim);
		assert(excit_size(it, &size) == ES);
		assert(expected_size == size);
	}
	excit_free(it);
}

excit_t create_test_product(int count, excit_t *its)
{
	excit_t it;

	it = excit_alloc_test(EXCIT_PRODUCT);
	for (int i = 0; i < count; i++)
		assert(excit_product_add_copy(it, its[i]) == ES);
	return it;
}

void test_next_product_helper(int count, excit_t it, excit_t *its,
			      ssize_t *indexes1, ssize_t *indexes2,
			      ssize_t *cindexes2)
{
	if (count == 0) {
		assert(excit_next(it, indexes1) == ES);
		assert(memcmp
		       (indexes1, indexes2,
			(intptr_t) cindexes2 - (intptr_t) indexes2) == 0);
	} else {
		excit_t cit;
		ssize_t dim;

		cit = excit_dup(*its);
		assert(cit != NULL);

		assert(excit_dimension(cit, &dim) == ES);
		while (excit_next(cit, cindexes2) == ES) {
			test_next_product_helper(count - 1, it, its + 1,
						 indexes1, indexes2,
						 cindexes2 + dim);
		}
		excit_free(cit);
	}
}

void test_next_product(int count, excit_t *its)
{
	excit_t it;
	ssize_t *indexes1, *indexes2;
	ssize_t dim;

	it = create_test_product(count, its);

	assert(excit_dimension(it, &dim) == ES);
	indexes1 = (ssize_t *) malloc(dim * sizeof(ssize_t));
	indexes2 = (ssize_t *) malloc(dim * sizeof(ssize_t));

	test_next_product_helper(count, it, its, indexes1, indexes2, indexes2);
	assert(excit_next(it, NULL) == EXCIT_STOPIT);

	free(indexes1);
	free(indexes2);

	excit_free(it);
}

void test_product_iterator(int count, excit_t *its)
{
	test_alloc_init_prod(count, its);

	test_next_product(count, its);

	int i = 0;

	while (synthetic_tests[i]) {
		excit_t it = create_test_product(count, its);

		synthetic_tests[i] (it);
		excit_free(it);
		i++;
	}
}

void test_product_split_dim(void)
{
	excit_t it;
	excit_t its[3];
	excit_t new_its[3];
	ssize_t indexes[3];

	its[0] = create_test_range(0, 3, 1);
	its[1] = create_test_range(1, -1, -1);
	its[2] = create_test_range(-5, 5, 1);
	it = create_test_product(3, its);

	//first
	assert(excit_product_split_dim(it, 0, 2, new_its) == ES);
	for (int i = 0; i <= 1; i++) {
		for (int j = 1; j >= -1; j--) {
			for (int k = -5; k <= 5; k++) {
				assert(excit_next(new_its[0], indexes) == ES);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(new_its[0], indexes) == EXCIT_STOPIT);
	for (int i = 2; i <= 3; i++) {
		for (int j = 1; j >= -1; j--) {
			for (int k = -5; k <= 5; k++) {
				assert(excit_next(new_its[1], indexes) == ES);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(new_its[1], indexes) == EXCIT_STOPIT);
	excit_free(new_its[0]);
	excit_free(new_its[1]);

	//second
	assert(excit_product_split_dim(it, 1, 2, new_its) == ES);
	for (int i = 0; i <= 3; i++) {
		for (int j = 1; j >= 0; j--) {
			for (int k = -5; k <= 5; k++) {
				assert(excit_next(new_its[0], indexes) == ES);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(new_its[0], indexes) == EXCIT_STOPIT);
	for (int i = 0; i <= 3; i++) {
		for (int j = -1; j >= -1; j--) {
			for (int k = -5; k <= 5; k++) {
				assert(excit_next(new_its[1], indexes) == ES);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(new_its[1], indexes) == EXCIT_STOPIT);
	excit_free(new_its[0]);
	excit_free(new_its[1]);

	//third
	assert(excit_product_split_dim(it, 2, 2, new_its) == ES);
	for (int i = 0; i <= 3; i++) {
		for (int j = 1; j >= -1; j--) {
			for (int k = -5; k <= 0; k++) {
				assert(excit_next(new_its[0], indexes) == ES);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(new_its[0], indexes) == EXCIT_STOPIT);
	for (int i = 0; i <= 3; i++) {
		for (int j = 1; j >= -1; j--) {
			for (int k = 1; k <= 5; k++) {
				assert(excit_next(new_its[1], indexes) == ES);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(new_its[1], indexes) == EXCIT_STOPIT);
	excit_free(new_its[0]);
	excit_free(new_its[1]);

	excit_free(its[0]);
	excit_free(its[1]);
	excit_free(its[2]);

	excit_free(it);
}

int main(int argc, char *argv[])
{
	excit_t its[4];

	its[0] = create_test_range(0, 3, 1);
	test_product_iterator(1, its);
	its[1] = create_test_range(1, -1, -1);
	test_product_iterator(2, its);
	its[2] = create_test_range(-5, 5, 1);
	test_product_iterator(3, its);
	its[3] = create_test_product(3, its);
	test_product_iterator(4, its);

	for (int i = 0; i < 4; i++)
		excit_free(its[i]);

	test_product_split_dim();
}
