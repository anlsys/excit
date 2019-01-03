#include "slice.h"

struct excit_func_table_s excit_slice_func_table = {
	slice_it_alloc,
	slice_it_free,
	slice_it_copy,
	slice_it_next,
	slice_it_peek,
	slice_it_size,
	slice_it_rewind,
	slice_it_split,
	slice_it_nth,
	slice_it_rank,
	slice_it_pos
};

#define EXCIT_DATA(data, it)				\
  if(data == NULL) { return EXCIT_EINVAL; }		\
  do{							\
    int err = excit_get_data(data, (void**)(&it));	\
    if(err != EXCIT_SUCCESS) { return err; }		\
  } while(0);						\
  if(it == NULL){ return EXCIT_EINVAL; }

int slice_it_alloc(excit_t data)
{
  struct slice_it_s *it;
  EXCIT_DATA(data,it);
  it->src = NULL;
  it->indexer = NULL;
  return EXCIT_SUCCESS;
}

void slice_it_free(excit_t data)
{
  struct slice_it_s *it;
  if(data == NULL) { return ; }
  int err = excit_get_data(data, (void**)(&it));
  if(err != EXCIT_SUCCESS) { return; }
  if(it == NULL){ return; }
  excit_free(it->src);
  excit_free(it->indexer);
}

int slice_it_copy(excit_t dst, const excit_t src)
{
  struct slice_it_s *it;
  EXCIT_DATA(src,it);
  struct slice_it_s *result;
  EXCIT_DATA(dst,result);

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

int slice_it_next(excit_t data, ssize_t *indexes)
{
  struct slice_it_s *it;
  EXCIT_DATA(data,it);
  ssize_t n;
  int err = excit_next(it->indexer, &n);

  if (err)
    return err;
  return excit_nth(it->src, n, indexes);
}

int slice_it_peek(const excit_t data, ssize_t *indexes)
{
  const struct slice_it_s *it;
  EXCIT_DATA(data,it);
  ssize_t n;
  int err = excit_peek(it->indexer, &n);

  if (err)
    return err;
  return excit_nth(it->src, n, indexes);
}

int slice_it_size(const excit_t data, ssize_t *size)
{
  const struct slice_it_s *it;
  EXCIT_DATA(data,it);
  return excit_size(it->indexer, size);
}

int slice_it_rewind(excit_t data)
{
  struct slice_it_s *it;
  EXCIT_DATA(data,it);

  return excit_rewind(it->indexer);
}

int slice_it_nth(const excit_t data, ssize_t n, ssize_t *indexes)
{
  struct slice_it_s *it;
  EXCIT_DATA(data,it);
  ssize_t p;
  int err = excit_nth(it->indexer, n, &p);
  
  if (err)
    return err;
  return excit_nth(it->src, p, indexes);
}

int slice_it_rank(const excit_t data, const ssize_t *indexes, ssize_t *n)
{
  struct slice_it_s *it;
  EXCIT_DATA(data,it);
  ssize_t inner_n;
  int err = excit_rank(it->src, indexes, &inner_n);
  
  if (err)
    return err;
  return excit_rank(it->indexer, &inner_n, n);
}

int slice_it_pos(const excit_t data, ssize_t *n)
{
  struct slice_it_s *it;
  EXCIT_DATA(data,it);
  return excit_pos(it->indexer, n);
}

int slice_it_split(const excit_t data, ssize_t n, excit_t *results)
{
  struct slice_it_s *it;
  EXCIT_DATA(data,it);
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

int excit_slice_init(excit_t it, excit_t src, excit_t indexer)
{
  
  if (!it || !src || !indexer || excit_get_dimension(indexer) != 1)
    return -EXCIT_EINVAL;  
  struct slice_it_s *slice_it;
  EXCIT_DATA(it,slice_it);
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
  excit_set_dimension(it, excit_get_dimension(src));
  return EXCIT_SUCCESS;
}

