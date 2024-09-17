/*******************************************************************************
 * Copyright 2019 UChicago Argonne, LLC.
 * (c.f. AUTHORS, LICENSE)
 *
 * This file is part of the EXCIT project.
 * For more info, see https://github.com/anlsys/excit
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef EXCIT_H
#define EXCIT_H 1

#include <stdlib.h>
#include <sys/types.h>

/*
 * excit library provides an interface to build multi-dimensional iterators over 
 * indexes.
 * excit iterators walk an array of n elements indexed from 0 to n-1
 * and return the aforementioned elements.
 * excit iterators return elements whose type can be just an index of type ssize_t,
 * or an array of indexes (a multi-dimensional index) if the defined iterator
 * has several dimensions. ssize_t elements can fit pointers. Thus, it makes the
 * excit library an ideal toolbox for indexing and walking complex structures.
 *
 * excit implements its own interface with several iterators (see excit_type_e).
 * For instance, the excit implementation of product iterators enables the mixing of
 * iterators to create more complex ones. The library balanced tree "tleaf" iterator
 * is built on top of the product iterator.
 *
 * excit library uses the concept of "ownership".
 * An excit iterator has the ownership of its internal data, i.e., it will free
 * its own data upon call to excit_free(). This ownership
 * may be transferred to another iterator through a library call such as
 * excit_cons_init() or excit_product_add().
 * Thus, ownership must be carefully watched to avoid memory leaks or double-free.
 *
 * excit library provides a rank function to find the index given an element.
 */

enum excit_type_e {
	/*!< Tag for invalid iterators */
	EXCIT_INVALID,
	/*!<
	 * Iterator over an array of indexes.
	 * If indexes are unique, the iterator is made invertible (see excit_rank()).
	 */
	EXCIT_INDEX,
	/*!<
	 * Iterator over a range of values.
	 * See function excit_range_init() for further details on this iterator's
	 * behaviour.
	 */
	EXCIT_RANGE,
	/*!<
	 * Sliding window iterator
	 * See function excit_cons_init() for further details on this iterator's
	 * behaviour.
	 */
	EXCIT_CONS,
	/*!<
	 * Iterator that repeat values a certain number of times.
	 * Builds an iterator on top of another iterator repeating the latter's elements.
	 * See function excit_repeat_init() for further details on this iterator's
	 * behaviour.
	 */
	EXCIT_REPEAT,
	/*!< Hilbert space-filling curve */
	EXCIT_HILBERT2D,
	/*!<
	 * Iterator over the cartesian product of iterators.
	 * The resulting iterator's dimension is the sum of input iterators' dimensions.
	 */
	EXCIT_PRODUCT,
	/*!<
	 * Iterator composing two iterators,
	 * i.e., using one iterator to index another.
	 * It is possible to chain composition iterators as long as
	 * input and output sets are compatible.
	 * (Only the dimension compatibility is not enforced by the library).
	 * It is straightforward to build a composition iterator by composing two range iterators.
	 */
	EXCIT_COMPOSITION,
	/*!<
	 * Iterator on balanced tree leaves.
	 * The iterator walks an index of leaves according to a policy.
	 * tleaf iterator has a special tleaf_it_split() function for splitting the
	 * tree at a specific level.
	 * See tleaf_it_policy_e and excit_tleaf_init() for further explanation.
	 */
	EXCIT_TLEAF,
	/*!<
	 * User-defined iterator
	 * excit library allows users to define their own iterators.
	 * To do so, they need to populate the function table excit_func_table_s
	 * with the routines to manipulate the aforementioned iterator.
	 * The outcome is that users will enjoy the functionality of the library
	 * for mixing with other iterators.
	 */
	EXCIT_USER,
	/*!<
	 * Interator looping a given amount of time over another iterator.
	 * See excit_loop_init() for further explanation.
         */
	EXCIT_LOOP,
	/*!< Guard */
	EXCIT_TYPE_MAX
};

/*
 * Returns the string representation of an iterator type.
 */
const char *excit_type_name(enum excit_type_e type);

/*
 * The different possible return codes of an excit function.
 */
enum excit_error_e {
	EXCIT_SUCCESS,		/*!< Success */
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
typedef const struct excit_s *const_excit_t;

/*******************************************************************************
 * Programming interface for user-defined iterators:
 ******************************************************************************/

/*
 * Sets the dimension of a (user-defined) iterator.
 * "it": a (user-defined) iterator.
 * "dimension": the new dimension of the iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_set_dimension(excit_t it, ssize_t dimension);

/*
 * Gets the dimension of an iterator.
 * "it": the iterator.
 * Returns the dimension or -1 if the iterator is NULL.
 */
ssize_t excit_get_dimension(excit_t it);

/*
 * Gets the inner data pointer of a (user-defined) iterator.
 * "it": a (user-defined) iterator.
 * "data": an address of a pointer variable where the result will be stored.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_get_data(excit_t it, void **data);

/*
 * Function table used by iterators. A user-defined iterator must provide its
 * own table; if some of the function pointers are set to NULL, the corresponding
 * functionality will be considered unsupported and the broker will return
 * -EXCIT_ENOTSUP.
 */
struct excit_func_table_s {
	/*
	 * This function is called during excit_alloc(), after the memory
	 * allocation; the inner data pointer will already be set.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*alloc)(excit_t it);
	/*
	 * This function is called during excit_free(). After this function
	 * returns, the iterator and the data will be freed.
	 */
	void (*free)(excit_t it);
	/*
	 * This function is called during excit_dup(). It is responsible for
	 * duplicating the content of the inner data between src_it and dst_it.
	 * The internal state of the iterator must also be copied, i.e., subsequent
	 * calls to excit_next() must return the same results for both iterators.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*copy)(excit_t dst_it, const_excit_t src_it);
	/*
	 * This function is responsible for implementing the succession functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS, EXCIT_STOPIT or an error code.
	 */
	int (*next)(excit_t it, ssize_t *indexes);
	/*
	 * This function is responsible for implementing the peek functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS, EXCIT_STOPIT or an error code.
	 */
	int (*peek)(const_excit_t it, ssize_t *indexes);
	/*
	 * This function is responsible for implementing the size functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*size)(const_excit_t it, ssize_t *size);
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
	int (*split)(const_excit_t it, ssize_t n, excit_t *results);
	/*
	 * This function is responsible for implementing the nth functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*nth)(const_excit_t it, ssize_t n, ssize_t *indexes);
	/*
	 * This function is responsible for implementing the rank functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS or an error code.
	 */
	int (*rank)(const_excit_t it, const ssize_t *indexes, ssize_t *n);
	/*
	 * This function is responsible for implementing the pos functionality
	 * of the iterator.
	 * Returns EXCIT_SUCCESS, EXCIT_STOPIT or an error code.
	 */
	int (*pos)(const_excit_t it, ssize_t *n);
};

/*
 * Sets the function table of an iterator.
 * "it": an iterator.
 * "func_table": a pointer to the new function table.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_set_func_table(excit_t it, const struct excit_func_table_s *func_table);

/*
 * Gets a pointer to the function table of an iterator.
 * "it": an iterator.
 * "func_table": an address of a pointer variable where the result will be
 *               stored.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_get_func_table(const_excit_t it, const struct excit_func_table_s **func_table);

/*
 * Allocates a new iterator of the given type.
 * "type": the type of the iterator, cannot be EXCIT_USER.
 * Returns an iterator (that will need to be freed unless ownership is
 * transferred) or NULL if an error occurred.
 */
excit_t excit_alloc(enum excit_type_e type);

/*
 * Allocates a user-defined iterator.
 * "func_table": the function table the iterator will use.
 * "data_size": the size of the inner data to allocate.
 * Returns an iterator (that will need to be freed unless ownership is
 * transferred) or NULL if an error occurred.
 */
excit_t excit_alloc_user(const struct excit_func_table_s *func_table,
			 size_t data_size);

/*
 * Duplicates an iterator and keeps its internal state.
 * "it": iterator to duplicate.
 * Returns an iterator (that will need to be freed unless ownership is
 * transferred) or NULL if an error occurred.
 */
excit_t excit_dup(const_excit_t it);

/*
 * Frees an iterator and all the iterators it acquired ownership to.
 * "it": iterator to free.
 */
void excit_free(excit_t it);

/*
 * Get the type of an iterator
 * "it": an iterator.
 * "type": a pointer to the variable where the result will be stored.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_type(const_excit_t it, enum excit_type_e *type);

/*
 * Get the dimension of an iterator. This is the number of elements of the array
 * of ssize_t that is expected by the peek, next, nth and rank functionalities.
 * "it": an iterator.
 * "dimension": a pointer to the variable where the result will be stored.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_dimension(const_excit_t it, ssize_t *dimension);

/*
 * Gets the current element of an iterator and increments it.
 * "it": an iterator.
 * "indexes": a pointer to an array of indexes with a dimension corresponding to
 *            that of the iterator, where the result will be stored; no result
 *            is returned if NULL.
 * Returns EXCIT_SUCCESS if a valid element was returned, EXCIT_STOPIT if the
 * iterator is depleted, or an error code.
 */
int excit_next(excit_t it, ssize_t *indexes);

/*
 * Gets the current element of an iterator.
 * "it": an iterator.
 * "indexes": a pointer to an array of indexes with a dimension corresponding to
 *            that of the iterator, where the result will be stored; no result
 *            is returned if NULL.
 * Returns EXCIT_SUCCESS if a valid element was returned, EXCIT_STOPIT if the
 * iterator is depleted, or an error code.
 */
int excit_peek(const_excit_t it, ssize_t *indexes);

/*
 * Rewinds an iterator to its initial state.
 * "it": an iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_rewind(excit_t it);

/*
 * Gets the number of elements of an iterator.
 * "it": an iterator.
 * "size": a pointer to the variable where the result will be stored.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_size(const_excit_t it, ssize_t *size);

/*
 * Splits an iterator evenly into several subiterators.
 * "it": an iterator.
 * "n": the number of iterators desired.
 * "results": a pointer to an array of at least n excit_t, where the result
 *            will be stored, or NULL in which case no iterator is created.
 * Returns EXCIT_SUCCESS, -EXCIT_EDOM if the source iterator is too small to be
 * subdivided into the desired number of subiterators, or an error code.
 */
int excit_split(const_excit_t it, ssize_t n, excit_t *results);

/*
 * Gets the nth element of an iterator. If an iterator has k dimensions,
 * then the nth element is an array of k nth elements along each dimension.
 * "it": an iterator.
 * "rank": rank of the element, comprised between 0 and the size of the
 *         iterator.
 * "indexes": a pointer to an array of indexes with a dimension corresponding to
 *            that of the iterator, where the result will be stored; no result
 *            is returned if NULL.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_nth(const_excit_t it, ssize_t rank, ssize_t *indexes);

/*
 * Gets the rank of an element of an iterator. The rank of an element is its
 * iteration index, i.e., excit_nth(excit_rank(element)) should return the element.
 * If the iterator has k dimensions, element is an array of the k values
 * composing element.
 * "it": an iterator.
 * "element": an array of indexes corresponding to the element of the iterator.
 * "rank": a pointer to a variable where the result will be stored; no result is
 *         returned if NULL.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_rank(const_excit_t it, const ssize_t *element, ssize_t *rank);

/*
 * Gets the position of the iterator.
 * "it": an iterator.
 * "rank": a pointer to a variable where the rank of the current element will be
 *         stored; no result is returned if NULL.
 * Returns EXCIT_SUCCESS, EXCIT_STOPIT if the iterator is depleted, or an error
 * code.
 */
int excit_pos(const_excit_t it, ssize_t *rank);

/*
 * Increments the iterator.
 * "it": an iterator.
 * Returns EXCIT_SUCCESS, EXCIT_STOPIT if the iterator is depleted, or an error
 * code.
 */
int excit_skip(excit_t it);

/*
 * Gets the current element of an iterator, rewinding it first if the iterator
 * was depleted. The iterator is incremented.
 * "it": an iterator.
 * "indexes": a pointer to an array of indexes with a dimension corresponding to
 *            that of the iterator, where the result will be stored; no result
 *            is returned if NULL.
 * "looped": a pointer to a variable where the information will be stored on
 *           whether the iterator was rewound (1) or not (0).
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_cyclic_next(excit_t it, ssize_t *indexes, int *looped);

/*
 * Initialize an index iterator with a set of indexes.
 * "it": an index iterator.
 * "len": length of the "index" array (dimension of the iterator).
 * "index": an array of indexes.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_index_init(excit_t it, ssize_t len, const ssize_t *index);

/*
 * Initializes a range iterator to iterate from first to last (included) by step.
 * "it": a range iterator.
 * "first": first value of the range.
 * "last": last value of the range.
 * "step": between elements of the range. Must be nonzero, can be negative.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_range_init(excit_t it, ssize_t first, ssize_t last, ssize_t step);

/*
 * Initializes a sliding window iterator over another iterator.
 * "it": a sliding window iterator.
 * "src": the original iterator. Ownership is transferred.
 * "n": size of the window, must not exceed the size of the src
 *      iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_cons_init(excit_t it, excit_t src, ssize_t n);

/*
 * Initializes a repeat iterator over another iterator.
 * "it": a repeat iterator.
 * "src": the original iterator. Ownership is transferred.
 * "n": the number of repeat of each element of the src iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_repeat_init(excit_t it, excit_t src, ssize_t n);

/*
 * Splits a repeat iterator between repetitions.
 * "it": a repeat iterator.
 * "n": the number of iterators desired.
 * "results": a pointer to an array of at least n excit_t where the result will
 *            be stored, or NULL in which case no iterator is created.
 * Returns EXCIT_SUCCESS, -EXCIT_EDOM if the selected iterator is too small to
 * be subdivided in the desired number of iterators, or an error code.
 */
int excit_repeat_split(const_excit_t it, ssize_t n, excit_t *results);

/*
 * Creates a two-dimensional Hilbert space-filling curve iterator.
 * "it": a hilbert2d iterator.
 * "order": determines the iteration space: (2^order - 1)^2.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_hilbert2d_init(excit_t it, ssize_t order);

/*
 * Adds another iterator to a product iterator.
 * "it": a product iterator.
 * "added_it": the added iterator. Ownership is transferred.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_product_add(excit_t it, excit_t added_it);

/*
 * Adds another iterator to a product iterator without transferring ownership.
 * "it": a product iterator.
 * "added_it": the added iterator. Ownership is not transferred; a duplicate is
 *             created instead.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_product_add_copy(excit_t it, excit_t added_it);

/*
 * Gets the number of iterators inside a product iterator.
 * "it": a product iterator.
 * "count": a pointer to a variable where the result will be stored.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_product_count(const_excit_t it, ssize_t *count);

/*
 * Splits a product iterator along the nth iterator.
 * "it": a product iterator.
 * "dim": the number of the iterator to split, must be comprised between 0 and
 *        the number of iterator inside the product iterator - 1.
 * "n": the number of iterators desired.
 * "results": a pointer to an array of at least n excit_t, where the result
 *            will be stored, or NULL in which case no iterator is created.
 * Returns EXCIT_SUCCESS, -EXCIT_EDOM if the selected iterator is too small to
 * be subdivided into the desired number of iterators, or an error code.
 */
int excit_product_split_dim(const_excit_t it, ssize_t dim, ssize_t n,
			    excit_t *results);

/*
 * Initializes a composition iterator by giving a src iterator and an indexer iterator.
 * "it": a composition iterator.
 * "src": the iterator whose elements are to be returned.
 * "indexer": the iterator that will provide the rank of the elements to return.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_composition_init(excit_t it, excit_t src, excit_t indexer);

/*
 * Initializes a loop iterator by giving a src iterator and a number of loops to achieve.
 * "it": a loop iterator.
 * "src": the iterator to loop over.
 * "n": the number of time to loop over the src iterator.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_loop_init(excit_t it, excit_t src, ssize_t n);

enum tleaf_it_policy_e {
  TLEAF_POLICY_ROUND_ROBIN, /* Iterate on tree leaves in a round-robin fashion */
  TLEAF_POLICY_SCATTER, /* Iterate on tree leaves spreading as much as possible */
  TLEAF_POLICY_USER /* Policy is user-defined */
};

/*
 * Initializes a tleaf iterator by giving its depth, levels of arity and iteration policy.
 * Example building a user scatter policy:
 *         excit_tleaf_init(it, 4, {3, 2, 4}, NULL, TLEAF_POLICY_USER, {2, 1, 0});
 * gives the output index:
 *         0,6,12,18,3,9,15,21,1,7,13,19,4,10,16,22,2,8,14,20,5,11,17,23.
 * "it": a tleaf iterator.
 * "depth": the total number of levels of the tree, including leaves.
 * "arity": An array  of size (depth-1). For each level, specifies the number of children attached to a node.
 *          Leaves have no children. Arities are organized from root to leaves.
 * "index": NULL or an array of depth excit_t to re-index levels.
 *          It is intended to prune node of certain levels while keeping index of the initial structure.
 *          Ownership of index is not taken. The iterator allocates a copy of index and manages it internally.
 * "policy": A policy for iteration on leaves.
 * "user_policy": If policy is TLEAF_POLICY_USER, then this argument must be an array of size (depth-1) providing the
 *                order (from 0 to (depth-2)) in which levels are walked
 *                when resolving indexes. Underneath, a product iterator of range iterator returns indexes on last
 *                levels upon iterator queries. This set of indexes is then
 *                computed to a single leaf index. For instance TLEAF_POLICY_ROUND_ROBIN is obtained from walking
 *                from leaves to root whereas TLEAF_POLICY_SCATTER is
 *                obtained from walking from root to leaves.
 * Returns EXCIT_SUCCESS or an error code.
 */
int excit_tleaf_init(excit_t it,
		     ssize_t depth,
		     const ssize_t *arities,
		     excit_t *index,
		     enum tleaf_it_policy_e policy,
		     const ssize_t *user_policy);

/*
 * Splits a tree at a given level. The behaviour is different from the generic function excit_split for the
 * split might be sparse, depending on the tree level where the split occurs and the number of parts.
 * "it": a tleaf iterator.
 * "level": The level to split.
 * "n": The number of parts. n must divide the target level arity.
 * "out": an array of n allocated tleaf iterators where the result will be stored.
 * Returns EXCIT_SUCCESS, -EXCIT_EDOM if the arity of the selected level is too small to
 * be subdivided into the desired number of iterators, or an error code.
 */
int tleaf_it_split(const_excit_t it, ssize_t level, ssize_t n, excit_t *out);

#endif
