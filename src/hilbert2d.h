#ifndef EXCIT_HILBERT2D_H
#define EXCIT_HILBERT2D_H

#include "excit.h"

struct hilbert2d_it_s {
	ssize_t n;
	excit_t range_it;
};

extern struct excit_func_table_s excit_hilbert2d_func_table;

#endif //EXCIT_HILBERT2D_H

