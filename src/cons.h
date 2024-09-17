/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef EXCIT_CONS_H
#define EXCIT_CONS_H

#include "excit.h"
#include "dev/excit.h"

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

extern struct excit_func_table_s excit_cons_func_table;

#endif //EXCIT_CONS_H
