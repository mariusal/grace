/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000 Grace Development Team
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

#include <config.h>

#include "globals.h"
#include "defines.h"
#include "utils.h"
#include "draw.h"
#include "t1fonts.h"
#include "graphs.h"
#include "graphutils.h"
#include "objutils.h"
#include "files.h"
#include "xfile.h"
#include "protos.h"

/*
 * XML project output
 */

static void xmlio_set_active(Attributes *attrs, int active)
{
    attributes_set_bval(attrs, "active", active);
}

static void xmlio_set_world_value(Attributes *attrs, char *name, double value)
{
    attributes_set_dval_formatted(attrs, name, value, grace->project->sformat);
}

static void xmlio_set_inout_placement(Attributes *attrs, int inout)
{
    char *s = "in";
    switch (inout) {
    case TICKS_IN:
        s ="in";
        break;
    case TICKS_OUT:
        s ="out";
        break;
    case TICKS_BOTH:
        s ="both";
        break;
    }
    attributes_set_sval(attrs, "inout-placement", s);
}

static void xmlio_set_side_placement(Attributes *attrs, PlacementType placement)
{
    char *s = "normal";
    switch (placement) {
    case PLACEMENT_NORMAL:
        s ="normal";
        break;
    case PLACEMENT_OPPOSITE:
        s ="opposite";
        break;
    case PLACEMENT_BOTH:
        s ="both";
        break;
    }
    attributes_set_sval(attrs, "side-placement", s);
}

static void xmlio_set_offset(Attributes *attrs, double offset1, double offset2)
{
    char buf[32];
    sprintf(buf, "(%g, %g)", offset1, offset2);
    attributes_set_sval(attrs, "offset", buf);
}

static void xmlio_set_offset_placement(Attributes *attrs,
    int autoplace, double offset1, double offset2)
{
    if (autoplace) {
        attributes_set_sval(attrs, "offset", "auto");
    } else {
        xmlio_set_offset(attrs, offset1, offset2);
    }
}

static void xmlio_set_angle(Attributes *attrs, double angle)
{
    attributes_set_dval(attrs, "angle", angle);
}

static void xmlio_set_font_ref(Attributes *attrs, int font)
{
    attributes_set_ival(attrs, "font-id", get_font_mapped_id(font));
}

static void xmlio_set_color_ref(Attributes *attrs, int color)
{
    attributes_set_ival(attrs, "color-id", color);
}

static void xmlio_set_pattern_ref(Attributes *attrs, int pattern)
{
    attributes_set_ival(attrs, "pattern-id", pattern);
}


static void xmlio_write_location(XFile *xf, Attributes *attrs,
    int loctype, double x, double y)
{
    attributes_reset(attrs);
    attributes_set_sval(attrs, "type", w_or_v(loctype));
    if (loctype == LOCWORLD) {
        xmlio_set_world_value(attrs, "x", x);
        xmlio_set_world_value(attrs, "y", y);
    } else {
        attributes_set_dval(attrs, "x", x);
        attributes_set_dval(attrs, "y", y);
    }
}

static void xmlio_write_format_spec(XFile *xf, Attributes *attrs,
    char *name, int format, int prec)
{
    attributes_reset(attrs);
    attributes_set_sval(attrs, "format", get_format_types(format));
    attributes_set_ival(attrs, "prec", prec);
    xfile_empty_element(xf, name, attrs);
}

static void xmlio_write_face_spec(XFile *xf, Attributes *attrs,
    int font, double charsize, int color)
{
    attributes_reset(attrs);

    xmlio_set_font_ref(attrs, font);
    xmlio_set_color_ref(attrs, color);
    attributes_set_dval(attrs, "char-size", charsize);
    xfile_empty_element(xf, "face-spec", attrs);
}

static void xmlio_write_fill_spec(XFile *xf, Attributes *attrs, Pen *pen)
{
    attributes_reset(attrs);

    xmlio_set_color_ref(attrs, pen->color);
    xmlio_set_pattern_ref(attrs, pen->pattern);
    xfile_empty_element(xf, "fill-spec", attrs);
}

static void xmlio_write_line_spec(XFile *xf, Attributes *attrs,
    Pen *pen, double linew, int lines)
{
    attributes_reset(attrs);

    xmlio_set_color_ref(attrs, pen->color);
    xmlio_set_pattern_ref(attrs, pen->pattern);
    attributes_set_ival(attrs, "style-id", lines);
    attributes_set_dval(attrs, "width", linew);
    xfile_empty_element(xf, "line-spec", attrs);
}

static void xmlio_write_text(XFile *xf, char *text)
{
    if (is_empty_string(text)) {
        xfile_empty_element(xf, "text", NULL);
    } else {
        xfile_text_element(xf, "text", NULL, text);
    }
}

int save_fontmap(XFile *xf)
{
    int i;
    Attributes *attrs;
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    xfile_begin_element(xf, "fontmap", NULL);
    for (i = 0; i < number_of_fonts(); i++) {
        if (get_font_mapped_id(i) != BAD_FONT_ID) {
            attributes_reset(attrs);
            attributes_set_ival(attrs, "id", get_font_mapped_id(i));
            attributes_set_sval(attrs, "name", get_fontalias(i));
            attributes_set_sval(attrs, "fallback", get_fontfallback(i));
            xfile_empty_element(xf, "font-def", attrs);
        }
    }
    xfile_end_element(xf, "fontmap");
    
    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

int save_colormap(XFile *xf)
{
    int i;
    Attributes *attrs;
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    xfile_begin_element(xf, "colormap", NULL);
    for (i = 0; i < number_of_colors(); i++) {
        CMap_entry *cmap;
        cmap = get_cmap_entry(i);
        if (cmap != NULL && cmap->ctype == COLOR_MAIN) {
            char buf[16];
            attributes_reset(attrs);
            attributes_set_ival(attrs, "id", i);
            sprintf(buf, "#%02x%02x%02x",
                cmap->rgb.red, cmap->rgb.green, cmap->rgb.blue);
            attributes_set_sval(attrs, "rgb", buf);
            attributes_set_sval(attrs, "name", cmap->cname);
            xfile_empty_element(xf, "color-def", attrs);
        }
    }
    xfile_end_element(xf, "colormap");

    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

int save_axis_properties(XFile *xf, tickmarks *t)
{
    Attributes *attrs;
    
    if (!t) {
        return RETURN_SUCCESS;
    }
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    attributes_set_bval(attrs, "zero", t->zero);
    xmlio_set_offset(attrs, t->offsx, t->offsy);
    xfile_empty_element(xf, "placement", attrs);
    
    attributes_reset(attrs);
    xmlio_set_active(attrs, t->t_drawbar);
    xfile_begin_element(xf, "axisbar", attrs);
    {
        Pen pen;
        pen.pattern = 1;
        pen.color = t->t_drawbarcolor;
        xmlio_write_line_spec(xf, attrs,
            &pen, t->t_drawbarlinew, t->t_drawbarlines);
    }
    xfile_end_element(xf, "axisbar");

    attributes_reset(attrs);
    attributes_set_sval(attrs, "layout",
        t->label_layout == LAYOUT_PERPENDICULAR ? "perpendicular":"parallel");
    xmlio_set_offset_placement(attrs,
        t->label_place == TYPE_AUTO, t->label.offset.x, t->label.offset.y);
    xmlio_set_side_placement(attrs, t->label_op);
    xfile_begin_element(xf, "axislabel", attrs);
    {
        xmlio_write_face_spec(xf, attrs,
            t->label.font, t->label.charsize, t->label.color);
        xmlio_write_text(xf, t->label.s);
    }
    xfile_end_element(xf, "axislabel");

    attributes_reset(attrs);
    xmlio_set_world_value(attrs, "major-step", t->tmajor);
    attributes_set_ival(attrs, "minor-divisions", t->nminor);
    attributes_set_ival(attrs, "auto-ticking", t->t_autonum);
    attributes_set_bval(attrs, "rounded-position", t->t_round);
    xfile_begin_element(xf, "ticks", attrs);
    {
        char *s = "none";
        switch (t->t_spec) {
        case TICKS_SPEC_NONE:
            s = "none";
            break;
        case TICKS_SPEC_MARKS:
            s = "ticks";
            break;
        case TICKS_SPEC_BOTH:
            s = "both";
            break;
        }
        attributes_reset(attrs);
        attributes_set_sval(attrs, "type", s);
        if (t->t_spec == TICKS_SPEC_NONE) {
            xfile_empty_element(xf, "userticks", attrs);
        } else {
            xfile_begin_element(xf, "userticks", attrs);
            {
                int i;
                for (i = 0; i < MIN2(t->nticks, MAX_TICKS); i++) {
                    attributes_reset(attrs);
                    attributes_set_sval(attrs, "type",
                        t->tloc[i].type == TICK_TYPE_MAJOR ? "major":"minor");
                    xmlio_set_world_value(attrs,
                        "position", t->tloc[i].wtpos);
                    if (t->t_spec == TICKS_SPEC_BOTH) {
                        attributes_set_sval(attrs, "label", t->tloc[i].label);
                    }
                    xfile_empty_element(xf, "tick", attrs);
                }
            }
            xfile_end_element(xf, "userticks");
        }

        attributes_reset(attrs);
        xmlio_set_active(attrs, t->t_flag);
        xmlio_set_side_placement(attrs, t->t_op);
        xmlio_set_inout_placement(attrs, t->t_inout);
        xfile_begin_element(xf, "tickmarks", attrs);
        {
            Pen pen;
            
            attributes_reset(attrs);
            attributes_set_dval(attrs, "size", t->props.size);
            attributes_set_bval(attrs, "grid-lines", t->props.gridflag);
            xfile_begin_element(xf, "major", attrs);
            {
                pen.color = t->props.color;
                pen.pattern = 1;
                xmlio_write_line_spec(xf, attrs,
                    &pen, t->props.linew, t->props.lines);
            }
            xfile_end_element(xf, "major");

            attributes_reset(attrs);
            attributes_set_dval(attrs, "size", t->mprops.size);
            attributes_set_bval(attrs, "grid-lines", t->mprops.gridflag);
            xfile_begin_element(xf, "minor", attrs);
            {
                pen.color = t->mprops.color;
                pen.pattern = 1;
                xmlio_write_line_spec(xf, attrs,
                    &pen, t->mprops.linew, t->mprops.lines);
            }
            xfile_end_element(xf, "minor");
        }
        xfile_end_element(xf, "tickmarks");

        attributes_reset(attrs);
        xmlio_set_active(attrs, t->tl_flag);
        xmlio_set_side_placement(attrs, t->tl_op);
        attributes_set_sval(attrs, "transform", t->tl_formula);
        attributes_set_sval(attrs, "prepend", t->tl_prestr);
        attributes_set_sval(attrs, "append", t->tl_appstr);
        xmlio_set_offset_placement(attrs,
            t->tl_gaptype == TYPE_AUTO, t->tl_gap.x, t->tl_gap.y);
        xmlio_set_angle(attrs, (double) t->tl_angle);
        attributes_set_ival(attrs, "skip", t->tl_skip);
        attributes_set_ival(attrs, "stagger", t->tl_staggered);
        if (t->tl_starttype == TYPE_AUTO) {
            attributes_set_sval(attrs, "start", "auto");
        } else {
            xmlio_set_world_value(attrs, "start", t->tl_start);
        }
        if (t->tl_stoptype == TYPE_AUTO) {
            attributes_set_sval(attrs, "stop", "auto");
        } else {
            xmlio_set_world_value(attrs, "stop", t->tl_stop);
        }
        xfile_begin_element(xf, "ticklabels", attrs);
        {
            xmlio_write_face_spec(xf, attrs,
                t->tl_font, t->tl_charsize, t->tl_color);
            xmlio_write_format_spec(xf, attrs,
                "format", t->tl_format, t->tl_prec);
        }
        xfile_end_element(xf, "ticklabels");
    }
    xfile_end_element(xf, "ticks");

    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

int save_graph_properties(XFile *xf, graph *g)
{
    int i;
    Attributes *attrs;
    
    if (!g) {
        return RETURN_FAILURE;
    }
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }
    
    attributes_reset(attrs);
    attributes_set_sval(attrs, "type", graph_types(g->type));
    attributes_set_bval(attrs, "stacked", g->stacked);
    attributes_set_dval(attrs, "bargap", g->bargap);
    xfile_empty_element(xf, "presentation-spec", attrs);

    /* Viewport */
    attributes_reset(attrs);
    attributes_set_dval(attrs, "xmin", g->v.xv1);
    attributes_set_dval(attrs, "xmax", g->v.xv2);
    attributes_set_dval(attrs, "ymin", g->v.yv1);
    attributes_set_dval(attrs, "ymax", g->v.yv2);
    xfile_empty_element(xf, "viewport", attrs);

    /* World coordinate scales */
    attributes_reset(attrs);
    xmlio_set_world_value(attrs, "min", g->w.xg1);
    xmlio_set_world_value(attrs, "max", g->w.xg2);
    attributes_set_sval(attrs, "type", scale_types(g->xscale));
    attributes_set_bval(attrs, "invert", g->xinvert);
    xfile_empty_element(xf, "xscale", attrs);
    attributes_reset(attrs);
    xmlio_set_world_value(attrs, "min", g->w.yg1);
    xmlio_set_world_value(attrs, "max", g->w.yg2);
    attributes_set_sval(attrs, "type", scale_types(g->yscale));
    attributes_set_bval(attrs, "invert", g->yinvert);
    xfile_empty_element(xf, "yscale", attrs);
    attributes_reset(attrs);
    attributes_set_dval(attrs, "norm", g->znorm);
    xfile_empty_element(xf, "zscale", attrs);

    /* Legend */
    attributes_reset(attrs);
    xmlio_set_active(attrs, g->l.active);
    attributes_set_ival(attrs, "length", g->l.len);
    attributes_set_ival(attrs, "vgap", g->l.vgap);
    attributes_set_ival(attrs, "hgap", g->l.hgap);
    attributes_set_bval(attrs, "invert", g->l.invert);
    xfile_begin_element(xf, "legend", attrs);
    {
        xmlio_write_face_spec(xf, attrs,
            g->l.font, g->l.charsize, g->l.color);
        xfile_begin_element(xf, "legframe", NULL);
        {
            xmlio_write_location(xf, attrs,
                g->l.loctype, g->l.legx, g->l.legy);
            xmlio_write_line_spec(xf, attrs,
                &(g->l.boxpen), g->l.boxlinew, g->l.boxlines);
            xmlio_write_fill_spec(xf, attrs, &(g->l.boxfillpen));
        }
        xfile_end_element(xf, "legframe");
    }
    xfile_end_element(xf, "legend");

    /* Locator */
    xfile_begin_element(xf, "locator", NULL);
    {
        attributes_reset(attrs);
        xmlio_set_active(attrs, g->locator.pointset);
        attributes_set_ival(attrs, "type", g->locator.pt_type); /* FIXME: textual */
        attributes_set_dval(attrs, "x", g->locator.dsx);
        attributes_set_dval(attrs, "y", g->locator.dsy);
        xfile_empty_element(xf, "fixedpoint", attrs);
        
        xmlio_write_format_spec(xf, attrs,
            "xformat", g->locator.fx, g->locator.px);
        xmlio_write_format_spec(xf, attrs,
            "yformat", g->locator.fy, g->locator.py);
    }
    xfile_end_element(xf, "locator");

    /* Frame */
    attributes_reset(attrs);
    attributes_set_ival(attrs, "type", g->f.type); /* FIXME: textual */
    xfile_begin_element(xf, "frame", attrs);
    {
        xmlio_write_line_spec(xf, attrs, &(g->f.pen), g->f.linew, g->f.lines);
        xmlio_write_fill_spec(xf, attrs, &(g->f.fillpen));
    }
    xfile_end_element(xf, "frame");

    /* Title/subtitle */
    xfile_begin_element(xf, "title", NULL);
    {
        xmlio_write_face_spec(xf, attrs,
            g->labs.title.font, g->labs.title.charsize, g->labs.title.color);
        xmlio_write_text(xf, g->labs.title.s);
    }
    xfile_end_element(xf, "title");
    xfile_begin_element(xf, "subtitle", NULL);
    {
        xmlio_write_face_spec(xf, attrs,
            g->labs.stitle.font, g->labs.stitle.charsize, g->labs.stitle.color);
        xmlio_write_text(xf, g->labs.stitle.s);
    }
    xfile_end_element(xf, "subtitle");

    /* FIXME: world stack */
    
    for (i = 0; i < MAXAXES; i++) {
        char *s = "";
        tickmarks *t;
        
        t = g->t[i];

        switch (i) {
        case 0:
            s = "x";
            break;
        case 1:
            s = "y";
            break;
        case 2:
            s = "altx";
            break;
        case 3:
            s = "alty";
            break;
        }

        attributes_reset(attrs);
        attributes_set_sval(attrs, "type", s);
        xmlio_set_active(attrs, t && t->active);
        xfile_begin_element(xf, "axis", attrs);
        save_axis_properties(xf, t);
        xfile_end_element(xf, "axis");
    }

    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

int save_set_properties(XFile *xf, set *p)
{
    Attributes *attrs;
    
    if (!p) {
        return RETURN_FAILURE;
    }
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    attributes_set_sval(attrs, "type", set_types(p->type));
    xfile_empty_element(xf, "presentation-spec", attrs);
    
    attributes_reset(attrs);
    attributes_set_ival(attrs, "type", p->sym); /* FIXME: textual */
    attributes_set_dval(attrs, "size", p->symsize);
    attributes_set_ival(attrs, "skip", p->symskip);
    attributes_set_ival(attrs, "char", (int) p->symchar);
    xmlio_set_font_ref(attrs, p->charfont);
    xfile_begin_element(xf, "symbol", attrs);
    {
        xmlio_write_line_spec(xf, attrs, &(p->sympen), p->symlinew, p->symlines);
        xmlio_write_fill_spec(xf, attrs, &(p->symfillpen));
    }
    xfile_end_element(xf, "symbol");
    
    attributes_reset(attrs);
    attributes_set_ival(attrs, "type", p->linet); /* FIXME: textual */
    attributes_set_ival(attrs, "fill-type", p->filltype); /* FIXME: textual */
    attributes_set_sval(attrs, "fill-rule",
        p->fillrule == FILLRULE_WINDING ? "winding":"evenodd");
    attributes_set_ival(attrs, "baseline-type", p->baseline_type); /* FIXME: textual */
    attributes_set_bval(attrs, "draw-baseline", p->baseline);
    attributes_set_bval(attrs, "draw-droplines", p->dropline);
    xfile_begin_element(xf, "line", attrs);
    {
        xmlio_write_line_spec(xf, attrs, &(p->linepen), p->linew, p->lines);
        xmlio_write_fill_spec(xf, attrs, &(p->setfillpen));
    }
    xfile_end_element(xf, "line");

    attributes_reset(attrs);
    xmlio_set_active(attrs, p->avalue.active);
    attributes_set_ival(attrs, "type", p->avalue.type); /* FIXME: textual */
    xmlio_set_angle(attrs, (double) p->avalue.angle);
    xmlio_set_offset(attrs, p->avalue.offset.x, p->avalue.offset.y);
    attributes_set_sval(attrs, "prepend", p->avalue.prestr);
    attributes_set_sval(attrs, "append", p->avalue.appstr);
    xfile_begin_element(xf, "annotation", attrs);
    {
        xmlio_write_face_spec(xf, attrs,
            p->avalue.font, p->avalue.size, p->avalue.color);
        xmlio_write_format_spec(xf, attrs,
            "format", p->avalue.format, p->avalue.prec);
    }
    xfile_end_element(xf, "annotation");

    attributes_reset(attrs);
    xmlio_set_active(attrs, p->errbar.active);
    xmlio_set_side_placement(attrs, p->errbar.ptype);
    xfile_begin_element(xf, "errorbar", attrs);
    {
        attributes_reset(attrs);
        attributes_set_dval(attrs, "size", p->errbar.barsize);
        xfile_begin_element(xf, "barline", attrs);
        {
            xmlio_write_line_spec(xf, attrs,
                &(p->errbar.pen), p->errbar.linew, p->errbar.lines);
        }
        xfile_end_element(xf, "barline");
        
        attributes_reset(attrs);
        attributes_set_bval(attrs, "arrow-clip", p->errbar.arrow_clip);
        attributes_set_dval(attrs, "clip-length", p->errbar.cliplen);
        xfile_begin_element(xf, "riserline", attrs);
        {
            xmlio_write_line_spec(xf, attrs,
                &(p->errbar.pen), p->errbar.riser_linew, p->errbar.riser_lines);
        }
        xfile_end_element(xf, "riserline");
    }
    xfile_end_element(xf, "errorbar");

    xfile_begin_element(xf, "legend-entry", NULL);
    xmlio_write_text(xf, p->legstr);
    xfile_end_element(xf, "legend-entry");

    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

static int save_dataset(XFile *xf, Dataset *data)
{
    Attributes *attrs;
    int i, nc;

    if (!data) {
        return RETURN_FAILURE;
    }
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    for (i = 0; i < data->len; i++) {
        attributes_reset(attrs);
        for (nc = 0; nc < MAX_SET_COLS; nc++) {
            if (data->ex[nc]) {
                xmlio_set_world_value(attrs,
                    dataset_colname(nc), data->ex[nc][i]);
            }
        }
        if (data->s) {
            attributes_set_sval(attrs, "s", data->s[i]);
        }
        xfile_empty_element(xf, "row", attrs);
    }

    attributes_free(attrs);

    return RETURN_SUCCESS;
}

int save_object(XFile *xf, Attributes *attrs, DObject *o)
{
    if (!xf || !attrs || !o) {
        return RETURN_FAILURE;
    }
    
    attributes_reset(attrs);
    xmlio_set_active(attrs, o->active);
    xmlio_set_angle(attrs, o->angle);
    xmlio_set_offset(attrs, o->offset.x, o->offset.y);
    xfile_begin_element(xf, "object", attrs);
    {
        char buf[32];
        xmlio_write_location(xf, attrs, o->loctype, o->ap.x, o->ap.y);
        xmlio_write_line_spec(xf, attrs, &(o->pen), o->linew, o->lines);
        xmlio_write_fill_spec(xf, attrs, &(o->fillpen));
        attributes_reset(attrs);
        switch (o->type) {
        case DO_LINE:
            {
                DOLineData *l = (DOLineData *) o->odata;
                attributes_set_dval(attrs, "length", l->length);
                /* FIXME arrows */;
            }
            break;
        case DO_BOX:
            {
                DOBoxData *b = (DOBoxData *) o->odata;
                attributes_set_dval(attrs, "width", b->width);
                attributes_set_dval(attrs, "height", b->height);
            }
            break;
        case DO_ARC:
            {
                DOArcData *a = (DOArcData *) o->odata;
                attributes_set_dval(attrs, "width", a->width);
                attributes_set_dval(attrs, "height", a->height);
                attributes_set_dval(attrs, "start-angle", a->angle1);
                attributes_set_dval(attrs, "stop-angle", a->angle2);
                attributes_set_ival(attrs, "fill-mode", a->fillmode); /* FIXME: textual */
            }
            break;
        case DO_STRING:
            {
                DOStringData *s = (DOStringData *) o->odata;
                xmlio_set_font_ref(attrs, s->font);
                attributes_set_dval(attrs, "char-size", s->size);
                attributes_set_ival(attrs, "justification", s->just); /* FIXME: textual */
            }
            break;
        }
        sprintf(buf, "%s-data", object_types(o->type));
        if (o->type == DO_STRING) {
            xfile_begin_element(xf, buf, attrs);
            {
                DOStringData *s = (DOStringData *) o->odata;
                xmlio_write_text(xf, s->s);
            }
            xfile_end_element(xf, buf);
        } else {
            xfile_empty_element(xf, buf, attrs);
        }
    }
    xfile_end_element(xf, "object");
    
    return RETURN_SUCCESS;
}

int save_objects(XFile *xf, int gno)
{
    Attributes *attrs;
    int i, n;
    int *ids;

    if (!xf) {
        return RETURN_FAILURE;
    }
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    n = get_object_ids(&ids);
    for (i = 0; i < n; i++) {
        DObject *o;
        
        o = object_get(ids[i]);
        if (o->gno == gno) {
            save_object(xf, attrs, o);
        }
    }

    attributes_free(attrs);

    return RETURN_SUCCESS;
}
    
int save_canvas_objects(XFile *xf)
{
    return save_objects(xf, -1);
}
    
int save_graph_objects(XFile *xf, int gno)
{
    return save_objects(xf, gno);
}
    
int save_regions(XFile *xf)
{
    Attributes *attrs;

    if (!xf) {
        return RETURN_FAILURE;
    }
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    /* FIXME */

    attributes_free(attrs);

    return RETURN_SUCCESS;
}
    
int save_preferences(XFile *xf)
{
    Attributes *attrs;

    if (!xf) {
        return RETURN_FAILURE;
    }
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }
    
    attributes_free(attrs);

    return RETURN_SUCCESS;
}


int save_project(char *fn)
{
    XFile *xf;
    Attributes *attrs;
    int gno;
    
    xf = xfile_new(fn);
    attrs = attributes_new();
    
    if (xf == NULL || attrs == NULL) {
        return RETURN_FAILURE;
    }
    
    attributes_reset(attrs);
    attributes_set_ival(attrs, "version", bi_version_id());
    xfile_begin(xf, "ISO-8859-1", FALSE, NULL, "grace.dtd", "grace", attrs);

    xfile_comment(xf, "Description");
    xfile_begin_element(xf, "description", NULL);
    {
        xmlio_write_text(xf, grace->project->description);
    }
    xfile_end_element(xf, "description");
    
    xfile_comment(xf, "Definitions");
    xfile_begin_element(xf, "definitions", NULL);
    {
        xfile_comment(xf, "Color map");
        save_colormap(xf);
        xfile_comment(xf, "Font map");
        save_fontmap(xf);
    }
    xfile_end_element(xf, "definitions");

    xfile_comment(xf, "Page properties");
    attributes_reset(attrs);
    attributes_set_ival(attrs, "width", grace->project->page_wpp);
    attributes_set_ival(attrs, "height", grace->project->page_hpp);
    xfile_begin_element(xf, "page", attrs);
    {
        Pen pen;
        pen.color = getbgcolor();
        pen.pattern = getbgfill() ? 1:0;
        xmlio_write_fill_spec(xf, attrs, &pen);
    }
    xfile_end_element(xf, "page");

    xfile_comment(xf, "Data formats");
    xfile_begin_element(xf, "data-formats", NULL);
    {
        attributes_reset(attrs);
        xmlio_set_world_value(attrs, "reference", get_ref_date());
        attributes_set_bval(attrs, "wrap", two_digits_years_allowed());
        attributes_set_ival(attrs, "wrap-year", get_wrap_year());
        xfile_empty_element(xf, "dates", attrs);
        
        attributes_reset(attrs);
        attributes_set_sval(attrs, "format", grace->project->sformat);
        xfile_empty_element(xf, "world", attrs);
    }
    xfile_end_element(xf, "data-formats");

    xfile_comment(xf, "Time stamp");
    attributes_reset(attrs);
    xmlio_set_active(attrs, grace->project->timestamp.active);
    xmlio_set_offset(attrs, grace->project->timestamp.offset.x,
        grace->project->timestamp.offset.y);
    xmlio_set_angle(attrs, grace->project->timestamp.angle);
    attributes_set_sval(attrs, "value", grace->project->timestamp.s);
    /* FIXME: justification */
    xfile_begin_element(xf, "time-stamp", attrs);
    {
        xmlio_write_face_spec(xf, attrs,
            grace->project->timestamp.font, grace->project->timestamp.charsize,
                grace->project->timestamp.color);
    }
    xfile_end_element(xf, "time-stamp");
    
    save_canvas_objects(xf);
    
    save_regions(xf);
    
    save_preferences(xf);

    xfile_comment(xf, "Graphs");
    storage_rewind(grace->project->graphs);
    while (storage_get_id(grace->project->graphs, &gno) == RETURN_SUCCESS) {
        graph *g = graph_get(gno);
        int setno;

        attributes_reset(attrs);
        attributes_set_ival(attrs, "id", gno);
        xmlio_set_active(attrs, !(g->hidden));
        xfile_begin_element(xf, "graph", attrs);
        {
            save_graph_properties(xf, g);
            
            save_graph_objects(xf, gno);

            storage_rewind(g->sets);
            while (storage_get_id(g->sets, &setno) == RETURN_SUCCESS) {
                char data_ref[32];
                set *p = set_get(gno, setno);

                attributes_reset(attrs);
                sprintf(data_ref, "G%d.S%d-data", gno, setno);
                attributes_set_sval(attrs, "id", data_ref);
                attributes_set_sval(attrs, "comment", p->comment);
                if (p->hotlink) {
                    attributes_set_sval(attrs, "hotfile", p->hotfile);
                    /* FIXME: hotsrc */
                }
                xfile_begin_element(xf, "dataset", attrs);
                {
                    save_dataset(xf, p->data);
                }
                xfile_end_element(xf, "dataset");

                attributes_reset(attrs);
                attributes_set_ival(attrs, "id", setno);
                xmlio_set_active(attrs, !(p->hidden));
                attributes_set_sval(attrs, "data-ref", data_ref);
                xfile_begin_element(xf, "set", attrs);
                {
                    save_set_properties(xf, p);
                }
                xfile_end_element(xf, "set");

                if (storage_next(g->sets) != RETURN_SUCCESS) {
                    break;
                }
            }
        }
        xfile_end_element(xf, "graph");

        if (storage_next(grace->project->graphs) != RETURN_SUCCESS) {
            break;
        }
    }
    
    attributes_free(attrs);
    
    xfile_end(xf);
    xfile_free(xf);
    
    grace->project->docname = copy_string(grace->project->docname, fn);

    clear_dirtystate();
    
    return RETURN_SUCCESS;
}


/*
save_prefs()
{
    xfile_comment(xf, "Defaults");
    xfile_begin_element(xf, "defaults", NULL);
    {
        Pen pen;
        defaults grdefaults = grace->rt->grdefaults;
        pen.color = grdefaults.color;
        pen.pattern = grdefaults.pattern;
        xmlio_write_line_spec(xf, attrs,
            &pen, grdefaults.linew, grdefaults.lines);
        xmlio_write_fill_spec(xf, attrs, &pen);
        xmlio_write_face_spec(xf, attrs,
            grdefaults.font, grdefaults.charsize, grdefaults.color);
    }
    xfile_end_element(xf, "defaults");

    xfile_comment(xf, "Preferences");
    xfile_begin_element(xf, "preferences", attrs);
    {
        (int) rint(grace->rt->scrollper * 100);
        (int) rint(grace->rt->shexper * 100);
        grace->rt->scrolling_islinked ? "on" : "off";
    }
    xfile_end_element(xf, "preferences");

}
*/
