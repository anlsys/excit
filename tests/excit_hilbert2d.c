#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "excit.h"
#include "excit_test.h"

/* Helper functions from: https://en.wikipedia.org/wiki/Hilbert_curve */

//rotate/flip a quadrant appropriately
static void rot(ssize_t n, ssize_t *x, ssize_t *y, ssize_t rx, ssize_t ry)
{
	if (ry == 0) {
		if (rx == 1) {
			*x = n - 1 - *x;
			*y = n - 1 - *y;
		}
		//Swap x and y
		ssize_t t = *x;

		*x = *y;
		*y = t;
	}
}

//convert d to (x,y)
static void d2xy(ssize_t n, ssize_t d, ssize_t *x, ssize_t *y)
{
	ssize_t rx, ry, s, t = d;

	*x = *y = 0;
	for (s = 1; s < n; s *= 2) {
		rx = 1 & (t / 2);
		ry = 1 & (t ^ rx);
		rot(s, x, y, rx, ry);
		*x += s * rx;
		*y += s * ry;
		t /= 4;
	}
}

/* End helper functions */

void test_alloc_init_hilbert2d(int order)
{
	excit_t it;
	ssize_t dim, size;

	it = excit_alloc_test(EXCIT_HILBERT2D);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 0);

	assert(excit_hilbert2d_init(it, order) == ES);
	assert(excit_dimension(it, &dim) == ES);
	assert(dim == 2);
	assert(excit_size(it, &size) == ES);
	assert(size == (1 << order) * (1 << order));

	excit_free(it);
}

excit_t create_test_hilbert2d(int order)
{
	excit_t it;
	it = excit_alloc_test(EXCIT_HILBERT2D);
	assert(excit_hilbert2d_init(it, order) == ES);
	return it;
}

void test_next_hilbert2d(int order)
{
	excit_t it = create_test_hilbert2d(order);
	ssize_t indexes1[2], indexes2[2];

	for (int i = 0; i < (1 << order) * (1 << order); i++) {
		assert(excit_next(it, indexes1) == ES);
		d2xy(1<<order, i, indexes2, indexes2 + 1);
		assert(indexes1[0] == indexes2[0]);
		assert(indexes1[1] == indexes2[1]);
	}
	excit_free(it);
}

void test_hilbert2d_iterator(int order)
{
	test_alloc_init_hilbert2d(order);

	test_next_hilbert2d(order);

	int i = 0;
	while (synthetic_tests[i]) {
		excit_t it = create_test_hilbert2d(order);

		synthetic_tests[i](it);
		excit_free(it);
		i++;
	}
	
}

int main(int argc, char *argv[])
{
	test_hilbert2d_iterator(3);
	test_hilbert2d_iterator(4);
}
