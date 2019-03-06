/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://xgitlab.cels.anl.gov/argo/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef EXCIT_REPEAT_H
#define EXCIT_REPEAT_H

#include "excit.h"
#include "dev/excit.h"

struct repeat_it_s {
	excit_t it;
	ssize_t n;
	ssize_t counter;
};

extern struct excit_func_table_s excit_repeat_func_table;

#endif //EXCIT_REPEAT_H

