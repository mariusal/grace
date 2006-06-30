/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2006 Grace Development Team
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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "grace/grace.h"


static void *obj_proc(void *context, const char *name, void *udata)
{
    Quark *q = (Quark *) context;
    if (q) {
        return quark_find_child_by_idstr(q, name);
    } else {
        return NULL;
    }
}

static GVarType get_proc(const void *obj,
    const char *name, GVarData *prop, void *udata)
{
    Quark *q = (Quark *) obj;
    Grace *grace = grace_from_quark(q);
    DataColumn col;
    int column;
    
    if (!q) {
        return GVarNil;
    }
    
    if (strings_are_equal(name, "idstr")) {
        prop->str = copy_string(NULL, QIDSTR(q));
        return GVarStr;
    } else
    if (strings_are_equal(name, "active")) {
        prop->bool = quark_is_active(q);
        return GVarBool;
    }
    
    switch (quark_fid_get(q)) {
    case QFlavorSSD:
        if (strings_are_equal(name, "nrows")) {
            prop->num = ssd_get_nrows(q);
            return GVarNum;
        } else
        if (strings_are_equal(name, "ncols")) {
            prop->num = ssd_get_ncols(q);
            return GVarNum;
        } else
        if ((column = ssd_get_column_by_name(q, name)) >= 0) {
            prop->arr = ssd_get_darray(q, column);
            return GVarArr;
        } else {
            return GVarNil;
        }
        break;
    case QFlavorSet:
        if (strings_are_equal(name, "length")) {
            prop->num = set_get_length(q);
            return GVarNum;
        } else
        if ((col = get_dataset_col_by_name(grace, name)) != DATA_BAD) {
            prop->arr = set_get_darray(q, col);
            return GVarArr;
        } else {
            return GVarNil;
        }
        break;
    default:
        return GVarNil;
        break;
    }
}

static int set_proc(void *obj,
    const char *name, GVarType type, GVarData prop, void *udata)
{
    Quark *q = (Quark *) obj;
    Grace *grace = grace_from_quark(q);
    DataColumn col;
    int column;
    
    if (!q) {
        return RETURN_FAILURE;
    }

    if (strings_are_equal(name, "idstr") && type == GVarStr) {
        return quark_idstr_set(q, prop.str);
    } else
    if (strings_are_equal(name, "active") && type == GVarBool) {
        return quark_set_active(q, prop.bool);
    }

    switch (quark_fid_get(q)) {
    case QFlavorSSD:
        if ((column = ssd_get_column_by_name(q, name)) >= 0) {
            return ssd_set_darray(q, column, prop.arr);
        } else {
            return RETURN_FAILURE;
        }
        break;
    case QFlavorSet:
        if ((col = get_dataset_col_by_name(grace, name)) != DATA_BAD) {
            return set_set_darray(q, col, prop.arr);
        } else {
            return RETURN_FAILURE;
        }
        break;
    default:
        return RETURN_FAILURE;
        break;
    }
}

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

int grace_init(void)
{
    return canvas_init();
}
   
Grace *grace_new(void)
{
    Grace *grace;
    char *s;
    
    QuarkFlavor container_qf = {
        QFlavorContainer,
        container_data_new,
        container_data_free,
        container_data_copy
    };

    grace = xmalloc(sizeof(Grace));
    if (!grace) {
        return NULL;
    }
    memset(grace, 0, sizeof(Grace));

    grace->qfactory = qfactory_new();
    if (!grace->qfactory) {
        grace_free(grace);
        return NULL;
    }

    /* register quark flavors */
    project_qf_register(grace->qfactory);
    ssd_qf_register(grace->qfactory);
    frame_qf_register(grace->qfactory);
    graph_qf_register(grace->qfactory);
    set_qf_register(grace->qfactory);
    axisgrid_qf_register(grace->qfactory);
    axis_qf_register(grace->qfactory);
    object_qf_register(grace->qfactory);
    atext_qf_register(grace->qfactory);
    region_qf_register(grace->qfactory);

    quark_flavor_add(grace->qfactory, &container_qf);

    grace->canvas = canvas_new();
    if (!grace->canvas) {
        grace_free(grace);
        return NULL;
    }
    canvas_set_fmap_proc(grace->canvas, fmap_proc);
    canvas_set_csparse_proc(grace->canvas, csparse_proc);
    
    grace->graal = graal_new();
    if (!grace->graal) {
        grace_free(grace);
        return NULL;
    }
    graal_set_udata(grace->graal, grace);
    graal_set_lookup_procs(grace->graal, obj_proc, get_proc, set_proc);
    
    grace->safe_mode = TRUE;
    
    grace->grace_home   = NULL;
    grace->username     = NULL;
    grace->userhome     = NULL;

    /* Grace home directory */
    if ((s = getenv("GRACE_HOME")) == NULL) {
	s = bi_home();
    }
    grace->grace_home = copy_string(NULL, s);

    /* username */
    s = getenv("LOGNAME");
    if (string_is_empty(s)) {
        s = getlogin();
        if (string_is_empty(s)) {
            s = "a user";
        }
    }
    grace->username = copy_string(NULL, s);
    canvas_set_username(grace->canvas, grace->username);

    /* userhome */
    if ((s = getenv("HOME")) == NULL) {
        s = "/";
    }
    grace->userhome = copy_string(NULL, s);
    if (grace->userhome[strlen(grace->userhome) - 1] != '/') {
        grace->userhome = concat_strings(grace->userhome, "/");
    }

    if (!grace->grace_home   ||
        !grace->username     ||
        !grace->userhome) {
        grace_free(grace);
        return NULL;
    }

    /* dictionaries */
    if (grace_init_dicts(grace) != RETURN_SUCCESS) {
        grace_free(grace);
        return NULL;
    }

    quark_factory_set_udata(grace->qfactory, grace);

    return grace;
}

void grace_free(Grace *grace)
{
    if (grace) {
        xfree(grace->grace_home);
        xfree(grace->username);
        xfree(grace->userhome);

        qfactory_free(grace->qfactory);

        canvas_free(grace->canvas);

        graal_free(grace->graal);

        grace_free_dicts(grace);

        xfree(grace);
    }
}

void grace_set_udata(Grace *grace, void *udata)
{
    grace->udata = udata;
}

void *grace_get_udata(const Grace *grace)
{
    return grace->udata;
}

Grace *grace_from_quark(const Quark *q)
{
    return (Grace *) quark_factory_get_udata(quark_get_qfactory(q));
}
