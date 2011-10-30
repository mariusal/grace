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

/* Core API - private part */

#ifndef __COREP_H_
#define __COREP_H_

#include <grace/core.h>

typedef struct {
    Quark_cb cb;
    void *cbdata;
} QuarkCBEntry;

struct _QuarkFactory {
    unsigned int refcount;
    
    QuarkFlavor *qflavours;
    unsigned int nflavours;
    
    AMem *amem;
    unsigned int cbcount;    /* user-supplied callbacks */
    QuarkCBEntry *cblist;

    void *udata;
};

struct _Quark {
    AMem *amem;
    
    QuarkFactory *qfactory;
    
    unsigned int fid;
    char *idstr;
    
    int active;              /* on/off flag             */
    
    struct _Quark *parent;
    Storage *children;
    unsigned int refcount;

    unsigned int dirtystate;
    unsigned int statestamp;
    
    void *data;              /* the actual payload      */
    
    unsigned int cbcount;    /* user-supplied callbacks */
    QuarkCBEntry *cblist;
    
    void *udata;             /* user data               */
};

Project *project_data_new(AMem *amem);
void project_data_free(AMem *amem, Project *pr);

ss_data *ssd_data_new(AMem *amem);
void ssd_data_free(AMem *amem, ss_data *ssd);
ss_data *ssd_data_copy(AMem *amem, ss_data *ssd);

frame *frame_data_new(AMem *amem);
void frame_data_free(AMem *amem, frame *f);
frame *frame_data_copy(AMem *amem, frame *f);

graph *graph_data_new(AMem *amem);
void graph_data_free(AMem *amem, graph *g);
graph *graph_data_copy(AMem *amem, graph *g);

tickmarks *axisgrid_data_new(AMem *amem);
tickmarks *axisgrid_data_copy(AMem *amem, tickmarks *t);
void axisgrid_data_free(AMem *amem, tickmarks *t);

Axis *axis_data_new(AMem *amem);
Axis *axis_data_copy(AMem *amem, Axis *a);
void axis_data_free(AMem *amem, Axis *a);

set *set_data_new(AMem *amem);
void set_data_free(AMem *amem, set *p);
set *set_data_copy(AMem *amem, set *p);

region *region_data_new(AMem *amem);
void region_data_free(AMem *amem, region *r);
region *region_data_copy(AMem *amem, region *r);

DObject *object_data_new(AMem *amem);
void object_data_free(AMem *amem, DObject *o);
DObject *object_data_copy(AMem *amem, DObject *o);

AText *atext_data_new(AMem *amem);
void atext_data_free(AMem *amem, AText *at);
AText *atext_data_copy(AMem *amem, AText *at);


void set_default_arrow(Arrow *arrowp);

#endif /* __COREP_H_ */
