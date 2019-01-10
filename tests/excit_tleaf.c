#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "excit.h"
#include "excit_test.h"

static excit_t create_test_tleaf(const ssize_t depth,
				 const ssize_t *arities,
				 const enum tleaf_it_policy_e policy,
				 const ssize_t *user_policy)
{
	int err = EXCIT_SUCCESS;
	excit_t it;	
        
	it = excit_alloc_test(EXCIT_TLEAF);
	assert(it != NULL);

	err = excit_tleaf_init(it, depth+1, arities, policy, user_policy);
	assert(err == EXCIT_SUCCESS);

	ssize_t i, size = 1, it_size;
	
	for(i=0; i<depth; i++)
		size *= arities[i];
	assert(excit_size(it, &it_size) == EXCIT_SUCCESS);
	assert(it_size == size);
	
	return it;
}

static void tleaf_test_round_robin_policy(excit_t tleaf){	
	ssize_t i, value, size;
	
	assert(excit_size(tleaf, &size) == EXCIT_SUCCESS);
	assert(excit_rewind(tleaf) == EXCIT_SUCCESS);
	for(i = 0; i < size; i++){
		assert(excit_next(tleaf, &value) == EXCIT_SUCCESS);
		assert(value == i);
	}
	assert(excit_next(tleaf, &value) == EXCIT_STOPIT);
}


static void tleaf_test_scatter_policy_no_split(excit_t tleaf, const ssize_t depth, const ssize_t *arities){
	ssize_t i, j, r, n, c, value, val, size;

	assert(excit_size(tleaf, &size) == EXCIT_SUCCESS);
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

void run_tests(const ssize_t depth, const ssize_t *arities){
	excit_t rrobin = create_test_tleaf(depth, arities, TLEAF_POLICY_ROUND_ROBIN, NULL);

	assert(rrobin != NULL);
	tleaf_test_round_robin_policy(rrobin);
	excit_free(rrobin);

	excit_t scatter = create_test_tleaf(depth, arities, TLEAF_POLICY_SCATTER, NULL);

	assert(scatter != NULL);
	tleaf_test_scatter_policy_no_split(scatter, depth, arities);
	excit_free(scatter);

	int i = 0;
	while (synthetic_tests[i]) {
		excit_t it = create_test_tleaf(depth, arities, TLEAF_POLICY_ROUND_ROBIN, NULL);

		synthetic_tests[i](it);
		excit_free(it);
		i++;
	}	
}

int main(int argc, char *argv[])
{
	const ssize_t depth = 4;
        const ssize_t arities[4] = {4,8,2,4};

	run_tests(depth, arities);
	return 0;
}

