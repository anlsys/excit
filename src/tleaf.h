/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/
#ifndef TLEAF_H
#define TLEAF_H

#include "excit.h"

struct tleaf_it_s {
	ssize_t depth;
	ssize_t *arities;
	ssize_t *buf;
	ssize_t *order;
	excit_t levels;
	ssize_t *order_inverse;
	excit_t levels_inverse;	
};

extern struct excit_func_table_s excit_tleaf_func_table;

#endif //TLEAF_H
