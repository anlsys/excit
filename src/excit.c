#include <stdlib.h>
#include <excit.h>

#define CASE(val) case val: return #val; break;

const char * excit_type_name(enum excit_type_e type)
{
	switch(type) {
		CASE(EXCIT_RANGE)
		CASE(EXCIT_CONS)
		CASE(EXCIT_REPEAT)
		CASE(EXCIT_HILBERT2D)
		CASE(EXCIT_PRODUCT)
		CASE(EXCIT_SLICE)
		CASE(EXCIT_USER)
		CASE(EXCIT_TYPE_MAX)
	default:
		return NULL;
	}
}

const char * excit_error_name(enum excit_error_e err)
{
	switch(err) {
		CASE(EXCIT_SUCCESS)
		CASE(EXCIT_STOPIT)
		CASE(EXCIT_ENOMEM)
		CASE(EXCIT_EINVAL)
		CASE(EXCIT_EDOM)
		CASE(EXCIT_ENOTSUP)
		CASE(EXCIT_ERROR_MAX)
	default:
		return NULL;
	}
}

#undef CASE

/*--------------------------------------------------------------------*/

struct slice_it_s {
	excit_t src;
	excit_t indexer;
};

static int slice_it_alloc(excit_t data)
{
	data->data = malloc(sizeof(struct slice_it_s));
	if (!data->data)
		return -EXCIT_ENOMEM;
	struct slice_it_s *it = (struct slice_it_s *) data->data;

	it->src = NULL;
	it->indexer = NULL;
	return EXCIT_SUCCESS;
}

static void slice_it_free(excit_t data)
{
	struct slice_it_s *it = (struct slice_it_s *) data->data;

	excit_free(it->src);
	excit_free(it->indexer);
	free(data->data);
}

static int slice_it_copy(excit_t dst, const excit_t src)
{
	const struct slice_it_s *it = (const struct slice_it_s *) src->data;
	struct slice_it_s *result = (struct slice_it_s *) dst->data;

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
	struct slice_it_s *it = (struct slice_it_s *) data->data;
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
	struct slice_it_s *it = (struct slice_it_s *) data->data;

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

static int slice_it_n(const excit_t data, const ssize_t *indexes, ssize_t *n)
{
	const struct slice_it_s *it = (const struct slice_it_s *)data->data;
	ssize_t inner_n;
	int err = excit_n(it->src, indexes, &inner_n);

	if (err)
		return err;
	return excit_n(it->indexer, &inner_n, n);
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
		err = excit_slice_init(results[i], tmp, tmp2);
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

static const struct excit_func_table_s excit_slice_func_table = {
	slice_it_alloc,
	slice_it_free,
	slice_it_copy,
	slice_it_next,
	slice_it_peek,
	slice_it_size,
	slice_it_rewind,
	slice_it_split,
	slice_it_nth,
	slice_it_n,
	slice_it_pos
};

int excit_slice_init(excit_t it, excit_t src, excit_t indexer)
{
	if (!it || it->type != EXCIT_SLICE || !src || !indexer
		|| indexer->dimension != 1)
		return -EXCIT_EINVAL;
	struct slice_it_s *slice_it = (struct slice_it_s *) it->data;
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

/*--------------------------------------------------------------------*/

struct prod_it_s {
	ssize_t count;
	excit_t *its;
};

static int prod_it_alloc(excit_t data)
{
	data->data = malloc(sizeof(struct prod_it_s));
	if (!data->data)
		return -EXCIT_ENOMEM;
	struct prod_it_s *it = (struct prod_it_s *) data->data;

	it->count = 0;
	it->its = NULL;
	return EXCIT_SUCCESS;
}

static void prod_it_free(excit_t data)
{
	struct prod_it_s *it = (struct prod_it_s *) data->data;
	if (it->its) {
		for (ssize_t i = 0; i < it->count; i++)
			excit_free(it->its[i]);
		free(it->its);
	}
	free(data->data);
}

static int prod_it_copy(excit_t dst, const excit_t src)
{
	const struct prod_it_s *it = (const struct prod_it_s *)src->data;
	struct prod_it_s *result = (struct prod_it_s *) dst->data;

	result->its = (excit_t *) malloc(it->count * sizeof(excit_t));
	if (!result->its)
		return -EXCIT_ENOMEM;
	ssize_t i;

	for (i = 0; i < it->count; i++) {
		result->its[i] = excit_dup(it->its[i]);
		if (!result->its[i]) {
			i--;
			goto error;
		}
	}
	result->count = it->count;
	return EXCIT_SUCCESS;
error:
	while (i >= 0) {
		free(result->its[i]);
		i--;
	}
	free(result->its);
	return -EXCIT_ENOMEM;
}

static int prod_it_rewind(excit_t data)
{
	struct prod_it_s *it = (struct prod_it_s *) data->data;

	for (ssize_t i = 0; i < it->count; i++) {
		int err = excit_rewind(it->its[i]);

		if (err)
			return err;
	}
	return EXCIT_SUCCESS;
}

static int prod_it_size(const excit_t data, ssize_t *size)
{
	const struct prod_it_s *it = (const struct prod_it_s *) data->data;
	ssize_t tmp_size = 0;

	if (!size)
		return -EXCIT_EINVAL;
	if (it->count == 0)
		*size = 0;
	else {
		*size = 1;
		for (ssize_t i = 0; i < it->count; i++) {
			int err = excit_size(it->its[i], &tmp_size);

			if (err) {
				*size = 0;
				return err;
			}
			*size *= tmp_size;
		}
	}
	return EXCIT_SUCCESS;
}

static int prod_it_nth(const excit_t data, ssize_t n, ssize_t *indexes)
{
	ssize_t size;
	int err = prod_it_size(data, &size);

	if (err)
		return err;
	if (n < 0 || n >= size)
		return -EXCIT_EDOM;
	const struct prod_it_s *it = (const struct prod_it_s *) data->data;

	if (indexes) {
		ssize_t subsize = 0;
		ssize_t offset = data->dimension;

		for (ssize_t i = it->count - 1; i >= 0; i--) {
			offset -= it->its[i]->dimension;
			err = excit_size(it->its[i], &subsize);
			if (err)
				return err;
			err = excit_nth(it->its[i], n % subsize,
					indexes + offset);
			if (err)
				return err;
			n /= subsize;
		}
	}
	return EXCIT_SUCCESS;
}

static int prod_it_n(const excit_t data, const ssize_t *indexes, ssize_t *n)
{
	const struct prod_it_s *it = (const struct prod_it_s *) data->data;

	if (it->count == 0)
		return -EXCIT_EINVAL;
	ssize_t offset = 0;
	ssize_t product = 0;
	ssize_t inner_n;
	ssize_t subsize;

	for (ssize_t i = 0; i < it->count; i++) {
		int err = excit_n(it->its[i], indexes + offset, &inner_n);
		if (err)
			return err;
		err = excit_size(it->its[i], &subsize);
		if (err)
			return err;
		product *= subsize;
		product += inner_n;
		offset += it->its[i]->dimension;
	}
	if (n)
		*n = product;
	return EXCIT_SUCCESS;
}

static int prod_it_pos(const excit_t data, ssize_t *n)
{
	const struct prod_it_s *it = (const struct prod_it_s *) data->data;

	if (it->count == 0)
		return -EXCIT_EINVAL;
	ssize_t product = 0;
	ssize_t inner_n;
	ssize_t subsize;

	for (ssize_t i = 0; i < it->count; i++) {
		int err = excit_pos(it->its[i], &inner_n);

		if (err)
			return err;
		err = excit_size(it->its[i], &subsize);
		if (err)
			return err;
		product *= subsize;
		product += inner_n;
	}
	if (n)
		*n = product;
	return EXCIT_SUCCESS;
}

static inline int prod_it_peeknext_helper(excit_t data, ssize_t *indexes,
					  int next)
{
	struct prod_it_s *it = (struct prod_it_s *) data->data;
	int err;
	int looped;
	ssize_t i;
	ssize_t *next_indexes;
	ssize_t offset = data->dimension;

	if (it->count == 0)
		return -EXCIT_EINVAL;
	looped = next;
	for (i = it->count - 1; i > 0; i--) {
		if (indexes) {
			offset -= it->its[i]->dimension;
			next_indexes = indexes + offset;
		} else
			next_indexes = NULL;
		if (looped)
			err = excit_cyclic_next(it->its[i], next_indexes,
						&looped);
		else
			err = excit_peek(it->its[i], next_indexes);
		if (err)
			return err;
	}
	if (indexes) {
		offset -= it->its[i]->dimension;
		next_indexes = indexes + offset;
	} else
		next_indexes = NULL;
	if (looped)
		err = excit_next(it->its[0], next_indexes);
	else
		err = excit_peek(it->its[0], next_indexes);
	if (err)
		return err;
	return EXCIT_SUCCESS;
}

static int prod_it_peek(const excit_t data, ssize_t *indexes)
{
	return prod_it_peeknext_helper(data, indexes, 0);
}

static int prod_it_next(excit_t data, ssize_t *indexes)
{
	return prod_it_peeknext_helper(data, indexes, 1);
}

static int prod_it_split(const excit_t data, ssize_t n, excit_t *results)
{
	ssize_t size;
	int err = prod_it_size(data, &size);

	if (err)
		return err;
	if (size < n)
		return -EXCIT_EDOM;
	if (!results)
		return EXCIT_SUCCESS;
	excit_t range = excit_alloc(EXCIT_RANGE);

	if (!range)
		return -EXCIT_ENOMEM;
	err = excit_range_init(range, 0, size - 1, 1);
	if (err)
		goto error1;
	err = excit_split(range, n, results);
	if (err)
		goto error1;
	for (int i = 0; i < n; i++) {
		excit_t tmp, tmp2;

		tmp = excit_dup(data);
		if (!tmp)
			goto error2;
		tmp2 = results[i];
		results[i] = excit_alloc(EXCIT_SLICE);
		if (!results[i]) {
			excit_free(tmp2);
			goto error2;
		}
		err = excit_slice_init(results[i], tmp, tmp2);
		if (err) {
			excit_free(tmp2);
			goto error2;
		}
	}
	excit_free(range);
	return EXCIT_SUCCESS;
error2:
	for (int i = 0; i < n; i++)
		excit_free(results[i]);
error1:
	excit_free(range);
	return err;
}

int excit_product_count(const excit_t it, ssize_t *count)
{
	if (!it || it->type != EXCIT_PRODUCT || !count)
		return -EXCIT_EINVAL;
	*count = ((struct prod_it_s *) it->data)->count;
	return EXCIT_SUCCESS;
}

int excit_product_split_dim(const excit_t it, ssize_t dim, ssize_t n,
			    excit_t *results)
{
	if (!it || it->type != EXCIT_PRODUCT)
		return -EXCIT_EINVAL;
	if (n <= 0)
		return -EXCIT_EDOM;
	ssize_t count;
	int err = excit_product_count(it, &count);

	if (err)
		return err;
	if (dim >= count)
		return -EXCIT_EDOM;
	struct prod_it_s *prod_it = (struct prod_it_s *) it->data;

	err = excit_split(prod_it->its[dim], n, results);
	if (err)
		return err;
	if (!results)
		return EXCIT_SUCCESS;
	for (int i = 0; i < n; i++) {
		excit_t tmp = results[i];

		results[i] = excit_dup(it);
		if (!tmp) {
			excit_free(tmp);
			err = -EXCIT_ENOMEM;
			goto error;
		}
		struct prod_it_s *new_prod_it =
		    (struct prod_it_s *) results[i]->data;
		excit_free(new_prod_it->its[dim]);
		new_prod_it->its[dim] = tmp;
	}
	return EXCIT_SUCCESS;
error:
	for (int i = 0; i < n; i++)
		excit_free(results[i]);
	return err;
}

int excit_product_add_copy(excit_t it, excit_t added_it)
{
	int err = 0;
	excit_t copy = excit_dup(added_it);

	if (!copy)
		return -EXCIT_EINVAL;
	err = excit_product_add(it, copy);
	if (err) {
		excit_free(added_it);
		return err;
	}
	return EXCIT_SUCCESS;
}

int excit_product_add(excit_t it, excit_t added_it)
{
	if (!it || it->type != EXCIT_PRODUCT || !it->data
	    || !added_it)
		return -EXCIT_EINVAL;

	struct prod_it_s *prod_it = (struct prod_it_s *) it->data;
	ssize_t mew_count = prod_it->count + 1;

	excit_t *new_its =
	    (excit_t *) realloc(prod_it->its, mew_count * sizeof(excit_t));
	if (!new_its)
		return -EXCIT_ENOMEM;
	prod_it->its = new_its;
	prod_it->its[prod_it->count] = added_it;
	prod_it->count = mew_count;
	it->dimension += added_it->dimension;
	return EXCIT_SUCCESS;
}

static const struct excit_func_table_s excit_product_func_table = {
	prod_it_alloc,
	prod_it_free,
	prod_it_copy,
	prod_it_next,
	prod_it_peek,
	prod_it_size,
	prod_it_rewind,
	prod_it_split,
	prod_it_nth,
	prod_it_n,
	prod_it_pos
};

/*--------------------------------------------------------------------*/

struct circular_fifo_s {
	ssize_t length;
	ssize_t start;
	ssize_t end;
	ssize_t size;
	ssize_t *buffer;
};

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

struct cons_it_s {
	excit_t it;
	ssize_t n;
	struct circular_fifo_s fifo;
};

static int cons_it_alloc(excit_t data)
{
	data->data = malloc(sizeof(struct cons_it_s));
	if (!data->data)
		return -EXCIT_ENOMEM;
	struct cons_it_s *it = (struct cons_it_s *) data->data;

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
	struct cons_it_s *it = (struct cons_it_s *) data->data;

	excit_free(it->it);
	free(it->fifo.buffer);
	free(data->data);
}

static int cons_it_copy(excit_t ddst, const excit_t dsrc)
{
	struct cons_it_s *dst = (struct cons_it_s *) ddst->data;
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

static int cons_it_split(const excit_t data, ssize_t n, excit_t *results)
{
	ssize_t size;
	int err = cons_it_size(data, &size);

	if (err)
		return err;
	if (size < n)
		return -EXCIT_EDOM;
	if (!results)
		return EXCIT_SUCCESS;
	excit_t range = excit_alloc(EXCIT_RANGE);

	if (!range)
		return -EXCIT_ENOMEM;
	err = excit_range_init(range, 0, size - 1, 1);
	if (err)
		goto error1;
	err = excit_split(range, n, results);
	if (err)
		goto error1;
	int i;

	for (i = 0; i < n; i++) {
		excit_t tmp, tmp2;

		tmp = excit_dup(data);
		if (!tmp)
			goto error2;
		tmp2 = results[i];
		results[i] = excit_alloc(EXCIT_SLICE);
		if (!results[i]) {
			excit_free(tmp2);
			goto error2;
		}
		err = excit_slice_init(results[i], tmp, tmp2);
		if (err) {
			excit_free(tmp2);
			goto error2;
		}
	}
	excit_free(range);
	return EXCIT_SUCCESS;
error2:
	for (; i >= 0; i--)
		excit_free(results[i]);
error1:
	excit_free(range);
	return err;
}

static int cons_it_nth(const excit_t data, ssize_t n, ssize_t *indexes)
{
	ssize_t size;
	int err = cons_it_size(data, &size);

	if (err)
		return err;
	if (n < 0 || n >= size)
		return -EXCIT_EDOM;
	const struct cons_it_s *it = (const struct cons_it_s *) data->data;
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

static int cons_it_n(const excit_t data, const ssize_t *indexes, ssize_t *n)
{
	const struct cons_it_s *it = (const struct cons_it_s *) data->data;
	ssize_t inner_n, inner_n_tmp;
	int err = excit_n(it->it, indexes, &inner_n);

	if (err)
		return err;
	int dim = it->it->dimension;

	for (int i = 1; i < it->n; i++) {
		err = excit_n(it->it, indexes + dim * i, &inner_n_tmp);
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
	const struct cons_it_s *it = (const struct cons_it_s *) data->data;
	int err = excit_pos(it->it, &inner_n);

	if (err)
		return err;
	if (n)
		*n = inner_n - (it->n - 1);
	return EXCIT_SUCCESS;
}

static int cons_it_peek(const excit_t data, ssize_t *indexes)
{
	const struct cons_it_s *it = (const struct cons_it_s *) data->data;
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
	struct cons_it_s *it = (struct cons_it_s *) data->data;
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
	struct cons_it_s *it = (struct cons_it_s *) data->data;
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
	struct cons_it_s *cons_it = (struct cons_it_s *) it->data;

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

static const struct excit_func_table_s excit_cons_func_table = {
	cons_it_alloc,
	cons_it_free,
	cons_it_copy,
	cons_it_next,
	cons_it_peek,
	cons_it_size,
	cons_it_rewind,
	cons_it_split,
	cons_it_nth,
	cons_it_n,
	cons_it_pos
};

/*--------------------------------------------------------------------*/

struct repeat_it_s {
	excit_t it;
	ssize_t n;
	ssize_t counter;
};

static int repeat_it_alloc(excit_t data)
{
	data->data = malloc(sizeof(struct repeat_it_s));
	if (!data->data)
		return -EXCIT_ENOMEM;
	struct repeat_it_s *it = (struct repeat_it_s *) data->data;

	it->it = NULL;
	it->n = 0;
	it->counter = 0;
	return EXCIT_SUCCESS;
}

static void repeat_it_free(excit_t data)
{
	struct repeat_it_s *it = (struct repeat_it_s *) data->data;

	excit_free(it->it);
	free(data->data);
}

static int repeat_it_copy(excit_t ddst, const excit_t dsrc)
{
	struct repeat_it_s *dst = (struct repeat_it_s *) ddst->data;
	const struct repeat_it_s *src = (const struct repeat_it_s *)dsrc->data;
	excit_t copy = excit_dup(src->it);

	if (!copy)
		return -EXCIT_EINVAL;
	dst->it = copy;
	dst->n = src->n;
	dst->counter = src->counter;
	return EXCIT_SUCCESS;
}

static int repeat_it_peek(const excit_t data, ssize_t *indexes)
{
	const struct repeat_it_s *it = (const struct repeat_it_s *) data->data;

	return excit_peek(it->it, indexes);
}

static int repeat_it_next(excit_t data, ssize_t *indexes)
{
	struct repeat_it_s *it = (struct repeat_it_s *) data->data;

	it->counter++;
	if (it->counter < it->n)
		return excit_peek(it->it, indexes);
	it->counter = 0;
	return excit_next(it->it, indexes);
}

static int repeat_it_size(const excit_t data, ssize_t *size)
{
	const struct repeat_it_s *it = (const struct repeat_it_s *) data->data;
	int err = excit_size(it->it, size);

	if (err)
		return err;
	*size *= it->n;
	return EXCIT_SUCCESS;
}

static int repeat_it_rewind(excit_t data)
{
	struct repeat_it_s *it = (struct repeat_it_s *) data->data;

	it->counter = 0;
	return excit_rewind(it->it);
}

static int repeat_it_nth(const excit_t data, ssize_t n, ssize_t *val)
{
	ssize_t size;
	int err = repeat_it_size(data, &size);

	if (err)
		return err;
	if (n < 0 || n >= size)
		return -EXCIT_EDOM;
	const struct repeat_it_s *it = (const struct repeat_it_s *) data->data;

	return excit_nth(it->it, n / it->n, val);
}

static int repeat_it_pos(const excit_t data, ssize_t *n)
{
	ssize_t inner_n;
	const struct repeat_it_s *it = (const struct repeat_it_s *) data->data;
	int err = excit_pos(it->it, &inner_n);

	if (err)
		return err;
	if (n)
		*n = inner_n * it->n + it->counter;
	return EXCIT_SUCCESS;
}

static int repeat_it_split(const excit_t data, ssize_t n, excit_t *results)
{
	const struct repeat_it_s *it = (const struct repeat_it_s *) data->data;
	int err = excit_split(it->it, n, results);

	if (err)
		return err;
	if (!results)
		return EXCIT_SUCCESS;
	for (int i = 0; i < n; i++) {
		excit_t tmp;

		tmp = results[i];
		results[i] = excit_alloc(EXCIT_REPEAT);
		if (!results[i]) {
			excit_free(tmp);
			err = -EXCIT_ENOMEM;
			goto error;
		}
		err = excit_repeat_init(results[i], tmp, it->n);
		if (err)
			goto error;
	}
	return EXCIT_SUCCESS;
error:
	for (int i = 0; i < n; i++)
		excit_free(results[i]);
	return err;
}

static const struct excit_func_table_s excit_repeat_func_table = {
	repeat_it_alloc,
	repeat_it_free,
	repeat_it_copy,
	repeat_it_next,
	repeat_it_peek,
	repeat_it_size,
	repeat_it_rewind,
	repeat_it_split,
	repeat_it_nth,
	NULL,
	repeat_it_pos
};

int excit_repeat_init(excit_t it, excit_t src, ssize_t n)
{
	if (!it || it->type != EXCIT_REPEAT || !src || n <= 0)
		return -EXCIT_EINVAL;
	struct repeat_it_s *repeat_it = (struct repeat_it_s *) it->data;
	excit_free(repeat_it->it);
	it->dimension = src->dimension;
	repeat_it->it = src;
	repeat_it->n = n;
	repeat_it->counter = 0;
	return EXCIT_SUCCESS;
}

/*--------------------------------------------------------------------*/

struct hilbert2d_it_s {
	ssize_t n;
	excit_t range_it;
};

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

//convert (x,y) to d
static ssize_t xy2d(ssize_t n, ssize_t x, ssize_t y)
{
	ssize_t rx, ry, s, d = 0;

	for (s = n / 2; s > 0; s /= 2) {
		rx = (x & s) > 0;
		ry = (y & s) > 0;
		d += s * s * ((3 * rx) ^ ry);
		rot(s, &x, &y, rx, ry);
	}
	return d;
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

static int hilbert2d_it_alloc(excit_t data)
{
	data->data = malloc(sizeof(struct hilbert2d_it_s));
	if (!data->data)
		return -EXCIT_ENOMEM;
	struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;

	it->n = 0;
	it->range_it = NULL;
	return EXCIT_SUCCESS;
}

static void hilbert2d_it_free(excit_t data)
{
	struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;

	excit_free(it->range_it);
	free(data->data);
}

static int hilbert2d_it_copy(excit_t ddst, const excit_t dsrc)
{
	struct hilbert2d_it_s *dst = (struct hilbert2d_it_s *) ddst->data;
	const struct hilbert2d_it_s *src =
	    (const struct hilbert2d_it_s *)dsrc->data;
	excit_t copy = excit_dup(src->range_it);

	if (!copy)
		return -EXCIT_EINVAL;
	dst->range_it = copy;
	dst->n = src->n;
	return EXCIT_SUCCESS;
}

static int hilbert2d_it_rewind(excit_t data)
{
	struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;

	return excit_rewind(it->range_it);
}

static int hilbert2d_it_peek(const excit_t data, ssize_t *val)
{
	struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;
	ssize_t d;
	int err;

	err = excit_peek(it->range_it, &d);
	if (err)
		return err;
	if (val)
		d2xy(it->n, d, val, val + 1);
	return EXCIT_SUCCESS;
}

static int hilbert2d_it_next(excit_t data, ssize_t *val)
{
	struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;
	ssize_t d;
	int err = excit_next(it->range_it, &d);

	if (err)
		return err;
	if (val)
		d2xy(it->n, d, val, val + 1);
	return EXCIT_SUCCESS;
}

static int hilbert2d_it_size(const excit_t data, ssize_t *size)
{
	const struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;
	return excit_size(it->range_it, size);
}

static int hilbert2d_it_nth(const excit_t data, ssize_t n, ssize_t *val)
{
	ssize_t d;
	struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;
	int err = excit_nth(it->range_it, n, &d);

	if (err)
		return err;
	if (val)
		d2xy(it->n, d, val, val + 1);
	return EXCIT_SUCCESS;
}

static int hilbert2d_it_n(const excit_t data, const ssize_t *indexes,
			  ssize_t *n)
{
	struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;

	if (indexes[0] < 0 || indexes[0] >= it->n || indexes[1] < 0
	    || indexes[1] >= it->n)
		return -EXCIT_EINVAL;
	ssize_t d = xy2d(it->n, indexes[0], indexes[1]);

	return excit_n(it->range_it, &d, n);
}

static int hilbert2d_it_pos(const excit_t data, ssize_t *n)
{
	struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;

	return excit_pos(it->range_it, n);
}

static int hilbert2d_it_split(const excit_t data, ssize_t n, excit_t *results)
{
	const struct hilbert2d_it_s *it = (struct hilbert2d_it_s *) data->data;
	int err = excit_split(it->range_it, n, results);

	if (err)
		return err;
	if (!results)
		return EXCIT_SUCCESS;
	for (int i = 0; i < n; i++) {
		excit_t tmp;

		tmp = results[i];
		results[i] = excit_alloc(EXCIT_HILBERT2D);
		if (!results[i]) {
			excit_free(tmp);
			err = -EXCIT_ENOMEM;
			goto error;
		}
		results[i]->dimension = 2;
		struct hilbert2d_it_s *res_it =
		    (struct hilbert2d_it_s *) results[i]->data;
		res_it->n = it->n;
		res_it->range_it = tmp;
	}
	return EXCIT_SUCCESS;
error:
	for (int i = 0; i < n; i++)
		excit_free(results[i]);
	return err;
}

int excit_hilbert2d_init(excit_t it, ssize_t order)
{
	struct hilbert2d_it_s *hilbert2d_it;

	if (!it || it->type != EXCIT_HILBERT2D || order <= 0)
		return -EXCIT_EINVAL;
	it->dimension = 2;
	hilbert2d_it = (struct hilbert2d_it_s *) it->data;
	hilbert2d_it->range_it = excit_alloc(EXCIT_RANGE);
	if (!hilbert2d_it->range_it)
		return -EXCIT_ENOMEM;
	int n = 1 << order;
	int err = excit_range_init(hilbert2d_it->range_it, 0, n * n - 1, 1);

	if (err)
		return err;
	hilbert2d_it->n = n;
	return EXCIT_SUCCESS;
}

static const struct excit_func_table_s excit_hilbert2d_func_table = {
	hilbert2d_it_alloc,
	hilbert2d_it_free,
	hilbert2d_it_copy,
	hilbert2d_it_next,
	hilbert2d_it_peek,
	hilbert2d_it_size,
	hilbert2d_it_rewind,
	hilbert2d_it_split,
	hilbert2d_it_nth,
	hilbert2d_it_n,
	hilbert2d_it_pos
};

/*--------------------------------------------------------------------*/

struct range_it_s {
	ssize_t v;
	ssize_t first;
	ssize_t last;
	ssize_t step;
};

static int range_it_alloc(excit_t data)
{
	data->data = malloc(sizeof(struct range_it_s));
	if (!data->data)
		return -EXCIT_ENOMEM;
	struct range_it_s *it = (struct range_it_s *) data->data;

	it->v = 0;
	it->first = 0;
	it->last = -1;
	it->step = 1;
	return EXCIT_SUCCESS;
}

static void range_it_free(excit_t data)
{
	free(data->data);
}

static int range_it_copy(excit_t ddst, const excit_t dsrc)
{
	struct range_it_s *dst = (struct range_it_s *) ddst->data;
	const struct range_it_s *src = (const struct range_it_s *)dsrc->data;

	dst->v = src->v;
	dst->first = src->first;
	dst->last = src->last;
	dst->step = src->step;
	return EXCIT_SUCCESS;
}

static int range_it_rewind(excit_t data)
{
	struct range_it_s *it = (struct range_it_s *) data->data;

	it->v = it->first;
	return EXCIT_SUCCESS;
}

static int range_it_peek(const excit_t data, ssize_t *val)
{
	struct range_it_s *it = (struct range_it_s *) data->data;

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
	struct range_it_s *it = (struct range_it_s *) data->data;
	int err = range_it_peek(data, val);

	if (err)
		return err;
	it->v += it->step;
	return EXCIT_SUCCESS;
}

static int range_it_size(const excit_t data, ssize_t *size)
{
	const struct range_it_s *it = (struct range_it_s *) data->data;

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
		struct range_it_s *it = (struct range_it_s *) data->data;
		*val = it->first + n * it->step;
	}
	return EXCIT_SUCCESS;
}

static int range_it_n(const excit_t data, const ssize_t *val, ssize_t *n)
{
	ssize_t size;
	int err = range_it_size(data, &size);

	if (err)
		return err;
	const struct range_it_s *it = (struct range_it_s *) data->data;
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
	const struct range_it_s *it = (struct range_it_s *) data->data;

	if (n)
		*n = (val - it->first) / it->step;
	return EXCIT_SUCCESS;
}

static int range_it_split(const excit_t data, ssize_t n, excit_t *results)
{
	const struct range_it_s *it = (struct range_it_s *) data->data;
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
	range_it = (struct range_it_s *) it->data;
	range_it->first = first;
	range_it->v = first;
	range_it->last = last;
	range_it->step = step;
	return EXCIT_SUCCESS;
}

static const struct excit_func_table_s excit_range_func_table = {
	range_it_alloc,
	range_it_free,
	range_it_copy,
	range_it_next,
	range_it_peek,
	range_it_size,
	range_it_rewind,
	range_it_split,
	range_it_nth,
	range_it_n,
	range_it_pos
};

/*--------------------------------------------------------------------*/

#define ALLOC_EXCIT(it, func_table) {\
	if (!func_table.alloc)\
		goto error;\
	it->functions = &func_table;\
	it->dimension = 0;\
	if (func_table.alloc(it))\
		goto error;\
}

excit_t excit_alloc(enum excit_type_e type)
{
	excit_t it;

	it = malloc(sizeof(const struct excit_s));
	if (!it)
		return NULL;
	switch (type) {
	case EXCIT_RANGE:
		ALLOC_EXCIT(it, excit_range_func_table);
		break;
	case EXCIT_CONS:
		ALLOC_EXCIT(it, excit_cons_func_table);
		break;
	case EXCIT_REPEAT:
		ALLOC_EXCIT(it, excit_repeat_func_table);
		break;
	case EXCIT_HILBERT2D:
		ALLOC_EXCIT(it, excit_hilbert2d_func_table);
		break;
	case EXCIT_PRODUCT:
		ALLOC_EXCIT(it, excit_product_func_table);
		break;
	case EXCIT_SLICE:
		ALLOC_EXCIT(it, excit_slice_func_table);
		break;
	default:
		goto error;
	}
	it->type = type;
	return it;
error:
	free(it);
	return NULL;
}

excit_t excit_alloc_user(const struct excit_func_table_s *functions)
{
	excit_t it;

	it = malloc(sizeof(const struct excit_s));
	if (!it)
		return NULL;
	ALLOC_EXCIT(it, (*functions));
	it->type = EXCIT_USER;
	return it;
error:
	free(it);
	return NULL;
}

excit_t excit_dup(excit_t it)
{
	excit_t result = NULL;

	if (!it || !it->data || !it->functions
	    || !it->functions->copy)
		return NULL;
	result = excit_alloc(it->type);
	if (!result)
		return NULL;
	result->dimension = it->dimension;
	if (it->functions->copy(result, it))
		goto error;
	return result;
error:
	excit_free(result);
	return NULL;
}

void excit_free(excit_t it)
{
	if (!it)
		return;
	if (!it->functions)
		goto error;
	if (it->functions->free)
		it->functions->free(it);
error:
	free(it);
}

int excit_dimension(excit_t it, ssize_t *dimension)
{
	if (!it || !dimension)
		return -EXCIT_EINVAL;
	*dimension = it->dimension;
	return EXCIT_SUCCESS;
}

int excit_type(excit_t it, enum excit_type_e *type)
{
	if (!it || !type)
		return -EXCIT_EINVAL;
	*type = it->type;
	return EXCIT_SUCCESS;
}

int excit_next(excit_t it, ssize_t *indexes)
{
	if (!it || !it->functions)
		return -EXCIT_EINVAL;
	if (!it->functions->next)
		return -EXCIT_ENOTSUP;
	return it->functions->next(it, indexes);
}

int excit_peek(const excit_t it, ssize_t *indexes)
{
	if (!it || !it->functions)
		return -EXCIT_EINVAL;
	if (!it->functions->peek)
		return -EXCIT_ENOTSUP;
	return it->functions->peek(it, indexes);
}

int excit_size(const excit_t it, ssize_t *size)
{
	if (!it || !it->functions || !size)
		return -EXCIT_EINVAL;
	if (!it->functions->size)
		return -EXCIT_ENOTSUP;
	return it->functions->size(it, size);
}

int excit_rewind(excit_t it)
{
	if (!it || !it->functions)
		return -EXCIT_EINVAL;
	if (!it->functions->rewind)
		return -EXCIT_ENOTSUP;
	return it->functions->rewind(it);
}

int excit_split(const excit_t it, ssize_t n, excit_t *results)
{
	if (!it || !it->functions)
		return -EXCIT_EINVAL;
	if (n <= 0)
		return -EXCIT_EDOM;
	if (!it->functions->split)
		return -EXCIT_ENOTSUP;
	return it->functions->split(it, n, results);
}

int excit_nth(const excit_t it, ssize_t n, ssize_t *indexes)
{
	if (!it || !it->functions)
		return -EXCIT_EINVAL;
	if (!it->functions->nth)
		return -EXCIT_ENOTSUP;
	return it->functions->nth(it, n, indexes);
}

int excit_n(const excit_t it, const ssize_t *indexes, ssize_t *n)
{
	if (!it || !it->functions || !indexes)
		return -EXCIT_EINVAL;
	if (!it->functions->n)
		return -EXCIT_ENOTSUP;
	return it->functions->n(it, indexes, n);
}

int excit_pos(const excit_t it, ssize_t *n)
{
	if (!it || !it->functions)
		return -EXCIT_EINVAL;
	if (!it->functions->pos)
		return -EXCIT_ENOTSUP;
	return it->functions->pos(it, n);
}

int excit_cyclic_next(excit_t it, ssize_t *indexes, int *looped)
{
	int err;

	if (!it)
		return -EXCIT_EINVAL;
	*looped = 0;
	err = excit_next(it, indexes);
	switch (err) {
	case EXCIT_SUCCESS:
		break;
	case EXCIT_STOPIT:
		err = excit_rewind(it);
		if (err)
			return err;
		*looped = 1;
		err = excit_next(it, indexes);
		if (err)
			return err;
		break;
	default:
		return err;
	}
	if (excit_peek(it, NULL) == EXCIT_STOPIT) {
		err = excit_rewind(it);
		if (err)
			return err;
		*looped = 1;
	}
	return EXCIT_SUCCESS;
}

int excit_skip(excit_t it)
{
	return excit_next(it, NULL);
}
