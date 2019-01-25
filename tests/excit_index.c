#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include "excit.h"
#include "excit_test.h"

#define NTESTS  1
#define MAX_LEN 4096
#define MIN_LEN 16

static void rand_seed(void)
{
	struct timeval tv;
	unsigned int us;

	gettimeofday(&tv, NULL);
	us = (unsigned int)(1000000 * tv.tv_sec + tv.tv_usec);
	srand(us);
}

static void shuffle(const ssize_t len, ssize_t *x)
{
	ssize_t i, swap, rnd;

	for (i = 0; i < len; i++) {
		swap = x[i];
		rnd = rand() % (len - i);
		x[i] = x[i + rnd];
		x[i + rnd] = swap;
	}
}

static ssize_t *make_unique_index(const ssize_t len)
{
	ssize_t i;
	ssize_t *index = malloc(len * sizeof(*index));

	assert(index != NULL);
	for (i = 0; i < len; i++)
		index[i] = i;
	shuffle(len, index);
	return index;
}

static ssize_t *make_index(const ssize_t len)
{
	ssize_t i;
	ssize_t *index = malloc(len * sizeof(*index));

	assert(index != NULL);
	for (i = 0; i < len; i++)
		index[i] = rand() % len;
	return index;
}

void run_tests(const ssize_t len, const ssize_t *index)
{
	ssize_t i = 0;
	excit_t it;

	while (synthetic_tests[i]) {
		it = excit_alloc(EXCIT_INDEX);
		assert(it != NULL);
		assert(excit_index_init(it, len, index) == EXCIT_SUCCESS);
		synthetic_tests[i] (it);
		excit_free(it);
		i++;
	}
}

int main(int argc, char *argv[])
{
	ssize_t n = NTESTS;

	rand_seed();

	while (n--) {
		ssize_t len = MIN_LEN + rand() % (MAX_LEN - MIN_LEN);
		ssize_t *uind = make_unique_index(len);

		run_tests(len, uind);
		free(uind);

		ssize_t *ind = make_index(len);

		run_tests(len, ind);
		free(ind);
	}

	return 0;
}
