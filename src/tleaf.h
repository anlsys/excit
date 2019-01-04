#ifndef TLEAF_H
#define TLEAF_H

#include "excit.h"

struct tleaf_it_s{
  ssize_t                cur;          // cursor of iterator position
  ssize_t                depth;        // The tree depth
  ssize_t*               arity;        // Number of children per node on each level
  ssize_t*               nleaves;      // Number of leaves below each level
  ssize_t                offset;       // When split occures, a resulting chunk can be offset as a subtree of the original one.
  ssize_t*               strides;      // When split occures, indexing might not be contiguous anymore, the stride is applied by level. It shall be increased by 1 on level where split occures.
  enum tleaf_it_policy_e policy;       // iteration policy
};

/* See excit.h for functions meaning */
int  tleaf_it_alloc(excit_t);
void tleaf_it_free(excit_t);
int  tleaf_it_copy(excit_t, const excit_t);
int  tleaf_it_next(excit_t, ssize_t*);
int  tleaf_it_peek(const excit_t, ssize_t*);
int  tleaf_it_size(const excit_t, ssize_t*);
int  tleaf_it_rewind(excit_t);
int  tleaf_it_split(const excit_t, ssize_t, excit_t*);
int  tleaf_it_nth(const excit_t, ssize_t, ssize_t*);
int  tleaf_it_rank(const excit_t, const ssize_t*, ssize_t*);
int  tleaf_it_pos(const excit_t, ssize_t*);

extern struct excit_func_table_s excit_tleaf_func_table;

#endif //TLEAF_H

