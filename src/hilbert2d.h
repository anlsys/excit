/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef EXCIT_HILBERT2D_H
#define EXCIT_HILBERT2D_H

#include "excit.h"

struct hilbert2d_it_s {
	ssize_t n;
	excit_t range_it;
};

extern struct excit_func_table_s excit_hilbert2d_func_table;

#endif //EXCIT_HILBERT2D_H

