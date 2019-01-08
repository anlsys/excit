#ifndef EXCIT_DEV_H
#define EXCIT_DEV_H

#include "../excit.h"

struct excit_s {
	struct excit_func_table_s *func_table;
	ssize_t dimension;
	enum excit_type_e type;
	void *data;
};

#endif

