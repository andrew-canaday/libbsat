##=============================================================================
##
## libbsat: lib/Makefile.am
##
##=============================================================================

CPPFLAGS=\
 	-I@top_srcdir@/include \
 	-I@top_srcdir@/lib \
	-I@top_builddir@/include \
	-DYMO_ASSERT_VERBOSE=1 \
	-Wno-error

LDADD=\
      @top_builddir@/lib/libbsat.la

AM_DEFAULT_SOURCE_EXT=.c
check_HEADERS=\
	bsat_test.h \
	ymo_assert.h

check_PROGRAMS=\
	test_toq \
	test_timeout \
	test_invoke \
	test_clear

TESTS=\
	test_toq \
	test_timeout \
	test_invoke \
	test_clear
