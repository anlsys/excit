#ifndef EXCIT_H
#define EXCIT_H 1

#include <stdlib.h>

/*
 * The different types of iterator supported. All iterators use the same
 * integer type (ssize_t) for values.
 */
enum excit_type_e {
	EXCIT_INVALID,		/*!< Tag for invalid iterators */
	EXCIT_RANGE,		/*!< Iterator over a range of values */
	EXCIT_CONS,		/*!< Sliding window iterator */
	EXCIT_REPEAT,		/*!< Ierator that stutters a certain amount of times */
	EXCIT_HILBERT2D,	/*!< Hilbert space filing curve */
	EXCIT_PRODUCT,		/*!< Iterator over the catesian product of iterators */
	EXCIT_SLICE,		/*!< Iterator using another iterator to index a third */
	EXCIT_USER,		/*!< User-defined iterator */
	EXCIT_TYPE_MAX		/*!< Guard */
};

/*
 * Returns the string representation of an iterator type.
 */
const char *excit_type_name(enum excit_type_e type);

/*
 * The different possible return codes of an excit function.
 */
enum excit_error_e {
	EXCIT_SUCCESS,		/*!< Sucess */
	EXCIT_STOPIT,		/*!< Iteration space is depleted */
	EXCIT_ENOMEM,		/*!< Out of memory */
	EXCIT_EINVAL,		/*!< Parameter has an invalid value */
	EXCIT_EDOM,		/*!< Parameter is out of possible domain */
	EXCIT_ENOTSUP,		/*!< Operation is not supported */
	EXCIT_ERROR_MAX		/*!< Guard */
};

/*
 * Returns the string representation of a return code.
 */
const char *excit_error_name(enum excit_error_e err);

/*
 * Opaque structure of an iterator
 */
typedef struct excit_s *excit_t;

/*******************************************************************************
 * Programming interface for user-defined iterators:
 ******************************************************************************/

/*
 * Sets the dimension of a user-defined iterator.
 * "it": a user-defined iterator.
 * "dimension": the new dimension of the iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_set_dimension(excit_t it, ssize_t dimension);

/*
 * Gets the inner data pointer of a user-defined iterator.
 * "it": a user-defined iterator.
 * "data": a pointer to a pointer variable where the result will be written.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_get_data(excit_t it, void **data);

/*
 * Function table used by iterators. A user-defined iterator must provide it's
 * own table, if some of the functions are defined NULL, the corresponding
 * functionalities will be considered unsupported and the broker will return
 * -EXCIT_ENOTSUP.
 */
struct excit_func_table_s {
	/*
	 * This function is called during excit_alloc, after the memory
	 * allocation, the inner data pointer will already be set.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*alloc)(excit_t it);
	/*
	 * This function is called during excit_free. After this function is
	 * called the iterator and the data will be freed.
	 */
	void (*free)(excit_t it);
	/*
	 * This funciton is called during excit_dup. It is responsible for
	 * duplicating the content of the inner data between src_it and dst_it.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*copy)(excit_t dst_it, const excit_t src_it);
	/*
	 * This function is responsible for implementing the next functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS, EXCIT_STOPIT or an error code.
	 */
	int (*next)(excit_t it, ssize_t *indexes);
	/*
	 * This function is responsible for implementing the peek functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS, EXCIT_STOPIT or an error code.
	 */
	int (*peek)(const excit_t it, ssize_t *indexes);
	/*
	 * This function is responsible for implementing the size functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*size)(const excit_t it, ssize_t *size);
	/*
	 * This function is responsible for implementing the rewind
	 * functionality of the iterator.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*rewind)(excit_t it);
	/*
	 * This function is responsible for implementing the split
	 * functionality of the iterator.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*split)(const excit_t it, ssize_t n, excit_t *results);
	/*
	 * This function is responsible for implementing the nth functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*nth)(const excit_t it, ssize_t n, ssize_t *indexes);
	/*
	 * This function is responsible for implementing the rank functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*rank)(const excit_t it, const ssize_t *indexes, ssize_t *n);
	/*
	 * This function is responsible for implementing the pos functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS, EXCIT_STOPIT or an error code.
	 */
	int (*pos)(const excit_t it, ssize_t *n);
};

/*
 * Sets the function table of an iterator.
 * "it": an iterator.
 * "func_table": a pointer to the new function table.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_set_func_table(excit_t it, struct excit_func_table_s *func_table);

/*
 * Gets a pointer to the function table of an iterator.
 * "it": an iterator.
 * "func_table": a pointer to a pointer variable where the result will be
 *               written.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_get_func_table(excit_t it, struct excit_func_table_s **func_table);

/*
 * Allocates a new iterator of the given type.
 * "type": the type of the iterator, cannot be EXCIT_USER.
 * Returns an iterator that will need to be freed unless ownership is
 * transfered or NULL if an error occured.
 */
excit_t excit_alloc(enum excit_type_e type);

/*
 * Allocates a user-defined iterator.
 * "func_table": the table of function excit will use.
 * "data_size": the size of the inner data to allocate.
 * Returns an iterator that will need to be freed unless ownership is
 * transfered or NULL if an error occured.
 */
excit_t excit_alloc_user(struct excit_func_table_s *func_table,
			 size_t data_size);

/*
 * Duplicates an iterator.
 * "it": iterator to duplicate.
 * Returns an iterator that will need to be freed unless ownership is
 * transfered or NULL if an error occured.
 */
excit_t excit_dup(const excit_t it);

/*
 * Frees an iterator and all the iterators it aquired ownership to.
 * "it": iterator to free.
 */
void excit_free(excit_t it);

/*
 * Get the type of an iterator
 * "it": an iterator.
 * "type": a pointer where the result will be written.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_type(excit_t it, enum excit_type_e *type);

/*
 * Get the dimension of an iterator. This is the number of elements of the array
 * of ssize_t that is expected by the peek, next, nth and n functionalities.
 * "it": an iterator.
 * "dimension": a pointer where the result will be written.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_dimension(excit_t it, ssize_t *dimension);

/*
 * Gets the current element of an iterator and increments it.
 * "it": an iterator.
 * "indexes": an array of indexes with a dimension corresponding to that of the
 *            iterator, no results is returned if NULL.
 * Returns EXCIT_SUCCESS if a valid element was retured or EXCIT_STOPIT if the
 * iterator is depleted or an error code.
 */
int excit_next(excit_t it, ssize_t *indexes);

/*
 * Gets the current element of an iterator.
 * "it": an iterator.
 * "indexes": an array of indexes with a dimension corresponding to that of the
 *            iterator, no results is returned if NULL.
 * Returns EXCIT_SUCCESS if a valid element was retured or EXCIT_STOPIT if the
 * iterator is depleted or an error code.
 */
int excit_peek(const excit_t it, ssize_t *indexes);

/*
 * Rewinds an iterator to its initial state.
 * "it": an iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_rewind(excit_t it);

/*
 * Gets the number of elements of an iterator.
 * "it": an iterator.
 * "size": an pointer to the variable where the result will be stored.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_size(const excit_t it, ssize_t *size);

/*
 * Splits an iterator envenly into several suub iterators.
 * "it": an iterator.
 * "n": number of iterators desired.
 * "results": an array of at least n excit_t, or NULL in which case no iterator
 *            is created.
 * Returns EXCIT_SUCCESS, -EXCIT_EDOM if the source iterator is too small to be
 * subdivised in the wanted number or an error code.
 */
int excit_split(const excit_t it, ssize_t n, excit_t *results);

/*
 * Gets the nth element of an iterator.
 * "it": an iterator.
 * "rank": rank of the element, comprised between 0 and the size of the
 *         iterator.
 * "indexes": an array of indexes with a dimension corresponding to that of the
 *            iterator, no results is returned if NULL.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_nth(const excit_t it, ssize_t rank, ssize_t *indexes);

/*
 * Gets the rank of an element of an iterator.
 * "it": an iterator.
 * "indexes": an array of indexes corresponding to the element of the iterator.
 * "rank": a pointer to a variable where the result will be stored, no result is
 *         returned if NULL.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_rank(const excit_t it, const ssize_t *indexes, ssize_t *rank);

/*
 * Gets the position of the iterator.
 * "it": an iterator.
 * "rank": a pointer to a variable where the rank of the current element will be
 *         stored, no result is returned if NULL.
 * Returns EXCIT_SUCCESS or EXCIT_STOPIT if the iterator is depleted or an error
 * code.
 */
int excit_pos(const excit_t it, ssize_t *rank);

/*
 * Increments the iterator.
 * "it": an iterator.
 * Returns EXCIT_SUCCESS or EXCIT_STOPIT if the iterator is depleted or an error
 * code.
 */
int excit_skip(excit_t it);

/*
 * Gets the current element of an iterator, rewinding it first if the iterator
 * was depleted. The iterator is incremented.
 * "it": an iterator.
 * "indexes": an array of indexes with a dimension corresponding to that of the
 *            iterator, no results is returned if NULL.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_cyclic_next(excit_t it, ssize_t *indexes, int *looped);

/*
 * Initializes a range iterator to iterate from first to last (included) by sep.
 * "it": a range iterator.
 * "first": first value of the range.
 * "last": last value of the range.
 * "step": between elements of the range. Must be non null, can be negative.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_range_init(excit_t it, ssize_t first, ssize_t last, ssize_t step);

/*
 * Initializes a sliding window iterator over another iterator.
 * "it": a cons iterator.
 * "src": the original iterator. Ownership is transfered.
 * "n": size of the window, must not be superior to the size of the src
 *      iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_cons_init(excit_t it, excit_t src, ssize_t n);

/*
 * Initializes a repeat iterator over another iterator.
 * "it": a repeat iterator.
 * "src": the original iterator. Ownership is transfered.
 * "n": number of repeat of each element of the src iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_repeat_init(excit_t it, excit_t src, ssize_t n);

/*
 * Splits a repeat iterator between repetitions.
 * "it": a product iterator.
 * "n": number of iterators desired.
 * "results": an array of at least n excit_t, or NULL in which case no iterator
 *            is created.
 * Returns EXCIT_SUCCESS, -EXCIT_EDOM if the selected iterator is too small to
 * be subdivised in the wanted number or an error code.
 */
int excit_repeat_split(const excit_t it, ssize_t n, excit_t *results);

/*
 * Creates a two dimensional Hilbert space-filling curve iterator.
 * "it": an hilbert2d iterator.
 * "order": the iteration space is (2^order - 1)^2.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_hilbert2d_init(excit_t it, ssize_t order);

/*
 * Adds another iterator to a product iterator.
 * "it": a repeat iterator.
 * "added_it": the added iterator. Ownership is transfered.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_product_add(excit_t it, excit_t added_it);

/*
 * Adds another iterator to a product iterator without transfering ownership.
 * "it": a product iterator.
 * "added_it": the added iterator. Ownership is not transfered, a duplicate is
 *             created instead.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_product_add_copy(excit_t it, excit_t added_it);

/*
 * Gets the number of iterator inside a product iterator.
 * "it": a product iterator.
 * "count": a pointer to a variable where the result will be stored.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_product_count(const excit_t it, ssize_t *count);

/*
 * Splits a product iterator along the nth iterator.
 * "it": a product iterator.
 * "dim": the number of the iterator to split, must be comprised between 0 and
 *        the number of iterator inside the product iterator - 1.
 * "n": number of iterators desired.
 * "results": an array of at least n excit_t, or NULL in which case no iterator
 *            is created.
 * Returns EXCIT_SUCCESS, -EXCIT_EDOM if the selected iterator is too small to
 * be subdivised in the wanted number or an error code.
 */
int excit_product_split_dim(const excit_t it, ssize_t dim, ssize_t n,
			    excit_t *results);

/*
 * Initializes a slice iterator by giving asrc iterator and an indexer iterator.
 * "it": a slice iterator.
 * "src": the iterator whom elements are to be returned.
 * "indexer": the iterator that will provide the rank of the elements to return.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_slice_init(excit_t it, excit_t src, excit_t indexer);
#endif
