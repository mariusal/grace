/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2012 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
 * 
 * 
 *                           All Rights Reserved
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Base stuff - private declarations
 */

#ifndef __BASEP_H_
#define __BASEP_H_

#include "grace/base.h"

/* Min/Max useful macros */
#define MIN2(a, b) (((a) < (b)) ? (a) : (b))
#define MAX2(a, b) (((a) > (b)) ? (a) : (b))
#define MIN3(a, b, c) (((a) < (b)) ? MIN2(a, c) : MIN2(b, c))
#define MAX3(a, b, c) (((a) > (b)) ? MAX2(a, c) : MAX2(b, c))

#define SQR(a) ((a)*(a))

/* AMem stuff */
typedef void *(*AMemMallocProc) (AMem *amem, size_t size);
typedef void *(*AMemReallocProc)(AMem *amem, void *ptr, size_t size);
typedef void  (*AMemFreeProc)   (AMem *amem, void *ptr);

typedef int          (*AMemSnapshotProc) (AMem *amem);
typedef int          (*AMemUndoProc)     (AMem *amem);
typedef int          (*AMemRedoProc)     (AMem *amem);
typedef unsigned int (*AMemUndoCountProc)(AMem *amem);
typedef unsigned int (*AMemRedoCountProc)(AMem *amem);


struct _AMem {
    int model;
    void *heap;
    
    int undoable;
    
    AMemMallocProc    malloc_proc;
    AMemReallocProc   realloc_proc;
    AMemFreeProc      free_proc;
    
    AMemSnapshotProc  snapshot_proc;
    AMemUndoProc      undo_proc;
    AMemRedoProc      redo_proc;
    AMemUndoCountProc undo_count_proc;
    AMemRedoCountProc redo_count_proc;
};


/* Replacements for some missing libc functions */

#if defined(__VMS)
#  ifndef __CRTL_VER
#    define __CRTL_VER __VMS_VER
#  endif
   int system_spawn(const char *command);
#  if __ALPHA || __DECC_VER >= 60000000
#    include <builtins.h>
#  endif
#  if __CRTL_VER < 70000000 
struct passwd {
    char  *pw_name;
    char  *pw_passwd;
    int   pw_uid;
    int   pw_gid;
    short pw_salt;
    int   pw_encrypt;
    char  *pw_age;
    char  *pw_comment;
    char  *pw_gecos;
    char  *pw_dir;
    char  *pw_shell;
};
#  endif  /* __CRTL_VER */
#endif /* __VMS */

#ifndef HAVE_MEMMOVE
#  define memmove(a, b, c) bcopy((b), (a), (c))
#endif

#ifndef HAVE_MEMCPY
#  define memcpy(a, b, c) bcopy ((b), (a), (c))
#endif

#ifndef HAVE_GETHOSTNAME
#  define get_hostname(a, n) (strncpy((a), "localhost", n)?0:1)
#endif

#ifndef HAVE_DRAND48
#  define srand48 srand
#  define lrand48 rand
double drand48(void);
#else
#  ifndef HAVE_DRAND48_DECL
extern double drand48(void);
#  endif
#endif

#ifndef HAVE_GETCWD
#  ifdef OS2
#    define getcwd _getcwd2
#    define chdir _chdir2
#  endif
#endif

#ifndef HAVE_UNLINK
#  ifdef VMS
#    include <unixio.h>
#    define unlink delete
#  endif
#endif

#ifndef HAVE_POPEN
FILE *popen(char *cmd, char *mode);
int   pclose(FILE *fp);
#endif

#ifndef HAVE_GETTIMEOFDAY
#  include <time.h>
int gettimeofday (struct timeval *tp, void *tzp);
#endif

#ifndef HAVE_ALLOCA
void *alloca(unsigned int);
#endif
#if defined(__VMS) && (__ALPHA || __DECC_VER >= 60000000)
#  define alloca __ALLOCA
#endif

#ifdef __EMX__
char *exe_path_translate(char *path);
#else
#  define exe_path_translate(p) (p)
#endif

#ifdef __VMS
char *path_translate(const char *path);
#else
#  define path_translate(p) (p)
#endif

#ifndef HAVE_GETLOGIN
char *getlogin(void);
#endif

#ifndef O_NONBLOCK
# ifdef O_NDELAY
#  define O_NONBLOCK O_NDELAY
# else
#  define O_NONBLOCK 0
# endif
#endif

/* Replacements for some missing libm functions */

#if defined(HAVE_MATH_H)
#  include <math.h>
#endif
#if defined(HAVE_FLOAT_H)
#  include <float.h>
#endif
#if defined(HAVE_IEEEFP_H)
#  include <ieeefp.h>
#endif

#ifndef __GRACE_SOURCE_

#ifndef MACHEP
extern double MACHEP;
#endif

#ifndef UFLOWTHRESH
extern double UFLOWTHRESH;
#endif

#ifndef MAXNUM
extern double MAXNUM;
#endif

#endif /* __GRACE_SOURCE_ */

#ifndef M_PI
#  define M_PI  3.14159265358979323846
#endif

#ifndef M_SQRT2
#  define M_SQRT2     1.41421356237309504880      /* sqrt(2) */
#endif

#ifndef M_SQRT1_2
#  define M_SQRT1_2   0.70710678118654752440      /* 1/sqrt(2) */
#endif

#ifndef M_SQRT1_3
#  define M_SQRT1_3   0.57735026918962576451      /* 1/sqrt(3) */
#endif

#ifndef HAVE_HYPOT
#  define hypot(x, y) sqrt((x)*(x) + (y)*(y))
#endif

extern double round ( double x );
#ifndef HAVE_RINT
#  define rint round
#else
#  ifndef HAVE_RINT_DECL
extern double rint ( double x );
#  endif
#endif

#ifndef HAVE_CBRT_DECL
extern double cbrt ( double x );
#endif

/* Cygnus gnuwin32 has the log2 macro */
#ifdef log2
#  undef log2
#endif

#ifndef HAVE_LOG2_DECL
extern double log2 ( double x );
#endif

#ifndef HAVE_LGAMMA
extern int sgngam;
#  define lgamma lgam
#  define signgam sgngam
extern double lgam ( double x );
#else
#  ifndef HAVE_LGAMMA_DECL
extern double lgamma ( double x );
#  endif
#  ifndef HAVE_SIGNGAM_DECL
extern int signgam;
#  endif
#  define lgam lgamma
#  define sgngam signgam
#endif

#ifndef HAVE_ACOSH_DECL
extern double acosh ( double x );
#endif

#ifndef HAVE_ASINH_DECL
extern double asinh ( double x );
#endif

#ifndef HAVE_ATANH_DECL
extern double atanh ( double x );
#endif

#ifndef HAVE_ERF_DECL
extern double erf ( double x );
#endif

#ifndef HAVE_ERFC_DECL
extern double erfc ( double x );
#endif

#ifndef HAVE_Y0_DECL
extern double y0 ( double x );
#endif
#ifndef HAVE_Y1_DECL
extern double y1 ( double x );
#endif
#ifndef HAVE_YN_DECL
extern double yn ( int n, double x );
#endif
#ifndef HAVE_J0_DECL
extern double j0 ( double x );
#endif
#ifndef HAVE_J1_DECL
extern double j1 ( double x );
#endif
#ifndef HAVE_JN_DECL
extern double jn ( int n, double x );
#endif

/* isfinite is a macro */
#ifdef isfinite
#  define HAVE_ISFINITE_MACRO
#endif

#ifndef HAVE_FINITE
#  define finite isfinite
#  if !defined(HAVE_ISFINITE_DECL) && !defined(HAVE_ISFINITE_MACRO)
extern int isfinite ( double x );
#  endif
#else
#  ifndef HAVE_FINITE_DECL
extern int finite ( double x );
#  endif
#endif

/* isnan is a macro */
#ifdef isnan
#  define HAVE_ISNAN_MACRO
#endif

#if !defined(HAVE_ISNAN_DECL) && !defined(HAVE_ISNAN_MACRO)
extern int isnan ( double x );
#endif



/* XFile */
struct _XStackEntry {
    char *name;
    void *data;
};

struct _XStack {
    int size;
    int depth;
    XStackEntry *entries;
};

struct _ElementAttribute {
    char *name;
    char *value;
};

struct _Attributes {
    ElementAttribute *args;
    unsigned int size;
    unsigned int count;
};

struct _XFile {
    FILE *fp;
    XStack *tree;
    unsigned int indent;
    char *indstr;
    int curpos;
    int convert;
    char *ns_prefix;
    char *ns_uri;
    int ns_force;
};

XStack *xstack_new(void);
void xstack_free(XStack *xs);
int xstack_increment(XStack *xs, const char *name, void *data);
int xstack_decrement(XStack *xs, const char *name);
int xstack_get_first(XStack *xs, char **name, void **data);
int xstack_get_last(XStack *xs, char **name, void **data);
int xstack_is_empty(XStack *xs);

#endif /* __BASEP_H_ */
