/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2002-2004 Grace Development Team
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

#include <stdlib.h>
#include <string.h>

#include "grace/coreP.h"

QuarkFlavor *quark_flavor_get(const QuarkFactory *qfactory, unsigned int fid)
{
    unsigned int i;
    
    if (!qfactory) {
        return NULL;
    }
    
    for (i = 0; i < qfactory->nflavours; i++) {
        QuarkFlavor *qf = &qfactory->qflavours[i];
        if (qf->fid == fid) {
            return qf;
        }
    }
    
    return NULL;
}

QuarkFactory *qfactory_new(void)
{
    static QuarkFactory *qfactory = 0;
    
    if (qfactory) return qfactory;

    qfactory = xmalloc(sizeof(QuarkFactory));
    if (qfactory) {
        memset(qfactory, 0, sizeof(QuarkFactory));
    }
    
    return qfactory;
}

void qfactory_free(QuarkFactory *qfactory)
{
    if (qfactory) {
        xfree(qfactory->cblist);
        xfree(qfactory->qflavours);
        xfree(qfactory);
    }
}

int quark_factory_set_udata(QuarkFactory *qfactory, void *udata)
{
    if (qfactory) {
        qfactory->udata = udata;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void *quark_factory_get_udata(const QuarkFactory *qfactory)
{
    if (qfactory) {
        return qfactory->udata;
    } else {
        return NULL;
    }
}


int quark_flavor_add(QuarkFactory *qfactory, const QuarkFlavor *qf)
{
    void *p;
    
    if (!qfactory || !qf) {
        return RETURN_FAILURE;
    }
    
    p = xrealloc(qfactory->qflavours,
        (qfactory->nflavours + 1)*sizeof(QuarkFlavor));
    if (!p) {
        return RETURN_FAILURE;
    } else {
        qfactory->qflavours = p;
    }
    
    qfactory->qflavours[qfactory->nflavours] = *qf;
    qfactory->nflavours++;
    
    return RETURN_SUCCESS;
}
