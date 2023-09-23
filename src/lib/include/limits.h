#ifndef LIMITS_H
#define LIMITS_H

/* Implementation-defined values */

#define CHAR_BIT      8             /* Number of bits in a char */
#define SCHAR_MIN     (-128)        /* Minimum value for a signed char */
#define SCHAR_MAX     127           /* Maximum value for a signed char */
#define UCHAR_MAX     255           /* Maximum value for an unsigned char */
#define CHAR_MIN      SCHAR_MIN     /* Minimum value for a char */
#define CHAR_MAX      SCHAR_MAX     /* Maximum value for a char */
#define MB_LEN_MAX    16            /* Maximum length of a multibyte character */

#define SHRT_MIN      (-32768)      /* Minimum value for a short */
#define SHRT_MAX      32767         /* Maximum value for a short */
#define USHRT_MAX     65535         /* Maximum value for an unsigned short */
#define INT_MIN       (-2147483647 - 1) /* Minimum value for an int */
#define INT_MAX       2147483647    /* Maximum value for an int */
#define UINT_MAX      4294967295U   /* Maximum value for an unsigned int */
#define LONG_MIN      (-9223372036854775807L - 1) /* Minimum value for a long */
#define LONG_MAX      9223372036854775807L    /* Maximum value for a long */
#define ULONG_MAX     18446744073709551615UL  /* Maximum value for an unsigned long */

/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#define NR_OPEN	        1024

#define NGROUPS_MAX    65536	/* supplemental group IDs are available */
#define ARG_MAX       131072	/* # bytes of args + environ for exec() */
#define LINK_MAX         127	/* # links a file may have */
#define MAX_CANON        255	/* size of the canonical input queue */
#define MAX_INPUT        255	/* size of the type-ahead buffer */
#define NAME_MAX         255	/* # chars in a file name */
#define PATH_MAX        4096	/* # chars in a path name including nul */
#define PIPE_BUF        4096	/* # bytes in atomic write to a pipe */
#define XATTR_NAME_MAX   255	/* # chars in an extended attribute name */
#define XATTR_SIZE_MAX 65536	/* size of an extended attribute value (64k) */
#define XATTR_LIST_MAX 65536	/* size of extended attribute namelist (64k) */

#define RTSIG_MAX	  32

#endif /* LIMITS_H */
