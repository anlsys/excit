#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "excit.h"
#include "excit_test.h"

static const ssize_t depth = 4;
static const ssize_t arities[4] = {4,8,2,4};

static excit_t create_test_tleaf(const enum tleaf_it_policy_e policy, const ssize_t *user_policy)
{
	int err = EXCIT_SUCCESS;
	excit_t it;	
        
	it = excit_alloc_test(EXCIT_RANGE);
	assert(it != NULL);

	err = excit_tleaf_init(it, depth, arities, policy, user_policy);
	assert(err == EXCIT_SUCCESS);
		
	return it;
}

static inline ssize_t tleaf_size()
{
	ssize_t i, size=1;
	
	for(i=0; i<depth; i++)
		size *= arities[i];
	return size;
}

static ssize_t tleaf_test_size(excit_t tleaf){
	ssize_t i, it_size, size = tlesf_size();;

	assert(excit_size(tleaf, &it_size) == EXCIT_SUCCESS);
	assert(it_size == size);
}

static void tleaf_test_round_robin_policy(excit_t tleaf){	
	ssize_t i, size, value;

	assert(excit_rewind(tleaf) == EXCIT_SUCCESS);
	for(i = 0; i < tleaf_size(); i++){
		assert(excit_next(tleaf, &value) == EXCIT_SUCCESS);
		assert(value == i);
	}
	assert(excit_next(tleaf, &value) == EXCIT_STOPIT);
}


void tleaf_test_scatter_policy_no_split(excit_t tleaf){	
	ssize_t i, j, r, n, c, value, val, size = tleaf_size();

	assert(excit_rewind(tleaf) == EXCIT_SUCCESS);	
	for(i = 0; i < size; i++){
		c = i;
		n = size;
		val = 0;
		assert(excit_next(tleaf, &value) == EXCIT_SUCCESS);
		for(j=0; j<depth; j++){
			r = c % arities[j];
			n = n / arities[j];
			c = c / arities[j];
			val += n*r;
		}
		assert(value == val);
	}
	assert(excit_next(tleaf, &value) == EXCIT_STOPIT);
}

int main(int argc, char *argv[])
{
	excit_t rrobin = create_test_tleaf(TLEAF_POLICY_ROUND_ROBIN, NULL);
	
	assert(rrobin != NULL);
	tleaf_test_size(rrobin);
	tleaf_test_round_robin_policy(rrobin);
	excit_free(rrobin);
	
	excit_t scatter = create_test_tleaf(TLEAF_POLICY_ROUND_ROBIN, NULL);

	assert(scatter != NULL);
	tleaf_test_scatter_policy_no_split(scatter);
	excit_free(scatter);

	return 0;
}


