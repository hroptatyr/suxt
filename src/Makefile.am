### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=201001L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE
AM_CPPFLAGS += -D_HAVE_VERSION_H
AM_LDFLAGS = $(XCCLDFLAGS)

bin_PROGRAMS =
noinst_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
pkglib_LTLIBRARIES =
noinst_HEADERS =
BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)
CLEANFILES = 
SUFFIXES =

CLEANFILES += version.c
EXTRA_DIST += version.c.in
EXTRA_DIST += nifty.h

bin_PROGRAMS += xmlsplit
xmlsplit_SOURCES = xmlsplit.c xmlsplit.yuck
xmlsplit_CPPFLAGS = $(AM_CPPFLAGS)
xmlsplit_LDFLAGS = $(AM_LDFLAGS)
xmlsplit_LDADD =
if HAVE_LIBXML2
xmlsplit_CPPFLAGS += $(libxml2_CFLAGS)
xmlsplit_LDADD += $(libxml2_LIBS)
else  !HAVE_LIBXML2
if HAVE_EXPAT
xmlsplit_CPPFLAGS += $(expat_CFLAGS)
xmlsplit_LDADD += $(expat_LIBS)
endif  HAVE_EXPAT
endif  !HAVE_LIBXML2
BUILT_SOURCES += xmlsplit.yucc

if HAVE_LIBXSLT
bin_PROGRAMS += xsltmultiproc
xsltmultiproc_SOURCES = xsltmultiproc.c xsltmultiproc.yuck
xsltmultiproc_CPPFLAGS = $(AM_CPPFLAGS)
xsltmultiproc_LDFLAGS = $(AM_LDFLAGS)
xsltmultiproc_LDADD =
xsltmultiproc_CPPFLAGS += $(libxml2_CFLAGS)
xsltmultiproc_LDADD += $(libxml2_LIBS)
xsltmultiproc_CPPFLAGS += $(libxslt_CFLAGS)
xsltmultiproc_LDADD += $(libxslt_LIBS)
xsltmultiproc_CPPFLAGS += $(libexslt_CFLAGS)
xsltmultiproc_LDADD += $(libexslt_LIBS)
BUILT_SOURCES += xsltmultiproc.yucc
endif  HAVE_LIBXSLT


## version rules
version.c: version.c.in $(top_builddir)/.version
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		yuck$(EXEEXT) scmver --ignore-noscm --force -o $@ \
			--use-reference --reference $(top_builddir)/.version $<

## yuck rule
SUFFIXES += .yuck
SUFFIXES += .yucc
.yuck.yucc:
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		yuck$(EXEEXT) gen -o $@ $<

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
