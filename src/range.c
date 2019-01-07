#include "dev/excit.h"
#include "range.h"

static int range_it_alloc(excit_t data)
{
	struct range_it_s *it = (struct range_it_s *)data->data;

	it->v = 0;
	it->first = 0;
	it->last = -1;
	it->step = 1;
	return EXCIT_SUCCESS;
}

static void range_it_free(excit_t data)
{
}

static int range_it_copy(excit_t ddst, const excit_t dsrc)
{
	struct range_it_s *dst = (struct range_it_s *)ddst->data;
	const struct range_it_s *src = (const struct range_it_s *)dsrc->data;

	dst->v = src->v;
	dst->first = src->first;
	dst->last = src->last;
	dst->step = src->step;
	return EXCIT_SUCCESS;
}

static int range_it_rewind(excit_t data)
{
	struct range_it_s *it = (struct range_it_s *)data->data;

	it->v = it->first;
	return EXCIT_SUCCESS;
}

static int range_it_peek(const excit_t data, ssize_t *val)
{
	struct range_it_s *it = (struct range_it_s *)data->data;

	if (it->step < 0) {
		if (it->v < it->last)
			return EXCIT_STOPIT;
	} else if (it->step > 0) {
		if (it->v > it->last)
			return EXCIT_STOPIT;
	} else
		return -EXCIT_EINVAL;
	if (val)
		*val = it->v;
	return EXCIT_SUCCESS;
}

static int range_it_next(excit_t data, ssize_t *val)
{
	struct range_it_s *it = (struct range_it_s *)data->data;
	int err = range_it_peek(data, val);

	if (err)
		return err;
	it->v += it->step;
	return EXCIT_SUCCESS;
}

static int range_it_size(const excit_t data, ssize_t *size)
{
	const struct range_it_s *it = (struct range_it_s *)data->data;

	if (it->step < 0)
		if (it->first < it->last)
			*size = 0;
		else
			*size = 1 + (it->first - it->last) / (-it->step);
	else if (it->step > 0)
		if (it->first > it->last)
			*size = 0;
		else
			*size = 1 + (it->last - it->first) / (it->step);
	else
		return -EXCIT_EINVAL;
	return EXCIT_SUCCESS;
}

static int range_it_nth(const excit_t data, ssize_t n, ssize_t *val)
{
	ssize_t size;
	int err = range_it_size(data, &size);

	if (err)
		return err;
	if (n < 0 || n >= size)
		return -EXCIT_EDOM;
	if (val) {
		struct range_it_s *it = (struct range_it_s *)data->data;
		*val = it->first + n * it->step;
	}
	return EXCIT_SUCCESS;
}

static int range_it_rank(const excit_t data, const ssize_t *val, ssize_t *n)
{
	ssize_t size;
	int err = range_it_size(data, &size);

	if (err)
		return err;
	const struct range_it_s *it = (struct range_it_s *)data->data;
	ssize_t pos = (*val - it->first) / it->step;

	if (pos < 0 || pos >= size || it->first + pos * it->step != *val)
		return -EXCIT_EINVAL;
	if (n)
		*n = pos;
	return EXCIT_SUCCESS;
}

static int range_it_pos(const excit_t data, ssize_t *n)
{
	ssize_t val;
	int err = range_it_peek(data, &val);

	if (err)
		return err;
	const struct range_it_s *it = (struct range_it_s *)data->data;

	if (n)
		*n = (val - it->first) / it->step;
	return EXCIT_SUCCESS;
}

static int range_it_split(const excit_t data, ssize_t n, excit_t *results)
{
	const struct range_it_s *it = (struct range_it_s *)data->data;
	ssize_t size;
	int err = range_it_size(data, &size);

	if (err)
		return err;
	int block_size = size / n;

	if (block_size <= 0)
		return -EXCIT_EDOM;
	if (!results)
		return EXCIT_SUCCESS;
	ssize_t new_last = it->last;
	ssize_t new_first;
	int i;

	for (i = n - 1; i >= 0; i--) {
		block_size = size / (i + 1);
		results[i] = excit_alloc(EXCIT_RANGE);
		if (!results[i]) {
			err = -EXCIT_ENOMEM;
			goto error;
		}
		new_first = new_last - (block_size - 1) * it->step;
		err =
		    excit_range_init(results[i], new_first, new_last, it->step);
		if (err) {
			excit_free(results[i]);
			goto error;
		}
		new_last = new_first - it->step;
		size = size - block_size;
	}
	return EXCIT_SUCCESS;
error:
	i += 1;
	for (; i < n; i++)
		excit_free(results[i]);
	return err;
}

int excit_range_init(excit_t it, ssize_t first, ssize_t last, ssize_t step)
{
	struct range_it_s *range_it;

	if (!it || it->type != EXCIT_RANGE)
		return -EXCIT_EINVAL;
	it->dimension = 1;
	range_it = (struct range_it_s *)it->data;
	range_it->first = first;
	range_it->v = first;
	range_it->last = last;
	range_it->step = step;
	return EXCIT_SUCCESS;
}

struct excit_func_table_s excit_range_func_table = {
	range_it_alloc,
	range_it_free,
	range_it_copy,
	range_it_next,
	range_it_peek,
	range_it_size,
	range_it_rewind,
	range_it_split,
	range_it_nth,
	range_it_rank,
	range_it_pos
};
