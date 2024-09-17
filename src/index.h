/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef EXCIT_INDEX_H
#define EXCIT_INDEX_H

struct index_s {
	ssize_t value;
	ssize_t sorted_value;
	ssize_t sorted_index;
};

struct index_it_s {
	ssize_t pos;
	ssize_t len;
	struct index_s *index;
	int inversible;
};

extern struct excit_func_table_s excit_index_func_table;

#endif //EXCIT_INDEX_H
