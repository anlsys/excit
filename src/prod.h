#ifndef EXCIT_PROD_H
#define EXCIT_PROD_H

#include "excit.h"
#include "dev/excit.h"

struct prod_it_s {
	ssize_t count;
	excit_t *its;
};

extern struct excit_func_table_s excit_prod_func_table;

#endif //EXCIT_PROD_H
