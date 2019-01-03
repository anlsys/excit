#include "hilbert2d.h"

struct excit_func_table_s excit_hilbert2d_func_table = {
  hilbert2d_it_alloc,
  hilbert2d_it_free,
  hilbert2d_it_copy,
  hilbert2d_it_next,
  hilbert2d_it_peek,
  hilbert2d_it_size,
  hilbert2d_it_rewind,
  hilbert2d_it_split,
  hilbert2d_it_nth,
  hilbert2d_it_rank,
  hilbert2d_it_pos
};

#define EXCIT_DATA(data, it)				\
  if(data == NULL) { return EXCIT_EINVAL; }		\
  do{							\
    int err = excit_get_data(data, (void**)(&it));	\
    if(err != EXCIT_SUCCESS) { return err; }		\
  } while(0);						\
  if(it == NULL){ return EXCIT_EINVAL; }


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

int hilbert2d_it_alloc(excit_t data)
{
  struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);

  it->n = 0;
  it->range_it = NULL;
  return EXCIT_SUCCESS;
}

void hilbert2d_it_free(excit_t data)
{
  struct hilbert2d_it_s *it;
  if(data == NULL) { return ; }
  int err = excit_get_data(data, (void**)(&it));
  if(err != EXCIT_SUCCESS) { return; }
  if(it == NULL){ return; }

  excit_free(it->range_it);
}

int hilbert2d_it_copy(excit_t ddst, const excit_t dsrc)
{
  struct hilbert2d_it_s *dst;
  EXCIT_DATA(ddst, dst);
  const struct hilbert2d_it_s *src;
  EXCIT_DATA(dsrc, src);
  excit_t copy = excit_dup(src->range_it);

  if (!copy)
    return -EXCIT_EINVAL;
  dst->range_it = copy;
  dst->n = src->n;
  return EXCIT_SUCCESS;
}

int hilbert2d_it_rewind(excit_t data)
{
  struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);

  return excit_rewind(it->range_it);
}

int hilbert2d_it_peek(const excit_t data, ssize_t *val)
{
  struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);
  ssize_t d;
  int err;

  err = excit_peek(it->range_it, &d);
  if (err)
    return err;
  if (val)
    d2xy(it->n, d, val, val + 1);
  return EXCIT_SUCCESS;
}

int hilbert2d_it_next(excit_t data, ssize_t *val)
{
  struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);
  ssize_t d;
  int err = excit_next(it->range_it, &d);

  if (err)
    return err;
  if (val)
    d2xy(it->n, d, val, val + 1);
  return EXCIT_SUCCESS;
}

int hilbert2d_it_size(const excit_t data, ssize_t *size)
{
  const struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);
  return excit_size(it->range_it, size);
}

int hilbert2d_it_nth(const excit_t data, ssize_t n, ssize_t *val)
{
  ssize_t d;
  struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);
  int err = excit_nth(it->range_it, n, &d);

  if (err)
    return err;
  if (val)
    d2xy(it->n, d, val, val + 1);
  return EXCIT_SUCCESS;
}

int hilbert2d_it_rank(const excit_t data, const ssize_t *indexes,
		      ssize_t *n)
{
  struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);

  if (indexes[0] < 0 || indexes[0] >= it->n || indexes[1] < 0
      || indexes[1] >= it->n)
    return -EXCIT_EINVAL;
  ssize_t d = xy2d(it->n, indexes[0], indexes[1]);

  return excit_rank(it->range_it, &d, n);
}

int hilbert2d_it_pos(const excit_t data, ssize_t *n)
{
  struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);

  return excit_pos(it->range_it, n);
}

int hilbert2d_it_split(const excit_t data, ssize_t n, excit_t *results)
{
  const struct hilbert2d_it_s *it;
  EXCIT_DATA(data, it);
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
    excit_set_dimension(results[i], 2);
    struct hilbert2d_it_s *res_it;
    EXCIT_DATA(results[i], res_it);
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

  if (!it || order <= 0)
    return -EXCIT_EINVAL;
  excit_set_dimension(it, 2);
  EXCIT_DATA(it, hilbert2d_it);
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

