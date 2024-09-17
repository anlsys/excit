/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ******************************************************************************/
#ifndef EXCIT_PROD_H
#define EXCIT_PROD_H

#include "excit.h"
#include "dev/excit.h"

struct prod_it_s {
	ssize_t count;
	ssize_t* buff;
	excit_t *its;
};

extern struct excit_func_table_s excit_prod_func_table;

#endif //EXCIT_PROD_H

