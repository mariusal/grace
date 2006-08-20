/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2006 Grace Development Team
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
 * file utils
 */

#include <sys/stat.h>
#include <string.h>

#include "grace/base.h"

/* replacement for fgets() to fix up reading DOS text files */
char *grace_fgets(char *s, int size, FILE *stream)
{
    int  slen;
    char *endptr;

    s = fgets(s, size, stream);
    if (!s) {
        return NULL;
    }

    slen = strlen(s);
    if (slen >= 2) {
        endptr = s + slen - 2;
        /* check for DOS ending "\r\n" */
        if (*endptr == '\r') {
            /* 'move' un*x string tail "\n\0" one char forward */
            *endptr     = '\n';
            *(endptr+1) = '\0';
        }
    }
    return s;
}

void grfile_free(GrFILE *grf)
{
    if (grf) {
        xfree(grf->fname);
        if (grf->fp) {
            fclose(grf->fp);
        }
        xfree(grf);
    }
}

GrFILE *grfile_new(const char *fname)
{
    GrFILE *grf;
    
    if (!fname) {
        return NULL;
    }
    
    grf = xmalloc(sizeof(GrFILE));
    if (!grf) {
        return NULL;
    }
    memset(grf, 0, sizeof(GrFILE));
    
    grf->fname = copy_string(NULL, fname);
    if (!grf->fname) {
        grfile_free(grf);
        return NULL;
    }
    
    return grf;
}


GrFILE *grfile_openr(const char *fname)
{
    GrFILE *grf;
    
    grf = grfile_new(fname);
    
    if (!grf) {
        return NULL;
    }
    
    grf->fp = fopen(fname, "rb");
    if (!grf->fp) {
        grfile_free(grf);
        return NULL;
    }
    
    grf->mode = GRFILE_MODE_READ;
    
    return grf;
}

GrFILE *grfile_openw(const char *fname)
{
    GrFILE *grf;
    
    grf = grfile_new(fname);
    
    if (!grf) {
        return NULL;
    }
    
    grf->fp = fopen(fname, "wb");
    if (!grf->fp) {
        grfile_free(grf);
        return NULL;
    }
    
    grf->mode = GRFILE_MODE_WRITE;
    
    return grf;
}

int grfile_close(GrFILE *grf)
{
    if (grf && grf->fp) {
        /* Never close standard streams */
        if (grf->fp != stdin && grf->fp != stderr && grf->fp != stdout) {
            fclose(grf->fp);
            grf->fp = NULL;
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

time_t grfile_get_mtime(const GrFILE *grf)
{
    struct stat statb;
    
    if (grf && grf->fname && !stat(grf->fname, &statb)) {
        return statb.st_mtime;
    } else {
        return 0;
    }
}
