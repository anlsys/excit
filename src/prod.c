#include <string.h>
#include "dev/excit.h"
#include "prod.h"

static int prod_it_alloc(excit_t data)
{
	struct prod_it_s *it = (struct prod_it_s *)data->data;

	it->count = 0;
	it->its = NULL;
	it->buff = NULL;
	return EXCIT_SUCCESS;
}

static void prod_it_free(excit_t data)
{
	struct prod_it_s *it = (struct prod_it_s *)data->data;

	if (it->its) {
		for (ssize_t i = 0; i < it->count; i++)
			excit_free(it->its[i]);
		free(it->its);
		free(it->buff);
	}
}

static int prod_it_copy(excit_t dst, const excit_t src)
{
	const struct prod_it_s *it = (const struct prod_it_s *)src->data;
	struct prod_it_s *result = (struct prod_it_s *)dst->data;

	result->its = (excit_t *) malloc(it->count * sizeof(excit_t));
	if (!result->its)
		return -EXCIT_ENOMEM;
	result->buff = (ssize_t *) malloc(src->dimension * sizeof(ssize_t));
	if (!result->buff){
		free(result->its);
		return -EXCIT_ENOMEM;
	}

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
	struct prod_it_s *it = (struct prod_it_s *)data->data;

	for (ssize_t i = 0; i < it->count; i++) {
		int err = excit_rewind(it->its[i]);

		if (err)
			return err;
	}
	return EXCIT_SUCCESS;
}

static int prod_it_size(const excit_t data, ssize_t *size)
{
	const struct prod_it_s *it = (const struct prod_it_s *)data->data;
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
	const struct prod_it_s *it = (const struct prod_it_s *)data->data;

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

static int prod_it_rank(const excit_t data, const ssize_t *indexes,
			ssize_t *n)
{
	const struct prod_it_s *it = (const struct prod_it_s *)data->data;

	if (it->count == 0)
		return -EXCIT_EINVAL;
	ssize_t offset = 0;
	ssize_t product = 0;
	ssize_t inner_n;
	ssize_t subsize;

	for (ssize_t i = 0; i < it->count; i++) {
		int err = excit_rank(it->its[i], indexes + offset, &inner_n);
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
	const struct prod_it_s *it = (const struct prod_it_s *)data->data;

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
	struct prod_it_s *it = (struct prod_it_s *)data->data;
	int err;
	int looped;
	ssize_t i;
	ssize_t *next_indexes;
	ssize_t offset = data->dimension;

	if (it->count == 0)
		return -EXCIT_EINVAL;
	looped = next;
	for (i = it->count - 1; i > 0; i--) {
		offset -= it->its[i]->dimension;
		next_indexes = it->buff + offset;
		if (looped)
			err = excit_cyclic_next(it->its[i], next_indexes,
						&looped);
		else
			err = excit_peek(it->its[i], next_indexes);
		if (err)
			return err;
	}
	offset -= it->its[i]->dimension;
	next_indexes = it->buff + offset;
	if (looped)
		err = excit_next(it->its[0], next_indexes);
	else
		err = excit_peek(it->its[0], next_indexes);
	if (err)
		return err;

	if(indexes)
		memcpy(indexes, it->buff, data->dimension * sizeof(ssize_t)); 
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

int excit_product_count(const excit_t it, ssize_t *count)
{
	if (!it || it->type != EXCIT_PRODUCT || !count)
		return -EXCIT_EINVAL;
	*count = ((struct prod_it_s *)it->data)->count;
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
	struct prod_it_s *prod_it = (struct prod_it_s *)it->data;

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
		    (struct prod_it_s *)results[i]->data;
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
	int err = EXCIT_SUCCESS;
	
	if (!it || it->type != EXCIT_PRODUCT || !it->data || !added_it)
		return -EXCIT_EINVAL;

	struct prod_it_s *prod_it = (struct prod_it_s *)it->data;
	ssize_t mew_count = prod_it->count + 1;

	excit_t *new_its =
	    (excit_t *) realloc(prod_it->its, mew_count * sizeof(excit_t));	

	if (!new_its)
		return -EXCIT_ENOMEM;

	ssize_t *new_buff =
		realloc(prod_it->buff,
			(added_it->dimension + it->dimension) * sizeof(ssize_t));

	if (!new_buff){
	        err = -EXCIT_ENOMEM;
		goto exit_with_new_its;
	}
	
	prod_it->its = new_its;
	prod_it->buff = new_buff;
	prod_it->its[prod_it->count] = added_it;
	prod_it->count = mew_count;	
	it->dimension += added_it->dimension;
	return EXCIT_SUCCESS;

 exit_with_new_its:
	free(new_its);
	return err;
}

struct excit_func_table_s excit_prod_func_table = {
	prod_it_alloc,
	prod_it_free,
	prod_it_copy,
	prod_it_next,
	prod_it_peek,
	prod_it_size,
	prod_it_rewind,
	NULL,
	prod_it_nth,
	prod_it_rank,
	prod_it_pos
};
