#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "utils.h"

static Quark *quark_new_raw(Quark *parent, unsigned int fid, void *data)
{
    Quark *q;

    q = xmalloc(sizeof(Quark));
    if (q) {
        memset(q, 0, sizeof(Quark));
        
        q->fid = fid;
        q->data = data;
        
        q->parent = parent;
        if (parent) {
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
        qf->data_free(q->data);
        
        if (q->refcount == 0) {
            xfree(q);
        } else {
            errmsg("Tried freeing a referenced quark!");
        }
    }
}

void quark_data_free(Quark *q)
{
    if (q) {
        QuarkFlavor *qf;
        
        qf = quark_flavor_get(q->grace, q->fid);
        if (q->data) {
            quark_dirtystate_set(q, TRUE);
            qf->data_free(q->data);
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
