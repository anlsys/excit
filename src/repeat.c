/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include "dev/excit.h"
#include "repeat.h"

static int repeat_it_alloc(excit_t data)
{
	struct repeat_it_s *it = (struct repeat_it_s *)data->data;

	it->it = NULL;
	it->n = 0;
	it->counter = 0;
	return EXCIT_SUCCESS;
}

static void repeat_it_free(excit_t data)
{
	struct repeat_it_s *it = (struct repeat_it_s *)data->data;

	excit_free(it->it);
}

static int repeat_it_copy(excit_t ddst, const excit_t dsrc)
{
	struct repeat_it_s *dst = (struct repeat_it_s *)ddst->data;
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
	const struct repeat_it_s *it = (const struct repeat_it_s *)data->data;

	return excit_peek(it->it, indexes);
}

static int repeat_it_next(excit_t data, ssize_t *indexes)
{
	struct repeat_it_s *it = (struct repeat_it_s *)data->data;

	it->counter++;
	if (it->counter < it->n)
		return excit_peek(it->it, indexes);
	it->counter = 0;
	return excit_next(it->it, indexes);
}

static int repeat_it_size(const excit_t data, ssize_t *size)
{
	const struct repeat_it_s *it = (const struct repeat_it_s *)data->data;
	int err = excit_size(it->it, size);

	if (err)
		return err;
	*size *= it->n;
	return EXCIT_SUCCESS;
}

static int repeat_it_rewind(excit_t data)
{
	struct repeat_it_s *it = (struct repeat_it_s *)data->data;

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
	const struct repeat_it_s *it = (const struct repeat_it_s *)data->data;

	return excit_nth(it->it, n / it->n, val);
}

static int repeat_it_pos(const excit_t data, ssize_t *n)
{
	ssize_t inner_n;
	const struct repeat_it_s *it = (const struct repeat_it_s *)data->data;
	int err = excit_pos(it->it, &inner_n);

	if (err)
		return err;
	if (n)
		*n = inner_n * it->n + it->counter;
	return EXCIT_SUCCESS;
}

struct excit_func_table_s excit_repeat_func_table = {
	repeat_it_alloc,
	repeat_it_free,
	repeat_it_copy,
	repeat_it_next,
	repeat_it_peek,
	repeat_it_size,
	repeat_it_rewind,
	NULL,
	repeat_it_nth,
	NULL,
	repeat_it_pos
};

int excit_repeat_init(excit_t it, excit_t src, ssize_t n)
{
	if (!it || it->type != EXCIT_REPEAT || !src || n <= 0)
		return -EXCIT_EINVAL;
	struct repeat_it_s *repeat_it = (struct repeat_it_s *)it->data;
	excit_free(repeat_it->it);
	it->dimension = src->dimension;
	repeat_it->it = src;
	repeat_it->n = n;
	repeat_it->counter = 0;
	return EXCIT_SUCCESS;
}

int excit_repeat_split(const excit_t data, ssize_t n, excit_t *results)
{
	const struct repeat_it_s *it = (const struct repeat_it_s *)data->data;
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

