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
 * memory stuff - malloc & friends
 */

#include <stdlib.h>
#include <string.h>

#include "grace/base.h"

extern void errmsg(char *msg);

/*
 * free and check for NULL pointer
 */
void xfree(void *ptr)
{
    if (ptr != NULL) {
	free(ptr);
    }
}

void *xmalloc(size_t size)
{
    void *retval;

    if (size == 0) {
        retval = NULL;
    } else {
        retval = malloc(size);
    }

    if (retval == NULL && size != 0) {
        errmsg("Memory storage exceeded!");
    }
    return retval;
}

void *xrealloc(void *ptr, size_t size)
{
    void *retval;

#if defined(REALLOC_IS_BUGGY)
    if (ptr == NULL) {
        retval = malloc(size);
    } else if (size == 0) {
        if (ptr) {
            free(ptr);
        }
        retval = NULL;
    } else {
        retval = realloc(ptr, size); 
    }
#else
    retval = realloc(ptr, size);
#endif
    
    if (retval == NULL && size != 0) {
        errmsg("Memory storage exceeded!");
    }
    return retval;
}


void *xcalloc(size_t nmemb, size_t size)
{
    void *retval;

    retval = xmalloc(nmemb*size);
    if (retval) {
        memset(retval, 0, nmemb*size);
    }

    return retval;
}
