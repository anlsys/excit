#include "dev/excit.h"
#include "slice.h"

static int slice_it_alloc(excit_t data)
{
	struct slice_it_s *it = (struct slice_it_s *)data->data;

	it->src = NULL;
	it->indexer = NULL;
	return EXCIT_SUCCESS;
}

static void slice_it_free(excit_t data)
{
	struct slice_it_s *it = (struct slice_it_s *)data->data;

	excit_free(it->src);
	excit_free(it->indexer);
}

static int slice_it_copy(excit_t dst, const excit_t src)
{
	const struct slice_it_s *it = (const struct slice_it_s *)src->data;
	struct slice_it_s *result = (struct slice_it_s *)dst->data;

	result->src = excit_dup(it->src);
	if (!result->src)
		return -EXCIT_ENOMEM;
	result->indexer = excit_dup(it->indexer);
	if (!result->indexer) {
		excit_free(it->src);
		return -EXCIT_ENOMEM;
	}
	return EXCIT_SUCCESS;
}

static int slice_it_next(excit_t data, ssize_t *indexes)
{
	struct slice_it_s *it = (struct slice_it_s *)data->data;
	ssize_t n;
	int err = excit_next(it->indexer, &n);

	if (err)
		return err;
	return excit_nth(it->src, n, indexes);
}

static int slice_it_peek(const excit_t data, ssize_t *indexes)
{
	const struct slice_it_s *it = (const struct slice_it_s *)data->data;
	ssize_t n;
	int err = excit_peek(it->indexer, &n);

	if (err)
		return err;
	return excit_nth(it->src, n, indexes);
}

static int slice_it_size(const excit_t data, ssize_t *size)
{
	const struct slice_it_s *it = (const struct slice_it_s *)data->data;

	return excit_size(it->indexer, size);
}

static int slice_it_rewind(excit_t data)
{
	struct slice_it_s *it = (struct slice_it_s *)data->data;

	return excit_rewind(it->indexer);
}

static int slice_it_nth(const excit_t data, ssize_t n, ssize_t *indexes)
{
	const struct slice_it_s *it = (const struct slice_it_s *)data->data;
	ssize_t p;
	int err = excit_nth(it->indexer, n, &p);

	if (err)
		return err;
	return excit_nth(it->src, p, indexes);
}

static int slice_it_rank(const excit_t data, const ssize_t *indexes,
			 ssize_t *n)
{
	const struct slice_it_s *it = (const struct slice_it_s *)data->data;
	ssize_t inner_n;
	int err = excit_rank(it->src, indexes, &inner_n);

	if (err)
		return err;
	return excit_rank(it->indexer, &inner_n, n);
}

static int slice_it_pos(const excit_t data, ssize_t *n)
{
	const struct slice_it_s *it = (const struct slice_it_s *)data->data;

	return excit_pos(it->indexer, n);
}

static int slice_it_split(const excit_t data, ssize_t n, excit_t *results)
{
	const struct slice_it_s *it = (const struct slice_it_s *)data->data;
	int err = excit_split(it->indexer, n, results);

	if (err)
		return err;
	if (!results)
		return EXCIT_SUCCESS;
	for (int i = 0; i < n; i++) {
		excit_t tmp;
		excit_t tmp2;

		tmp = results[i];
		results[i] = excit_alloc(EXCIT_SLICE);
		if (!results[i]) {
			excit_free(tmp);
			err = -EXCIT_ENOMEM;
			goto error;
		}
		tmp2 = excit_dup(it->src);
		if (!tmp2) {
			excit_free(tmp);
			err = -EXCIT_ENOMEM;
			goto error;
		}
		err = excit_slice_init(results[i], tmp2, tmp);
		if (err) {
			excit_free(tmp);
			excit_free(tmp2);
			goto error;
		}
	}
	return EXCIT_SUCCESS;
error:
	for (int i = 0; i < n; i++)
		excit_free(results[i]);
	return err;
}

struct excit_func_table_s excit_slice_func_table = {
	slice_it_alloc,
	slice_it_free,
	slice_it_copy,
	slice_it_next,
	slice_it_peek,
	slice_it_size,
	slice_it_rewind,
	slice_it_split,
	slice_it_nth,
	slice_it_rank,
	slice_it_pos
};

int excit_slice_init(excit_t it, excit_t src, excit_t indexer)
{
	if (!it || it->type != EXCIT_SLICE || !src || !indexer
	    || indexer->dimension != 1)
		return -EXCIT_EINVAL;
	struct slice_it_s *slice_it = (struct slice_it_s *)it->data;
	ssize_t size_src;
	ssize_t size_indexer;
	int err = excit_size(src, &size_src);

	if (err)
		return err;
	err = excit_size(indexer, &size_indexer);
	if (err)
		return err;
	if (size_indexer > size_src)
		return -EXCIT_EDOM;
	slice_it->src = src;
	slice_it->indexer = indexer;
	it->dimension = src->dimension;
	return EXCIT_SUCCESS;
}

