/*** xmlstream.c -- chunked XSLT processor
 *
 * Copyright (C) 2018 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of glod.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#if defined HAVE_LIBEXSLT_EXSLT_H
# include <libexslt/exslt.h>
#endif	/* HAVE_LIBEXSLT_EXSLT_H */
#include "nifty.h"

#define strlenof(x)	(sizeof(x) - 1U)
#define _X(x)		((const xmlChar*)(x))
#define _C(x)		((const char*)(x))


static int
__attribute__((format(printf, 1, 2)))
error(const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(errno), stderr);
	}
	fputc('\n', stderr);
	return errno;
}

static int
xopen(const char *fn, int fl)
{
	int fd;

	if (fn == NULL || *fn == '-' && fn[1U] == '\0') {
		fd = STDIN_FILENO;
	} else {
		fd = open(fn, fl);
	}
	return fd;
}

static char*
xmemmem(const char *hay, const size_t hayz, const char *ndl, const size_t ndlz)
{
	const char *const eoh = hay + hayz;
	const char *const eon = ndl + ndlz;
	const char *hp;
	const char *np;
	const char *cand;
	unsigned int hsum;
	unsigned int nsum;
	unsigned int eqp;

	/* trivial checks first
         * a 0-sized needle is defined to be found anywhere in haystack
         * then run strchr() to find a candidate in HAYSTACK (i.e. a portion
         * that happens to begin with *NEEDLE) */
	if (ndlz == 0UL) {
		return deconst(hay);
	} else if ((hay = memchr(hay, *ndl, hayz)) == NULL) {
		/* trivial */
		return NULL;
	}

	/* First characters of haystack and needle are the same now. Both are
	 * guaranteed to be at least one character long.  Now computes the sum
	 * of characters values of needle together with the sum of the first
	 * needle_len characters of haystack. */
	for (hp = hay + 1U, np = ndl + 1U, hsum = *hay, nsum = *hay, eqp = 1U;
	     hp < eoh && np < eon;
	     hsum ^= *hp, nsum ^= *np, eqp &= *hp == *np, hp++, np++);

	/* HP now references the (NZ + 1)-th character. */
	if (np < eon) {
		/* haystack is smaller than needle, :O */
		return NULL;
	} else if (eqp) {
		/* found a match */
		return deconst(hay);
	}

	/* now loop through the rest of haystack,
	 * updating the sum iteratively */
	for (cand = hay; hp < eoh; hp++) {
		hsum ^= *cand++;
		hsum ^= *hp;

		/* Since the sum of the characters is already known to be
		 * equal at that point, it is enough to check just NZ - 1
		 * characters for equality,
		 * also CAND is by design < HP, so no need for range checks */
		if (hsum == nsum && memcmp(cand, ndl, ndlz - 1U) == 0) {
			return deconst(cand);
		}
	}
	return NULL;
}


static const char *params[64U];
static size_t nparams;

static int
push_buf(xmlParserCtxtPtr ptx, const char *buf, size_t len, int lastp)
{
/* comb out \f */
	const char *bp = buf;
	const char *const ep = buf + len;

	for (const char *tp;
	     bp < ep && (tp = memchr(bp, '\f', ep - bp)) != NULL;
	     bp = tp + 1U) {
		xmlParseChunk(ptx, bp, tp - bp, 0);
	}
	/* and pass on the rest */
	return xmlParseChunk(ptx, bp, ep - bp, lastp);
}

static int
appl_sty(xmlParserCtxtPtr ptx)
{
	xsltStylesheetPtr sty = ptx->_private;
	xmlDocPtr doc = ptx->myDoc;
	xmlDocPtr xfd;

	if (!ptx->wellFormed) {
		errno = 0, error("\
Error: cannot parse XML document");
		return -1;
	} else if ((xfd = xsltApplyStylesheet(sty, doc, params)) == NULL) {
		errno = 0, error("\
Error: cannot apply stylesheet");
		return -1;
	}
	/* pump to stdout and clear resources */
	xsltSaveResultToFile(stdout, xfd, sty);
	xmlFreeDoc(xfd);
	return 0;
}

static ssize_t
proc_buf(xmlParserCtxtPtr ptx, const char *bp, const char *const ep)
{
/* process buffer BP and return the number of unconsumed bytes */
	static const char pi[] = "<?xml version=\"1.0\"";

	if (ep == NULL) {
		push_buf(ptx, bp, strlenof(pi), 1);
		if (!ptx->wellFormed) {
			errno = 0, error("\
Error: cannot parse XML document");
			return -1;
		}
		/* apply style sheet, ignore return value */
		(void)appl_sty(ptx);
		return 0;
	}

	/* we always start scanning at offset 1 of bp as we are
	 * looking for initiators not terminators and an empty
	 * string before the very first initiator does not make sense */
	for (const char *tp;
	     (tp = xmemmem(bp + 1U, ep - (bp + 1U), pi, strlenof(pi))) != NULL;
	     bp = tp) {
		push_buf(ptx, bp, tp - bp, 1);
		/* apply style sheet, ignore return value */
		(void)appl_sty(ptx);
		/* reset the parser */
		xmlCtxtResetPush(ptx, NULL, 0, NULL, NULL);
	}
	/* okidoke, no more *full* occurrences of PI
	 * that means we feed the rest of the buffer that we can guarantee
	 * not to contain any substrings of the PI to the XML parser and
	 * hand control back to the buffer reader */
	push_buf(ptx, bp, ep - strlenof(pi) - bp, 0);
	return strlenof(pi);
}

static int
proc1(xsltStylesheetPtr sty, const char *fn)
{
	static char buf[16 * 4096U];
	xmlParserCtxtPtr ptx;
	int rc = 0;
	int fd;

	if ((fd = xopen(fn, O_RDONLY)) < 0) {
		rc = -1;
		goto out;
	}

	if ((ptx = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, fn)) == NULL) {
		errno = 0, error("\
Error: cannot instantiate XML push parser");
		rc = -1;
		goto out;
	}
	/* hide the stylesheet in the parser context */
	ptx->_private = sty;

	for (ssize_t nrd, i = 0;
	     (nrd = read(fd, buf + i, sizeof(buf) - i)) > 0;) {
		const char *const ep = buf + i + nrd;
		const char *bp = buf;

		if (UNLIKELY((i = proc_buf(ptx, bp, ep)) < 0)) {
			rc = -1;
			goto out;
		}
		/* move the unconsumed bytes to the beginning of buf */
		memmove(buf, ep - i, i);
	}
	/* final processing of the last bytes,
	 * let's hope they know how many bytes need processing */
	proc_buf(ptx, buf, NULL);

	/* deinitialise */
	xmlFreeParserCtxt(ptx);
out:
	if (!(fd < 0)) {
		close(fd);
	}
	return rc;
}


#include "xsltmultiproc.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	xsltStylesheetPtr sty;
	int rc = 0;

	if (yuck_parse(argi, argc, argv) < 0) {
		rc = 1;
		goto out;
	} else if (argi->nargs < 1U) {
		errno = 0, error("\
Error: STYLESHEET argument is mandatory.  See --help");
		rc = 1;
		goto out;
	}

	for (size_t i = 0U; i < argi->param_nargs; i++) {
		char *p;

		if ((p = strchr(argi->param_args[i], '=')) == NULL) {
			errno = 0, error("\
Error: parameters should have form NAME=VALUE.");
			rc = 1;
			goto out;
		}
		*p++ = '\0';
		params[nparams++] = argi->param_args[i];
		params[nparams++] = p;
	}

	/* here we go */
	xmlLoadExtDtdDefaultValue = 1;
	xmlSubstituteEntitiesDefault(1);
#if defined HAVE_LIBEXSLT_EXSLT_H
	exsltRegisterAll();
#endif	/* HAVE_LIBEXSLT_EXSLT_H */
	if ((sty = xsltParseStylesheetFile(_X(*argi->args))) == NULL) {
		rc = 1;
		goto out;
	}

	/* now then */
	with (size_t i = 1U) {
		if (argi->nargs == 1U) {
			/* just stdin then innit */
			goto one_off;
		}
		for (; i < argi->nargs; i++) {
		one_off:
			rc |= proc1(sty, argi->args[i]);
		}
		rc &= 0x1U;
	}

	/* clean up */
	xsltFreeStylesheet(sty);

out:
	yuck_free(argi);
	return rc;
}

/* xsltmultiproc.c ends here */
