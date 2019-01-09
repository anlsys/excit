#ifndef EXCIT_TEST_H
#define EXCIT_TEST_H
#include "excit.h"

#define ES EXCIT_SUCCESS


extern excit_t excit_alloc_test(enum excit_type_e type);

extern void (*synthetic_tests[])(excit_t);
#endif
