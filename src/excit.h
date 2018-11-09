#ifndef EXCIT_H
#define EXCIT_H 1

enum excit_type_e {
	EXCIT_RANGE,
	EXCIT_CONS,
	EXCIT_REPEAT,
	EXCIT_HILBERT2D,
	EXCIT_PRODUCT,
	EXCIT_SLICE,
	EXCIT_USER,
	EXCIT_TYPE_MAX
};

const char * excit_type_name(enum excit_type_e type);

enum excit_error_e {
	EXCIT_SUCCESS,
	EXCIT_STOPIT,
	EXCIT_ENOMEM,
	EXCIT_EINVAL,
	EXCIT_EDOM,
	EXCIT_ENOTSUP,
	EXCIT_ERROR_MAX
};

const char * excit_error_name(enum excit_error_e err);

struct excit_s {
	const struct excit_func_table_s *functions;
	ssize_t dimension;
	enum excit_type_e type;
	void *data;
};

typedef struct excit_s *excit_t;

struct excit_func_table_s {
	int (*alloc)(excit_t it);
	void (*free)(excit_t it);
	int (*copy)(excit_t dst_it, const excit_t src_it);
	int (*next)(excit_t it, ssize_t *indexes);
	int (*peek)(const excit_t it, ssize_t *indexes);
	int (*size)(const excit_t it, ssize_t *size);
	int (*rewind)(excit_t it);
	int (*split)(const excit_t it, ssize_t n, excit_t *results);
	int (*nth)(const excit_t it, ssize_t n, ssize_t *indexes);
	int (*n)(const excit_t it, const ssize_t *indexes, ssize_t *n);
	int (*pos)(const excit_t it, ssize_t *n);
};

excit_t excit_alloc(enum excit_type_e type);
excit_t excit_alloc_user(const struct excit_func_table_s *functions);
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
