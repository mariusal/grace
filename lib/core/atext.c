/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2004 Grace Development Team
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

/*
 *
 * Annotating texts
 *
 */

#include <string.h>

#include "grace/core.h"

TextProps *textprops_new(void)
{
    TextProps *retval;
    retval = xmalloc(sizeof(TextProps));
    if (retval) {
        memset(retval, 0, sizeof(TextProps));
    }
    return retval;
}

void set_default_textprops(TextProps *pstr, const defaults *grdefs)
{
    pstr->color    = grdefs->line.pen.color;
    pstr->angle    = 0.0;
    pstr->font     = grdefs->font;
    pstr->just     = JUST_LEFT|JUST_BLINE;
    pstr->charsize = 1.0;
}

AText *atext_data_new(void)
{
    AText *at;
    at = xmalloc(sizeof(AText));
    memset(at, 0, sizeof(AText));
    
    return at;
}

void atext_data_free(AText *at)
{
    if (at) {
        xfree(at->s);
        xfree(at);
    }
}

AText *atext_data_copy(AText *at)
{
    AText *retval;
    if (at == NULL) {
        return NULL;
    } else {
        retval = atext_data_new();
        if (retval) {
            memcpy(retval, at, sizeof(AText));
            retval->s = copy_string(NULL, at->s);
        }
    }
    
    return retval;
}

Quark *atext_new(Quark *q)
{
    Quark *qnew = quark_new(q, QFlavorAText);
    AText *at = atext_get_data(qnew);

    Project *pr = project_get_data(get_parent_project(qnew));

    if (at && pr) {
        defaults grdefs = pr->grdefaults;
        set_default_textprops(&at->text_props, &grdefs);
        at->offset.x = at->offset.y = 0.0;

        at->line = grdefs.line;
    }
    return qnew;
}

AText *atext_get_data(const Quark *q)
{
    if (q && q->fid == QFlavorAText) {
        return (AText *) q->data;
    } else {
        return NULL;
    }
}

int atext_set_active(Quark *q, int flag)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->active = flag;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_string(Quark *q, const char *s)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->s = copy_string(at->s, s);
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_ap(Quark *q, const APoint *ap)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->ap = *ap;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_offset(Quark *q, const VPoint *offset)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->offset = *offset;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_tprops(Quark *q, const TextProps *tprops)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->text_props = *tprops;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_font(Quark *q, int font)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->text_props.font = font;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_char_size(Quark *q, double size)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->text_props.charsize = size;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_color(Quark *q, int color)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->text_props.color = color;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_just(Quark *q, int just)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->text_props.just = just;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int atext_set_angle(Quark *q, double angle)
{
    if (q && q->fid == QFlavorAText) {
        AText *at = atext_get_data(q);
        
        at->text_props.angle = angle;
        
        quark_dirtystate_set(q, TRUE);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
