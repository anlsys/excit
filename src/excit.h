#ifndef EXCIT_H
#define EXCIT_H 1

enum excit_type_e {
	EXCIT_RANGE,
	EXCIT_CONS,
	EXCIT_REPEAT,
	EXCIT_HILBERT2D,
	EXCIT_PRODUCT,
	EXCIT_SLICE
};

typedef struct excit_s *excit_t;

typedef ssize_t excit_index_t;

excit_t excit_alloc(enum excit_type_e type);
void excit_free(excit_t iterator);
excit_t excit_dup(const excit_t iterator);

int excit_type(excit_t iterator, enum excit_type_e *type);
int excit_dimension(excit_t iterator, excit_index_t *dimension);

int excit_next(excit_t iterator, excit_index_t *indexes);
int excit_peek(const excit_t iterator, excit_index_t *indexes);
int excit_size(const excit_t iterator, excit_index_t *size);
int excit_rewind(excit_t iterator);
int excit_split(const excit_t iterator, excit_index_t n,
		    excit_t *results);
int excit_nth(const excit_t iterator, excit_index_t n,
		  excit_index_t *indexes);
int excit_n(const excit_t iterator, const excit_index_t *indexes,
		excit_index_t *n);
int excit_pos(const excit_t iterator, excit_index_t *n);

int excit_skip(excit_t iterator);
int excit_cyclic_next(excit_t iterator, excit_index_t *indexes,
			  int *looped);

int excit_range_init(excit_t iterator, excit_index_t first,
			 excit_index_t last, excit_index_t step);

int excit_cons_init(excit_t iterator, excit_t src,
			excit_index_t n);

int excit_repeat_init(excit_t iterator, excit_t src,
			  excit_index_t n);

int excit_hilbert2d_init(excit_t iterator, excit_index_t order);

int excit_product_add(excit_t iterator, excit_t added_iterator);
int excit_product_add_copy(excit_t iterator,
			       excit_t added_iterator);
int excit_product_count(const excit_t iterator,
			    excit_index_t *count);
int excit_product_split_dim(const excit_t iterator,
				excit_index_t dim, excit_index_t n,
				excit_t *results);

int excit_slice_init(excit_t iterator, excit_t src,
			 excit_t indexer);
#endif
