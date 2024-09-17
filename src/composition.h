/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef EXCIT_SLICE_H
#define EXCIT_SLICE_H

#include "excit.h"
#include "dev/excit.h"

struct composition_it_s {
	excit_t src;
	excit_t indexer;
};

extern struct excit_func_table_s excit_composition_func_table;

#endif //EXCIT_SLICE_H

