/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include "dev/excit.h"
#include "loop.h"

static int loop_it_alloc(excit_t data)
{
	struct loop_it_s *it = (struct loop_it_s *)data->data;

	it->it = NULL;
	it->n = 0;
	it->counter = 0;
	return EXCIT_SUCCESS;
}

static void loop_it_free(excit_t data)
{
	struct loop_it_s *it = (struct loop_it_s *)data->data;

	excit_free(it->it);
}

static int loop_it_copy(excit_t ddst, const_excit_t dsrc)
{
	struct loop_it_s *dst = (struct loop_it_s *)ddst->data;
	const struct loop_it_s *src = (const struct loop_it_s *)dsrc->data;
	excit_t copy = excit_dup(src->it);

	if (!copy)
		return -EXCIT_EINVAL;
	dst->it = copy;
	dst->n = src->n;
	dst->counter = src->counter;
	return EXCIT_SUCCESS;
}

static int loop_it_peek(const_excit_t data, ssize_t *indexes)
{
	const struct loop_it_s *it = (const struct loop_it_s *)data->data;

	return excit_peek(it->it, indexes);
}

static int loop_it_next(excit_t data, ssize_t *indexes)
{
	struct loop_it_s *it = (struct loop_it_s *)data->data;
	int err;

	if (it->counter == it->n - 1)
		return excit_next(it->it, indexes);
	int looped;
	err = excit_cyclic_next(it->it, indexes, &looped);
	if (looped)
		it->counter++;
	return err;
}

static int loop_it_size(const_excit_t data, ssize_t *size)
{
	const struct loop_it_s *it = (const struct loop_it_s *)data->data;
	int err = excit_size(it->it, size);

	if (err)
		return err;
	*size *= it->n;
	return EXCIT_SUCCESS;
}

static int loop_it_rewind(excit_t data)
{
	struct loop_it_s *it = (struct loop_it_s *)data->data;

	it->counter = 0;
	return excit_rewind(it->it);
}

static int loop_it_nth(const_excit_t data, ssize_t n, ssize_t *val)
{
	ssize_t size;
	const struct loop_it_s *it = (const struct loop_it_s *)data->data;
	int err = excit_size(it->it, &size);

	if (err)
		return err;
	if (n < 0 || n >= size*it->n)
		return -EXCIT_EDOM;

	return excit_nth(it->it, n % size, val);
}

static int loop_it_pos(const_excit_t data, ssize_t *n)
{
	ssize_t inner_n;
	ssize_t size;
	const struct loop_it_s *it = (const struct loop_it_s *)data->data;
	int err = excit_pos(it->it, &inner_n);

	if (err)
		return err;
	err = excit_size(it->it, &size);
	if (err)
		return err;
	if (n)
		*n = inner_n + it->counter * size;
	return EXCIT_SUCCESS;
}

struct excit_func_table_s excit_loop_func_table = {
	loop_it_alloc,
	loop_it_free,
	loop_it_copy,
	loop_it_next,
	loop_it_peek,
	loop_it_size,
	loop_it_rewind,
	NULL,
	loop_it_nth,
	NULL,
	loop_it_pos
};

int excit_loop_init(excit_t it, excit_t src, ssize_t n)
{
	if (!it || it->type != EXCIT_LOOP || !src || n <= 0)
		return -EXCIT_EINVAL;
	struct loop_it_s *loop_it = (struct loop_it_s *)it->data;
	excit_free(loop_it->it);
	it->dimension = src->dimension;
	loop_it->it = src;
	loop_it->n = n;
	loop_it->counter = 0;
	return EXCIT_SUCCESS;
}

