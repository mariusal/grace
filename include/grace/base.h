/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2002 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik <fnevgeny@plasma-gate.weizmann.ac.il>
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
 * Base stuff - public declarations
 */

#ifndef __BASE_H_
#define __BASE_H_

#include <config.h>

/* for FILE */
#include <stdio.h>

/* for size_t */
#include <sys/types.h>

/* boolean values */
#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

/* function return codes */
#define RETURN_SUCCESS (0)
#define RETURN_FAILURE (1)

/* FIXME! */
extern void errmsg(char *msg);

/* generic memory allocation & friend */
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
void xfree(void *ptr);
#define XCFREE(ptr) xfree(ptr); ptr = NULL

/* string (re)allocation */
char *copy_string(char *dest, const char *src);
char *concat_strings(char *dest, const char *src);

/* string comparison etc */
int compare_strings(const char *s1, const char *s2);
int is_empty_string(const char *s);

/* bit manipulations */
int bin_dump(char *value, int i, int pad);
unsigned char reversebits(unsigned char inword);

/* file i/o */
char *grace_fgets(char *s, int size, FILE *stream);

#endif /* __BASE_H_ */
