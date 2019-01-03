#include "repeat.h"

struct excit_func_table_s excit_repeat_func_table = {
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

#define EXCIT_DATA(data, it) \
  if(data == NULL) { return EXCIT_EINVAL; }		\
  do{							\
    int err = excit_get_data(data, (void**)(&it));	\
    if(err != EXCIT_SUCCESS) { return err; }		\
  } while(0);						\
  if(it == NULL){ return EXCIT_EINVAL; }


int repeat_it_alloc(excit_t data)
{
  struct repeat_it_s *it;
  EXCIT_DATA(data, it);

  it->it = NULL;
  it->n = 0;
  it->counter = 0;
  return EXCIT_SUCCESS;
}

void repeat_it_free(excit_t data)
{
  struct repeat_it_s *it;
  if(data == NULL) { return ; }
  int err = excit_get_data(data, (void**)(&it));
  if(err != EXCIT_SUCCESS) { return; }
  if(it == NULL){ return; }
  excit_free(it->it);
}

int repeat_it_copy(excit_t ddst, const excit_t dsrc)
{
  struct repeat_it_s *dst;
  EXCIT_DATA(ddst, dst);
  const struct repeat_it_s *src;
  EXCIT_DATA(dsrc, src);
  excit_t copy = excit_dup(src->it);

  if (!copy)
    return -EXCIT_EINVAL;
  dst->it = copy;
  dst->n = src->n;
  dst->counter = src->counter;
  return EXCIT_SUCCESS;
}

int repeat_it_peek(const excit_t data, ssize_t *indexes)
{
  struct repeat_it_s *it;
  EXCIT_DATA(data, it);
  return excit_peek(it->it, indexes);
}

int repeat_it_next(excit_t data, ssize_t *indexes)
{
  struct repeat_it_s *it;
  EXCIT_DATA(data, it);

  it->counter++;
  if (it->counter < it->n)
    return excit_peek(it->it, indexes);
  it->counter = 0;
  return excit_next(it->it, indexes);
}

int repeat_it_size(const excit_t data, ssize_t *size)
{
  struct repeat_it_s *it;
  EXCIT_DATA(data, it);
  int err = excit_size(it->it, size);
  
  if (err)
    return err;
  *size *= it->n;
  return EXCIT_SUCCESS;
}

int repeat_it_rewind(excit_t data)
{
  struct repeat_it_s *it;
  EXCIT_DATA(data, it);

  it->counter = 0;
  return excit_rewind(it->it);
}

int repeat_it_nth(const excit_t data, ssize_t n, ssize_t *val)
{
  ssize_t size;
  int err = repeat_it_size(data, &size);

  if (err)
    return err;
  if (n < 0 || n >= size)
    return -EXCIT_EDOM;
  struct repeat_it_s *it;
  EXCIT_DATA(data, it);

  return excit_nth(it->it, n / it->n, val);
}

int repeat_it_pos(const excit_t data, ssize_t *n)
{
  ssize_t inner_n;
  struct repeat_it_s *it;
  EXCIT_DATA(data, it);
  int err = excit_pos(it->it, &inner_n);

  if (err)
    return err;
  if (n)
    *n = inner_n * it->n + it->counter;
  return EXCIT_SUCCESS;
}

int repeat_it_split(const excit_t data, ssize_t n, excit_t *results)
{
  struct repeat_it_s *it;
  EXCIT_DATA(data, it);
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

int excit_repeat_init(excit_t it, excit_t src, ssize_t n)
{
  if (!it || !src || n <= 0)
    return -EXCIT_EINVAL;
  struct repeat_it_s *repeat_it;
  EXCIT_DATA(it, repeat_it);
  excit_free(repeat_it->it);
  excit_set_dimension(it, excit_get_dimension(src));
  repeat_it->it = src;
  repeat_it->n = n;
  repeat_it->counter = 0;
  return EXCIT_SUCCESS;
}

