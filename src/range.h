#ifndef EXCIT_RANGE_H
#define EXCIT_RANGE_H

#include "excit.h"

struct range_it_s {
	ssize_t v;
	ssize_t first;
	ssize_t last;
	ssize_t step;
};

extern struct excit_func_table_s excit_range_func_table;

#endif //EXCIT_RANGE_H
