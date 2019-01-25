#include <stdlib.h>
#include <stdio.h>
#include "dev/excit.h"
#include "tleaf.h"

static int tleaf_init_with_it(excit_t it,
			      const ssize_t depth,
			      const ssize_t *arities,
			      const excit_t *indexes,
			      const enum tleaf_it_policy_e policy,
			      const ssize_t *user_policy,
			      excit_t levels, excit_t levels_inverse);

static int tleaf_it_alloc(excit_t it)
{
	it->dimension = 1;
	struct tleaf_it_s *data_it = it->data;

	data_it->depth = 0;
	data_it->arities = NULL;
	data_it->buf = NULL;
	data_it->order = NULL;
	data_it->levels = NULL;
	data_it->order_inverse = NULL;
	data_it->levels_inverse = NULL;
	return EXCIT_SUCCESS;
}

static void tleaf_it_free(excit_t it)
{
	struct tleaf_it_s *data_it = it->data;

	free(data_it->arities);
	free(data_it->buf);
	free(data_it->order);
	excit_free(data_it->levels);
	free(data_it->order_inverse);
	excit_free(data_it->levels_inverse);
}

static int tleaf_it_size(const excit_t it, ssize_t *size)
{
	struct tleaf_it_s *data_it = it->data;
	int err = excit_size(data_it->levels, size);

	if (err != EXCIT_SUCCESS)
		return err;
	return EXCIT_SUCCESS;
}

static int tleaf_it_rewind(excit_t it)
{
	struct tleaf_it_s *data_it = it->data;

	return excit_rewind(data_it->levels);
}

static int tleaf_it_copy(excit_t dst_it, const excit_t src_it)
{
	int err = EXCIT_SUCCESS;
	struct tleaf_it_s *dst = dst_it->data;
	struct tleaf_it_s *src = src_it->data;

	/* dst is initialised, then wipe it */
	if (dst->buf != NULL) {
		free(dst->buf);
		free(dst->arities);
		free(dst->order);
		free(dst->order_inverse);
		excit_free(dst->levels);
		excit_free(dst->levels_inverse);
	}

	/* dst is not initialized (anymore) */
	excit_t levels = excit_dup(src->levels);

	if (levels == NULL) {
		err = -EXCIT_ENOMEM;
		goto error;
	}

	excit_t levels_inverse = excit_dup(src->levels_inverse);

	if (levels_inverse == NULL) {
		err = -EXCIT_ENOMEM;
		goto error_with_levels;
	}

	err = tleaf_init_with_it(dst_it, src->depth + 1, src->arities, NULL,
				 TLEAF_POLICY_USER, src->order, levels,
				 levels_inverse);
	if (err != EXCIT_SUCCESS)
		goto error_with_levels_inverse;

	return EXCIT_SUCCESS;

error_with_levels_inverse:
	excit_free(levels_inverse);
error_with_levels:
	excit_free(levels);
error:
	return err;
}

static int tleaf_it_pos(const excit_t it, ssize_t *value)
{
	struct tleaf_it_s *data_it = it->data;

	return excit_pos(data_it->levels, value);
}

static ssize_t tleaf_it_value(struct tleaf_it_s *it)
{
	ssize_t i, acc = 1, val = 0;

	for (i = 0; i < it->depth; i++) {
		/* levels are stacked following order, then decode result backward order_inverse */
		val += acc * it->buf[it->order_inverse[it->depth - i - 1]];
		acc *= it->arities[it->depth - i - 1];
	}
	return val;
}

static int tleaf_it_nth(const excit_t it, ssize_t n, ssize_t *indexes)
{
	struct tleaf_it_s *data_it = it->data;
	int err = excit_nth(data_it->levels, n, data_it->buf);

	if (err != EXCIT_SUCCESS)
		return err;
	if (indexes != NULL)
		*indexes = tleaf_it_value(data_it);
	return EXCIT_SUCCESS;
}

static int tleaf_it_peek(const excit_t it, ssize_t *value)
{
	struct tleaf_it_s *data_it = it->data;
	int err = excit_peek(data_it->levels, data_it->buf);

	if (err != EXCIT_SUCCESS)
		return err;
	if (value != NULL)
		*value = tleaf_it_value(data_it);
	return EXCIT_SUCCESS;
}

static int tleaf_it_next(excit_t it, ssize_t *indexes)
{
	struct tleaf_it_s *data_it = it->data;
	int err = excit_next(data_it->levels, data_it->buf);

	if (err != EXCIT_SUCCESS)
		return err;

	if (indexes != NULL)
		*indexes = tleaf_it_value(data_it);
	return EXCIT_SUCCESS;
}

static int tleaf_it_rank(const excit_t it, const ssize_t *indexes, ssize_t *n)
{
	ssize_t size;
	int err;

	err = tleaf_it_size(it, &size);
	if (err != EXCIT_SUCCESS)
		return err;

	if (indexes == NULL || *indexes < 0 || *indexes >= size)
		return -EXCIT_EINVAL;

	struct tleaf_it_s *data_it = it->data;

	err = excit_nth(data_it->levels_inverse, *indexes, data_it->buf);
	if (err != EXCIT_SUCCESS)
		return err;

	ssize_t i, acc = 1, val = 0;

	for (i = data_it->depth - 1; i >= 0; i--) {
		val += acc * data_it->buf[data_it->order[i]];
		acc *= data_it->arities[i];
	}

	if (n != NULL)
		*n = val;
	return EXCIT_SUCCESS;
}

static int tleaf_it_make_levels(struct tleaf_it_s *tleaf, const excit_t *indexes,
			 ssize_t *order, excit_t *levels)
{
	ssize_t i;
	int err;
	excit_t index, range, comp;

	*levels = excit_alloc(EXCIT_PRODUCT);
	if (*levels == NULL)
		return -EXCIT_ENOMEM;

	for (i = 0; i < tleaf->depth; i++) {
		ssize_t l = order[i];

		index = indexes == NULL ? NULL : indexes[l];

		range = excit_alloc(EXCIT_RANGE);

		if (range == NULL) {
			err = -EXCIT_ENOMEM;
			goto error_with_levels;
		}
		err = excit_range_init(range, 0, tleaf->arities[l] - 1, 1);
		if (err != EXCIT_SUCCESS)
			goto error_with_range;

		if (index != NULL) {
			comp = excit_alloc(EXCIT_COMPOSITION);
			if (comp == NULL) {
				err = -EXCIT_ENOMEM;
				goto error_with_range;
			}
			index = excit_dup(index);
			if (index == NULL) {
				err = -EXCIT_ENOMEM;
				goto error_with_comp;
			}
			err = excit_composition_init(comp, range, index);
			if (err != EXCIT_SUCCESS)
				goto error_with_index;
			err = excit_product_add(*levels, comp);
			if (err != EXCIT_SUCCESS)
				goto error_with_index;
		} else {
			err = excit_product_add(*levels, range);
			if (err != EXCIT_SUCCESS)
				goto error_with_range;
		}
	}

	return EXCIT_SUCCESS;

error_with_index:
	excit_free(index);
error_with_comp:
	excit_free(comp);
error_with_range:
	excit_free(range);
error_with_levels:
	excit_free(*levels);
	*levels = NULL;
	return err;
}

static int tleaf_init_with_it(excit_t it,
			      const ssize_t depth,
			      const ssize_t *arities,
			      const excit_t *indexes,
			      const enum tleaf_it_policy_e policy,
			      const ssize_t *user_policy,
			      excit_t levels, excit_t levels_inverse)
{
	if (it == NULL || it->data == NULL)
		return -EXCIT_EINVAL;
	int err = EXCIT_SUCCESS;
	struct tleaf_it_s *data_it = it->data;
	ssize_t i;

	data_it->depth = depth - 1;

	/* Set order according to policy */
	data_it->order = malloc(sizeof(*data_it->order) * data_it->depth);
	if (data_it->order == NULL) {
		err = -EXCIT_ENOMEM;
		goto error;
	}
	switch (policy) {
	case TLEAF_POLICY_ROUND_ROBIN:
		for (i = 0; i < data_it->depth; i++)
			data_it->order[i] = i;
		break;
	case TLEAF_POLICY_SCATTER:
		for (i = 0; i < data_it->depth; i++)
			data_it->order[i] = data_it->depth - i - 1;
		break;
	case TLEAF_POLICY_USER:
		for (i = 0; i < data_it->depth; i++)
			data_it->order[i] = user_policy[i];
		break;
	default:
		err = -EXCIT_EINVAL;
		goto error_with_order;
	}

	/* Set order inverse */
	data_it->order_inverse =
	    malloc(sizeof(*data_it->order_inverse) * data_it->depth);
	if (data_it->order_inverse == NULL) {
		err = -EXCIT_ENOMEM;
		goto error_with_order;
	}
	for (i = 0; i < data_it->depth; i++)
		data_it->order_inverse[data_it->order[i]] = i;

	/* Set levels arity. */
	data_it->arities = malloc(sizeof(*data_it->arities) * data_it->depth);
	if (data_it->arities == NULL) {
		err = -EXCIT_ENOMEM;
		goto error_with_order_inverse;
	}
	for (i = 0; i < data_it->depth; i++)
		data_it->arities[i] = arities[i];

	/* Set storage buffer for output of product iterator */
	data_it->buf = malloc(sizeof(*data_it->buf) * data_it->depth);
	if (data_it->buf == NULL) {
		err = -EXCIT_ENOMEM;
		goto error_with_arity;
	}

	/* Set product iterator if not provided */
	data_it->levels = levels;
	data_it->levels_inverse = levels_inverse;
	if (levels == NULL)
		err =
		    tleaf_it_make_levels(data_it, indexes, data_it->order,
					 &(data_it->levels));
	if (err != EXCIT_SUCCESS)
		goto error_with_buf;

	if (levels_inverse == NULL)
		err =
		    tleaf_it_make_levels(data_it, indexes,
					 data_it->order_inverse,
					 &(data_it->levels_inverse));
	if (err != EXCIT_SUCCESS)
		goto error_with_levels;

	return EXCIT_SUCCESS;

error_with_levels:
	excit_free(data_it->levels);
	data_it->levels = NULL;
error_with_buf:
	free(data_it->buf);
	data_it->buf = NULL;
error_with_arity:
	free(data_it->arities);
	data_it->arities = NULL;
error_with_order_inverse:
	free(data_it->order_inverse);
	data_it->order_inverse = NULL;
error_with_order:
	free(data_it->order);
	data_it->order = NULL;
error:
	return err;
}

int excit_tleaf_init(excit_t it,
		     const ssize_t depth,
		     const ssize_t *arities,
		     const excit_t *indexes,
		     const enum tleaf_it_policy_e policy,
		     const ssize_t *user_policy)
{
	return tleaf_init_with_it(it, depth, arities, indexes, policy,
				  user_policy, NULL, NULL);
}

static int tleaf_split_levels(excit_t levels, const ssize_t depth, const ssize_t n,
		       excit_t **out)
{
	*out = malloc(sizeof(**out) * n);
	if (*out == NULL)
		return -EXCIT_ENOMEM;

	int err = excit_product_split_dim(levels, depth, n, *out);

	if (err != EXCIT_SUCCESS) {
		free(*out);
		*out = NULL;
		return err;
	}
	return EXCIT_SUCCESS;
}

int tleaf_it_split(const excit_t it, const ssize_t depth,
		   const ssize_t n, excit_t *out)
{
	ssize_t i;

	if (out == NULL)
		return EXCIT_SUCCESS;
	if (it == NULL || it->data == NULL)
		return -EXCIT_EINVAL;

	struct tleaf_it_s *data_it = it->data;

	if (data_it->arities[depth] % n != 0)
		return -EXCIT_EINVAL;
	if (out == NULL)
		return EXCIT_SUCCESS;

	int err;
	excit_t *levels, *levels_inverse;

	err =
	    tleaf_split_levels(data_it->levels, data_it->order[depth], n,
			       &levels);
	if (err != EXCIT_SUCCESS)
		return err;
	err =
	    tleaf_split_levels(data_it->levels_inverse,
			       data_it->order_inverse[depth], n,
			       &levels_inverse);
	if (err != EXCIT_SUCCESS)
		goto error_with_levels;

	for (i = 0; i < n; i++) {
		out[i] = excit_alloc(EXCIT_TLEAF);
		if (out == NULL) {
			err = -EXCIT_ENOMEM;
			goto error_with_levels_inverse;
		}

		err = tleaf_init_with_it(out[i],
					 data_it->depth + 1,
					 data_it->arities,
					 NULL,
					 TLEAF_POLICY_USER,
					 data_it->order, levels[i],
					 levels_inverse[i]);
		if (err != EXCIT_SUCCESS)
			goto error_with_levels_inverse;
	}

	free(levels);
	free(levels_inverse);
	return EXCIT_SUCCESS;

error_with_levels_inverse:
	for (i = 0; i < n; i++)
		excit_free(levels_inverse[i]);
	free(levels_inverse);
error_with_levels:
	for (i = 0; i < n; i++)
		excit_free(levels[i]);
	free(levels);
	return err;
}

struct excit_func_table_s excit_tleaf_func_table = {
	tleaf_it_alloc,
	tleaf_it_free,
	tleaf_it_copy,
	tleaf_it_next,
	tleaf_it_peek,
	tleaf_it_size,
	tleaf_it_rewind,
	NULL,
	tleaf_it_nth,
	tleaf_it_rank,
	tleaf_it_pos
};
