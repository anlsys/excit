#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "excit.h"

void test_range_iterator(void)
{
	excit_t it;
	excit_index_t dim;
	excit_index_t size;
	excit_index_t indexes[1];
	enum excit_type_e type;
	excit_t its[3];
	int looped;
	int i;
	excit_index_t ith;

	it = excit_alloc(EXCIT_RANGE);
	assert(it != NULL);
	assert(excit_type(it, &type) == 0);
	assert(type == EXCIT_RANGE);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 0);

	assert(excit_range_init(it, 0, 3, 1) == 0);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 1);
	assert(excit_size(it, &size) == 0);
	assert(size == 4);

	for (i = 0; i < 4; i++) {
		assert(excit_nth(it, i, indexes) == 0);
		assert(indexes[0] == i);
		assert(excit_n(it, indexes, &ith) == 0);
		assert(ith == i);
		ith = -1;
		assert(excit_pos(it, &ith) == 0);
		assert(ith == i);
		assert(excit_peek(it, indexes) == 0);
		assert(indexes[0] == i);
		assert(excit_next(it, indexes) == 0);
		assert(indexes[0] == i);
	}
	assert(excit_next(it, indexes) == -ERANGE);
	assert(excit_peek(it, indexes) == -ERANGE);

	assert(excit_rewind(it) == 0);
	looped = 0;
	i = 0;
	while (!looped) {
		assert(excit_cyclic_next(it, indexes, &looped) == 0);
		assert(indexes[0] == i);
		i++;
	}
	assert(excit_peek(it, indexes) == 0);
	assert(indexes[0] == 0);
	assert(excit_cyclic_next(it, indexes, &looped) == 0);
	assert(indexes[0] == 0);
	assert(looped == 0);

	for (i = 1; i < 4; i++) {
		assert(excit_next(it, indexes) == 0);
		assert(indexes[0] == i);
	}
	assert(excit_next(it, indexes) == -ERANGE);
	assert(excit_next(it, indexes) == -ERANGE);
	assert(excit_cyclic_next(it, indexes, &looped) == 0);
	assert(indexes[0] == 0);
	assert(looped == 1);

	assert(excit_range_init(it, 3, 0, -1) == 0);
	assert(excit_size(it, &size) == 0);
	assert(size == 4);

	for (i = 3; i >= 0; i--) {
		assert(excit_next(it, indexes) == 0);
		assert(indexes[0] == i);
	}
	assert(excit_next(it, indexes) == -ERANGE);

	assert(excit_range_init(it, 3, 0, 1) == 0);
	assert(excit_size(it, &size) == 0);
	assert(size == 0);

	assert(excit_range_init(it, 0, 9, 1) == 0);
	assert(excit_size(it, &size) == 0);
	assert(size == 10);
	assert(excit_split(it, 3, its) == 0);
	assert(excit_size(its[0], &size) == 0);
	assert(size == 4);
	for (int i = 0; i < 4; i++) {
		assert(excit_next(its[0], indexes) == 0);
		assert(indexes[0] == i);
	}
	assert(excit_size(its[1], &size) == 0);
	assert(size == 3);
	for (int i = 4; i < 7; i++) {
		assert(excit_next(its[1], indexes) == 0);
		assert(indexes[0] == i);
	}
	assert(excit_size(its[2], &size) == 0);
	assert(size == 3);
	for (int i = 7; i < 10; i++) {
		assert(excit_next(its[2], indexes) == 0);
		assert(indexes[0] == i);
	}
	excit_free(its[0]);
	excit_free(its[1]);
	excit_free(its[2]);

	assert(excit_split(it, 24, its) == -EDOM);
	assert(excit_split(it, 0, its) == -EDOM);
	assert(excit_split(it, -1, its) == -EDOM);
	excit_free(it);
}

void test_product_iterators(void)
{
	int i, j, k;
	int looped;
	excit_t it;
	excit_t tmp;
	excit_index_t dim;
	excit_index_t count;
	excit_index_t size;
	excit_index_t indexes[3];
	excit_index_t indexes2[3];
	enum excit_type_e type;
	excit_t its[5];
	excit_index_t ith;

	it = excit_alloc(EXCIT_PRODUCT);
	assert(it != NULL);
	assert(excit_type(it, &type) == 0);
	assert(type == EXCIT_PRODUCT);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 0);
	assert(excit_product_count(it, &count) == 0);
	assert(count == 0);
	assert(excit_size(it, &size) == 0);
	assert(size == 0);
	assert(excit_peek(it, indexes) == -EINVAL);

	tmp = excit_alloc(EXCIT_RANGE);
	excit_range_init(tmp, 0, 3, 1);
	assert(excit_product_add(it, tmp) == 0);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 1);
	assert(excit_product_count(it, &count) == 0);
	assert(count == 1);
	assert(excit_size(it, &size) == 0);
	assert(size == 4);
	assert(excit_peek(it, indexes) == 0);
	assert(indexes[0] == 0);

	tmp = excit_alloc(EXCIT_RANGE);
	excit_range_init(tmp, 1, -1, -1);
	assert(excit_product_add(it, tmp) == 0);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 2);
	assert(excit_product_count(it, &count) == 0);
	assert(count == 2);
	assert(excit_size(it, &size) == 0);
	assert(size == 3 * 4);
	assert(excit_peek(it, indexes) == 0);
	assert(indexes[0] == 0);
	assert(indexes[1] == 1);

	k = 0;
	for (i = 0; i <= 3; i++) {
		for (j = 1; j >= -1; j--, k++) {
			assert(excit_nth(it, k, indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == j);
			assert(excit_pos(it, &ith) == 0);
			assert(ith == k);
			ith = -1;
			assert(excit_n(it, indexes, &ith) == 0);
			assert(ith == k);
			assert(excit_peek(it, indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == j);
			assert(excit_next(it, indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == j);
		}
	}
	assert(excit_peek(it, indexes) == -ERANGE);
	assert(excit_next(it, indexes) == -ERANGE);
	assert(excit_next(it, indexes) == -ERANGE);
	assert(excit_cyclic_next(it, indexes, &looped) == 0);
	assert(looped == 1);
	assert(indexes[0] == 0);
	assert(indexes[1] == 1);

	assert(excit_rewind(it) == 0);
	for (k = 0; k < 3; k++) {
		for (i = 0; i <= 3; i++) {
			for (j = 1; j >= -1; j--) {
				assert(excit_cyclic_next
				       (it, indexes, &looped) == 0);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
			}
		}
		assert(looped == 1);
	}

	excit_rewind(it);

	tmp = excit_alloc(EXCIT_RANGE);
	excit_range_init(tmp, -5, 5, 1);
	assert(excit_product_add(it, tmp) == 0);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 3);
	assert(excit_size(it, &size) == 0);
	assert(size == 3 * 4 * 11);

	//split first dimension
	assert(excit_product_split_dim(it, 0, 2, its) == 0);
	for (i = 0; i <= 1; i++) {
		for (j = 1; j >= -1; j--) {
			for (k = -5; k <= 5; k++) {
				assert(excit_next(its[0], indexes) == 0);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(its[0], indexes) == -ERANGE);
	for (i = 2; i <= 3; i++) {
		for (j = 1; j >= -1; j--) {
			for (k = -5; k <= 5; k++) {
				assert(excit_next(its[1], indexes) == 0);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(its[1], indexes) == -ERANGE);
	excit_free(its[0]);
	excit_free(its[1]);

	//split second dimension
	assert(excit_product_split_dim(it, 1, 2, its) == 0);
	for (i = 0; i <= 3; i++) {
		for (j = 1; j >= 0; j--) {
			for (k = -5; k <= 5; k++) {
				assert(excit_next(its[0], indexes) == 0);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(its[0], indexes) == -ERANGE);
	for (i = 0; i <= 3; i++) {
		for (j = -1; j >= -1; j--) {
			for (k = -5; k <= 5; k++) {
				assert(excit_next(its[1], indexes) == 0);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(its[1], indexes) == -ERANGE);
	excit_free(its[0]);
	excit_free(its[1]);

	//split third dimension
	assert(excit_product_split_dim(it, 2, 2, its) == 0);
	for (i = 0; i <= 3; i++) {
		for (j = 1; j >= -1; j--) {
			for (k = -5; k <= 0; k++) {
				assert(excit_next(its[0], indexes) == 0);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(its[0], indexes) == -ERANGE);
	for (i = 0; i <= 3; i++) {
		for (j = 1; j >= -1; j--) {
			for (k = 1; k <= 5; k++) {
				assert(excit_next(its[1], indexes) == 0);
				assert(indexes[0] == i);
				assert(indexes[1] == j);
				assert(indexes[2] == k);
			}
		}
	}
	assert(excit_next(its[1], indexes) == -ERANGE);
	excit_free(its[0]);
	excit_free(its[1]);

	assert(excit_product_split_dim(it, 2, 15, its) == -EDOM);

	assert(excit_split(it, 5, its) == 0);

	assert(excit_rewind(it) == 0);
	for (i = 0; i < 5; i++) {
		while (excit_next(its[i], indexes)) {
			assert(excit_next(it, indexes2) == 0);
			assert(indexes[0] == indexes2[0]);
			assert(indexes[1] == indexes2[1]);
			assert(indexes[2] == indexes2[2]);
		}
	}

	for (i = 0; i < 5; i++)
		excit_free(its[i]);
	excit_free(it);

}

void test_repeat_iterator(void)
{
	excit_t it;
	excit_t tmp;
	excit_index_t dim;
	excit_index_t size;
	enum excit_type_e type;
	excit_index_t indexes[1];
	excit_t its[2];
	excit_index_t ith;

	it = excit_alloc(EXCIT_REPEAT);
	assert(it != NULL);
	assert(excit_type(it, &type) == 0);
	assert(type == EXCIT_REPEAT);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 0);

	tmp = excit_alloc(EXCIT_RANGE);
	excit_range_init(tmp, 0, 2, 1);
	excit_repeat_init(it, tmp, 2);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 1);
	assert(excit_size(it, &size) == 0);
	assert(size == 6);

	for (int i = 0, k = 0; i <= 2; i++) {
		for (int j = 0; j < 2; j++, k++) {
			assert(excit_nth(it, k, indexes) == 0);
			assert(indexes[0] == i);
			assert(excit_pos(it, &ith) == 0);
			assert(ith == k);
			assert(excit_peek(it, indexes) == 0);
			assert(indexes[0] == i);
			assert(excit_next(it, indexes) == 0);
			assert(indexes[0] == i);
		}
	}
	assert(excit_peek(it, indexes) == -ERANGE);
	assert(excit_next(it, indexes) == -ERANGE);

	assert(excit_rewind(it) == 0);
	for (int i = 0; i <= 2; i++) {
		for (int j = 0; j < 2; j++) {
			assert(excit_peek(it, indexes) == 0);
			assert(indexes[0] == i);
			assert(excit_next(it, indexes) == 0);
			assert(indexes[0] == i);
		}
	}
	assert(excit_peek(it, indexes) == -ERANGE);
	assert(excit_next(it, indexes) == -ERANGE);

	assert(excit_split(it, 2, its) == 0);
	for (int i = 0; i <= 1; i++) {
		for (int j = 0; j < 2; j++) {
			assert(excit_peek(its[0], indexes) == 0);
			assert(indexes[0] == i);
			assert(excit_next(its[0], indexes) == 0);
			assert(indexes[0] == i);
		}
	}
	assert(excit_peek(its[0], indexes) == -ERANGE);
	assert(excit_next(its[0], indexes) == -ERANGE);
	for (int i = 2; i <= 2; i++) {
		for (int j = 0; j < 2; j++) {
			assert(excit_peek(its[1], indexes) == 0);
			assert(indexes[0] == i);
			assert(excit_next(its[1], indexes) == 0);
			assert(indexes[0] == i);
		}
	}
	assert(excit_peek(its[1], indexes) == -ERANGE);
	assert(excit_next(its[1], indexes) == -ERANGE);

	excit_free(its[0]);
	excit_free(its[1]);
	excit_free(it);
}

void test_cons_iterator(void)
{
	excit_t it;
	excit_t tmp;
	excit_index_t dim;
	excit_index_t size;
	enum excit_type_e type;
	excit_index_t indexes[4];
	excit_t its[2];
	excit_index_t ith;

	it = excit_alloc(EXCIT_CONS);
	assert(it != NULL);
	assert(excit_type(it, &type) == 0);
	assert(type == EXCIT_CONS);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 0);

	tmp = excit_alloc(EXCIT_RANGE);
	excit_range_init(tmp, 0, 4, 1);
	assert(excit_cons_init(it, tmp, 3) == 0);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 3);
	assert(excit_size(it, &size) == 0);
	assert(size == 3);
	for (int j = 0; j < 2; j++) {
		for (int i = 0, k = 0; i < 3; i++, k++) {
			assert(excit_nth(it, k, indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
			assert(excit_pos(it, &ith) == 0);
			assert(ith == k);
			ith = -1;
			assert(excit_n(it, indexes, &ith) == 0);
			assert(ith == k);
			assert(excit_peek(it, indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
			assert(excit_next(it, indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
		}
		assert(excit_peek(it, indexes) == -ERANGE);
		assert(excit_next(it, indexes) == -ERANGE);
		assert(excit_rewind(it) == 0);
	}

	assert(excit_split(it, 2, its) == 0);

	for (int j = 0; j < 2; j++) {
		for (int i = 0, k = 0; i < 2; i++, k++) {
			assert(excit_nth(its[0], k, indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
			assert(excit_peek(its[0], indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
			assert(excit_next(its[0], indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
		}
		assert(excit_peek(its[0], indexes) == -ERANGE);
		assert(excit_next(its[0], indexes) == -ERANGE);
		assert(excit_rewind(its[0]) == 0);
	}

	for (int j = 0; j < 2; j++) {
		for (int i = 2, k = 0; i < 3; i++, k++) {
			assert(excit_nth(its[1], k, indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
			assert(excit_peek(its[1], indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
			assert(excit_next(its[1], indexes) == 0);
			assert(indexes[0] == i);
			assert(indexes[1] == i + 1);
			assert(indexes[2] == i + 2);
		}
		assert(excit_peek(its[1], indexes) == -ERANGE);
		assert(excit_next(its[1], indexes) == -ERANGE);
		assert(excit_rewind(its[1]) == 0);
	}
	excit_free(its[0]);
	excit_free(its[1]);
	excit_free(it);

	it = excit_alloc(EXCIT_PRODUCT);
	tmp = excit_alloc(EXCIT_RANGE);
	excit_range_init(tmp, 0, 3, 1);
	excit_product_add(it, tmp);
	tmp = excit_alloc(EXCIT_RANGE);
	excit_range_init(tmp, 1, -1, -1);
	excit_product_add(it, tmp);

	tmp = excit_alloc(EXCIT_CONS);
	assert(excit_cons_init(tmp, it, 2) == 0);
	assert(excit_dimension(tmp, &dim) == 0);
	assert(dim == 4);
	assert(excit_size(tmp, &size) == 0);
	assert(size == 11);
	assert(excit_peek(tmp, indexes) == 0);
	assert(indexes[0] == 0);
	assert(indexes[1] == 1);
	assert(indexes[2] == 0);
	assert(indexes[3] == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(indexes[0] == 3);
	assert(indexes[1] == 1);
	assert(indexes[2] == 3);
	assert(indexes[3] == 0);
	assert(excit_next(tmp, indexes) == 0);
	assert(indexes[0] == 3);
	assert(indexes[1] == 0);
	assert(indexes[2] == 3);
	assert(indexes[3] == -1);
	assert(excit_next(tmp, indexes) == -ERANGE);

	excit_free(tmp);

}

void test_hilbert2d_iterator(void)
{
	excit_index_t dim;
	excit_index_t size;
	excit_index_t indexes[2];
	excit_index_t indexes2[2];
	enum excit_type_e type;
	excit_t it;
	excit_t its[3];
	excit_index_t ith;

	it = excit_alloc(EXCIT_HILBERT2D);
	assert(it != NULL);
	assert(excit_type(it, &type) == 0);
	assert(type == EXCIT_HILBERT2D);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 0);
	assert(excit_hilbert2d_init(it, 2) == 0);
	assert(excit_dimension(it, &dim) == 0);
	assert(dim == 2);
	assert(excit_size(it, &size) == 0);
	assert(size == 16);

	assert(excit_peek(it, indexes) == 0);
	assert(indexes[0] == 0);
	assert(indexes[1] == 0);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 0);
	assert(indexes[1] == 0);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 1);
	assert(indexes[1] == 0);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 1);
	assert(indexes[1] == 1);
	assert(excit_pos(it, &ith) == 0);
	assert(ith == 3);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 0);
	assert(indexes[1] == 1);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 0);
	assert(indexes[1] == 2);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 0);
	assert(indexes[1] == 3);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 1);
	assert(indexes[1] == 3);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 1);
	assert(indexes[1] == 2);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 2);
	assert(indexes[1] == 2);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 2);
	assert(indexes[1] == 3);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 3);
	assert(indexes[1] == 3);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 3);
	assert(indexes[1] == 2);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 3);
	assert(indexes[1] == 1);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 2);
	assert(indexes[1] == 1);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 2);
	assert(indexes[1] == 0);
	assert(excit_next(it, indexes) == 0);
	assert(indexes[0] == 3);
	assert(indexes[1] == 0);
	assert(excit_peek(it, indexes) == -ERANGE);
	assert(excit_next(it, indexes) == -ERANGE);
	assert(excit_rewind(it) == 0);
	assert(excit_peek(it, indexes) == 0);
	assert(indexes[0] == 0);
	assert(indexes[1] == 0);
	assert(excit_peek(it, NULL) == 0);
	assert(excit_skip(it) == 0);
	assert(excit_peek(it, indexes) == 0);
	assert(indexes[0] == 1);
	assert(indexes[1] == 0);

	assert(excit_rewind(it) == 0);
	int j = 0;

	while (excit_next(it, indexes) == 0) {
		assert(excit_nth(it, j, indexes2) == 0);
		assert(indexes[0] == indexes2[0]);
		assert(indexes[1] == indexes2[1]);
		assert(excit_n(it, indexes, &ith) == 0);
		assert(j == ith);
		j++;
	}

	assert(excit_split(it, 3, its) == 0);
	assert(excit_rewind(it) == 0);
	for (int i = 0; i < 3; i++) {
		while (excit_next(its[i], indexes) == 0) {
			assert(excit_next(it, indexes2) == 0);
			assert(indexes[0] == indexes2[0]);
			assert(indexes[1] == indexes2[1]);
		}
	}

	excit_free(its[0]);
	excit_free(its[1]);
	excit_free(its[2]);

	assert(excit_split(it, 17, its) == -EDOM);

	excit_free(it);
}

int main(int argc, char *argv[])
{
	test_range_iterator();
	test_product_iterators();
	test_cons_iterator();
	test_repeat_iterator();
	test_hilbert2d_iterator();
	return 0;
}
