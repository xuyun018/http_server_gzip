/* gzclose.c -- zlib gzclose() function
 * Copyright (C) 2004, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "gzguts.h"

/* gzclose() is in a separate file so that it is linked in only if it is used.
   That way the other gzclose functions can be used instead to avoid linking in
   unneeded compression or decompression routines. */
int ZEXPORT gzclose(gzFile file, unsigned char *dst, unsigned int *ofs) {
	gzclose_w(file, dst, ofs);
	return(0);
}
