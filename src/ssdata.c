/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2005 Grace Development Team
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
 *
 * spreadsheet data stuff
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "core_utils.h"
#include "files.h"
#include "ssdata.h"

#include "xprotos.h"

double *copy_data_column_simple(double *src, int nrows)
{
    double *dest;
    
    if (!src) {
        return NULL;
    }
    
    dest = xmalloc(nrows*SIZEOF_DOUBLE);
    if (dest != NULL) {
        memcpy(dest, src, nrows*SIZEOF_DOUBLE);
    }
    return dest;
}

/* TODO: index_shift */
double *allocate_index_data(int nrows)
{
    int i;
    double *retval;
    
    retval = xmalloc(nrows*SIZEOF_DOUBLE);
    if (retval != NULL) {
        for (i = 0; i < nrows; i++) {
            retval[i] = i;
        }
    }
    return retval;
}

double *allocate_mesh(double start, double stop, int len)
{
    int i;
    double *retval;
    
    retval = xmalloc(len*SIZEOF_DOUBLE);
    if (retval != NULL) {
        double s = (start + stop)/2, d = (stop - start)/2;
        for (i = 0; i < len; i++) {
            retval[i] = s + d*((double) (2*i + 1 - len)/(len - 1));
        }
    }
    return retval;
}

int field_string_to_cols(const char *fs, int *nc, int **cols, int *scol)
{
    int col;
    char *s, *buf;

    buf = copy_string(NULL, fs);
    if (buf == NULL) {
        return RETURN_FAILURE;
    }

    s = buf;
    *nc = 0;
    while ((s = strtok(s, ":")) != NULL) {
	(*nc)++;
	s = NULL;
    }
    *cols = xmalloc((*nc)*SIZEOF_INT);
    if (*cols == NULL) {
        xfree(buf);
        return RETURN_FAILURE;
    }

    strcpy(buf, fs);
    s = buf;
    *nc = 0;
    *scol = -1;
    while ((s = strtok(s, ":")) != NULL) {
        int strcol;
        if (*s == '{') {
            char *s1;
            strcol = TRUE;
            s++;
            if ((s1 = strchr(s, '}')) != NULL) {
                *s1 = '\0';
            }
        } else {
            strcol = FALSE;
        }
        col = atoi(s);
        col--;
        if (strcol) {
            *scol = col;
        } else {
            (*cols)[*nc] = col;
	    (*nc)++;
        }
	s = NULL;
    }
    
    xfree(buf);
    
    return RETURN_SUCCESS;
}

char *cols_to_field_string(int nc, unsigned int *cols, int scol)
{
    int i;
    char *s, buf[32];
    
    s = NULL;
    for (i = 0; i < nc; i++) {
        sprintf(buf, "%d", cols[i] + 1);
        if (i != 0) {
            s = concat_strings(s, ":");
        }
        s = concat_strings(s, buf);
    }
    if (scol >= 0) {
        sprintf(buf, ":{%d}", scol + 1);
        s = concat_strings(s, buf);
    }
    
    return s;
}


static char *next_token(char *s, char **token, int *quoted)
{
    *quoted = FALSE;
    *token = NULL;
    
    if (s == NULL) {
        return NULL;
    }
    
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    if (*s == '"') {
        s++;
        *token = s;
        while (*s != '\0' && (*s != '"' || (*s == '"' && *(s - 1) == '\\'))) {
            s++;
        }
        if (*s == '"') {
            /* successfully identified a quoted string */
            *quoted = TRUE;
        }
    } else {
        *token = s;
        if (**token == '\n') {
            /* EOL reached */
            return NULL;
        }
        while (*s != '\n' && *s != '\0' && *s != ' ' && *s != '\t') {
            s++;
        }
    }
    
    if (*s != '\0') {
        *s = '\0';
        s++;
        return s;
    } else {
        return NULL;
    }
}

int parse_ss_row(Quark *pr, const char *s, int *nncols, int *nscols, int **formats)
{
    int ncols;
    int quoted;
    char *buf, *s1, *token;
    double value;
    Dates_format df_pref, ddummy;
    const char *sdummy;
    GraceApp *gapp = gapp_from_quark(pr);

    *nscols = 0;
    *nncols = 0;
    *formats = NULL;
    df_pref = get_date_hint(gapp);
    buf = copy_string(NULL, s);
    s1 = buf;
    while ((s1 = next_token(s1, &token, &quoted)) != NULL) {
        if (token == NULL) {
            *nscols = 0;
            *nncols = 0;
            XCFREE(*formats);
            xfree(buf);
            return RETURN_FAILURE;
        }
        
        ncols = *nncols + *nscols;
        /* reallocate the formats array */
        if (ncols % 10 == 0) {
            *formats = xrealloc(*formats, (ncols + 10)*SIZEOF_INT);
        }

        if (quoted) {
            (*formats)[ncols] = FFORMAT_STRING;
            (*nscols)++;
        } else if (parse_date(pr, token, df_pref, FALSE, &value, &ddummy) ==
            RETURN_SUCCESS) {
            (*formats)[ncols] = FFORMAT_DATE;
            (*nncols)++;
        } else if (parse_float(token, &value, &sdummy) == RETURN_SUCCESS) {
            (*formats)[ncols] = FFORMAT_NUMBER;
            (*nncols)++;
        } else {
            /* last resort - treat the field as string, even if not quoted */
            (*formats)[ncols] = FFORMAT_STRING;
            (*nscols)++;
        }
    }
    xfree(buf);
    
    return RETURN_SUCCESS;
}


/* NOTE: the input string will be corrupted! */
int insert_data_row(Quark *q, unsigned int row, char *s)
{
    unsigned int i, ncols = ssd_get_ncols(q);
    char *token;
    int quoted;
    char  **sp;
    double *np;
    Dates_format df_pref, ddummy;
    const char *sdummy;
    int res;
    AMem *amem = quark_get_amem(q);
    Quark *pr = get_parent_project(q); 
    GraceApp *gapp = gapp_from_quark(q);
    
    df_pref = get_date_hint(gapp);
    for (i = 0; i < ncols; i++) {
        ss_column *pcol = ssd_get_col(q, i);
        s = next_token(s, &token, &quoted);
        if (s == NULL || token == NULL) {
            /* invalid line */
            return RETURN_FAILURE;
        } else {
            pcol->hotcol = i;
            if (pcol->format == FFORMAT_STRING) {
                sp = (char **) pcol->data;
                sp[row] = amem_strcpy(amem, sp[row], token);
                if (sp[row] != NULL) {
                    res = RETURN_SUCCESS;
                } else {
                    res = RETURN_FAILURE;
                }
            } else if (pcol->format == FFORMAT_DATE) {
                np = (double *) pcol->data;
                res = parse_date(pr, token, df_pref, FALSE, &np[row], &ddummy);
            } else {
                np = (double *) pcol->data;
                res = parse_float(token, &np[row], &sdummy);
            }
            if (res != RETURN_SUCCESS) {
                return RETURN_FAILURE;
            }
        }
    }
    
    return RETURN_SUCCESS;
}

static int create_set_fromblock(Quark *ss, int type,
    unsigned int nc, const unsigned int *coli, int acol)
{
    int i, ncols, blockncols, column;
    Quark *pset;
    Dataset *dsp;
    ss_data *blockdata = ssd_get_data(ss);

    ncols = settype_cols(type);
    if (nc > ncols) {
        errmsg("Too many columns for this set type");
        return RETURN_FAILURE;
    }
    
    pset = gapp_set_new(ss);
    dsp = set_get_dataset(pset);

    if (!blockdata || !dsp) {
        return RETURN_FAILURE;
    }

    blockncols = ssd_get_ncols(ss);
    
    set_set_type(pset, type);

    dsp->acol = acol;
    for (i = 0; i < nc; i++) {
        column = coli[i];
        if (column < blockncols) {
            if (ssd_get_col_format(ss, column) != FFORMAT_STRING) {
                dsp->cols[i] = column;
            } else {
                errmsg("Tried to read doubles from strings!");
                quark_free(pset);
                return RETURN_FAILURE;
            }
        }
    }

    return RETURN_SUCCESS;
}

int store_data(Quark *q, int load_type, int settype)
{
    int ncols, nncols, nscols, nrows;
    int i, j, acol;
    unsigned int coli[MAX_SET_COLS];
    
    if (load_type == LOAD_BLOCK) {
        return RETURN_SUCCESS;
    }
    
    ncols = ssd_get_ncols(q);
    nrows = ssd_get_nrows(q);
    if (ncols <= 0 || nrows <= 0) {
        return RETURN_FAILURE;
    }

    acol = COL_NONE;
    nncols = 0;
    for (j = 0; j < ncols; j++) {
        ss_column *pcol = ssd_get_col(q, j);
        if (pcol->format != FFORMAT_STRING) {
            if (nncols < MAX_SET_COLS) {
                coli[nncols] = j;
            }
            nncols++;
        } else {
            acol = j;
        }
    }
    nscols = ncols - nncols;
    
    switch (load_type) {
    case LOAD_SINGLE:
        if (nscols > 1) {
            errmsg("Cannot use more than one string column per set");
            return RETURN_FAILURE;
        }

        create_set_fromblock(q, settype, nncols, coli, acol);

        break;
    case LOAD_NXY:
        if (nscols != 0) {
            errmsg("Cannot use strings when reading in data as NXY");
            return RETURN_FAILURE;
        }
        
        for (i = 0; i < ncols - 1; i++) {
            coli[0] = 0;
            coli[1] = i + 1;
            create_set_fromblock(q, SET_XY, 2, coli, COL_NONE);
        }
    
        break;
    case LOAD_BLOCK:
        break;
    default:
        errmsg("Internal error");
        return RETURN_FAILURE;
    }
    
    return RETURN_SUCCESS;
}

int kill_ssd_cb(Quark *q, int etype, void *data)
{
#ifndef NONE_GUI
    if (etype == QUARK_ETYPE_DELETE) {
        unlink_ssd_ui(q);
    }
#endif    
    return RETURN_SUCCESS;
}


Quark *gapp_ssd_new(Quark *parent)
{
    Quark *q; 
    q = ssd_new(parent);
    if (q) {
        quark_cb_add(q, kill_ssd_cb, NULL);
    }
    return q;
}
