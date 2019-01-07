#include <stdlib.h>
#include "excit.h"
#include "dev/excit.h"
#include "slice.h"
#include "prod.h"
#include "cons.h"
#include "repeat.h"
#include "hilbert2d.h"
#include "range.h"

#define CASE(val) case val: return #val; break

const char *excit_type_name(enum excit_type_e type)
{
	switch (type) {
		CASE(EXCIT_RANGE);
		CASE(EXCIT_CONS);
		CASE(EXCIT_REPEAT);
		CASE(EXCIT_HILBERT2D);
		CASE(EXCIT_PRODUCT);
		CASE(EXCIT_SLICE);
		CASE(EXCIT_USER);
		CASE(EXCIT_TYPE_MAX);
	default:
		return NULL;
	}
}

const char *excit_error_name(enum excit_error_e err)
{
	switch (err) {
		CASE(EXCIT_SUCCESS);
		CASE(EXCIT_STOPIT);
		CASE(EXCIT_ENOMEM);
		CASE(EXCIT_EINVAL);
		CASE(EXCIT_EDOM);
		CASE(EXCIT_ENOTSUP);
		CASE(EXCIT_ERROR_MAX);
	default:
		return NULL;
	}
}

#undef CASE

int excit_set_dimension(excit_t it, ssize_t dimension)
{
	if (!it)
		return -EXCIT_EINVAL;
	if (it->type != EXCIT_USER)
		return -EXCIT_ENOTSUP;
	it->dimension = dimension;
	return EXCIT_SUCCESS;
}

int excit_get_data(excit_t it, void **data)
{
	if (!it)
		return -EXCIT_EINVAL;
	if (it->type != EXCIT_USER)
		return -EXCIT_ENOTSUP;
	*data = it->data;
	return EXCIT_SUCCESS;
}

int excit_set_func_table(excit_t it, struct excit_func_table_s *func_table)
{
	if (!it)
		return -EXCIT_EINVAL;
	it->func_table = func_table;
	return EXCIT_SUCCESS;
}

int excit_get_func_table(excit_t it, struct excit_func_table_s **func_table)
{
	if (!it)
		return -EXCIT_EINVAL;
	*func_table = it->func_table;
	return EXCIT_SUCCESS;
}

/*--------------------------------------------------------------------*/

#define ALLOC_EXCIT(op) { \
	it = malloc(sizeof(struct excit_s) + sizeof(struct op## _it_s)); \
	if (!it) \
		return NULL; \
	it->data = (void *)((char *)it + sizeof(struct excit_s)); \
	if (!excit_ ##op## _func_table.alloc) \
		goto error; \
	it->func_table = &excit_ ##op## _func_table; \
	it->dimension = 0; \
	if (excit_ ##op## _func_table.alloc(it)) \
		goto error; \
}

excit_t excit_alloc(enum excit_type_e type)
{
	excit_t it = NULL;

	switch (type) {
	case EXCIT_RANGE:
		ALLOC_EXCIT(range);
		break;
	case EXCIT_CONS:
		ALLOC_EXCIT(cons);
		break;
	case EXCIT_REPEAT:
		ALLOC_EXCIT(repeat);
		break;
	case EXCIT_HILBERT2D:
		ALLOC_EXCIT(hilbert2d);
		break;
	case EXCIT_PRODUCT:
		ALLOC_EXCIT(prod);
		break;
	case EXCIT_SLICE:
		ALLOC_EXCIT(slice);
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

excit_t excit_alloc_user(struct excit_func_table_s *func_table,
			 size_t data_size)
{
	excit_t it;

	if (!func_table || !data_size)
		return NULL;
	it = malloc(sizeof(struct excit_s) + data_size);
	if (!it)
		return NULL;
	it->data = (void *)((char *)it + sizeof(struct excit_s));
	if (!func_table->alloc)
		goto error;
	it->func_table = func_table;
	it->dimension = 0;
	it->type = EXCIT_USER;
	if (func_table->alloc(it))
		goto error;
	return it;
error:
	free(it);
	return NULL;
}

excit_t excit_dup(excit_t it)
{
	excit_t result = NULL;

	if (!it || !it->data || !it->func_table || !it->func_table->copy)
		return NULL;
	result = excit_alloc(it->type);
	if (!result)
		return NULL;
	result->dimension = it->dimension;
	if (it->func_table->copy(result, it))
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
	if (!it->func_table)
		goto error;
	if (it->func_table->free)
		it->func_table->free(it);
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
	if (!it || !it->func_table)
		return -EXCIT_EINVAL;
	if (!it->func_table->next)
		return -EXCIT_ENOTSUP;
	return it->func_table->next(it, indexes);
}

int excit_peek(const excit_t it, ssize_t *indexes)
{
	if (!it || !it->func_table)
		return -EXCIT_EINVAL;
	if (!it->func_table->peek)
		return -EXCIT_ENOTSUP;
	return it->func_table->peek(it, indexes);
}

int excit_size(const excit_t it, ssize_t *size)
{
	if (!it || !it->func_table || !size)
		return -EXCIT_EINVAL;
	if (!it->func_table->size)
		return -EXCIT_ENOTSUP;
	return it->func_table->size(it, size);
}

int excit_rewind(excit_t it)
{
	if (!it || !it->func_table)
		return -EXCIT_EINVAL;
	if (!it->func_table->rewind)
		return -EXCIT_ENOTSUP;
	return it->func_table->rewind(it);
}

int excit_split(const excit_t it, ssize_t n, excit_t *results)
{
	if (!it || !it->func_table)
		return -EXCIT_EINVAL;
	if (n <= 0)
		return -EXCIT_EDOM;
	if (!it->func_table->split) {
		ssize_t size;
		int err = excit_size(it, &size);

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

			tmp = excit_dup(it);
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
	} else
		return it->func_table->split(it, n, results);
}

int excit_nth(const excit_t it, ssize_t n, ssize_t *indexes)
{
	if (!it || !it->func_table)
		return -EXCIT_EINVAL;
	if (!it->func_table->nth)
		return -EXCIT_ENOTSUP;
	return it->func_table->nth(it, n, indexes);
}

int excit_rank(const excit_t it, const ssize_t *indexes, ssize_t *n)
{
	if (!it || !it->func_table || !indexes)
		return -EXCIT_EINVAL;
	if (!it->func_table->rank)
		return -EXCIT_ENOTSUP;
	return it->func_table->rank(it, indexes, n);
}

int excit_pos(const excit_t it, ssize_t *n)
{
	if (!it || !it->func_table)
		return -EXCIT_EINVAL;
	if (!it->func_table->pos)
		return -EXCIT_ENOTSUP;
	return it->func_table->pos(it, n);
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
