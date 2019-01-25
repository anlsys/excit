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
