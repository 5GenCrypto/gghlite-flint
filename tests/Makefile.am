AUTOMAKE_OPTIONS = foreign
AM_CFLAGS  = ${DEBUG_CFLAGS} -I$(top_srcdir) -I$(top_srcdir)/dgs -fopenmp

AM_LDFLAGS = -Wl,-rpath -Wl,$(top_builddir)/flint

LDADD = $(top_builddir)/gghlite/libgghlite.la \
        $(top_builddir)/dgs/libdgs.la \
        $(top_builddir)/oz/liboz.la \
	    $(top_builddir)/dgsl/libdgsl.la -lgmp

#LDFLAGS = -no-install

TESTS = test_rem_small test_instgen test_jigsaw
check_PROGRAMS = $(TESTS)

@VALGRIND_CHECK_RULES@

all: $(TESTS)
