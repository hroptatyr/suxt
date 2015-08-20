# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_CPPFLAGS += -DTEST
AM_CPPFLAGS += -UHAVE_CONFIG_H -UHAVE_VERSION_H

EXTRA_DIST = $(BUILT_SOURCES) $(TESTS)
TESTS =
NOTESTS =
TEST_EXTENSIONS =
BUILT_SOURCES =
check_PROGRAMS =
CLEANFILES = $(check_PROGRAMS)

## summon our helper
include clitoris.am
AM_CLIT_LOG_FLAGS = -v --builddir "$(top_builddir)/src"
TESTS_ENVIRONMENT = root="$(top_srcdir)"


TESTS += split_01.clit
TESTS += split_02.clit
TESTS += mproc_01.clit
TESTS += mproc_02.clit
EXTRA_DIST += ex_fpr.xml
EXTRA_DIST += ex_fpr.xsl

## Makefile.am ends here
