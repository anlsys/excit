#ifndef CONS_H
#define CONS_H

#include "excit.h"

struct circular_fifo_s {
  ssize_t length;
  ssize_t start;
  ssize_t end;
  ssize_t size;
  ssize_t *buffer;
};

struct cons_it_s {
	excit_t it;
	ssize_t n;
	struct circular_fifo_s fifo;
};

int cons_it_alloc(excit_t data);
void cons_it_free(excit_t data);
int cons_it_copy(excit_t ddst, const excit_t dsrc);
int cons_it_size(const excit_t data, ssize_t *size);
int cons_it_split(const excit_t data, ssize_t n, excit_t *results);
int cons_it_nth(const excit_t data, ssize_t n, ssize_t *indexes);
int cons_it_rank(const excit_t data, const ssize_t *indexes, ssize_t *n);
int cons_it_pos(const excit_t data, ssize_t *n);
int cons_it_peek(const excit_t data, ssize_t *indexes);
int cons_it_next(excit_t data, ssize_t *indexes);
int cons_it_rewind(excit_t data);

extern struct excit_func_table_s excit_cons_func_table;

#endif
