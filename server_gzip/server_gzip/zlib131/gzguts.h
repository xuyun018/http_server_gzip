/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004-2024 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifdef _LARGEFILE64_SOURCE
#  ifndef _LARGEFILE_SOURCE
#    define _LARGEFILE_SOURCE 1
#  endif
#  undef _FILE_OFFSET_BITS
#  undef _TIME_BITS
#endif

#ifdef HAVE_HIDDEN
#  define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#else
#  define ZLIB_INTERNAL
#endif

#include <stdio.h>
#include "zlib.h"
#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#endif

#ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE
#endif
#include <fcntl.h>

#ifdef _WIN32
#  include <stddef.h>
#endif

#if defined(__TURBOC__) || defined(_MSC_VER) || defined(_WIN32)
#  include <io.h>
#endif

#if defined(_WIN32)
#  define WIDECHAR
#endif

#ifdef WINAPI_FAMILY
#  define open _open
#  define read _read
#  define write _write
#  define close _close
#endif

#ifdef NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#endif

#if defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#if defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#if defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ifndef HAVE_VSNPRINTF
#  ifdef MSDOS
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
   but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef __TURBOC__
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    if !defined(vsnprintf) && !defined(NO_vsnprintf)
#      if !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
#         define vsnprintf _vsnprintf
#      endif
#    endif
#  endif
#  ifdef __SASC
#    define NO_vsnprintf
#  endif
#  ifdef VMS
#    define NO_vsnprintf
#  endif
#  ifdef __OS400__
#    define NO_vsnprintf
#  endif
#  ifdef __MVS__
#    define NO_vsnprintf
#  endif
#endif

/* unlike snprintf (which is required in C99), _snprintf does not guarantee
   null termination of the result -- however this is only used in gzlib.c where
   the result is assured to fit in the space provided */
#if defined(_MSC_VER) && _MSC_VER < 1900
#  define snprintf _snprintf
#endif

#ifndef local
#  define local static
#endif
/* since "static" is used to mean two completely different things in C, we
   define "local" for the non-static meaning of "static", for readability
   (compile with -Dlocal if your debugger can't find static symbols) */

/* gz* functions always use library allocation functions */
#ifndef STDC
  extern voidp  malloc(uInt size);
  extern void   free(voidpf ptr);
#endif

/* get errno and strerror definition */
#if defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)GetLastError())
#else
#  ifndef NO_STRERROR
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  else
#    define zstrerror() "stdio error (consult errno)"
#  endif
#endif

/* default memLevel */
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif

/* default i/o buffer size -- double this for output when reading (this and
   twice this must be able to fit in an unsigned type) */
#define GZBUFSIZE 8192

/* internal gzip file state data structure */
typedef struct {
        /* exposed contents for gzgetc() macro */
    struct gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    unsigned size;          /* buffer size, zero if not allocated yet */
    unsigned want;          /* requested buffer size, default is GZBUFSIZE */
    unsigned char *in;      /* input buffer (double-sized when writing) */
    unsigned char *out;     /* output buffer (double-sized when reading) */
    int direct;             /* 0 if processing gzip, 1 if transparent */

        /* just for writing */
    int level;              /* compression level */
    int strategy;           /* compression strategy */
    int reset;              /* true if a reset is pending after a Z_FINISH */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
    int seek;               /* true if seek request pending */
        /* error information */
    int err;                /* error code */
    char *msg;              /* error message */
        /* zlib inflate or deflate stream */
    z_stream strm;          /* stream structure in-place (not a pointer) */
} gz_state;
typedef gz_state FAR *gz_statep;

/* shared functions */

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
unsigned ZLIB_INTERNAL gz_intmax(void);
#define GT_OFF(x) (sizeof(int) == sizeof(z_off64_t) && (x) > gz_intmax())
