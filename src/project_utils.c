/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2003 Grace Development Team
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

#include <string.h>

#include "grace.h"
#include "utils.h"
#include "core_utils.h"
#include "protos.h"

static int project_free_cb(Quark *pr, int etype, void *data)
{
    if (etype == QUARK_ETYPE_DELETE) {
        Grace *grace = grace_from_quark(pr);
        if (pr == grace->project) {
            grace->project = NULL;
        }
    } else
    if (etype == QUARK_ETYPE_MODIFY) {
        project_update_timestamp(pr, NULL);
        update_app_title(pr);

#if 0
        /* TODO: */
	if ((dirtystate > SOME_LIMIT) || 
            (current_time - autosave_time > ANOTHER_LIMIT) ) {
	    autosave();
	}
#endif
    }
#ifndef NONE_GUI
    clean_graph_selectors(pr, etype, data);
#endif
    return RETURN_SUCCESS;
}

/* TODO */
#define MAGIC_FONT_SCALE    0.028
#define MAGIC_LINEW_SCALE   0.0015
Quark *project_new(QuarkFactory *qfactory)
{
    Quark *q;
    
    q = quark_root(qfactory, QFlavorProject);
    project_set_version_id(q, bi_version_id());
    project_set_docname(q, NONAME);
    project_set_fontsize_scale(q, MAGIC_FONT_SCALE);
    project_set_linewidth_scale(q, MAGIC_LINEW_SCALE);
#ifndef NONE_GUI
    if (q) {
        quark_cb_set(q, project_free_cb, NULL);
    }
#endif
    return q;
}

void project_reset_version(Quark *q)
{
    Project *pr = project_get_data(q);
    pr->version_id = bi_version_id();
}


typedef struct {
    int ngraphs;
    Quark **graphs;
} graph_hook_t;

static int graph_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    graph_hook_t *p = (graph_hook_t *) udata;
    
    if (q->fid == QFlavorGraph) {
        p->ngraphs++;
        p->graphs = xrealloc(p->graphs, p->ngraphs*SIZEOF_VOID_P);
        p->graphs[p->ngraphs - 1] = q;
    }
    
    return TRUE;
}

int project_get_graphs(Quark *q, Quark ***graphs)
{
    graph_hook_t p;
    
    p.ngraphs = 0;
    p.graphs  = NULL;
    
    quark_traverse(q, graph_hook, &p);
    
    *graphs = p.graphs;
    
    return p.ngraphs;
}

int get_font_by_name(const Quark *project, const char *name)
{
    Project *pr = project_get_data(project);
    unsigned int i;
    
    for (i = 0; i < pr->nfonts; i++) {
        Fontdef *f = &pr->fontmap[i];
        if (!strcmp(f->fontname, name)) {
            return f->id;
        }
    }
    
    return BAD_FONT_ID;
}

static int fcomp(const Quark *q1, const Quark *q2, void *udata)
{
    if (q1->fid == QFlavorAText) {
        return 1;
    } else
    if (q2->fid == QFlavorAText) {
        return -1;
    } else
    if (q1->fid == QFlavorDObject && q2->fid == QFlavorDObject) {
        DObject *o1 = object_get_data(q1), *o2 = object_get_data(q2);
        if (o1->type != o2->type) {
            return (o1->type - o2->type);
        } else {
            return strcmp(QIDSTR(q1), QIDSTR(q2));
        }
    } else
    if (q1->fid == QFlavorDObject) {
        return 1;
    } else
    if (q2->fid == QFlavorDObject) {
        return -1;
    } else
    if (q1->fid == QFlavorAxis) {
        tickmarks *t = axis_get_data(q1);
        if (t->props.gridflag || t->mprops.gridflag) {
            return -1;
        } else {
            return 1;
        }
    } else
    if (q2->fid == QFlavorAxis) {
        tickmarks *t = axis_get_data(q2);
        if (t->props.gridflag || t->mprops.gridflag) {
            return 1;
        } else {
            return -1;
        }
    } else
    if (q1->fid == q2->fid) {
        return strcmp(QIDSTR(q1), QIDSTR(q2));
    } else {
        return 0;
    }
}

static int project_postprocess_hook(Quark *q,
    void *udata, QTraverseClosure *closure)
{
    int version_id = *((int *) udata);
    Project *pr;
    frame *f;
    tickmarks *t;
    set *s;
    int gtype;
    
    switch (q->fid) {
    case QFlavorProject:
        pr = project_get_data(q);
        
        if (version_id < 50200) {
            quark_sort_children(q, fcomp, NULL);
        }

        if (version_id < 40005) {
            set_page_dimensions(grace_from_quark(q), 792, 612, FALSE);
        }

        if (version_id < 50002) {
            pr->bgfill = TRUE;
        }

        if (version_id < 50003) {
            pr->two_digits_years = TRUE;
            pr->wrap_year = 1900;
        }

        if (version_id <= 40102) {
            double ext_x, ext_y;
#ifndef NONE_GUI
            GUI *gui = gui_from_quark(q);
            gui_set_page_free(gui, FALSE);
#endif
            if (project_get_viewport(q, &ext_x, &ext_y) == RETURN_SUCCESS) {
                rescale_viewport(q, ext_x, ext_y);
            }
        }
        break;
    case QFlavorFrame:
        f = frame_get_data(q);
        
	if (version_id <= 40102) {
            f->l.vgap -= 0.01;
        }
	if (version_id < 50200) {
            f->l.acorner = CORNER_UL;
        }

        break;
    case QFlavorGraph:
        if (version_id < 50200) {
            quark_sort_children(q, fcomp, NULL);
        }

        break;
    case QFlavorAxis:
	t = axis_get_data(q);

        if (version_id <= 40102) {
            if ((axis_is_x(q) && islogx(q)) ||
                (axis_is_y(q) && islogy(q))) {
                t->tmajor = pow(10.0, t->tmajor);
            }

            /* TODO : world/view translation */
            t->offsx = 0.0;
            t->offsy = 0.0;
        }
	if (version_id < 50000) {
	    /* There was no label_op in Xmgr */
            t->label_op = t->tl_op;

            /* in xmgr, axis label placement was in x,y coordinates */
	    /* in Grace, it's parallel/perpendicular */
	    if (axis_is_y(q)) {
	        fswap(&t->label_offset.x,
                      &t->label_offset.y);
	    }
	    t->label_offset.y *= -1;
	}
	if (version_id >= 50000 && version_id < 50103) {
	    /* Autoplacement of axis labels wasn't implemented 
               in early versions of Grace */
            if (t->label_place == TYPE_AUTO) {
                t->label_offset.x = 0.0;
                t->label_offset.y = 0.08;
                t->label_place = TYPE_SPEC;
            }
        }
        if (version_id < 50105) {
            /* Starting with 5.1.5, X axis min & inverting is honored
               in pie charts */
            if (graph_get_type(q) == GRAPH_PIE) {
                world w;
                graph_get_world(q, &w);
                w.xg1 = 0.0;
                w.xg2 = 2*M_PI;
                graph_set_world(q, &w);
                graph_set_xinvert(q, FALSE);
            }
        }

        break;
    case QFlavorSet:
        s = set_get_data(q);
        gtype = graph_get_type(get_parent_graph(q));

        if (version_id < 50000) {
            switch (s->sym.type) {
            case SYM_NONE:
                break;
            case SYM_DOT_OBS:
                s->sym.type = SYM_CIRCLE;
                s->sym.size = 0.0;
                s->sym.line.style = 0;
                s->sym.fillpen.pattern = 1;
                break;
            default:
                s->sym.type--;
                break;
            }
        }
        if ((version_id < 40004 && gtype != GRAPH_CHART) ||
            s->sym.line.pen.color == -1) {
            s->sym.line.pen.color = s->line.line.pen.color;
        }
        if (version_id < 40200 || s->sym.fillpen.color == -1) {
            s->sym.fillpen.color = s->sym.line.pen.color;
        }

	if (version_id <= 40102 && gtype == GRAPH_CHART) {
            s->type       = SET_BAR;
            s->sym.line    = s->line.line;
            s->line.line.style = 0;

            s->sym.fillpen = s->line.fillpen;
            s->line.fillpen.pattern = 0;
        }
	if (version_id <= 40102 && s->type == SET_XYHILO) {
            s->sym.line.width = s->line.line.width;
        }
	if (version_id <= 50112 && s->type == SET_XYHILO) {
            s->avalue.active = FALSE;
        }
	if (version_id < 50100 && s->type == SET_BOXPLOT) {
            s->sym.line.width = s->line.line.width;
            s->sym.line.style = s->line.line.style;
            s->sym.size = 2.0;
            s->errbar.riser_linew = s->line.line.width;
            s->errbar.riser_lines = s->line.line.style;
            s->line.line.style = 0;
            s->errbar.barsize = 0.0;
        }
        if (version_id < 50003) {
            s->errbar.active = TRUE;
            s->errbar.pen.color = s->sym.line.pen.color;
            s->errbar.pen.pattern = 1;
            switch (s->errbar.ptype) {
            case PLACEMENT_NORMAL:
                s->errbar.ptype = PLACEMENT_OPPOSITE;
                break;
            case PLACEMENT_OPPOSITE:
                s->errbar.ptype = PLACEMENT_NORMAL;
                break;
            case PLACEMENT_BOTH:
                switch (s->type) {
                case SET_XYDXDX:
                case SET_XYDYDY:
                case SET_BARDYDY:
                    s->errbar.ptype = PLACEMENT_NORMAL;
                    break;
                }
                break;
            }
        }
        if (version_id < 50002) {
            s->errbar.barsize *= 2;
        }

        if (version_id < 50107) {
            /* Starting with 5.1.7, symskip is honored for all set types */
            switch (s->type) {
            case SET_BAR:
            case SET_BARDY:
            case SET_BARDYDY:
            case SET_XYHILO:
            case SET_XYR:
            case SET_XYVMAP:
            case SET_BOXPLOT:
                s->symskip = 0;
                break;
            }
        }
        
        break;
    case QFlavorDObject:
        if (version_id < 50200) {
            DObject *o = object_get_data(q);
            Quark *gr = get_parent_graph(q);
            if (object_get_loctype(q) == COORD_WORLD) {
                WPoint wp;
                VPoint vp1, vp2;

                switch (o->type) {
                case DO_BOX:
                    {
                        DOBoxData *b = (DOBoxData *) o->odata;
                        wp.x = o->ap.x - b->width/2;
                        wp.y = o->ap.y - b->height/2;
                        Wpoint2Vpoint(gr, &wp, &vp1);
                        wp.x = o->ap.x + b->width/2;
                        wp.y = o->ap.y + b->height/2;
                        Wpoint2Vpoint(gr, &wp, &vp2);

                        b->width  = fabs(vp2.x - vp1.x);
                        b->height = fabs(vp2.y - vp1.y);
                    }
                    break;
                case DO_ARC:
                    {
                        DOArcData *a = (DOArcData *) o->odata;
                        wp.x = o->ap.x - a->width/2;
                        wp.y = o->ap.y - a->height/2;
                        Wpoint2Vpoint(gr, &wp, &vp1);
                        wp.x = o->ap.x + a->width/2;
                        wp.y = o->ap.y + a->height/2;
                        Wpoint2Vpoint(gr, &wp, &vp2);

                        a->width  = fabs(vp2.x - vp1.x);
                        a->height = fabs(vp2.y - vp1.y);
                    }
                    break;
                case DO_LINE:
                    {
                        DOLineData *l = (DOLineData *) o->odata;
                        wp.x = o->ap.x;
                        wp.y = o->ap.y;
                        Wpoint2Vpoint(gr, &wp, &vp1);
                        wp.x = o->ap.x + l->vector.x;
                        wp.y = o->ap.y + l->vector.y;
                        Wpoint2Vpoint(gr, &wp, &vp2);

                        l->vector.x = vp2.x - vp1.x;
                        l->vector.y = vp2.y - vp1.y;
                    }
                    break;
                case DO_NONE:
                    break;
                }
            }
        }

        break;
    case QFlavorAText:
        if (version_id >= 40200 && version_id <= 50005 &&
            !compare_strings(q->idstr, "timestamp")) {
            /* BBox type justification was erroneously set */
            AText *at = atext_get_data(q);
            if (at) {
                at->text_props.just |= JUST_MIDDLE;
            }
        }
        break;
    }
    
    return TRUE;
}

void project_postprocess(Quark *project)
{
    int version_id = project_get_version_id(project);
    
    if (version_id >= bi_version_id()) {
        return;
    }
    
    quark_traverse(project, project_postprocess_hook, &version_id);
}

int project_get_viewport(const Quark *project, double *vx, double *vy)
{
    Project *pr = project_get_data(project);
    if (pr && pr->page_wpp > 0 && pr->page_hpp > 0) {
        if (pr->page_wpp < pr->page_hpp) {
            *vy = (double) pr->page_hpp/pr->page_wpp;
            *vx = 1.0;
        } else {
            *vx = (double) pr->page_wpp/pr->page_hpp;
            *vy = 1.0;
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}
