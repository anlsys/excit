/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
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

