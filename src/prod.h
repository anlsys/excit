#ifndef PROD_H
#define PROD_H

#include "excit.h"

struct prod_it_s {
	ssize_t count;
	excit_t *its;
};

int prod_it_alloc(excit_t data);
void prod_it_free(excit_t data);
int prod_it_copy(excit_t dst, const excit_t src);
int prod_it_rewind(excit_t data);
int prod_it_size(const excit_t data, ssize_t *size);
int prod_it_nth(const excit_t data, ssize_t n, ssize_t *indexes);
int prod_it_rank(const excit_t data, const ssize_t *indexes, ssize_t *n);
int prod_it_pos(const excit_t data, ssize_t *n);
int prod_it_peek(const excit_t data, ssize_t *indexes);
int prod_it_next(excit_t data, ssize_t *indexes);
int prod_it_split(const excit_t data, ssize_t n, excit_t *results);

struct excit_func_table_s excit_prod_func_table;

#endif
