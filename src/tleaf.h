#ifndef TLEAF_H
#define TLEAF_H

#include "excit.h"

struct tleaf_it_s {
	ssize_t depth;
	ssize_t *arities;
	ssize_t *order;
	ssize_t *order_inverse;
	excit_t levels;
	ssize_t *buf;
};

extern struct excit_func_table_s excit_tleaf_func_table;

#endif //TLEAF_H
