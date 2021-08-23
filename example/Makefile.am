##=============================================================================
##
## libbsat: example/Makefile.am
##
##=============================================================================

AM_CFLAGS=\
	-I@top_builddir@/include \
	-g

AM_LDFLAGS=\
	-lev

LDADD=@top_builddir@/lib/libbsat.la

AM_DEFAULT_SOURCE_EXT=.c
EXTRA_PROGRAMS=\
	bsat_example

doc: bsat_example
	pomd4c @srcdir@/bsat_example.c > @srcdir@/README.md

# EOF

