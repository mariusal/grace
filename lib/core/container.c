/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2003 Grace Development Team
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

#define ADVANCED_MEMORY_HANDLERS
#include "grace/coreP.h"

void *container_data_new(AMem *amem)
{
    return NULL;
}

void container_data_free(AMem *amem, void *data)
{
}

void *container_data_copy(AMem *amem, void *data)
{
    return data;
}

int container_qf_register(QuarkFactory *qfactory)
{
    QuarkFlavor qf = {
        QFlavorContainer,
        container_data_new,
        container_data_free,
        container_data_copy
    };

    return quark_flavor_add(qfactory, &qf);
}

Quark *container_new(QuarkFactory *qfactory, int mmodel)
{
    Quark *q;

    q = quark_root(NULL, mmodel, qfactory, QFlavorContainer);

    return q;
}
