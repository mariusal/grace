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

#include "core_utils.h"
#include "files.h"
#include "ssdata.h"
#include "parser.h"

#include "protos.h"

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

    *nscols = 0;
    *nncols = 0;
    *formats = NULL;
    df_pref = get_date_hint();
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
    
    df_pref = get_date_hint();
    for (i = 0; i < ncols; i++) {
        ss_column *pcol = ssd_get_col(q, i);
        s = next_token(s, &token, &quoted);
        if (s == NULL || token == NULL) {
            /* invalid line */
            return RETURN_FAILURE;
        } else {
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

/*
 * return the next available set in graph gr
 * If target is allocated but with no data, choose it (used for loading sets
 * from project files when sets aren't packed)
 */
static Quark *nextset(Quark *ss)
{
    Quark *pset;
    RunTime *rt = rt_from_quark(ss);
    
    if (!rt) {
        return NULL;
    }
    
    pset = rt->target_set;
    
    if (pset && set_is_dataless(pset)) {
        rt->target_set = NULL;
        quark_reparent(pset, ss);
        return pset;
    } else {
        
        return grace_set_new(ss);
    }
}

int store_data(Quark *q, int load_type)
{
    int ncols, nncols, nncols_req, nscols, nrows;
    int i, j;
    unsigned int coli[MAX_SET_COLS];
    int scol;
    Quark *gr, *pset;
    Quark *pr = get_parent_project(q); 
    RunTime *rt = rt_from_quark(pr);
    ss_data *ssd = ssd_get_data(q);
    
    if (ssd == NULL || rt == NULL) {
        return RETURN_FAILURE;
    }
    ncols = ssd_get_ncols(q);
    nrows = ssd_get_nrows(q);
    if (ncols <= 0 || nrows <= 0) {
        return RETURN_FAILURE;
    }

    nncols = 0;
    for (j = 0; j < ncols; j++) {
        ss_column *pcol = ssd_get_col(q, j);
        if (pcol->format != FFORMAT_STRING) {
            nncols++;
        }
    }
    nscols = ncols - nncols;
    
    gr = get_parser_gno();
    if (!gr) {
        return RETURN_FAILURE;
    }
    
    if (quark_reparent(q, gr) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    switch (load_type) {
    case LOAD_SINGLE:
        if (nscols > 1) {
            errmsg("Can not use more than one column of strings per set");
            return RETURN_FAILURE;
        }

        nncols_req = settype_cols(rt->curtype);
        if (nncols_req != nncols) {
	    errmsg("Column count incorrect");
	    return RETURN_FAILURE;
        }

        pset = nextset(q);
        
        nncols = 0;
        scol = -1;
        for (j = 0; j < ncols; j++) {
            ss_column *pcol = ssd_get_col(q, j);
            if (pcol->format == FFORMAT_STRING) {
                scol = j;
            } else {
                coli[nncols] = j;
                nncols++;
            }
        }

        create_set_fromblock(pset, rt->curtype, nncols, coli, scol);
        
        break;
    case LOAD_NXY:
        if (nscols != 0) {
            errmsg("Can not yet use strings when reading in data as NXY");
            return RETURN_FAILURE;
        }
        
        for (i = 0; i < ncols - 1; i++) {
            pset = grace_set_new(q);
            if (!pset) {
                return RETURN_FAILURE;
            }

            coli[0] = 0;
            coli[1] = i + 1;
            create_set_fromblock(pset, rt->curtype, 2, coli, -1);
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

int create_set_fromblock(Quark *pset, int type,
    unsigned int nc, unsigned int *coli, int scol)
{
    Dataset *dsp = set_get_dataset(pset);
    Quark *ss = get_parent_ssd(pset);
    ss_data *blockdata = ssd_get_data(ss);
    int i, ncols, blockncols, blocklen, column;

    if (!blockdata || !dsp) {
        return RETURN_FAILURE;
    }

    blockncols = ssd_get_ncols(ss);
    blocklen = ssd_get_nrows(ss);
    
    ncols = settype_cols(type);
    if (nc > ncols) {
        errmsg("Too many columns scanned in column string");
        return RETURN_FAILURE;
    }
    if (nc < ncols) {
	errmsg("Too few columns scanned in column string");
	return RETURN_FAILURE;
    }
    
    for (i = 0; i < nc; i++) {
	if (coli[i] >= blockncols) {
	    errmsg("Column index out of range");
	    return RETURN_FAILURE;
	}
    }
    
    if (scol >= blockncols) {
	errmsg("String column index out of range");
	return RETURN_FAILURE;
    }

    /* clear data stored in the set, if any */
    killsetdata(pset);
    
    set_set_type(pset, type);

    for (i = 0; i < nc; i++) {
        column = coli[i];
        if (ssd_get_col_format(ss, column) != FFORMAT_STRING) {
            dsp->cols[i] = column;
        } else {
            errmsg("Tried to read doubles from strings!");
            killsetdata(pset);
            return RETURN_FAILURE;
        }
    }

    /* strings, if any */
    if (scol >= 0) {
        if (ssd_get_col_format(ss, scol) != FFORMAT_STRING) {
            errmsg("Tried to read strings from doubles!");
            killsetdata(pset);
            return RETURN_FAILURE;
        } else {
            dsp->scol = scol;
        }
    }

    return RETURN_SUCCESS;
}
