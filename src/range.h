#ifndef RANGE_H
#define RANGE_H

#include "excit.h"

struct range_it_s {
	ssize_t v;
	ssize_t first;
	ssize_t last;
	ssize_t step;
};

int  range_it_alloc(excit_t data);
void range_it_free(excit_t data);
int  range_it_copy(excit_t ddst, const excit_t dsrc);
int  range_it_rewind(excit_t data);
int  range_it_peek(const excit_t data, ssize_t *val);
int  range_it_next(excit_t data, ssize_t *val);
int  range_it_size(const excit_t data, ssize_t *size);
int  range_it_nth(const excit_t data, ssize_t n, ssize_t *val);
int  range_it_rank(const excit_t data, const ssize_t *val, ssize_t *n);
int  range_it_pos(const excit_t data, ssize_t *n);
int  range_it_split(const excit_t data, ssize_t n, excit_t *results);

extern struct excit_func_table_s excit_range_func_table;
						    
#endif
