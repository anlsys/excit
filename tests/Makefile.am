AM_COLOR_TESTS = yes

AM_CFLAGS = -I$(top_srcdir)/src -Wall -Werror -pedantic
AM_LDFLAGS = ../src/libexcit.la

LIBHSOURCES = excit_test.h
LIBCSOURCES = excit_test.c

excit_range_SOURCES = $(LIBHSOURCES) $(LIBCSOURCES) excit_range.c
excit_product_SOURCES = $(LIBHSOURCES) $(LIBCSOURCES) excit_product.c
excit_repeat_SOURCES = $(LIBHSOURCES) $(LIBCSOURCES) excit_repeat.c
excit_cons_SOURCES = $(LIBHSOURCES) $(LIBCSOURCES) excit_cons.c
excit_hilbert2d_SOURCES = $(LIBHSOURCES) $(LIBCSOURCES) excit_hilbert2d.c
excit_slice_SOURCES = $(LIBHSOURCES) $(LIBCSOURCES) excit_slice.c

UNIT_TESTS = excit_range excit_product excit_repeat excit_cons excit_hilbert2d excit_slice

# all tests
check_PROGRAMS = $(UNIT_TESTS)
TESTS = $(UNIT_TESTS)
