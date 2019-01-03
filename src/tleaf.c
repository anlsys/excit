#include <stdlib.h>
#include <stdio.h>
#include "tleaf.h"

typedef struct tleaf_it_s* tleaf_it;

struct excit_func_table_s excit_tleaf_func_table = {
  tleaf_it_alloc,
  tleaf_it_free,
  tleaf_it_copy,
  tleaf_it_next,
  tleaf_it_peek,
  tleaf_it_size,
  tleaf_it_rewind,
  tleaf_it_split,
  tleaf_it_nth,
  tleaf_it_rank,
  tleaf_it_pos
};

#define EXCIT_DATA(it, data_it)				\
  if(it == NULL) { return EXCIT_EINVAL; }		\
  do{							\
    int err = excit_get_data(it, (void**)(&data_it));	\
    if(err != EXCIT_SUCCESS) {				\
      return err;					\
    }							\
  } while(0);						\
  if(data_it == NULL){					\
    return EXCIT_EINVAL;				\
  }

int tleaf_it_alloc(excit_t it)
{
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  excit_set_dimension(it, 1); 
  data_it->cur = 0;
  data_it->offset=0;
  data_it->depth = 0;
  data_it->arity = NULL;
  data_it->policy = ROUND_ROBIN;
  return EXCIT_SUCCESS;
}

void tleaf_it_free(excit_t it)
{
  if(it == NULL){ return; }
  tleaf_it data_it;
  excit_get_data(it, (void**)(&data_it));
  if(data_it == NULL){ return; }
  free(data_it->arity);
  return;
}

int excit_tleaf_init(excit_t it,
		  const ssize_t depth,
		  const ssize_t* arities,
		  const enum tleaf_it_policy_e policy)
{
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  data_it->arity = malloc(sizeof(*data_it->arity) * depth);
  if(data_it->arity == NULL){
    perror("malloc");
    return EXCIT_ENOMEM;
  }

  ssize_t i;
  data_it->leaves = 1;
  data_it->depth = depth;
  for(i=0; i<depth-1; i++){
    data_it->arity[i] = arities[i];
    data_it->leaves *= arities[i];
  }
  data_it->policy = policy;
  return EXCIT_SUCCESS;
}

int tleaf_it_size(const excit_t it, ssize_t *size)
{
  if(size == NULL){ return EXCIT_EINVAL; }
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  *size = data_it->leaves;
  return EXCIT_SUCCESS;
}

int tleaf_it_rewind(excit_t it)
{
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  data_it->cur=0;
  return EXCIT_SUCCESS;
}

int tleaf_it_copy(excit_t dst_it, const excit_t src_it)
{
  tleaf_it dst, src;
  int err;

  //check ptr
  if(src_it == NULL || dst_it == NULL){ return EXCIT_EINVAL; }
  err = excit_get_data(dst_it, (void**)(&dst));
  if(err != EXCIT_SUCCESS) return err;
  err = excit_get_data(src_it, (void**)(&src));
  if(err != EXCIT_SUCCESS) return err;  
  if(dst == NULL || src == NULL) return EXCIT_EINVAL;
  
  //actual copy
  err = excit_tleaf_init(dst_it, src->depth, src->arity, src->policy);
  if(err != EXCIT_SUCCESS) return err;
  dst->cur = src->cur;
  dst->offset = src->offset;
  return EXCIT_SUCCESS;
}

int tleaf_it_pos(const excit_t it, ssize_t* value)
{
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  *value = data_it->cur;
  if(data_it->cur >= data_it->leaves){ return EXCIT_STOPIT; }
  return EXCIT_SUCCESS;
}

int tleaf_it_nth_round_robin(const tleaf_it it, const ssize_t n, ssize_t* value)
{
  //no check performed here
  *value = n + it->offset;
  return EXCIT_SUCCESS;
}

int tleaf_it_nth_scatter(const tleaf_it it, ssize_t c, ssize_t* value)
{
  //no check performed here
  ssize_t depth = 0;
  ssize_t arity = 0;
  ssize_t r = 0;
  ssize_t n = it->leaves;
  ssize_t pos = 0;
  
  for(depth = 0; depth < (it->depth-1); depth++){
    arity = it->arity[depth];
    r = c % arity;
    n = n / arity;
    pos += n*r;
    c = c / arity;
  }
  *value = pos + it->offset;
  
  return EXCIT_SUCCESS;
}

int tleaf_it_nth(const excit_t it, ssize_t n, ssize_t *indexes)
{
  if(indexes == NULL)
    return EXCIT_EINVAL;
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  
  if(n < 0 || n >= data_it->leaves)
    return EXCIT_EDOM;

  int err = EXCIT_SUCCESS;
  
  switch(data_it->policy){
  case ROUND_ROBIN:
    err = tleaf_it_nth_round_robin(data_it, n, indexes);
    break;
  case SCATTER:
    err = tleaf_it_nth_scatter(data_it, n, indexes);
    break;
  default:
    err = EXCIT_EINVAL;
    break;
  }
  return err;
}

int tleaf_it_peek(const excit_t it, ssize_t* value)
{
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  if(data_it->cur >= data_it->leaves){ return EXCIT_STOPIT; }
  return tleaf_it_nth(it, data_it->cur, value);
}

int tleaf_it_next(excit_t it, ssize_t* indexes)
{
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  int err = tleaf_it_peek(it, indexes);
  if(err == EXCIT_SUCCESS){
    data_it->cur++;
  }
  return err;
}


int tleaf_it_rank(const excit_t it,
		  const ssize_t *indexes,
		  ssize_t *n)
{
  // the function nth is symmetric
  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  return tleaf_it_nth(it, *indexes - data_it->offset, n);
}
  
int tleaf_it_split(const excit_t it, ssize_t n, excit_t *results){
  // Split is done on a level which arity is a multiple of n.
  // From root to leaves, the first matching levels is used for the split.

  if(n<0 || (n>0 && results==NULL))
    return EXCIT_EINVAL;
  if(n==0 || results==NULL)
    return EXCIT_SUCCESS;

  tleaf_it data_it;
  EXCIT_DATA(it, data_it);
  if(n > data_it->leaves || data_it->leaves%n != 0)
    return EXCIT_EDOM;

  ssize_t i = 0;
  int split = 0;
  ssize_t depth=data_it->depth;
  enum tleaf_it_policy_e policy = data_it->policy;
  ssize_t* arities = malloc(sizeof(*arities) * depth);
  
  if(arities == NULL){
    perror("malloc");
    return EXCIT_ENOMEM;
  }
  for(i=0; i<data_it->depth; i++){
    if(! split && data_it->arity[i] % n == 0){
      arities[i] = data_it->arity[i]/n;
      split=1;
    } else {
      arities[i] = data_it->arity[i];
    }
  }

  int err;
  for(i=0; i<n; i++){
    results[i] = excit_alloc_user(&excit_tleaf_func_table, sizeof(*data_it));
    excit_tleaf_init(results[i], depth, arities, policy);
    err = excit_get_data(results[i], (void**)(&data_it));
    if(err != EXCIT_SUCCESS || data_it == NULL) {
      while(i--){ excit_free(results[i]); }
      return err;
    }
    data_it->offset = data_it->leaves*i;
  }

  free(arities);
  return EXCIT_SUCCESS;
}

