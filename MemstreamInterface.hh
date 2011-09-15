/*
 * MemstreamInterface.hh
 *
 *  Created on: Sep 15, 2011
 *      Author: nsoblath
 */

#ifndef MEMSTREAMINTERFACE_HH_
#define MEMSTREAMINTERFACE_HH_

#ifdef NO_MEMSTREAM

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct memstream {
  char **cp;
  size_t *lenp;
  size_t offset;
};

static void
memstream_grow(struct memstream *ms, size_t newsize);

static int
memstream_read(void *cookie, char *buf, int len);

static int
memstream_write(void *cookie, const char *buf, int len);

static fpos_t
memstream_seek(void *cookie, fpos_t pos, int whence);

static int
memstream_close(void *cookie);

FILE *
open_memstream(char **cp, size_t *lenp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* NO_MEMSTREAM */

#endif /* MEMSTREAMINTERFACE_HH_ */
