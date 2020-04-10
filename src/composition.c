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
#include "composition.h"

static int composition_it_alloc(excit_t data)
{
	struct composition_it_s *it = (struct composition_it_s *)data->data;

	it->src = NULL;
	it->indexer = NULL;
	return EXCIT_SUCCESS;
}

static void composition_it_free(excit_t data)
{
	struct composition_it_s *it = (struct composition_it_s *)data->data;

	excit_free(it->src);
	excit_free(it->indexer);
}

static int composition_it_copy(excit_t dst, const_excit_t src)
{
	const struct composition_it_s *it = (const struct composition_it_s *)src->data;
	struct composition_it_s *result = (struct composition_it_s *)dst->data;

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

static int composition_it_next(excit_t data, ssize_t *indexes)
{
	struct composition_it_s *it = (struct composition_it_s *)data->data;
	ssize_t n;
	int err = excit_next(it->indexer, &n);

	if (err)
		return err;
	return excit_nth(it->src, n, indexes);
}

static int composition_it_peek(const_excit_t data, ssize_t *indexes)
{
	const struct composition_it_s *it = (const struct composition_it_s *)data->data;
	ssize_t n;
	int err = excit_peek(it->indexer, &n);

	if (err)
		return err;
	return excit_nth(it->src, n, indexes);
}

static int composition_it_size(const_excit_t data, ssize_t *size)
{
	const struct composition_it_s *it = (const struct composition_it_s *)data->data;

	return excit_size(it->indexer, size);
}

static int composition_it_rewind(excit_t data)
{
	struct composition_it_s *it = (struct composition_it_s *)data->data;

	return excit_rewind(it->indexer);
}

static int composition_it_nth(const_excit_t data, ssize_t n, ssize_t *indexes)
{
	const struct composition_it_s *it = (const struct composition_it_s *)data->data;
	ssize_t p;
	int err = excit_nth(it->indexer, n, &p);

	if (err)
		return err;
	return excit_nth(it->src, p, indexes);
}

static int composition_it_rank(const_excit_t data, const ssize_t *indexes,
			 ssize_t *n)
{
	const struct composition_it_s *it = (const struct composition_it_s *)data->data;
	ssize_t inner_n;
	int err = excit_rank(it->src, indexes, &inner_n);

	if (err)
		return err;
	return excit_rank(it->indexer, &inner_n, n);
}

static int composition_it_pos(const_excit_t data, ssize_t *n)
{
	const struct composition_it_s *it = (const struct composition_it_s *)data->data;

	return excit_pos(it->indexer, n);
}

static int composition_it_split(const_excit_t data, ssize_t n, excit_t *results)
{
	const struct composition_it_s *it = (const struct composition_it_s *)data->data;
	int err = excit_split(it->indexer, n, results);

	if (err)
		return err;
	if (!results)
		return EXCIT_SUCCESS;
	for (int i = 0; i < n; i++) {
		excit_t tmp;
		excit_t tmp2;

		tmp = results[i];
		results[i] = excit_alloc(EXCIT_COMPOSITION);
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
		err = excit_composition_init(results[i], tmp2, tmp);
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

struct excit_func_table_s excit_composition_func_table = {
	composition_it_alloc,
	composition_it_free,
	composition_it_copy,
	composition_it_next,
	composition_it_peek,
	composition_it_size,
	composition_it_rewind,
	composition_it_split,
	composition_it_nth,
	composition_it_rank,
	composition_it_pos
};

int excit_composition_init(excit_t it, excit_t src, excit_t indexer)
{
	if (!it || it->type != EXCIT_COMPOSITION || !src || !indexer
	    || indexer->dimension != 1)
		return -EXCIT_EINVAL;
	struct composition_it_s *composition_it = (struct composition_it_s *)it->data;
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
	composition_it->src = src;
	composition_it->indexer = indexer;
	it->dimension = src->dimension;
	return EXCIT_SUCCESS;
}

