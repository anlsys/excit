#ifndef REPEAT_H
#define REPEAT_H

#include "excit.h"

struct repeat_it_s {
	excit_t it;
	ssize_t n;
	ssize_t counter;
};

int repeat_it_alloc(excit_t data);
void repeat_it_free(excit_t data);
int repeat_it_copy(excit_t ddst, const excit_t dsrc);
int repeat_it_peek(const excit_t data, ssize_t *indexes);
int repeat_it_next(excit_t data, ssize_t *indexes);
int repeat_it_size(const excit_t data, ssize_t *size);
int repeat_it_rewind(excit_t data);
int repeat_it_nth(const excit_t data, ssize_t n, ssize_t *val);
int repeat_it_pos(const excit_t data, ssize_t *n);
int repeat_it_split(const excit_t data, ssize_t n, excit_t *results);
int excit_repeat_init(excit_t it, excit_t src, ssize_t n);

struct excit_func_table_s excit_repeat_func_table;

#endif
