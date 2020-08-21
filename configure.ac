# see semver.org for version info
AC_INIT([excit],[0.0.1],[brice.videau@imag.fr])

# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/excit.h])

# build artefacts in separate dir
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.12])
AM_PROG_AR

# check for libtool
LT_INIT

# check for programs
AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_C99
AC_TYPE_SSIZE_T
AC_TYPE_SIZE_T
AC_HEADER_ASSERT

#check for valgrind
AX_VALGRIND_DFLT([helgrind], [off])
AX_VALGRIND_DFLT([drd], [off])
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

# Support for cross-compiling check programs
AM_EXTRA_RECURSIVE_TARGETS([check-programs])

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 tests/Makefile
		 libexcit.pc])
AC_OUTPUT
