/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dev/excit.h"
#include "index.h"

static int comp_index_val(const void *a_ptr, const void *b_ptr)
{
	const struct index_s *a = a_ptr;
	const struct index_s *b = b_ptr;

	if (a->sorted_value < b->sorted_value)
		return -1;
	if (a->sorted_value > b->sorted_value)
		return 1;
	return 0;
}

static struct index_s *make_index(const ssize_t len, const ssize_t *values)
{
	ssize_t i;
	struct index_s *index = malloc((len) * sizeof(*index));

	if (index == NULL)
		return NULL;
	for (i = 0; i < len; i++) {
		index[i].sorted_value = values[i];
		index[i].sorted_index = i;
	}
	qsort(index, len, sizeof(*index), comp_index_val);
	for (i = 0; i < len; i++)
		index[i].value = values[i];

	return index;
}

static inline struct index_s *copy_index(const ssize_t len,
					 const struct index_s *x)
{
	struct index_s *index = malloc((len) * sizeof(*index));

	if (index == NULL)
		return NULL;
	memcpy(index, x, (len) * sizeof(*x));
	return index;
}

static inline struct index_s *search_index(const ssize_t val, const ssize_t len,
					   const struct index_s *x)
{
	struct index_s key = { 0, val, 0 };

	return bsearch(&key, x, len, sizeof(*x), comp_index_val);
}

static inline ssize_t search_index_pos(const ssize_t val, const ssize_t len,
				       const struct index_s *x)
{
	struct index_s *found = search_index(val, len, x);

	if (found == NULL)
		return -1;
	return found->sorted_index;
}

/******************************************************************************/

static int index_it_alloc(excit_t it)
{
	it->dimension = 1;
	struct index_it_s *data_it = it->data;

	data_it->pos = 0;
	data_it->len = 0;
	data_it->inversible = 0;
	data_it->index = NULL;
	return EXCIT_SUCCESS;
}

static void index_it_free(excit_t it)
{
	struct index_it_s *data_it = it->data;

	if (data_it->index != NULL)
		free(data_it->index);
}

static int index_it_size(const_excit_t it, ssize_t *size)
{
	struct index_it_s *data_it = it->data;

	*size = data_it->len;
	return EXCIT_SUCCESS;
}

static int index_it_rewind(excit_t it)
{
	struct index_it_s *data_it = it->data;

	data_it->pos = 0;
	return EXCIT_SUCCESS;
}

static int index_it_copy(excit_t dst_it, const_excit_t src_it)
{
	int err = EXCIT_SUCCESS;
	struct index_it_s *dst = dst_it->data;
	struct index_it_s *src = src_it->data;

	dst->pos = src->pos;
	dst->len = src->len;

	if (src->len == 0)
		return EXCIT_SUCCESS;

	if (src->index != NULL) {
		dst->index = copy_index(src->len, src->index);
		if (dst->index == NULL) {
			err = -EXCIT_ENOMEM;
			goto exit_with_values;
		}
	}
	return EXCIT_SUCCESS;

exit_with_values:
	free(dst->index);
	dst->index = NULL;
	return err;
}

static int index_it_pos(const_excit_t it, ssize_t *value)
{
	if (value == NULL)
		return EXCIT_SUCCESS;

	struct index_it_s *data_it = it->data;
	*value = data_it->pos;
	if (data_it->pos >= data_it->len)
		return EXCIT_STOPIT;
	return EXCIT_SUCCESS;
}

static int index_it_nth(const_excit_t it, ssize_t n, ssize_t *indexes)
{
	struct index_it_s *data_it = it->data;

	if (n < 0 || n >= data_it->len)
		return -EXCIT_EDOM;

	if (indexes)
		*indexes = data_it->index[n].value;
	return EXCIT_SUCCESS;
}

static int index_it_peek(const_excit_t it, ssize_t *value)
{
	struct index_it_s *data_it = it->data;

	if (data_it->pos >= data_it->len)
		return EXCIT_STOPIT;
	if (value)
		*value = data_it->index[data_it->pos].value;

	return EXCIT_SUCCESS;
}

static int index_it_next(excit_t it, ssize_t *indexes)
{
	struct index_it_s *data_it = it->data;

	if (data_it->pos >= data_it->len)
		return EXCIT_STOPIT;
	if (indexes)
		*indexes = data_it->index[data_it->pos].value;
	data_it->pos++;
	return EXCIT_SUCCESS;
}

static int index_it_rank(const_excit_t it, const ssize_t *indexes, ssize_t *n)
{
	struct index_it_s *data_it = it->data;

	if (!data_it->inversible)
		return -EXCIT_ENOTSUP;

	if (indexes == NULL)
		return EXCIT_SUCCESS;

	if (*indexes < data_it->index[0].sorted_value
	    || *indexes >= data_it->index[data_it->len - 1].sorted_value)
		return -EXCIT_EDOM;

	if (n != NULL)
		*n = search_index_pos(*indexes, data_it->len, data_it->index);
	return EXCIT_SUCCESS;
}

int excit_index_init(excit_t it, const ssize_t len, const ssize_t *index)
{
	ssize_t i;

	if (it == NULL || it->data == NULL)
		return -EXCIT_EINVAL;

	struct index_it_s *data_it = it->data;

	data_it->len = len;

	data_it->index = make_index(len, index);
	if (data_it->index == NULL)
		return -EXCIT_ENOMEM;

	// Check for duplicates
	data_it->inversible = 1;
	for (i = 1; i < len; i++)
		if (data_it->index[i].sorted_value ==
		    data_it->index[i - 1].sorted_value) {
			data_it->inversible = 0;
			break;
		}
	return EXCIT_SUCCESS;
}

struct excit_func_table_s excit_index_func_table = {
	index_it_alloc,
	index_it_free,
	index_it_copy,
	index_it_next,
	index_it_peek,
	index_it_size,
	index_it_rewind,
	NULL,
	index_it_nth,
	index_it_rank,
	index_it_pos
};
