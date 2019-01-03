#ifndef HILBERT2D_H
#define HILBERT2D_H

#include "excit.h"

struct hilbert2d_it_s {
  ssize_t n;
  excit_t range_it;
};

int hilbert2d_it_alloc(excit_t data);
void hilbert2d_it_free(excit_t data);
int hilbert2d_it_copy(excit_t ddst, const excit_t dsrc);
int hilbert2d_it_rewind(excit_t data);
int hilbert2d_it_peek(const excit_t data, ssize_t *val);
int hilbert2d_it_next(excit_t data, ssize_t *val);
int hilbert2d_it_size(const excit_t data, ssize_t *size);
int hilbert2d_it_nth(const excit_t data, ssize_t n, ssize_t *val);
int hilbert2d_it_rank(const excit_t data, const ssize_t *indexes, ssize_t *n);
int hilbert2d_it_pos(const excit_t data, ssize_t *n);
int hilbert2d_it_split(const excit_t data, ssize_t n, excit_t *results);
		      
extern struct excit_func_table_s excit_hilbert2d_func_table;

#endif

