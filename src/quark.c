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

static void quark_storage_free(void *data)
{
    quark_free((Quark *) data);
}

static Quark *quark_new_raw(Quark *parent, unsigned int fid, void *data)
{
    Quark *q;

    q = xmalloc(sizeof(Quark));
    if (q) {
        memset(q, 0, sizeof(Quark));
        
        q->fid = fid;
        q->data = data;
        
        q->children = storage_new(quark_storage_free, NULL, NULL);
        
        if (!q->children) {
            xfree(q);
            return NULL;
        }
        
        if (parent) {
            q->parent = parent;
            q->grace = parent->grace;
            parent->refcount++;
            storage_add(parent->children, q);
            
            quark_dirtystate_set(parent, TRUE);
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
    
    if (!parent) {
        return NULL;
    }
    
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
            storage_extract_data(parent->children, q);
            quark_dirtystate_set(parent, TRUE);
        }
        
        qf = quark_flavor_get(q->grace, q->fid);
        if (q->cb) {
            q->cb(q, QUARK_ETYPE_DELETE, q->cbdata);
        }
        if (parent) {
            parent->refcount--;
        }
        
        storage_free(q->children);
        
        qf->data_free(q->data);
        xfree(q->idstr);
        if (q->refcount != 0) {
            errmsg("Freed a referenced quark!");
        }
        xfree(q);
    }
}

Quark *quark_parent_get(const Quark *q)
{
    if (q) {
        return q->parent;
    } else {
        return NULL;
    }
}

Quark *quark_copy2(Quark *newparent, const Quark *q);

static int copy_hook(unsigned int step, void *data, void *udata)
{
    Quark *child = (Quark *) data;
    Quark *newparent = (Quark *) udata;
    Quark *newchild;

    newchild = quark_copy2(newparent, child);
        
    return TRUE;
}

Quark *quark_copy2(Quark *newparent, const Quark *q)
{
    Quark *new;
    QuarkFlavor *qf;
    void *data;
    
    qf = quark_flavor_get(q->grace, q->fid);
    data = qf->data_copy(q->data);
    new = quark_new_raw(newparent, q->fid, data);

    storage_traverse(q->children, copy_hook, new);
    
    return new;
}

Quark *quark_copy(const Quark *q)
{
    return quark_copy2(q->parent, q);
}

void quark_dirtystate_set(Quark *q, int flag)
{
    if (flag) {
        q->dirtystate++;
        if (q->cb) {
            q->cb(q, QUARK_ETYPE_MODIFY, q->cbdata);
        }
        if (q->parent) {
            quark_dirtystate_set(q->parent, TRUE);
        }
    } else {
        q->dirtystate = 0;
        /* FIXME: for i in q->children do i->dirtystate = 0 */
    }
}

int quark_dirtystate_get(const Quark *q)
{
    return q->dirtystate;
}

void quark_idstr_set(Quark *q, const char *s)
{
    if (q) {
        q->idstr = copy_string(q->idstr, s);
    }
}

char *quark_idstr_get(const Quark *q)
{
    return q->idstr;
}

typedef struct {
    char *s;
    Quark *child;
} QTFindHookData;

static int find_hook(unsigned int step, void *data, void *udata)
{
    Quark *q = (Quark *) data;
    QTFindHookData *_cbdata = (QTFindHookData *) udata;
    
    if (compare_strings(q->idstr, _cbdata->s)) {
        _cbdata->child = q;
        return FALSE;
    } else {
        return TRUE;
    }
}

Quark *quark_find_child_by_idstr(Quark *q, const char *s)
{
    QTFindHookData _cbdata;
    _cbdata.s = (char *) s;
    _cbdata.child = NULL;
    if (q && s) {
        storage_traverse(q->children, find_hook, &_cbdata);
        
    }
    return _cbdata.child;
}

static int find_hook2(Quark *q,
    void *udata, QTraverseClosure *closure)
{
    QTFindHookData *_cbdata = (QTFindHookData *) udata;
    
    if (compare_strings(q->idstr, _cbdata->s)) {
        _cbdata->child = q;
        return FALSE;
    } else {
        return TRUE;
    }
}

Quark *quark_find_descendant_by_idstr(Quark *q, const char *s)
{
    QTFindHookData _cbdata;
    _cbdata.s = (char *) s;
    _cbdata.child = NULL;
    if (q && s) {
        quark_traverse(q, find_hook2, &_cbdata);
    }
    return _cbdata.child;
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

typedef struct {
    unsigned int depth;
    unsigned int step;
    int done;
    Quark_traverse_hook hook;
    void *udata;
} QTHookData;

static int _quark_traverse(Quark *q, QTHookData *_cbdata);

static int hook(unsigned int step, void *data, void *udata)
{
    Quark *q = (Quark *) data;
    QTHookData *_cbdata = (QTHookData *) udata;
    
    _cbdata->step = step;
    
    return _quark_traverse(q, _cbdata);
}

static int _quark_traverse(Quark *q, QTHookData *_cbdata)
{
    int res;
    QTraverseClosure closure;
    
    closure.depth   = _cbdata->depth;
    closure.step    = _cbdata->step;
    closure.post    = FALSE;
    closure.descend = TRUE;
    
    res = _cbdata->hook(q, _cbdata->udata, &closure);
    if (res) {
        _cbdata->depth++;

        if (closure.descend) {
            storage_traverse(q->children, hook, _cbdata);
        }
        
        if (closure.post) {
            res = _cbdata->hook(q, _cbdata->udata, &closure);
        }
        
        if (_cbdata->done) {
            return FALSE;
        } else {
            return TRUE;
        }
    } else {
        _cbdata->done = TRUE;
        return FALSE;
    }
}

void quark_traverse(Quark *q, Quark_traverse_hook hook, void *udata)
{
    QTHookData _cbdata;

    if (!q || !hook) {
        return;
    }
    
    _cbdata.depth = 0;
    _cbdata.step  = 0;
    _cbdata.done  = FALSE;
    _cbdata.hook  = hook;
    _cbdata.udata = udata;
    
    _quark_traverse(q, &_cbdata);
}

int quark_count_children(const Quark *q)
{
    return storage_count(q->children);
}

int quark_child_exist(const Quark *parent, const Quark *child)
{
    if (parent && child         &&
        child->parent == parent &&
        storage_data_exists(parent->children, child)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int quark_reparent(Quark *q, Quark *newparent)
{
    Quark *parent;
    if (!q || !newparent || q == newparent) {
        return RETURN_FAILURE;
    }
    
    parent = q->parent;
    if (parent == newparent) {
        return RETURN_SUCCESS;
    } else {
        storage_extract_data(parent->children, q);
        
        parent->refcount--;
        quark_dirtystate_set(parent, TRUE);
        
        q->parent = newparent;
        newparent->refcount++;
        storage_add(newparent->children, q);
        quark_dirtystate_set(newparent, TRUE);
        
        return RETURN_SUCCESS;
    }
}

static int reparent_hook(unsigned int step, void *data, void *udata)
{
    Quark *child = (Quark *) data;
    Quark *newparent = (Quark *) udata;

    quark_reparent(child, newparent);
    
    return TRUE;
}

int quark_reparent_children(Quark *parent, Quark *newparent)
{
    storage_traverse(parent->children, reparent_hook, newparent);
    
    return RETURN_SUCCESS;
}


int quark_move(const Quark *q, int forward)
{
    Storage *sto = q->parent->children;
    if (storage_scroll_to_data(sto, q) == RETURN_SUCCESS) {
        return storage_move(sto, forward);
    } else {
        return RETURN_FAILURE;
    }
}

int quark_push(const Quark *q, int forward)
{
    Storage *sto = q->parent->children;
    if (storage_scroll_to_data(sto, q) == RETURN_SUCCESS) {
        return storage_push(sto, forward);
    } else {
        return RETURN_FAILURE;
    }
}

typedef struct {
    Quark_comp_proc fcomp;
    void *udata;
} quark_comp_t;

static int _quark_fcomp(const void *d1, const void *d2, void *udata)
{
    Quark *q1 = (Quark *) d1, *q2 = (Quark *) d2;
    quark_comp_t *qc = (quark_comp_t *) udata;
    return qc->fcomp(q1, q2, qc->udata);
}

int quark_sort_children(Quark *q, Quark_comp_proc fcomp, void *udata)
{
    quark_comp_t qc;
    
    qc.fcomp = fcomp;
    qc.udata = udata;
    
    return storage_sort(q->children, _quark_fcomp, (void *) &qc);
}
