/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2002,2003 Grace Development Team
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

#include <stdlib.h>
#include <string.h>

#include "grace.h"

static Quark *quark_new_raw(Quark *parent, unsigned int fid, void *data)
{
    Quark *q;

    q = xmalloc(sizeof(Quark));
    if (q) {
        memset(q, 0, sizeof(Quark));
        
        q->fid = fid;
        q->data = data;
        
        if (parent) {
            q->parent = parent;
            q->grace = parent->grace;
            parent->refcount++;
        }
    }
    
    return q;
}

Quark *quark_root(Grace *grace, unsigned int fid)
{
    Quark *q;
    QuarkFlavor *qf;
    void *data;
    
    qf = quark_flavor_get(grace, fid);
    
    data = qf->data_new();
    q = quark_new_raw(NULL, fid, data);
    q->grace = grace;
    
    return q;
}

Quark *quark_new(Quark *parent, unsigned int fid)
{
    Quark *q;
    QuarkFlavor *qf;
    void *data;
    
    qf = quark_flavor_get(parent->grace, fid);
    
    if (!qf) {
        return NULL;
    }
    
    data = qf->data_new();
    q = quark_new_raw(parent, fid, data);
    
    return q;
}

void quark_free(Quark *q)
{
    if (q) {
        QuarkFlavor *qf;
        Quark *parent = q->parent;
        if (parent) {
            parent->refcount--;
            quark_dirtystate_set(parent, TRUE);
        }
        
        qf = quark_flavor_get(q->grace, q->fid);
        if (q->cb) {
            q->cb(q, QUARK_ETYPE_DELETE, q->cbdata);
        }
        qf->data_free(q->data);
        
        if (q->refcount == 0) {
            xfree(q->idstr);
            xfree(q);
        } else {
            errmsg("Tried freeing a referenced quark!");
        }
    }
}

Quark *quark_copy(const Quark *q)
{
    Quark *new;
    QuarkFlavor *qf;
    void *data;
    
    qf = quark_flavor_get(q->grace, q->fid);
    data = qf->data_copy(q->data);
    new = quark_new_raw(q->parent, q->fid, data);
    
    return new;
}

void quark_dirtystate_set(Quark *q, int flag)
{
    if (flag) {
        q->dirtystate++;
        if (q->parent) {
            quark_dirtystate_set(q->parent, TRUE);
        }
    } else {
        q->dirtystate = FALSE;
    }
}

int quark_dirtystate_get(const Quark *q)
{
    return q->dirtystate;
}

void quark_idstr_set(Quark *q, const char *s)
{
    q->idstr = copy_string(q->idstr, s);
}

char *quark_idstr_get(const Quark *q)
{
    return q->idstr;
}

int quark_cb_set(Quark *q, Quark_cb cb, void *cbdata)
{
    if (q) {
        q->cb     = cb;
        q->cbdata = cbdata;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
