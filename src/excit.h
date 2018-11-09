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

enum excit_error_e {
	EXCIT_SUCCESS,
	EXCIT_STOPIT,
	EXCIT_ENOMEM,
	EXCIT_EINVAL,
	EXCIT_EDOM,
	EXCIT_ENOTSUP
};

typedef struct excit_s *excit_t;

excit_t excit_alloc(enum excit_type_e type);
excit_t excit_dup(const excit_t it);
void excit_free(excit_t it);

int excit_type(excit_t it, enum excit_type_e *type);
int excit_dimension(excit_t it, ssize_t *dimension);

int excit_next(excit_t it, ssize_t *indexes);
int excit_peek(const excit_t it, ssize_t *indexes);
int excit_size(const excit_t it, ssize_t *size);
int excit_rewind(excit_t it);
int excit_split(const excit_t it, ssize_t n, excit_t *results);
int excit_nth(const excit_t it, ssize_t n, ssize_t *indexes);
int excit_n(const excit_t it, const ssize_t *indexes, ssize_t *n);
int excit_pos(const excit_t it, ssize_t *n);

int excit_skip(excit_t it);
int excit_cyclic_next(excit_t it, ssize_t *indexes, int *looped);

int excit_range_init(excit_t it, ssize_t first, ssize_t last, ssize_t step);

int excit_cons_init(excit_t it, excit_t src, ssize_t n);

int excit_repeat_init(excit_t it, excit_t src, ssize_t n);

int excit_hilbert2d_init(excit_t it, ssize_t order);

int excit_product_add(excit_t it, excit_t added_it);
int excit_product_add_copy(excit_t it, excit_t added_it);
int excit_product_count(const excit_t it, ssize_t *count);
int excit_product_split_dim(const excit_t it, ssize_t dim, ssize_t n,
			    excit_t *results);

int excit_slice_init(excit_t it, excit_t src, excit_t indexer);
#endif
