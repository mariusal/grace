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

#endif /* __COREP_H_ */
