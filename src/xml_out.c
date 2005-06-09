/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2005 Grace Development Team
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

#include <config.h>

#include <string.h>

#include "grace.h"
#include "utils.h"
#include "dicts.h"
#include "files.h"
#include "core_utils.h"
#include "xfile.h"
#include "xstrings.h"
#include "protos.h"

/*
 * XML project output
 */

static void xmlio_set_active(Attributes *attrs, int active)
{
    attributes_set_bval(attrs, AStrActive, active);
}

static void xmlio_set_world_value(Quark *q, Attributes *attrs, char *name, double value)
{
    attributes_set_dval_formatted(attrs, name, value,
        project_get_sformat(get_parent_project(q)));
}

static void xmlio_set_inout_placement(RunTime *rt, Attributes *attrs, int inout)
{
    char *s;
    
    s = inout_placement_name(rt, inout);
    
    attributes_set_sval(attrs, AStrInoutPlacement, s);
}

static void xmlio_set_side_placement(RunTime *rt, Attributes *attrs, PlacementType placement)
{
    char *s;
    
    s = side_placement_name(rt, placement);

    attributes_set_sval(attrs, AStrSidePlacement, s);
}

static void xmlio_set_axis_position(RunTime *rt, Attributes *attrs, int position)
{
    char *s;
    
    s = axis_position_name(rt, position);

    attributes_set_sval(attrs, AStrPosition, s);
}

static void xmlio_set_offset(Attributes *attrs, double offset1, double offset2)
{
    char buf[32];
    sprintf(buf, "(%g, %g)", offset1, offset2);
    attributes_set_sval(attrs, AStrOffset, buf);
}

static void xmlio_set_fpoint(Attributes *attrs,
    const char *aname, const FPoint *fp)
{
    char buf[32];
    sprintf(buf, "(%g, %g)", fp->x, fp->y);
    attributes_set_sval(attrs, aname, buf);
}

static void xmlio_set_angle(Attributes *attrs, double angle)
{
    attributes_set_dval(attrs, AStrAngle, angle);
}

static void xmlio_set_font_ref(Attributes *attrs, int font)
{
    attributes_set_ival(attrs, AStrFontId, font);
}

static void xmlio_set_color_ref(Attributes *attrs, int color)
{
    attributes_set_ival(attrs, AStrColorId, color);
}

static void xmlio_set_pattern_ref(Attributes *attrs, int pattern)
{
    attributes_set_ival(attrs, AStrPatternId, pattern);
}


static void xmlio_write_location(Quark *q, XFile *xf, Attributes *attrs,
    const APoint *ap)
{
    int loctype = object_get_loctype(q);
    attributes_reset(attrs);
    if (loctype == COORD_WORLD) {
        xmlio_set_world_value(q, attrs, AStrX, ap->x);
        xmlio_set_world_value(q, attrs, AStrY, ap->y);
    } else {
        attributes_set_dval(attrs, AStrX, ap->x);
        attributes_set_dval(attrs, AStrY, ap->y);
    }
    xfile_empty_element(xf, EStrLocation, attrs);
}

static void xmlio_write_format_spec(XFile *xf, Attributes *attrs,
    char *name, const Format *format)
{
    attributes_reset(attrs);
    attributes_set_sval(attrs, AStrFormat, get_format_types(format->type));
    attributes_set_ival(attrs, AStrPrec, format->prec);
    xfile_empty_element(xf, name, attrs);
}

static void xmlio_write_face_spec(XFile *xf, Attributes *attrs,
    int font, double charsize, int color)
{
    attributes_reset(attrs);

    xmlio_set_font_ref(attrs, font);
    xmlio_set_color_ref(attrs, color);
    attributes_set_dval(attrs, AStrCharSize, charsize);
    xfile_empty_element(xf, EStrFaceSpec, attrs);
}

static void xmlio_write_fill_spec(XFile *xf, Attributes *attrs, Pen *pen)
{
    attributes_reset(attrs);

    xmlio_set_color_ref(attrs, pen->color);
    xmlio_set_pattern_ref(attrs, pen->pattern);
    xfile_empty_element(xf, EStrFillSpec, attrs);
}

static void xmlio_write_line_spec(XFile *xf, Attributes *attrs,
    const Line *line)
{
    attributes_reset(attrs);

    xmlio_set_color_ref(attrs, line->pen.color);
    xmlio_set_pattern_ref(attrs, line->pen.pattern);
    attributes_set_ival(attrs, AStrStyleId, line->style);
    attributes_set_dval(attrs, AStrWidth, line->width);
    xfile_empty_element(xf, EStrLineSpec, attrs);
}

static void xmlio_write_text(XFile *xf, char *text)
{
    if (string_is_empty(text)) {
        xfile_empty_element(xf, EStrText, NULL);
    } else {
        xfile_text_element(xf, EStrText, NULL, text, TRUE);
    }
}

static void xmlio_write_arrow(RunTime *rt, XFile *xf, Attributes *attrs, Arrow *arrow)
{
    attributes_reset(attrs);
    
    attributes_set_sval(attrs, AStrType, arrow_type_name(rt, arrow->type));
    attributes_set_dval(attrs, AStrLength, arrow->length);
    attributes_set_dval(attrs, AStrDlFf, arrow->dL_ff);
    attributes_set_dval(attrs, AStrLlFf, arrow->lL_ff);
    xfile_empty_element(xf, EStrArrow, attrs);
}

static void xmlio_write_text_props(XFile *xf, Attributes *attrs,
    const TextProps *tprops)
{
    attributes_reset(attrs);

    xmlio_set_angle(attrs, tprops->angle);
    attributes_set_ival(attrs, AStrJustification, tprops->just); /* FIXME: textual */
    xfile_begin_element(xf, EStrTextProperties, attrs);
    {
        xmlio_write_face_spec(xf, attrs,
            tprops->font, tprops->charsize, tprops->color);
    }
    xfile_end_element(xf, EStrTextProperties);
}

int save_fontmap(XFile *xf, const Project *pr)
{
    unsigned int i;
    Attributes *attrs;
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    xfile_begin_element(xf, EStrFontmap, NULL);
    for (i = 0; i < pr->nfonts; i++) {
        Fontdef *f = &pr->fontmap[i];
        attributes_reset(attrs);
        attributes_set_ival(attrs, AStrId, f->id);
        attributes_set_sval(attrs, AStrName, f->fontname);
        attributes_set_sval(attrs, AStrFallback, f->fallback);
        xfile_empty_element(xf, EStrFontDef, attrs);
    }
    xfile_end_element(xf, EStrFontmap);
    
    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

int save_colormap(XFile *xf, const Project *pr)
{
    unsigned int i;
    Attributes *attrs;
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    xfile_begin_element(xf, EStrColormap, NULL);
    for (i = 0; i < pr->ncolors; i++) {
        Colordef *c = &pr->colormap[i];
        char buf[16];

        attributes_reset(attrs);
        attributes_set_ival(attrs, AStrId, c->id);
        sprintf(buf, "#%02x%02x%02x", c->rgb.red, c->rgb.green, c->rgb.blue);
        attributes_set_sval(attrs, AStrRgb, buf);
        attributes_set_sval(attrs, AStrName, c->cname);
        xfile_empty_element(xf, EStrColorDef, attrs);
    }
    xfile_end_element(xf, EStrColormap);

    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

int save_axisgrid_properties(XFile *xf, Quark *q)
{
    Attributes *attrs;
    tickmarks *t = axisgrid_get_data(q);
    char *s;
    RunTime *rt = rt_from_quark(q);
    
    if (!t) {
        return RETURN_SUCCESS;
    }
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    attributes_reset(attrs);
    xmlio_set_active(attrs, t->gprops.onoff);
    xfile_begin_element(xf, EStrMajorGridlines, attrs);
    {
        xmlio_write_line_spec(xf, attrs, &t->gprops.line);
    }
    xfile_end_element(xf, EStrMajorGridlines);

    attributes_reset(attrs);
    xmlio_set_active(attrs, t->mgprops.onoff);
    xfile_begin_element(xf, EStrMinorGridlines, attrs);
    {
        xmlio_write_line_spec(xf, attrs, &t->mgprops.line);
    }
    xfile_end_element(xf, EStrMinorGridlines);

    attributes_reset(attrs);
    xfile_begin_element(xf, EStrAxisbar, attrs);
    {
        xmlio_write_line_spec(xf, attrs, &t->bar);
    }
    xfile_end_element(xf, EStrAxisbar);

    s = spec_tick_name(rt, t->t_spec);

    attributes_reset(attrs);
    attributes_set_sval(attrs, AStrType, s);
    if (t->t_spec == TICKS_SPEC_NONE) {
        xfile_empty_element(xf, EStrUserticks, attrs);
    } else {
        xfile_begin_element(xf, EStrUserticks, attrs);
        {
            int i;
            for (i = 0; i < MIN2(t->nticks, MAX_TICKS); i++) {
                attributes_reset(attrs);
                attributes_set_sval(attrs, AStrType,
                    t->tloc[i].type == TICK_TYPE_MAJOR ? VStrMajor:VStrMinor);
                xmlio_set_world_value(q, attrs,
                    AStrPosition, t->tloc[i].wtpos);
                if (t->t_spec == TICKS_SPEC_BOTH) {
                    attributes_set_sval(attrs, AStrLabel, t->tloc[i].label);
                }
                xfile_empty_element(xf, EStrTick, attrs);
            }
        }
        xfile_end_element(xf, EStrUserticks);
    }

    attributes_reset(attrs);
    attributes_set_dval(attrs, AStrSize, t->props.size);
    xmlio_set_inout_placement(rt, attrs, t->props.inout);
    xfile_begin_element(xf, EStrMajorTickmarks, attrs);
    {
        xmlio_write_line_spec(xf, attrs, &t->props.line);
    }
    xfile_end_element(xf, EStrMajorTickmarks);

    attributes_reset(attrs);
    attributes_set_dval(attrs, AStrSize, t->mprops.size);
    xmlio_set_inout_placement(rt, attrs, t->mprops.inout);
    xfile_begin_element(xf, EStrMinorTickmarks, attrs);
    {
        xmlio_write_line_spec(xf, attrs, &t->mprops.line);
    }
    xfile_end_element(xf, EStrMinorTickmarks);

    attributes_reset(attrs);
    attributes_set_sval(attrs, AStrTransform, t->tl_formula);
    attributes_set_sval(attrs, AStrPrepend, t->tl_prestr);
    attributes_set_sval(attrs, AStrAppend, t->tl_appstr);
    xmlio_set_offset(attrs, t->tl_gap.x, t->tl_gap.y);

    attributes_set_ival(attrs, AStrSkip, t->tl_skip);
    attributes_set_ival(attrs, AStrStagger, t->tl_staggered);
    if (t->tl_starttype == TYPE_AUTO) {
        attributes_set_sval(attrs, AStrStart, VStrAuto);
    } else {
        xmlio_set_world_value(q, attrs, AStrStart, t->tl_start);
    }
    if (t->tl_stoptype == TYPE_AUTO) {
        attributes_set_sval(attrs, AStrStop, VStrAuto);
    } else {
        xmlio_set_world_value(q, attrs, AStrStop, t->tl_stop);
    }
    xfile_begin_element(xf, EStrTicklabels, attrs);
    {
        xmlio_write_text_props(xf, attrs, &t->tl_tprops);
        xmlio_write_format_spec(xf, attrs, EStrFormat, &t->tl_format);
    }
    xfile_end_element(xf, EStrTicklabels);

    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

int save_frame_properties(XFile *xf, frame *f)
{
    Attributes *attrs;
    if (!f) {
        return RETURN_FAILURE;
    }
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    /* Frame */
    attributes_reset(attrs);
    xmlio_write_line_spec(xf, attrs, &f->outline);
    xmlio_write_fill_spec(xf, attrs, &f->fillpen);
    
    /* Viewport */
    attributes_reset(attrs);
    attributes_set_dval(attrs, AStrXmin, f->v.xv1);
    attributes_set_dval(attrs, AStrXmax, f->v.xv2);
    attributes_set_dval(attrs, AStrYmin, f->v.yv1);
    attributes_set_dval(attrs, AStrYmax, f->v.yv2);
    xfile_empty_element(xf, EStrViewport, attrs);

    /* Legend */
    attributes_reset(attrs);
    xmlio_set_active(attrs, f->l.active);
    attributes_set_dval(attrs, AStrLength, f->l.len);
    attributes_set_dval(attrs, AStrVgap, f->l.vgap);
    attributes_set_dval(attrs, AStrHgap, f->l.hgap);
    attributes_set_bval(attrs, AStrSingleSymbol, f->l.singlesym);
    attributes_set_bval(attrs, AStrInvert, f->l.invert);
    xfile_begin_element(xf, EStrLegend, attrs);
    {
        xmlio_write_face_spec(xf, attrs,
            f->l.font, f->l.charsize, f->l.color);
        attributes_reset(attrs);
        xmlio_set_fpoint(attrs, AStrAnchor, &f->l.anchor);
        xmlio_set_offset(attrs, f->l.offset.x, f->l.offset.y);
        attributes_set_ival(attrs, AStrJustification, f->l.just); /* FIXME: textual */
        xfile_begin_element(xf, EStrLegframe, attrs);
        {
            xmlio_write_line_spec(xf, attrs, &f->l.boxline);
            xmlio_write_fill_spec(xf, attrs, &(f->l.boxfillpen));
        }
        xfile_end_element(xf, EStrLegframe);
    }
    xfile_end_element(xf, EStrLegend);

    return RETURN_SUCCESS;
}

int save_graph_properties(XFile *xf, Quark *gr)
{
    Attributes *attrs;
    graph *g;
    RunTime *rt = rt_from_quark(gr);
    
    if (!gr) {
        return RETURN_FAILURE;
    }
    
    g = graph_get_data(gr);
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }
    
    attributes_reset(attrs);
    attributes_set_sval(attrs, AStrType, graph_types(rt_from_quark(gr), g->type));
    attributes_set_bval(attrs, AStrStacked, g->stacked);
    attributes_set_dval(attrs, AStrBargap, g->bargap);
    xfile_empty_element(xf, EStrPresentationSpec, attrs);

    /* World coordinate scales */
    attributes_reset(attrs);
    xmlio_set_world_value(gr, attrs, AStrMin, g->w.xg1);
    xmlio_set_world_value(gr, attrs, AStrMax, g->w.xg2);
    attributes_set_sval(attrs, AStrType, scale_type_name(rt, g->xscale));
    attributes_set_bval(attrs, AStrInvert, g->xinvert);
    xfile_empty_element(xf, EStrXscale, attrs);
    attributes_reset(attrs);
    xmlio_set_world_value(gr, attrs, AStrMin, g->w.yg1);
    xmlio_set_world_value(gr, attrs, AStrMax, g->w.yg2);
    attributes_set_sval(attrs, AStrType, scale_type_name(rt, g->yscale));
    attributes_set_bval(attrs, AStrInvert, g->yinvert);
    xfile_empty_element(xf, EStrYscale, attrs);
    attributes_reset(attrs);
    attributes_set_dval(attrs, AStrNorm, g->znorm);
    xfile_empty_element(xf, EStrZscale, attrs);

    /* Locator */
    attributes_reset(attrs);
    attributes_set_sval(attrs, AStrType,
        glocator_type_name(rt_from_quark(gr), g->locator.type));
    xfile_begin_element(xf, EStrLocator, attrs);
    {
        attributes_reset(attrs);
        xmlio_set_active(attrs, g->locator.pointset);
        xmlio_set_world_value(gr, attrs, AStrX, g->locator.origin.x);
        xmlio_set_world_value(gr, attrs, AStrY, g->locator.origin.y);
        xfile_empty_element(xf, EStrFixedpoint, attrs);
        
        xmlio_write_format_spec(xf, attrs, EStrXformat, &g->locator.fx);
        xmlio_write_format_spec(xf, attrs, EStrYformat, &g->locator.fy);
    }
    xfile_end_element(xf, EStrLocator);

    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

int save_set_properties(XFile *xf, Quark *pset)
{
    Attributes *attrs;
    set *p;
    RunTime *rt = rt_from_quark(pset);
    
    if (!pset) {
        return RETURN_FAILURE;
    }
    
    p = set_get_data(pset);
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

    attributes_reset(attrs);
    attributes_set_sval(attrs, AStrType, sym_type_name(rt, p->sym.type));
    attributes_set_dval(attrs, AStrSize, p->sym.size);
    attributes_set_ival(attrs, AStrChar, (int) p->sym.symchar);
    xmlio_set_font_ref(attrs, p->sym.charfont);
    xfile_begin_element(xf, EStrSymbol, attrs);
    {
        xmlio_write_line_spec(xf, attrs, &p->sym.line);
        xmlio_write_fill_spec(xf, attrs, &(p->sym.fillpen));
    }
    xfile_end_element(xf, EStrSymbol);
    
    attributes_reset(attrs);
    attributes_set_sval(attrs, AStrType, line_type_name(rt, p->line.type));
    attributes_set_sval(attrs, AStrFillType,
        setfill_type_name(rt, p->line.filltype));
    attributes_set_sval(attrs, AStrFillRule,
        p->line.fillrule == FILLRULE_WINDING ? VStrWinding:VStrEvenodd);
    attributes_set_sval(attrs, AStrBaselineType,
        baseline_type_name(rt, p->line.baseline_type));
    attributes_set_bval(attrs, AStrDrawBaseline, p->line.baseline);
    attributes_set_bval(attrs, AStrDrawDroplines, p->line.droplines);
    xfile_begin_element(xf, EStrLine, attrs);
    {
        xmlio_write_line_spec(xf, attrs, &p->line.line);
        xmlio_write_fill_spec(xf, attrs, &p->line.fillpen);
    }
    xfile_end_element(xf, EStrLine);

    attributes_reset(attrs);
    xmlio_set_active(attrs, p->avalue.active);
    xmlio_set_offset(attrs, p->avalue.offset.x, p->avalue.offset.y);

    attributes_set_sval(attrs, AStrPrepend, p->avalue.prestr);
    attributes_set_sval(attrs, AStrAppend, p->avalue.appstr);
    xfile_begin_element(xf, EStrAnnotation, attrs);
    {
        xmlio_write_text_props(xf, attrs, &p->avalue.tprops);
        xmlio_write_format_spec(xf, attrs, EStrFormat, &p->avalue.format);
    }
    xfile_end_element(xf, EStrAnnotation);

    attributes_reset(attrs);
    xmlio_set_active(attrs, p->errbar.active);
    xmlio_set_side_placement(rt, attrs, p->errbar.ptype);
    xfile_begin_element(xf, EStrErrorbar, attrs);
    {
        attributes_reset(attrs);
        attributes_set_dval(attrs, AStrSize, p->errbar.barsize);
        xfile_begin_element(xf, EStrBarline, attrs);
        {
            Line line;
            line.pen   = p->errbar.pen;
            line.width = p->errbar.linew;
            line.style = p->errbar.lines;
            xmlio_write_line_spec(xf, attrs, &line);
        }
        xfile_end_element(xf, EStrBarline);
        
        attributes_reset(attrs);
        attributes_set_bval(attrs, AStrArrowClip, p->errbar.arrow_clip);
        attributes_set_dval(attrs, AStrClipLength, p->errbar.cliplen);
        xfile_begin_element(xf, EStrRiserline, attrs);
        {
            Line line;
            line.pen   = p->errbar.pen;
            line.width = p->errbar.riser_linew;
            line.style = p->errbar.riser_lines;
            xmlio_write_line_spec(xf, attrs, &line);
        }
        xfile_end_element(xf, EStrRiserline);
    }
    xfile_end_element(xf, EStrErrorbar);

    xfile_begin_element(xf, EStrLegendEntry, NULL);
    xmlio_write_text(xf, p->legstr);
    xfile_end_element(xf, EStrLegendEntry);

    attributes_free(attrs);
    
    return RETURN_SUCCESS;
}

static int save_ssd(XFile *xf, Quark *q)
{
    Attributes *attrs;
    unsigned int i, j, ncols, nrows;
    char *sformat;

    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }

#if 0
    attributes_set_sval(attrs, AStrComment, data->comment);
    if (data->hotlink) {
        attributes_set_sval(attrs, AStrHotfile, data->hotfile);
        /* FIXME: hotsrc */
    }
#endif

    ncols = ssd_get_ncols(q);
    nrows = ssd_get_nrows(q);
    sformat = project_get_sformat(get_parent_project(q));
    for (i = 0; i < ncols; i++) {
        ss_column *col = ssd_get_col(q, i);
        char *ename;
        
        attributes_reset(attrs);
        attributes_set_sval(attrs, AStrLabel, col->label);
        ename = col->format == FFORMAT_STRING ? EStrScolumn:EStrDcolumn;
        xfile_begin_element(xf, ename, attrs);
        
        for (j = 0; j < nrows; j++) {
            char *s, buf[32];
            if (col->format == FFORMAT_STRING) {
                s = ((char **) col->data)[j];
            } else {
                sprintf(buf, sformat, ((double *) col->data)[j]);
                s = buf;
            }

            xfile_text_element(xf, EStrCell, NULL, s, FALSE);
        }
        xfile_end_element(xf, ename);
    }
    
    attributes_free(attrs);

    return RETURN_SUCCESS;
}

static int save_dataset(XFile *xf, Quark *pset)
{
    Attributes *attrs;
    unsigned int nc, ncols;
    Dataset *data;

    data = set_get_dataset(pset);
    
    attrs = attributes_new();
    
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }
    
    ncols = settype_cols(set_get_type(pset));

    attributes_reset(attrs);
    for (nc = 0; nc < ncols; nc++) {
        if (data->cols[nc] != COL_NONE) {
            attributes_set_ival(attrs, dataset_colname(nc), data->cols[nc]);
        }
    }
    if (data->acol != COL_NONE) {
        attributes_set_ival(attrs, "s", data->acol);
    }
    
    attributes_set_sval(attrs, AStrComment, data->comment);
    xfile_empty_element(xf, EStrDataset, attrs);

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

static int project_save_hook(Quark *q,
    void *udata, QTraverseClosure *closure)
{
    XFile *xf = (XFile *) udata;
    Project *pr;
    frame *f;
    set *p;
    DObject *o;
    AText *at;
    region *r;
    Attributes *attrs;
    RunTime *rt = rt_from_quark(q);

    if (!q) {
        return RETURN_FAILURE;
    }

    attrs = attributes_new();
    if (attrs == NULL) {
        return RETURN_FAILURE;
    }
    
    switch (quark_fid_get(q)) {
    case QFlavorProject:
        pr = project_get_data(q);
        
        xfile_comment(xf, "Description");
        xfile_begin_element(xf, EStrDescription, NULL);
        {
            xmlio_write_text(xf, pr->description);
        }
        xfile_end_element(xf, EStrDescription);

        xfile_comment(xf, "Definitions");
        xfile_begin_element(xf, EStrDefinitions, NULL);
        {
            xfile_comment(xf, "Color map");
            save_colormap(xf, pr);
            xfile_comment(xf, "Font map");
            save_fontmap(xf, pr);
            attributes_reset(attrs);
            
            xfile_comment(xf, "Size scales");
            attributes_set_dval(attrs, AStrFontSize, pr->fscale);
            attributes_set_dval(attrs, AStrLineWidth, pr->lscale);
            xfile_empty_element(xf, EStrScales, attrs);
        }
        xfile_end_element(xf, EStrDefinitions);

        xfile_comment(xf, "Page properties");
        attributes_reset(attrs);
        attributes_set_ival(attrs, AStrWidth, pr->page_wpp);
        attributes_set_ival(attrs, AStrHeight, pr->page_hpp);
        attributes_set_bval(attrs, AStrFill, pr->bgfill);
        xmlio_set_color_ref(attrs, pr->bgcolor);
        xfile_empty_element(xf, EStrPage, attrs);

        xfile_comment(xf, "Data formats");
        xfile_begin_element(xf, EStrDataFormats, NULL);
        {
            attributes_reset(attrs);
            xmlio_set_world_value(q, attrs, AStrReference, pr->ref_date);
            attributes_set_bval(attrs, AStrWrap, pr->two_digits_years);
            attributes_set_ival(attrs, AStrWrapYear, pr->wrap_year);
            xfile_empty_element(xf, EStrDates, attrs);

            attributes_reset(attrs);
            attributes_set_sval(attrs, AStrFormat, pr->sformat);
            xfile_empty_element(xf, EStrWorld, attrs);
        }
        xfile_end_element(xf, EStrDataFormats);

        save_preferences(xf);
        break;
    case QFlavorSSD:
        if (!closure->post) {
            attributes_set_sval(attrs, AStrId, QIDSTR(q));
            xmlio_set_active(attrs, quark_is_active(q));
            attributes_set_ival(attrs, AStrRows, ssd_get_nrows(q));
            attributes_set_bval(attrs, AStrIndexed, ssd_is_indexed(q));

            xfile_begin_element(xf, EStrSSD, attrs);
            {
                save_ssd(xf, q);
            }
            
            closure->post = TRUE;
        } else {
            xfile_end_element(xf, EStrSSD);
        }
        break;
    case QFlavorFrame:
        if (!closure->post) {
            f = frame_get_data(q);

            attributes_set_sval(attrs, AStrId, QIDSTR(q));
            xmlio_set_active(attrs, quark_is_active(q));
            attributes_set_ival(attrs, AStrType, f->type); /* FIXME: textual */
            xfile_begin_element(xf, EStrFrame, attrs);
            save_frame_properties(xf, f);
            
            closure->post = TRUE;
        } else {
            xfile_end_element(xf, EStrFrame);
        }
        break;
    case QFlavorGraph:
        if (!closure->post) {
            attributes_set_sval(attrs, AStrId, QIDSTR(q));
            xmlio_set_active(attrs, quark_is_active(q));

            xfile_begin_element(xf, EStrGraph, attrs);
            save_graph_properties(xf, q);
    
            closure->post = TRUE;
        } else {
            xfile_end_element(xf, EStrGraph);
        }
        break;
    case QFlavorSet:
        p = set_get_data(q);
        
        attributes_set_sval(attrs, AStrId, QIDSTR(q));
        xmlio_set_active(attrs, quark_is_active(q));
        attributes_set_sval(attrs, AStrType, set_types(rt, p->type));
        attributes_set_ival(attrs, AStrSkip, p->symskip);
        attributes_set_dval(attrs, AStrSkipMinDist, p->symskipmindist);
        xfile_begin_element(xf, EStrSet, attrs);
        {
            save_dataset(xf, q);
            save_set_properties(xf, q);
        }
        xfile_end_element(xf, EStrSet);
        
        break;
    case QFlavorAGrid:
        if (!closure->post) {
            tickmarks *t = axisgrid_get_data(q);
            
            attributes_set_sval(attrs, AStrId, QIDSTR(q));
            xmlio_set_active(attrs, quark_is_active(q));

            attributes_set_sval(attrs, AStrType, axisgrid_is_x(q) ? "x":"y");
            xmlio_set_world_value(q, attrs, AStrMajorStep, t->tmajor);
            attributes_set_ival(attrs, AStrMinorDivisions, t->nminor);
            attributes_set_ival(attrs, AStrAutoTicking, t->t_autonum);
            attributes_set_bval(attrs, AStrRoundedPosition, t->t_round);
            xfile_begin_element(xf, EStrAGrid, attrs);
            
            save_axisgrid_properties(xf, q);
            
            closure->post = TRUE;
        } else {
            xfile_end_element(xf, EStrAGrid);
        }
        
        break;
    case QFlavorAxis:
        if (!closure->post) {
            attributes_set_sval(attrs, AStrId, QIDSTR(q));

            xmlio_set_active(attrs, quark_is_active(q));
            xmlio_set_axis_position(rt, attrs, axis_get_position(q));
            attributes_set_dval(attrs, AStrOffset, axis_get_offset(q));
            attributes_set_bval(attrs, AStrBar, axis_bar_enabled(q));
            attributes_set_bval(attrs, AStrTicks, axis_ticks_enabled(q));
            attributes_set_bval(attrs, AStrLabels, axis_labels_enabled(q));
            xfile_begin_element(xf, EStrAxis, attrs);
            
            closure->post = TRUE;
        } else {
            xfile_end_element(xf, EStrAxis);
        }
        
        break;
    case QFlavorDObject:
        o = object_get_data(q);

        attributes_set_sval(attrs, AStrId, QIDSTR(q));
        xmlio_set_active(attrs, quark_is_active(q));

        xmlio_set_angle(attrs, o->angle);
        xmlio_set_offset(attrs, o->offset.x, o->offset.y);
        xfile_begin_element(xf, EStrObject, attrs);
        {
            char buf[32];
            xmlio_write_location(q, xf, attrs, &o->ap);
            xmlio_write_line_spec(xf, attrs, &o->line);
            xmlio_write_fill_spec(xf, attrs, &o->fillpen);
            attributes_reset(attrs);
            switch (o->type) {
            case DO_LINE:
                {
                    DOLineData *l = (DOLineData *) o->odata;
                    attributes_set_dval(attrs, AStrX, l->vector.x);
                    attributes_set_dval(attrs, AStrY, l->vector.y);
                    attributes_set_sval(attrs, AStrArrowsAt,
                        arrow_placement_name(rt, l->arrow_end));
                }
                break;
            case DO_BOX:
                {
                    DOBoxData *b = (DOBoxData *) o->odata;
                    attributes_set_dval(attrs, AStrWidth, b->width);
                    attributes_set_dval(attrs, AStrHeight, b->height);
                }
                break;
            case DO_ARC:
                {
                    DOArcData *a = (DOArcData *) o->odata;
                    attributes_set_dval(attrs, AStrWidth, a->width);
                    attributes_set_dval(attrs, AStrHeight, a->height);
                    attributes_set_dval(attrs, AStrStartAngle, a->angle1);
                    attributes_set_dval(attrs, AStrExtentAngle, a->angle2);
                    attributes_set_sval(attrs, AStrClosureType,
                        arcclosure_type_name(rt, a->closure_type));
                    attributes_set_bval(attrs, AStrDrawClosure, a->draw_closure);
                }
                break;
            case DO_NONE:
                break;
            }
            sprintf(buf, "%s-data", object_types(rt, o->type));
            if (o->type == DO_LINE) {
                xfile_begin_element(xf, buf, attrs);
                {
                    DOLineData *l = (DOLineData *) o->odata;
                    xmlio_write_arrow(rt, xf, attrs, &l->arrow);
                }
                xfile_end_element(xf, buf);
            } else {
                xfile_empty_element(xf, buf, attrs);
            }
        }
        xfile_end_element(xf, EStrObject);

        break;
    case QFlavorAText:
        at = atext_get_data(q);

        attributes_set_sval(attrs, AStrId, QIDSTR(q));
        xmlio_set_active(attrs, quark_is_active(q));

        xmlio_set_offset(attrs, at->offset.x, at->offset.y);

        xfile_begin_element(xf, EStrAText, attrs);
        {
            xmlio_write_location(q, xf, attrs, &at->ap);
            xmlio_write_text_props(xf, attrs, &at->text_props);
            xmlio_write_text(xf, at->s);
            
            attributes_reset(attrs);
            attributes_set_sval(attrs, AStrType,
                framedecor_type_name(rt, at->frame_decor));
            attributes_set_dval(attrs, AStrOffset, at->frame_offset);
            xfile_begin_element(xf, EStrTextFrame, attrs);
            {
                xmlio_write_line_spec(xf, attrs, &at->line);
                xmlio_write_fill_spec(xf, attrs, &at->fillpen);
            }
            xfile_end_element(xf, EStrTextFrame);
            
            attributes_reset(attrs);
            attributes_set_bval(attrs, AStrActive, at->arrow_flag);
            xfile_begin_element(xf, EStrPointer, attrs);
            {
                xmlio_write_arrow(rt, xf, attrs, &at->arrow);
            }
            xfile_end_element(xf, EStrPointer);
        }
        xfile_end_element(xf, EStrAText);

        break;
    case QFlavorRegion:
        r = region_get_data(q);
        
        attributes_set_sval(attrs, AStrId, QIDSTR(q));
        xmlio_set_active(attrs, quark_is_active(q));
        
        attributes_set_sval(attrs,
            AStrType, region_types(rt, r->type));
        xmlio_set_color_ref(attrs, r->color);
        xfile_begin_element(xf, EStrRegion, attrs);
        {
            int i;
            for (i = 0; i < r->n; i++) {
                attributes_reset(attrs);
                xmlio_set_world_value(q, attrs, "X", r->wps[i].x);
                xmlio_set_world_value(q, attrs, "Y", r->wps[i].y);
                xfile_empty_element(xf, EStrRow, attrs);
            }
        }
        xfile_end_element(xf, EStrRegion);
        
        break;
    }
    
    attributes_free(attrs);
        
    return TRUE;
}


int save_project(Quark *project, char *fn)
{
    FILE *fp;
    XFile *xf;
    Attributes *attrs;
    GUI *gui = gui_from_quark(project);
    int noask_save;
    static int save_unsupported = FALSE;

    if (fn && strstr(fn, ".agr")) {
        errmsg("Cowardly refusing to overwrite an agr file");
        return RETURN_FAILURE;
    }
    if (!save_unsupported &&
        !yesno("The current format may be unsupported by the final release. Continue?",
            "Yea, I'm risky", NULL, "doc/UsersGuide.html#unsupported_format")) {
        return RETURN_FAILURE;
    }
    save_unsupported = TRUE;

    noask_save = gui->noask;
    if (strings_are_equal(project_get_docname(project), fn)) {
        /* If saving under the same name, don't warn about overwriting */
        gui->noask = TRUE;
    }
    
    fp = grace_openw(grace_from_quark(project), fn);
    gui->noask = noask_save;
    xf = xfile_new(fp);
    attrs = attributes_new();
    
    if (xf == NULL || attrs == NULL) {
        return RETURN_FAILURE;
    }
    
    xfile_set_ns(xf, GRACE_NS_PREFIX, GRACE_NS_URI, FALSE);
    
    attributes_reset(attrs);
    attributes_set_ival(attrs, AStrVersion, bi_version_id());
    xfile_begin(xf, FALSE, NULL, "grace.dtd", EStrGrace, attrs);

    quark_traverse(project, project_save_hook, xf);
        
    attributes_free(attrs);
    
    xfile_end(xf);
    xfile_free(xf);
    
    project_set_docname(project, fn);

    quark_dirtystate_set(project, FALSE);
    
    return RETURN_SUCCESS;
}
