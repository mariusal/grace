/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
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

#include <string.h>

#include "grace/coreP.h"

ss_data *ssd_data_new(void)
{
    ss_data *ssd;
    
    ssd = xmalloc(sizeof(ss_data));
    if (ssd) {
        memset(ssd, 0, sizeof(ss_data));
    }
    
    return ssd;
}

void ssd_data_free(ss_data *ssd)
{
    if (ssd) {
        int i, j;
        char **sp;

        for (i = 0; i < ssd->ncols; i++) {
            if (ssd->formats[i] == FFORMAT_STRING) {
                sp = (char **) ssd->data[i];
                for (j = 0; j < ssd->nrows; j++) {
                    xfree(sp[j]);
                }
            }
            xfree(ssd->data[i]);
        }
        xfree(ssd->data);
        xfree(ssd->formats);
        xfree(ssd->label);
        
        xfree(ssd);
    }
}

ss_data *ssd_data_copy(ss_data *ssd)
{
    /* FIXME! */
    errmsg("NIY");
    return NULL;
}

ss_data *ssd_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorSSD) {
        return (ss_data *) q->data;
    } else {
        return NULL;
    }
}

Quark *ssd_new(Quark *q)
{
    Quark *ss; 
    ss = quark_new(q, QFlavorSSD);
    return ss;
}

int ssd_get_ncols(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        return ssd->ncols;
    } else {
        return 0;
    }
}

int ssd_get_nrows(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        return ssd->nrows;
    } else {
        return 0;
    }
}

int *ssd_get_formats(const Quark *q)
{
    ss_data *ssd = ssd_get_data(q);
    if (ssd) {
        return ssd->formats;
    } else {
        return NULL;
    }
}
