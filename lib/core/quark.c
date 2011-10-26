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

#define ADVANCED_MEMORY_HANDLERS
#include "grace/coreP.h"

static void quark_storage_free(AMem *amem, void *data)
{
    quark_free((Quark *) data);
}

static void quark_call_cblist(Quark *q, int etype)
{
    unsigned int i, cbcount;
    QuarkCBEntry *cblist;
    QuarkFactory *qf = qfactory_new();

    cbcount = qf->cbcount;
    cblist = qf->cblist;
    for (i = 0; i < cbcount; i++) {
        QuarkCBEntry *cbentry = &cblist[i];
        cbentry->cb(q, etype, cbentry->cbdata);
    }

    cbcount = q->cbcount;
    cblist = q->cblist;
    for (i = 0; i < cbcount; i++) {
        QuarkCBEntry *cbentry = &cblist[i];
        cbentry->cb(q, etype, cbentry->cbdata);
    }
}

static Quark *quark_new_raw(AMem *amem,
    Quark *parent, unsigned int fid, void *data)
{
    Quark *q;

    q = amem_malloc(amem, sizeof(Quark));
    if (q) {
        char buf[32];
        memset(q, 0, sizeof(Quark));
        
        q->amem = amem;
        
        q->fid = fid;
        q->data = data;
        
        q->active = TRUE;
        
        q->children = storage_new(amem, quark_storage_free, NULL, NULL);
        
        if (!q->children) {
            amem_free(amem, q);
            return NULL;
        }
        
        q->qfactory = qfactory_new();

        if (parent) {
            q->parent   = parent;
            q->qfactory = parent->qfactory;
            parent->refcount++;
            storage_add(parent->children, q);
            
            quark_dirtystate_set(parent, TRUE);
        }

        quark_call_cblist(q, QUARK_ETYPE_NEW);

        sprintf(buf, "%p", (void *) q);
        quark_idstr_set(q, buf);
    }
    
    return q;
}

Quark *quark_root(int mmodel, QuarkFactory *qfactory, unsigned int fid)
{
    AMem *amem;
    Quark *q;
    QuarkFlavor *qf;
    void *data;
    
    amem = amem_amem_new(mmodel);
    
    qf = quark_flavor_get(qfactory, fid);
    
    data = qf->data_new(amem);
    q = quark_new_raw(amem, NULL, fid, data);
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
    
    data = qf->data_new(parent->amem);
    q = quark_new_raw(parent->amem, parent, fid, data);
    
    return q;
}

void quark_free(Quark *q)
{
    if (q) {
        QuarkFlavor *qf;
        Quark *parent = q->parent;
        AMem *amem = q->amem;
        
        if (parent) {
            storage_extract_data(parent->children, q);
            quark_dirtystate_set(parent, TRUE);
        }
        
        qf = quark_flavor_get(q->qfactory, q->fid);

        if (parent) {
            parent->refcount--;
        }
        
        storage_free(q->children);
        
        quark_call_cblist(q, QUARK_ETYPE_DELETE);

        qf->data_free(amem, q->data);
        amem_free(amem, q->idstr);
        if (q->refcount != 0) {
            errmsg("Freed a referenced quark!");
        }
        amem_free(amem, q->cblist);
        amem_free(amem, q);
        if (!parent) {
            /* Root quark -> clean up memory allocator */
            amem_amem_free(amem);
        }
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

AMem *quark_get_amem(const Quark *q)
{
    if (q) {
        return q->amem;
    } else {
        return NULL;
    }
}

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
    data = qf->data_copy(q->amem, q->data);
    new = quark_new_raw(newparent->amem, newparent, q->fid, data);
    new->active = q->active;

    new->cblist = amem_malloc(new->amem, q->cbcount*sizeof(QuarkCBEntry));
    if (q->cbcount && !new->cblist) {
        quark_free(new);
        return NULL;
    }
    memcpy(new->cblist, q->cblist, q->cbcount*sizeof(QuarkCBEntry));
    new->cbcount = q->cbcount;
    
    new->udata  = q->udata;

    if (newparent != q->parent) {
        quark_idstr_set(new, q->idstr);
        quark_call_cblist(new, QUARK_ETYPE_REPARENT);
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
        q->statestamp++;
        if (q->parent) {
            quark_dirtystate_set(q->parent, TRUE);
        }
    } else {
        q->dirtystate = 0;
        storage_traverse(q->children, dirtystate_hook, NULL);
    }
    quark_call_cblist(q, QUARK_ETYPE_MODIFY);
}

int quark_dirtystate_get(const Quark *q)
{
    return q->dirtystate;
}

unsigned int quark_get_statestamp(const Quark *q)
{
    return q->statestamp;
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
    if (q) {
        return q->active;
    } else {
        return FALSE;
    }
}

int quark_idstr_set(Quark *q, const char *s)
{
    if (q) {
        q->idstr = amem_strcpy(q->amem, q->idstr, s);
        quark_dirtystate_set(q, TRUE);
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

char *quark_idstr_get(const Quark *q)
{
    if (q) {
        return q->idstr;
    } else {
        return NULL;
    }
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
    
    if (strings_are_equal(q->idstr, _cbdata->s)) {
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
    
    if (strings_are_equal(q->idstr, _cbdata->s)) {
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

int quark_cb_add(Quark *q, Quark_cb cb, void *cbdata)
{
    static AMem *cblistamem;
    QuarkFactory *qf = qfactory_new();

    if (q) {
        void *p = amem_realloc(q->amem, q->cblist,
            (q->cbcount + 1)*sizeof(QuarkCBEntry));
        if (p) {
            QuarkCBEntry *cbentry;
            q->cblist = p;
            q->cbcount++;
            cbentry = &q->cblist[q->cbcount - 1];
            cbentry->cb     = cb;
            cbentry->cbdata = cbdata;
            
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
    } else {
        if (!cblistamem) {
            cblistamem = amem_amem_new(AMEM_MODEL_SIMPLE);
        }
        void *p = amem_realloc(cblistamem, qf->cblist,
            (qf->cbcount + 1)*sizeof(QuarkCBEntry));
        if (p) {
            QuarkCBEntry *cbentry;
            qf->cblist = p;
            qf->cbcount++;
            cbentry = &qf->cblist[qf->cbcount - 1];
            cbentry->cb     = cb;
            cbentry->cbdata = cbdata;

            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
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
        quark_call_cblist(q, QUARK_ETYPE_REPARENT);
        
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
