/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2004 Grace Development Team
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

struct _QuarkFactory {
    unsigned int refcount;
    
    QuarkFlavor *qflavours;
    unsigned int nflavours;
    
    void *udata;
};

struct _Quark {
    QuarkFactory *qfactory;
    
    unsigned int fid;
    char *idstr;
    
    int active;
    
    struct _Quark *parent;
    Storage *children;
    unsigned int dirtystate;
    unsigned int refcount;
    
    void *data;
    
    Quark_cb cb;
    void *cbdata;
    
    void *udata;
};

Project *project_data_new(void);
void project_data_free(Project *pr);

ss_data *ssd_data_new(void);
void ssd_data_free(ss_data *ssd);
ss_data *ssd_data_copy(ss_data *ssd);

frame *frame_data_new(void);
void frame_data_free(frame *f);
frame *frame_data_copy(frame *f);

graph *graph_data_new(void);
void graph_data_free(graph *g);
graph *graph_data_copy(graph *g);

tickmarks *axisgrid_data_new(void);
tickmarks *axisgrid_data_copy(tickmarks *t);
void axisgrid_data_free(tickmarks *t);

Axis *axis_data_new(void);
Axis *axis_data_copy(Axis *a);
void axis_data_free(Axis *a);

set *set_data_new(void);
void set_data_free(set *p);
set *set_data_copy(set *p);

region *region_data_new(void);
void region_data_free(region *r);
region *region_data_copy(region *r);

DObject *object_data_new(void);
void object_data_free(DObject *o);
DObject *object_data_copy(DObject *o);

AText *atext_data_new(void);
void atext_data_free(AText *at);
AText *atext_data_copy(AText *at);


void set_default_arrow(Arrow *arrowp);

#endif /* __COREP_H_ */
