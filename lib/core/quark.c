/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2002-2004 Grace Development Team
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
    QuarkFactory *qfactory;
    
    qfactory = xmalloc(sizeof(QuarkFactory));
    if (qfactory) {
        memset(qfactory, 0, sizeof(QuarkFactory));
    }
    
    return qfactory;
}

void qfactory_free(QuarkFactory *qfactory)
{
    if (qfactory) {
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

static void quark_storage_free(void *data)
{
    quark_free((Quark *) data);
}

static Quark *quark_new_raw(Quark *parent, unsigned int fid, void *data)
{
    Quark *q;

    q = xmalloc(sizeof(Quark));
    if (q) {
        char buf[32];
        memset(q, 0, sizeof(Quark));
        
        q->fid = fid;
        q->data = data;
        
        q->active = TRUE;
        
        q->children = storage_new(quark_storage_free, NULL, NULL);
        
        if (!q->children) {
            xfree(q);
            return NULL;
        }
        
        sprintf(buf, "%p", (void *) q);
        quark_idstr_set(q, buf);
        
        if (parent) {
            q->parent   = parent;
            q->qfactory = parent->qfactory;
            parent->refcount++;
            storage_add(parent->children, q);
            
            quark_dirtystate_set(parent, TRUE);
        }
    }
    
    return q;
}

Quark *quark_root(QuarkFactory *qfactory, unsigned int fid)
{
    Quark *q;
    QuarkFlavor *qf;
    void *data;
    
    qf = quark_flavor_get(qfactory, fid);
    
    data = qf->data_new();
    q = quark_new_raw(NULL, fid, data);
    q->qfactory = qfactory;
    
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
    
    qf = quark_flavor_get(parent->qfactory, fid);
    
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
        
        qf = quark_flavor_get(q->qfactory, q->fid);
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

int quark_set_udata(Quark *q, void *udata)
{
    if (q) {
        q->udata = udata;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void *quark_get_udata(const Quark *q)
{
    if (q) {
        return q->udata;
    } else {
        return NULL;
    }
}

QuarkFactory *quark_get_qfactory(const Quark *q)
{
    if (q) {
        return q->qfactory;
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
    
    qf = quark_flavor_get(q->qfactory, q->fid);
    data = qf->data_copy(q->data);
    new = quark_new_raw(newparent, q->fid, data);
    new->active = q->active;
    if (newparent != q->parent) {
        quark_idstr_set(new, q->idstr);
    }

    storage_traverse(q->children, copy_hook, new);
    
    return new;
}

Quark *quark_copy(const Quark *q)
{
    return quark_copy2(q->parent, q);
}

static int dirtystate_hook(unsigned int step, void *data, void *udata)
{
    Quark *q = (Quark *) data;

    quark_dirtystate_set(q, FALSE);
        
    return TRUE;
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
        storage_traverse(q->children, dirtystate_hook, NULL);
    }
}

int quark_dirtystate_get(const Quark *q)
{
    return q->dirtystate;
}

int quark_set_active(Quark *q, int onoff)
{
    if (q) {
        if (q->active != onoff) {
            q->active = onoff;
            quark_dirtystate_set(q, TRUE);
        }
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int quark_is_active(const Quark *q)
{
    return q->active;
}

int quark_idstr_set(Quark *q, const char *s)
{
    if (q) {
        q->idstr = copy_string(q->idstr, s);
        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *quark_idstr_get(const Quark *q)
{
    return q->idstr;
}

int quark_fid_set(Quark *q, int fid)
{
    if (q) {
        q->fid = fid;
        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int quark_fid_get(const Quark *q)
{
    return q->fid;
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

        if (closure.descend) {
            _cbdata->depth++;
            
            storage_traverse(q->children, hook, _cbdata);
            
            _cbdata->depth--;
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

Storage *quark_get_children(const Quark *q)
{
    return q->children;
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
        if (q->cb) {
            q->cb(q, QUARK_ETYPE_REPARENT, q->cbdata);
        }
        
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
        quark_dirtystate_set(q->parent, TRUE);
        return storage_move(sto, forward);
    } else {
        return RETURN_FAILURE;
    }
}

int quark_push(const Quark *q, int forward)
{
    Storage *sto = q->parent->children;
    if (storage_scroll_to_data(sto, q) == RETURN_SUCCESS) {
        quark_dirtystate_set(q->parent, TRUE);
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

int quark_is_first_child(const Quark *q)
{
    Quark *parent = quark_parent_get(q);
    if (parent) {
        Storage *sto = parent->children;
        void *p;
        storage_rewind(sto);
        storage_get_data(sto, &p);
        if (p == (void *) q) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}

int quark_is_last_child(const Quark *q)
{
    Quark *parent = quark_parent_get(q);
    if (parent) {
        Storage *sto = parent->children;
        void *p;
        storage_eod(sto);
        storage_get_data(sto, &p);
        if (p == (void *) q) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}
