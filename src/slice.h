#ifndef SLICE_H
#define SLICE_H

#include "excit.h"

struct slice_it_s {
	excit_t src;
	excit_t indexer;
};

int slice_it_alloc(excit_t data);
void slice_it_free(excit_t data);
int slice_it_copy(excit_t dst, const excit_t src);
int slice_it_next(excit_t data, ssize_t *indexes);
int slice_it_peek(const excit_t data, ssize_t *indexes);
int slice_it_size(const excit_t data, ssize_t *size);
int slice_it_rewind(excit_t data);
int slice_it_nth(const excit_t data, ssize_t n, ssize_t *indexes);
int slice_it_rank(const excit_t data, const ssize_t *indexes, ssize_t *n);
int slice_it_pos(const excit_t data, ssize_t *n);
int slice_it_split(const excit_t data, ssize_t n, excit_t *results);
int excit_slice_init(excit_t it, excit_t src, excit_t indexer);

extern struct excit_func_table_s excit_slice_func_table;

#endif
