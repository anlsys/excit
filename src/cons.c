#include "dev/excit.h"
#include "cons.h"

static void circular_fifo_add(struct circular_fifo_s *fifo, ssize_t elem)
{
	if (fifo->size == fifo->length) {
		fifo->start = (fifo->start + 1) % fifo->length;
		fifo->end = (fifo->end + 1) % fifo->length;
	} else {
		fifo->end = (fifo->end + 1) % fifo->length;
		fifo->size++;
	}
	fifo->buffer[fifo->end] = elem;
}

static void circular_fifo_dump(const struct circular_fifo_s *fifo,
			       ssize_t *vals)
{
	ssize_t i;
	ssize_t j;

	for (i = 0, j = fifo->start; i < fifo->size; i++) {
		vals[i] = fifo->buffer[j];
		j = (j + 1) % fifo->length;
	}
}

static int cons_it_alloc(excit_t data)
{
	struct cons_it_s *it = (struct cons_it_s *)data->data;

	it->it = NULL;
	it->n = 0;
	it->fifo.length = 0;
	it->fifo.start = 0;
	it->fifo.end = -1;
	it->fifo.size = 0;
	it->fifo.buffer = NULL;
	return EXCIT_SUCCESS;
}

static void cons_it_free(excit_t data)
{
	struct cons_it_s *it = (struct cons_it_s *)data->data;

	excit_free(it->it);
	free(it->fifo.buffer);
}

static int cons_it_copy(excit_t ddst, const excit_t dsrc)
{
	struct cons_it_s *dst = (struct cons_it_s *)ddst->data;
	const struct cons_it_s *src = (const struct cons_it_s *)dsrc->data;
	excit_t copy = excit_dup(src->it);

	if (!copy)
		return -EXCIT_EINVAL;
	dst->it = copy;
	dst->n = src->n;
	dst->fifo.length = src->fifo.length;
	dst->fifo.start = src->fifo.start;
	dst->fifo.end = src->fifo.end;
	dst->fifo.size = src->fifo.size;
	dst->fifo.buffer =
	    (ssize_t *) malloc(src->fifo.length * sizeof(ssize_t));
	if (!dst->fifo.buffer) {
		excit_free(copy);
		return -EXCIT_ENOMEM;
	}
	for (int i = 0; i < dst->fifo.length; i++)
		dst->fifo.buffer[i] = src->fifo.buffer[i];
	return EXCIT_SUCCESS;
}

static int cons_it_size(const excit_t data, ssize_t *size)
{
	const struct cons_it_s *it = (const struct cons_it_s *)data->data;
	ssize_t tmp_size = 0;
	int err = excit_size(it->it, &tmp_size);

	if (err)
		return err;
	*size = tmp_size - (it->n - 1);
	return EXCIT_SUCCESS;
}

static int cons_it_nth(const excit_t data, ssize_t n, ssize_t *indexes)
{
	ssize_t size;
	int err = cons_it_size(data, &size);

	if (err)
		return err;
	if (n < 0 || n >= size)
		return -EXCIT_EDOM;
	const struct cons_it_s *it = (const struct cons_it_s *)data->data;
	int dim = it->it->dimension;

	if (indexes) {
		for (int i = 0; i < it->n; i++) {
			err = excit_nth(it->it, n + i, indexes + dim * i);
			if (err)
				return err;
		}
	}
	return EXCIT_SUCCESS;
}

static int cons_it_rank(const excit_t data, const ssize_t *indexes,
			ssize_t *n)
{
	const struct cons_it_s *it = (const struct cons_it_s *)data->data;
	ssize_t inner_n, inner_n_tmp;
	int err = excit_rank(it->it, indexes, &inner_n);

	if (err)
		return err;
	int dim = it->it->dimension;

	for (int i = 1; i < it->n; i++) {
		err = excit_rank(it->it, indexes + dim * i, &inner_n_tmp);
		if (err)
			return err;
		if (inner_n_tmp != inner_n + 1)
			return -EXCIT_EINVAL;
		inner_n = inner_n_tmp;
	}
	if (n)
		*n = inner_n - (it->n - 1);
	return EXCIT_SUCCESS;
}

static int cons_it_pos(const excit_t data, ssize_t *n)
{
	ssize_t inner_n;
	const struct cons_it_s *it = (const struct cons_it_s *)data->data;
	int err = excit_pos(it->it, &inner_n);

	if (err)
		return err;
	if (n)
		*n = inner_n - (it->n - 1);
	return EXCIT_SUCCESS;
}

static int cons_it_peek(const excit_t data, ssize_t *indexes)
{
	const struct cons_it_s *it = (const struct cons_it_s *)data->data;
	int err;
	int dim = it->it->dimension;
	int n = it->n;

	if (indexes) {
		circular_fifo_dump(&it->fifo, indexes);
		err = excit_peek(it->it, indexes + dim * (n - 1));
	} else
		err = excit_peek(it->it, NULL);
	if (err)
		return err;
	return EXCIT_SUCCESS;
}

static int cons_it_next(excit_t data, ssize_t *indexes)
{
	struct cons_it_s *it = (struct cons_it_s *)data->data;
	int err;
	int dim = it->it->dimension;
	int n = it->n;

	if (indexes) {
		circular_fifo_dump(&it->fifo, indexes);
		err = excit_next(it->it, indexes + dim * (n - 1));
	} else
		err = excit_next(it->it, NULL);
	if (err)
		return err;
	if (indexes)
		for (int i = dim * (n - 1); i < dim * n; i++)
			circular_fifo_add(&it->fifo, indexes[i]);
	return EXCIT_SUCCESS;
}

static int cons_it_rewind(excit_t data)
{
	struct cons_it_s *it = (struct cons_it_s *)data->data;
	int err = excit_rewind(it->it);

	if (err)
		return err;
	it->fifo.start = 0;
	it->fifo.end = -1;
	it->fifo.size = 0;

	for (int i = 0; i < it->n - 1; i++) {
		int err;

		err =
		    excit_next(it->it, it->fifo.buffer + it->it->dimension * i);
		if (err)
			return err;
		it->fifo.size += it->it->dimension;
		it->fifo.end += it->it->dimension;
	}
	return EXCIT_SUCCESS;
}

int excit_cons_init(excit_t it, excit_t src, ssize_t n)
{
	ssize_t src_size;
	int err;

	if (!it || it->type != EXCIT_CONS || !src || n <= 0)
		return -EXCIT_EINVAL;
	err = excit_size(src, &src_size);
	if (err)
		return err;
	if (src_size < n)
		return -EXCIT_EINVAL;
	struct cons_it_s *cons_it = (struct cons_it_s *)it->data;

	free(cons_it->fifo.buffer);
	excit_free(cons_it->it);
	it->dimension = n * src->dimension;
	cons_it->it = src;
	cons_it->n = n;
	cons_it->fifo.length = src->dimension * (n - 1);
	cons_it->fifo.buffer =
	    (ssize_t *) malloc(cons_it->fifo.length * sizeof(ssize_t));
	if (!cons_it->fifo.buffer)
		return -EXCIT_ENOMEM;
	err = cons_it_rewind(it);
	if (err) {
		free(cons_it->fifo.buffer);
		return err;
	}
	return EXCIT_SUCCESS;
}

struct excit_func_table_s excit_cons_func_table = {
	cons_it_alloc,
	cons_it_free,
	cons_it_copy,
	cons_it_next,
	cons_it_peek,
	cons_it_size,
	cons_it_rewind,
	NULL,
	cons_it_nth,
	cons_it_rank,
	cons_it_pos
};
