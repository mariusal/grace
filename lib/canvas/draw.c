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

/* 
 *
 * Interface to device-independent drawing
 *
 */
 
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grace/baseP.h"
#include "grace/canvasP.h"
#include "patterns.h"

static int clip_polygon(VPoint *vps, int n, const view *clipview);
static int all_points_inside(const view *clipview, const VPoint *vps, int n);
static void purge_dense_points(const VPoint *vps, int n, VPoint *pvps, int *np);
static int realloc_colors(Canvas *canvas, unsigned int n);
static int RGB2YIQ(const RGB *rgb, YIQ *yiq);
static int RGB2CMY(const RGB *rgb, CMY *cmy);
static void canvas_stats_update(Canvas *canvas, int type);
static void canvas_char_stats_update(Canvas *canvas,
    int font, const char *s, int len);
static int realloc_patterns(Canvas *canvas, unsigned int n);
static int realloc_linestyles(Canvas *canvas, unsigned int n);

/* Default drawing properties */
static DrawProps default_draw_props = 
{{1, 1}, 0, 1, 0.0, LINECAP_BUTT, LINEJOIN_MITER, 1.0, 0, FILLRULE_WINDING};

#define CANVAS_STATS_COLOR     1
#define CANVAS_STATS_PATTERN   2
#define CANVAS_STATS_LINESTYLE 4
#define CANVAS_STATS_FONT      8

#define CANVAS_STATS_PEN  (CANVAS_STATS_COLOR | CANVAS_STATS_PATTERN)
#define CANVAS_STATS_LINE (CANVAS_STATS_PEN | CANVAS_STATS_LINESTYLE)

void canvas_dev_drawpixel(Canvas *canvas, const VPoint *vp)
{
    canvas_stats_update(canvas, CANVAS_STATS_COLOR);
    if (!canvas->drypass) {
        canvas->curdevice->drawpixel(canvas, canvas->curdevice->data, vp);
    }
}

void canvas_dev_drawpolyline(Canvas *canvas,
    const VPoint *vps, int n, int mode)
{
    canvas_stats_update(canvas, CANVAS_STATS_LINE);
    if (!canvas->drypass) {
        canvas->curdevice->drawpolyline(canvas, canvas->curdevice->data,
            vps, n, mode);
    }
}

void canvas_dev_fillpolygon(Canvas *canvas, const VPoint *vps, int nc)
{
    canvas_stats_update(canvas, CANVAS_STATS_PEN);
    if (!canvas->drypass) {
        canvas->curdevice->fillpolygon(canvas, canvas->curdevice->data,
            vps, nc);
    }
}

void canvas_dev_drawarc(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    canvas_stats_update(canvas, CANVAS_STATS_LINE);
    if (!canvas->drypass) {
        canvas->curdevice->drawarc(canvas, canvas->curdevice->data,
            vp1, vp2, a1, a2);
    }
}

void canvas_dev_fillarc(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    canvas_stats_update(canvas, CANVAS_STATS_PEN);
    if (!canvas->drypass) {
        canvas->curdevice->fillarc(canvas, canvas->curdevice->data,
            vp1, vp2, a1, a2, mode);
    }
}

void canvas_dev_putpixmap(Canvas *canvas, const VPoint *vp, const CPixmap *pm)
{
    if (!pm) {
        return;
    }
    
    if (pm->bpp == 1) {
        canvas_stats_update(canvas, CANVAS_STATS_COLOR);
        if (pm->type != PIXMAP_TRANSPARENT) {
            canvas->cmap[getbgcolor(canvas)].used = 1;
        }
    } else {
        int j, k;
        for (k = 0; k < pm->height; k++) {
            for (j = 0; j < pm->width; j++) {
                int cindex = (pm->bits)[k*pm->width+j];
                canvas->cmap[cindex].used = 1;
            }
        }
    }
    if (!canvas->drypass) {
        canvas->curdevice->putpixmap(canvas, canvas->curdevice->data, vp, pm);
    }
}

void canvas_dev_puttext(Canvas *canvas,
    const VPoint *vp, const char *s, int len, int font, const TextMatrix *tm,
    int underline, int overline, int kerning)
{
    if (canvas->curdevice->puttext == NULL) {
        errmsg("Device has no built-in fonts");
    } else {
        canvas_stats_update(canvas, CANVAS_STATS_PEN);
        canvas_char_stats_update(canvas, font, s, len);
        if (!canvas->drypass) {
            canvas->curdevice->puttext(canvas, canvas->curdevice->data,
                vp, s, len, font, tm, underline, overline, kerning);
        }
    }
}

static int csparse_proc_default(const Canvas *canvas,
    const char *s, CompositeString *cstring)
{
    CStringSegment *cseg;
    
    if (s && (cseg = cstring_seg_new(cstring))) {
        cseg->s = copy_string(NULL, s);
        cseg->len = strlen(s);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int fmap_proc_default(const Canvas *canvas, int font)
{
    return font;
}

Canvas *canvas_new(void)
{
    Canvas *canvas;
    
    canvas = xmalloc(sizeof(Canvas));
    if (canvas) {
        memset(canvas, 0, sizeof(Canvas));
        
        canvas->draw_props      = default_draw_props;
        canvas->clipflag        = TRUE;
        canvas->draw_mode       = TRUE;
        canvas->max_path_length = MAX_DRAWING_PATH;
        
        /* initialize colormap data */
        canvas_cmap_reset(canvas);
        /* initialize pattern data */
        initialize_patterns(canvas);
        /* initialize linestyle data */
        initialize_linestyles(canvas);
        
        canvas->csparse_proc = csparse_proc_default;
        canvas->fmap_proc    = fmap_proc_default;
        canvas->fscale       = 1.0;
        canvas->lscale       = 1.0;
        
        /* initialize T1lib */
        if (init_t1(canvas) != RETURN_SUCCESS) {
	    canvas_free(canvas);
            
            return NULL;
        }
    }
    
    return canvas;
}

void canvas_free(Canvas *canvas)
{
    if (canvas) {
        /* free fonts */
        while (canvas->nfonts) {
            FontDB *f = &canvas->FontDBtable[canvas->nfonts - 1];
            xfree(f->alias);
            canvas->nfonts--;
        }
        xfree(canvas->FontDBtable);
        
        /* free colors, patterns, linestyles */
        realloc_colors(canvas, 0);
        realloc_patterns(canvas, 0);
        realloc_linestyles(canvas, 0);
        
        /* free devices */
        while (canvas->ndevices) {
            Device_entry *d = canvas->device_table[canvas->ndevices - 1];
            device_free(d);
            canvas->ndevices--;
        }
        xfree(canvas->device_table);
        
        /* free some strings */
        xfree(canvas->username);
        xfree(canvas->docname);
        xfree(canvas->description);
        
        /* ... and the structure itself */
        xfree(canvas);
    }
}

/*
 * clip if clipflag = TRUE
 */
void setclipping(Canvas *canvas, int flag)
{
    canvas->clipflag = flag;
}

int canvas_set_clipview(Canvas *canvas, const view *v)
{
    if (isvalid_viewport(v)) {
        canvas->clipview = *v;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

void canvas_set_username(Canvas *canvas, const char *s)
{
    canvas->username = copy_string(canvas->username, s);
}

void canvas_set_docname(Canvas *canvas, const char *s)
{
    canvas->docname = copy_string(canvas->docname, s);
}

void canvas_set_description(Canvas *canvas, const char *s)
{
    canvas->description = copy_string(canvas->description, s);
}


void set_draw_mode(Canvas *canvas, int mode)
{
    canvas->draw_mode = mode ? TRUE:FALSE;
}

void set_max_path_limit(Canvas *canvas, int limit)
{
    canvas->max_path_length = limit;
}

/*
 * set pen properties
 */
void setpen(Canvas *canvas, const Pen *pen)
{
    canvas->draw_props.pen = *pen;
}

/*
 * set line drawing properties
 */
void setline(Canvas *canvas, const Line *line)
{
    canvas->draw_props.pen   = line->pen;
    canvas->draw_props.linew = line->width;
    canvas->draw_props.lines = line->style;
}

/*
 * make the current color color
 */
void setcolor(Canvas *canvas, int color)
{
    canvas->draw_props.pen.color = color;
}

void setpattern(Canvas *canvas, int pattern)
{
    canvas->draw_props.pen.pattern = pattern;
}

/*
 * set the background color of the canvas
 */
void setbgcolor(Canvas *canvas, int bgcolor)
{
    canvas->draw_props.bgcolor = bgcolor;
}

/*
 * make the current linestyle lines
 */
void setlinestyle(Canvas *canvas, int lines)
{
    canvas->draw_props.lines = lines;
}

/*
 * make the current line width linew
 */
void setlinewidth(Canvas *canvas, double linew)
{
    canvas->draw_props.linew = linew;
}

/*
 * set the current character size to size
 */
void setcharsize(Canvas *canvas, double charsize)
{
    canvas->draw_props.charsize = charsize;
}

/*
 * setfont - make font the current font to use for writing strings
 */
void setfont(Canvas *canvas, int font)
{
    canvas->draw_props.font = font;
}

/*
 * set the current fillrule
 */
void setfillrule(Canvas *canvas, int rule)
{
    canvas->draw_props.fillrule = rule;
}

/*
 * set the current linecap parameter
 */
void setlinecap(Canvas *canvas, int type)
{
    canvas->draw_props.linecap = type;
}

/*
 * set the current linejoin type
 */
void setlinejoin(Canvas *canvas, int type)
{
    canvas->draw_props.linejoin = type;
}

void getpen(const Canvas *canvas, Pen *pen)
{
    *pen = canvas->draw_props.pen;
}

int getcolor(const Canvas *canvas)
{
    return canvas->draw_props.pen.color;
}

int getbgcolor(const Canvas *canvas)
{
    return canvas->draw_props.bgcolor;
}

int getpattern(const Canvas *canvas)
{
    return canvas->draw_props.pen.pattern;
}

int getlinestyle(const Canvas *canvas)
{
    return canvas->draw_props.lines;
}

int getlinecap(const Canvas *canvas)
{
    return canvas->draw_props.linecap;
}

int getlinejoin(const Canvas *canvas)
{
    return canvas->draw_props.linejoin;
}

int getfillrule(const Canvas *canvas)
{
    return canvas->draw_props.fillrule;
}

double getlinewidth(const Canvas *canvas)
{
    return canvas->lscale*canvas->draw_props.linew;
}

double getcharsize(const Canvas *canvas)
{
    return canvas->fscale*canvas->draw_props.charsize;
}

int getfont(const Canvas *canvas)
{
    return canvas->draw_props.font;
}

char *canvas_get_username(const Canvas *canvas)
{
    return canvas->username;
}

char *canvas_get_docname(const Canvas *canvas)
{
    return canvas->docname;
}

char *canvas_get_description(const Canvas *canvas)
{
    return canvas->description;
}

void canvas_set_udata(Canvas *canvas, void *data)
{
    canvas->udata = data;
}

void *canvas_get_udata(const Canvas *canvas)
{
    return canvas->udata;
}

void canvas_set_csparse_proc(Canvas *canvas, CanvasCSParseProc csparse_proc)
{
    canvas->csparse_proc = csparse_proc;
}

void canvas_set_fmap_proc(Canvas *canvas, CanvasFMapProc fmap_proc)
{
    canvas->fmap_proc = fmap_proc;
}

void canvas_set_fontsize_scale(Canvas *canvas, double fscale)
{
    canvas->fscale = fscale;
}

void canvas_set_linewidth_scale(Canvas *canvas, double lscale)
{
    canvas->lscale = lscale;
}

void canvas_set_pagefill(Canvas *canvas, int flag)
{
    canvas->pagefill = flag;
}

void canvas_set_prstream(Canvas *canvas, FILE *prstream)
{
    canvas->prstream = prstream;
}

FILE *canvas_get_prstream(const Canvas *canvas)
{
    return canvas->prstream;
}

int get_draw_mode(const Canvas *canvas)
{
    return (canvas->draw_mode);
}

int get_max_path_limit(const Canvas *canvas)
{
    return canvas->max_path_length;
}


int initgraphics(Canvas *canvas, const CanvasStats *cstats)
{
    unsigned int i;
    int retval;

    for (i = 0; i < canvas->ncolors; i++) {
        CMap_entry *cmap = &canvas->cmap[i];
        canvas_color_trans(canvas, cmap);
    }
    
    retval = canvas->curdevice->initgraphics(canvas,
        canvas->curdevice->data, cstats);
    
    if (retval == RETURN_SUCCESS) {
        canvas->device_ready = TRUE;
    } else {
        canvas->device_ready = FALSE;
    }
    
    return retval;
}

void leavegraphics(Canvas *canvas, const CanvasStats *cstats)
{
    canvas->curdevice->leavegraphics(canvas, canvas->curdevice->data, cstats);
    canvas->device_ready = FALSE;
}


/*
 * DrawPixel - put a pixel in the current color at position vp
 */
void DrawPixel(Canvas *canvas, const VPoint *vp)
{
     if (is_validVPoint(canvas, vp)) {
         if (canvas->draw_mode) {
             canvas_dev_drawpixel(canvas, vp);
         }
         update_bboxes(canvas, vp);
     }
}

/*
 * DrawPolyline - draw a connected line in the current color and linestyle
 *            with nodes given by vps[]
 */
void DrawPolyline(Canvas *canvas, const VPoint *vps, int n, int mode)
{
    int i, nmax, nc, max_purge, npurged;
    VPoint vp1, vp2;
    VPoint vp1c, vp2c;
    VPoint *vpsc;
    
    if (getlinestyle(canvas) == 0 || getpattern(canvas) == 0) {
        return;
    }
    
    if (n <= 1) {
        return;
    }
    
    if (mode == POLYLINE_CLOSED) {
        nmax = n + 1;
    } else {
        nmax = n;
    }
    
    max_purge = get_max_path_limit(canvas);
    
/*
 *  in most real cases, all points of a set are inside the viewport;
 *  so we check it prior to going into compilated clipping mode
 */
    if (canvas->clipflag && !all_points_inside(&canvas->clipview, vps, n)) {
        
        vpsc = (VPoint *) xmalloc((nmax)*sizeof(VPoint));
        if (vpsc == NULL) {
            errmsg("xmalloc() failed in DrawPolyline()");
            return;
        }
        
        nc = 0;
        for (i = 0; i < nmax - 1; i++) {
            vp1 = vps[i];
            if (i < n - 1) {
                vp2 = vps[i + 1];
            } else {
                vp2 = vps[0];
            }
            if (clip_line(canvas, &vp1, &vp2, &vp1c, &vp2c)) {
                if (nc == 0) {
                    vpsc[nc] = vp1c;
                    nc++;
                }
                vpsc[nc] = vp2c;
                nc++;
                
                if (vp2.x != vp2c.x || vp2.y != vp2c.y || i == nmax - 2) {
                    update_bboxes_with_vpoints(canvas,
                        vpsc, nc, getlinewidth(canvas));
                    
                    if (get_draw_mode(canvas) == TRUE) {
                        if (nc != nmax) {
                            mode = POLYLINE_OPEN;
                        }
                        if (max_purge && nc > max_purge) {
                            npurged = max_purge;
                            purge_dense_points(vpsc, nc, vpsc, &npurged);
                        } else {
                            npurged = nc;
                        }
                        canvas_dev_drawpolyline(canvas, vpsc, npurged, mode);
                    }
                    
                    nc = 0;
                }
            }
        }
        xfree(vpsc);
    } else {
        update_bboxes_with_vpoints(canvas, vps, n, getlinewidth(canvas));

        if (get_draw_mode(canvas) == TRUE) {
            if (max_purge && n > max_purge) {
                npurged = max_purge;
                vpsc = xmalloc(max_purge*sizeof(VPoint));
                if (vpsc == NULL) {
                    errmsg("xmalloc() failed in DrawPolyline()");
                    return;
                }
                purge_dense_points(vps, n, vpsc, &npurged);
                canvas_dev_drawpolyline(canvas, vpsc, npurged, mode);
                xfree(vpsc);
            } else {
                canvas_dev_drawpolyline(canvas, vps, n, mode);
            }
        }
    }
}

/*
 * DrawPolygon - draw a filled polygon in the current color and pattern
 *      with nodes given by vps[]
 */
void DrawPolygon(Canvas *canvas, const VPoint *vps, int n)
{
    int nc, max_purge, npurged;
    VPoint *vptmp;

    if (getpattern(canvas) == 0) {
        return;
    }
    if (n < 3) {
        return;
    }

    max_purge = get_max_path_limit(canvas);
    
    if (canvas->clipflag && !all_points_inside(&canvas->clipview, vps, n)) {
        /* In the worst case, the clipped polygon may have twice more vertices */
        vptmp = xmalloc((2*n) * sizeof(VPoint));
        if (vptmp == NULL) {
            errmsg("xmalloc() failed in DrawPolygon");
            return;
        } else {
            memcpy(vptmp, vps, n * sizeof(VPoint));
            nc = clip_polygon(vptmp, n, &canvas->clipview);
            if (nc > 2) {
                update_bboxes_with_vpoints(canvas, vptmp, nc, 0.0);
                
                if (get_draw_mode(canvas) == TRUE) {
                    if (max_purge && nc > max_purge) {
                        npurged = max_purge;
                        purge_dense_points(vptmp, nc, vptmp, &npurged);
                    } else {
                        npurged = nc;
                    }
                    canvas_dev_fillpolygon(canvas, vptmp, npurged);
                }
            }
            xfree(vptmp);
        }
    } else {
        update_bboxes_with_vpoints(canvas, vps, n, 0.0);

        if (get_draw_mode(canvas) == TRUE) {
            if (max_purge && n > max_purge) {
                npurged = max_purge;
                vptmp = xmalloc(max_purge*sizeof(VPoint));
                if (vptmp == NULL) {
                    errmsg("xmalloc() failed in DrawPolygon()");
                    return;
                }
                purge_dense_points(vps, n, vptmp, &npurged);
                canvas_dev_fillpolygon(canvas, vptmp, npurged);
                xfree(vptmp);
            } else {
                canvas_dev_fillpolygon(canvas, vps, n);
            }
        }
    }
}

/*
 * DrawArc - draw an arc line 
 */
void DrawArc(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, double angle1, double angle2)
{
    view v;
    
    if (getlinestyle(canvas) == 0 || getpattern(canvas) == 0) {
        return;
    }
    
    /* TODO: clipping!!!*/
    if (get_draw_mode(canvas) == TRUE) {
        canvas_dev_drawarc(canvas, vp1, vp2, angle1, angle2);
    }
    
    /* TODO: consider open arcs! */
    VPoints2bbox(vp1, vp2, &v);
    view_extend(&v, getlinewidth(canvas)/2);
    update_bboxes_with_view(canvas, &v);
}

/*
 * DrawFilledArc - draw a filled arc 
 */
void DrawFilledArc(Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, double angle1, double angle2, int mode)
{
    if (getpattern(canvas) == 0) {
        return;
    }

    if (points_overlap(canvas, vp1, vp2)) {
        DrawPixel(canvas, vp1);
        return;
    }
        
    /* TODO: clipping!!!*/
    if (get_draw_mode(canvas) == TRUE) {
        canvas_dev_fillarc(canvas, vp1, vp2, angle1, angle2, mode);
    }
    /* TODO: consider open arcs! */
    update_bboxes(canvas, vp1);
    update_bboxes(canvas, vp2);
}



/*
 * DrawRect - draw a rectangle using the current color and linestyle
 */
void DrawRect(Canvas *canvas, const VPoint *vp1, const VPoint *vp2)
{
    VPoint vps[4];
    
    vps[0].x = vp1->x;
    vps[0].y = vp1->y;
    vps[1].x = vp1->x;
    vps[1].y = vp2->y;
    vps[2].x = vp2->x;
    vps[2].y = vp2->y;
    vps[3].x = vp2->x;
    vps[3].y = vp1->y;
    
    DrawPolyline(canvas, vps, 4, POLYLINE_CLOSED);
}

/*
 * DrawRect - draw a rectangle using the current color and linestyle
 */
void FillRect(Canvas *canvas, const VPoint *vp1, const VPoint *vp2)
{
    VPoint vps[4];
    
    vps[0].x = vp1->x;
    vps[0].y = vp1->y;
    vps[1].x = vp1->x;
    vps[1].y = vp2->y;
    vps[2].x = vp2->x;
    vps[2].y = vp2->y;
    vps[3].x = vp2->x;
    vps[3].y = vp1->y;
    
    DrawPolygon(canvas, vps, 4);
}

/*
 * DrawLine - draw a straight line in the current color and linestyle
 *            with nodes given by vp1 and vp2
 */
void DrawLine(Canvas *canvas, const VPoint *vp1, const VPoint *vp2)
{
    VPoint vps[2];
    
    vps[0] = *vp1;
    vps[1] = *vp2;
    
    DrawPolyline(canvas, vps, 2, POLYLINE_OPEN);
}

/*
 * DrawEllipse - draw an ellipse
 */
void DrawEllipse(Canvas *canvas, const VPoint *vp1, const VPoint *vp2)
{
    DrawArc(canvas, vp1, vp2, 0.0, 360.0);
}

/*
 * DrawFilledEllipse - draw a filled ellipse
 */
void DrawFilledEllipse(Canvas *canvas, const VPoint *vp1, const VPoint *vp2)
{
    DrawFilledArc(canvas, vp1, vp2, 0.0, 360.0, ARCFILL_CHORD);
}

/*
 * DrawCircle - draw a circle
 */
void DrawCircle(Canvas *canvas, const VPoint *vp, double radius)
{
    VPoint vp1, vp2;
    
    vp1.x = vp->x - radius;
    vp1.y = vp->y - radius;
    vp2.x = vp->x + radius;
    vp2.y = vp->y + radius;
    
    DrawArc(canvas, &vp1, &vp2, 0.0, 360.0);
}

/*
 * DrawFilledCircle - draw a filled circle
 */
void DrawFilledCircle(Canvas *canvas, const VPoint *vp, double radius)
{
    VPoint vp1, vp2;
    
    vp1.x = vp->x - radius;
    vp1.y = vp->y - radius;
    vp2.x = vp->x + radius;
    vp2.y = vp->y + radius;
    
    DrawFilledArc(canvas, &vp1, &vp2, 0.0, 360.0, ARCFILL_CHORD);
}



/* 
 * ------------------ Clipping routines ---------------
 */

void vpswap(VPoint *vp1, VPoint *vp2)
{
    VPoint vptmp;

    vptmp = *vp1;
    *vp1 = *vp2;
    *vp2 = vptmp;
}

int isvalid_viewport(const view *v)
{
    if ((v->xv2 <= v->xv1) || (v->yv2 <= v->yv1)) {
	return FALSE;
    } else {
        return TRUE;
    }
}

int points_overlap(const Canvas *canvas, const VPoint *vp1, const VPoint *vp2)
{
    double delta;
    
    delta = 1.0/MIN2(page_width(canvas), page_height(canvas));
    if (fabs(vp2->x - vp1->x) < delta || fabs(vp2->y - vp1->y) < delta) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int VPoints2bbox(const VPoint *vp1, const VPoint *vp2, view *bb)
{
    if (!bb || !vp1 || !vp2) {
        return RETURN_FAILURE;
    } else {
        if (vp1->x <= vp2->x) {
            bb->xv1 = vp1->x;
            bb->xv2 = vp2->x;
        } else {
            bb->xv1 = vp2->x;
            bb->xv2 = vp1->x;
        }
        if (vp1->y <= vp2->y) {
            bb->yv1 = vp1->y;
            bb->yv2 = vp2->y;
        } else {
            bb->yv1 = vp2->y;
            bb->yv2 = vp1->y;
        }
        
        return RETURN_SUCCESS;
    }
}


/* some to avoid round errors due to the finite FP precision */
#define VP_EPSILON  0.0001

/*
 * is_vpoint_inside() checks if point vp is inside of viewport rectangle v
 */
int is_vpoint_inside(const view *v, const VPoint *vp, double epsilon)
{
    return ((vp->x >= v->xv1 - epsilon) && (vp->x <= v->xv2 + epsilon) &&
            (vp->y >= v->yv1 - epsilon) && (vp->y <= v->yv2 + epsilon));
}

static int is_inside_boundary(const VPoint *vp,
    const VPoint *vp1c, const VPoint *vp2c)
{
    /* vector product should be positive if vp1c, vp2c and vp lie 
     * counter-clockwise
     */
    if ((vp2c->x - vp1c->x)*(vp->y   - vp2c->y) -
        (vp->x   - vp2c->x)*(vp2c->y - vp1c->y) >= 0.0){
        return TRUE;
    } else {
        return FALSE;
    }
}


#define LINE_FINITE     0
#define LINE_INFINITE   1

/* TODO: implement fpcomp() */
#define FPCMP_EPS      1.0e-6
/*
 * line_intersect() returns pointer to the intersection point of two
 * lines defined by points vp1, vp2 and vp1p, vp2p respectively. 
 * If the lines don't intersect, return NULL.
 * If mode == LINE_INFINTE, the second line is assumed to be infinite.
 * Note!! If the lines have more than single intersection point (parallel
 * partially coinsiding lines), the function returns NULL, too.
 * The routine uses the Liang-Barsky algorithm, slightly modified for the
 * sake of generality (but for the price of performance) 
 */
VPoint *line_intersect(const VPoint *vp1, const VPoint *vp2,
    const VPoint *vp1p, const VPoint *vp2p, int mode)
{
    static VPoint vpbuf;
    double vprod, t, tp;
    
    vprod = (vp2p->x - vp1p->x)*(vp2->y - vp1->y) -
            (vp2->x - vp1->x)*(vp2p->y - vp1p->y);
    if (vprod == 0) {
        return NULL;
    } else {
        t = ((vp1->x - vp1p->x)*vp2p->y + 
             (vp2p->x - vp1->x)*vp1p->y - 
             (vp2p->x - vp1p->x)*vp1->y)/vprod;
        if ((t >= 0.0 - FPCMP_EPS) && (t <= 1.0 + FPCMP_EPS)) {
            vpbuf.x = vp1->x + t*(vp2->x - vp1->x);
            vpbuf.y = vp1->y + t*(vp2->y - vp1->y);
            
            if (mode == LINE_INFINITE) {
                return &vpbuf;
            } else {
                if (vp1p->x != vp2p->x) {
                    tp = (vpbuf.x - vp1p->x)/(vp2p->x - vp1p->x);
                } else {
                    tp = (vpbuf.y - vp1p->y)/(vp2p->y - vp1p->y);
                }
                
                if ((tp >= 0.0 - FPCMP_EPS) && (tp <= 1.0 + FPCMP_EPS)) {
                    return &vpbuf;
                } else {
                    return NULL;
                }
            }
        } else {
            return NULL;
        }
    }
}

/* size of buffer array used in polygon clipping */
static int polybuf_length;

int intersect_polygon(VPoint *vps, int n, const VPoint *vp1p, const VPoint *vp2p)
{
    int i, nc, ishift;
    VPoint vp1, vp2, *vpp;
    
    nc = 0;
    ishift = polybuf_length - n;
    
    memmove(vps + ishift, vps, n * sizeof(VPoint));
    
    vp1 = vps[polybuf_length - 1];
    for (i = ishift; i < polybuf_length; i++) {
        vp2 = vps[i];
        if (is_inside_boundary(&vp2, vp1p, vp2p)) {
            if (is_inside_boundary(&vp1, vp1p, vp2p)) {
                vps[nc] = vp2;
                nc++;
            } else {
                vpp = line_intersect(&vp1, &vp2, vp1p, vp2p, LINE_INFINITE);
                if (vpp != NULL) {
                    vps[nc] = *vpp;
                    nc++;
                }
                vps[nc] = vp2;
                nc++;
            }
        } else if (is_inside_boundary(&vp1, vp1p, vp2p)) {
            vpp = line_intersect(&vp1, &vp2, vp1p, vp2p, LINE_INFINITE);
            if (vpp != NULL) {
                vps[nc] = *vpp;
                nc++;
            }
        }
        vp1 = vp2;
    }
    
    return nc;
}

static int clip_polygon(VPoint *vps, int n, const view *clipview)
{
    int nc, na;
    VPoint vpsa[5];
    
    polybuf_length = 2*n;
    
    vpsa[0].x = clipview->xv1;
    vpsa[0].y = clipview->yv1;
    vpsa[1].x = clipview->xv2;
    vpsa[1].y = clipview->yv1;
    vpsa[2].x = clipview->xv2;
    vpsa[2].y = clipview->yv2;
    vpsa[3].x = clipview->xv1;
    vpsa[3].y = clipview->yv2;
    vpsa[4] = vpsa[0];
    
    nc = n;
    for (na = 0; na < 4; na++) {
        nc = intersect_polygon(vps, nc, &vpsa[na], &vpsa[na + 1]);
        if (nc < 2) {
            break;
        }
    }
    
    return nc;
}


static int all_points_inside(const view *clipview, const VPoint *vps, int n)
{
    int i;
    
    for (i = 0; i < n; i++) {
        if (is_vpoint_inside(clipview, &vps[i], VP_EPSILON) != TRUE) {
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * is_validVPoint() checks if a point is inside of (current) graph viewport
 */
int is_validVPoint(const Canvas *canvas, const VPoint *vp)
{
    if (canvas->clipflag) {
        return (is_vpoint_inside(&canvas->clipview, vp, VP_EPSILON));
    } else {
        return TRUE;
    }
}

/*
 * clip_line() clips a straight line defined by points vp1 and vp2
 * onto viewport rectangle; endpoints of the clipped line are returned by
 * vp1c and vp2c, and the function itself returns TRUE if (a part of) the line
 * should be drawn and FALSE otherwise
 */
int clip_line(const Canvas *canvas,
    const VPoint *vp1, const VPoint *vp2, VPoint *vp1c, VPoint *vp2c)
{
    int ends_found = 0;
    int na;
    int vp1_ok = FALSE, vp2_ok = FALSE;
    VPoint *vpp, vptmp[2], vpsa[5];
    
    if (is_validVPoint(canvas, vp1)) {
        vp1_ok = TRUE;
        ends_found++;
    }
    
    if (is_validVPoint(canvas, vp2)) {
        vp2_ok = TRUE;
        ends_found++;
    }
    
    if (vp1_ok && vp2_ok) {
        *vp1c = *vp1;
        *vp2c = *vp2;
        return (TRUE);
    } else {
        vpsa[0].x = canvas->clipview.xv1 - VP_EPSILON;
        vpsa[0].y = canvas->clipview.yv1 - VP_EPSILON;
        vpsa[1].x = canvas->clipview.xv2 + VP_EPSILON;
        vpsa[1].y = canvas->clipview.yv1 - VP_EPSILON;
        vpsa[2].x = canvas->clipview.xv2 + VP_EPSILON;
        vpsa[2].y = canvas->clipview.yv2 + VP_EPSILON;
        vpsa[3].x = canvas->clipview.xv1 - VP_EPSILON;
        vpsa[3].y = canvas->clipview.yv2 + VP_EPSILON;
        vpsa[4] = vpsa[0];
        
        na = 0;
        while ((ends_found < 2) && na < 4) {
            if ((vpp = line_intersect(vp1, vp2, &vpsa[na], &vpsa[na + 1],
                LINE_FINITE)) != NULL) {
                vptmp[ends_found] = *vpp;
                ends_found++;
            }
            na++;
        }
        if (ends_found == 0) {
            return (FALSE);
        } else if (ends_found == 2) {
            if (vp1_ok) {
                *vp1c = *vp1;
                *vp2c = vptmp[1];
            } else if (vp2_ok) {
                *vp1c = vptmp[1];
                *vp2c = *vp2;
            } else {
                *vp1c = vptmp[0];
                *vp2c = vptmp[1];
            }
            return (TRUE);
        } else if (ends_found == 1) {
            /* one of the points was on a frame edge exactly, but 
             * line_intersect(), due to a final FP precision, didn't 
             * find it
             */
            return (FALSE);
        } else {
            /* this would be really strange! */
            errmsg("Internal error in clip_line()");
            return (FALSE);
        }
    }
}


#define PURGE_INIT_FACTOR   1.0
#define PURGE_ITER_FACTOR   M_SQRT2

/* Note: vps and pvps may be the same array! */
static void purge_dense_points(const VPoint *vps, int n, VPoint *pvps, int *np)
{
    int i, j, iter;
    int ok;
    double eps;
    VPoint vptmp;
    
    if (n <= *np) {
        memmove(pvps, vps, n*sizeof(VPoint));
    }
    
    if (*np <= 0) {
        *np = 0;
        return;
    }
    
    /* Start with 1/np epsilon */
    eps = PURGE_INIT_FACTOR/(*np);
    iter = 0;
    ok = FALSE;
    while (ok == FALSE) {
        j = 0;
        vptmp = vps[0];
        for (i = 0; i < n - 1; i++) {
            if (fabs(vps[i].x - vptmp.x) > eps ||
                fabs(vps[i].y - vptmp.y) > eps) {
                vptmp = vps[i];
                j++;
                if (j >= *np) {
                    break;
                }
            }
        }
        if (j < *np - 1) {
            ok = TRUE;
        } else {
            eps *= PURGE_ITER_FACTOR;
        }
        iter++;
    }

    /* actually fill the purged array */
    pvps[0] = vps[0];
    j = 0;
    for (i = 0; i < n - 1; i++) {
        if (fabs(vps[i].x - pvps[j].x) > eps ||
            fabs(vps[i].y - pvps[j].y) > eps) {
            pvps[++j] = vps[i];
        }
    }
    pvps[j++] = vps[n - 1];
    
    *np = j;
#if 0
    printf("Purging %d points to %d in %d iteration(s)\n", n, *np, iter);
#endif
}

/* 
 * ------------------ Colormap routines ---------------
 */

static int RGB2YIQ(const RGB *rgb, YIQ *yiq)
{
    if (is_valid_color(rgb)) {
        yiq->y = (0.299*rgb->red + 0.587*rgb->green + 0.114*rgb->blue)
                                                            /(MAXCOLORS - 1);
        yiq->i = (0.596*rgb->red - 0.275*rgb->green - 0.321*rgb->blue)
                                                            /(MAXCOLORS - 1);
        yiq->q = (0.212*rgb->red - 0.528*rgb->green + 0.311*rgb->blue)
                                                             /(MAXCOLORS - 1);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int RGB2CMY(const RGB *rgb, CMY *cmy)
{
    if (is_valid_color(rgb)) {
        cmy->cyan    = MAXCOLORS - 1 - rgb->red;
        cmy->magenta = MAXCOLORS - 1 - rgb->green;
        cmy->yellow  = MAXCOLORS - 1 - rgb->blue;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int fRGB2RGB(const fRGB *frgb, RGB *rgb)
{
    if (frgb && rgb) {
        rgb->red   = (int) rint(frgb->red  *(MAXCOLORS - 1));
        rgb->green = (int) rint(frgb->green*(MAXCOLORS - 1));
        rgb->blue  = (int) rint(frgb->blue *(MAXCOLORS - 1));
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int RGB2fRGB(const RGB *rgb, fRGB *frgb)
{
    if (frgb && rgb) {
        frgb->red   = (double) rgb->red   / (MAXCOLORS - 1);
        frgb->green = (double) rgb->green / (MAXCOLORS - 1);
        frgb->blue  = (double) rgb->blue  / (MAXCOLORS - 1);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int store_color(Canvas *canvas, unsigned int n, const RGB *rgb, int ctype)
{
    if (is_valid_color(rgb) != TRUE) {
        return RETURN_FAILURE;
    } else if (n >= canvas->ncolors &&
        realloc_colors(canvas, n + 1) == RETURN_FAILURE) {
        return RETURN_FAILURE;
    } else {
        CMap_entry *cmap = &canvas->cmap[n];
        cmap->rgb = *rgb;
        cmap->ctype = ctype;
        
        /* invalidate AA gray levels' cache */       
        canvas->aacolors_low_ok  = FALSE;
        canvas->aacolors_high_ok = FALSE;
        
        /* inform current device of changes in the cmap database */
        if (canvas->device_ready) {
            canvas_color_trans(canvas, cmap);
            if (canvas->curdevice->updatecmap != NULL) {
                canvas->curdevice->updatecmap(canvas, canvas->curdevice->data);
            }
        }
        return RETURN_SUCCESS;
    }
}

int canvas_store_color(Canvas *canvas, unsigned int n, const RGB *rgb)
{
    return store_color(canvas, n, rgb, COLOR_MAIN);
}

static RGB cmap_init[] = {
    /* white */
    {255, 255, 255},
    /* black */
    {  0,   0,   0}
};

/*
 * canvas_cmap_reset()
 *    Initialize the colormap segment data and setup the RGB values.
 */
int canvas_cmap_reset(Canvas *canvas)
{
    unsigned int i, n;
    
    n = sizeof(cmap_init)/sizeof(RGB);
    realloc_colors(canvas, n);
    for (i = 0; i < n; i++) {
        store_color(canvas, i, &cmap_init[i], COLOR_MAIN);
    }
    
    return RETURN_SUCCESS;
}

unsigned int number_of_colors(const Canvas *canvas)
{
    return canvas->ncolors;
}

int is_valid_color(const RGB *rgb)
{
    if (rgb &&
        ((rgb->red   <= 0xff) && (rgb->red   >= 0x00)) &&
        ((rgb->green <= 0xff) && (rgb->green >= 0x00)) &&
        ((rgb->blue  <= 0xff) && (rgb->blue  >= 0x00))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int compare_rgb(const RGB *rgb1, const RGB *rgb2)
{
    if ((rgb1->red   == rgb2->red)   &&
        (rgb1->green == rgb2->green) &&
        (rgb1->blue  == rgb2->blue)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int find_color(const Canvas *canvas, const RGB *rgb)
{
    unsigned int i;
    int cindex = BAD_COLOR;
    
    for (i = 0; i < canvas->ncolors; i++) {
        if (compare_rgb(&canvas->cmap[i].rgb, rgb) == TRUE) {
            cindex = i;
            break;
        }
    }
    
    return cindex;
}

static int realloc_colors(Canvas *canvas, unsigned int n)
{
    unsigned int i;
    CMap_entry *cmap_tmp;
    
    if (n > MAXCOLORS) {
        return RETURN_FAILURE;
    } else {
        cmap_tmp = xrealloc(canvas->cmap, n*sizeof(CMap_entry));
        if (n != 0 && cmap_tmp == NULL) {
            return RETURN_FAILURE;
        } else {
            canvas->cmap = cmap_tmp;
            for (i = canvas->ncolors; i < n; i++) {
                memset(&canvas->cmap[i], 0, sizeof(CMap_entry));
                canvas->cmap[i].ctype = COLOR_NONE;
            }
        }
        canvas->ncolors = n;
    }
    
    return RETURN_SUCCESS;
}

static int is_rgb_grey(const RGB *rgb)
{
    if (rgb->red == rgb->blue &&
        rgb->red == rgb->green) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void rgb_invert(const RGB *src, RGB *dest)
{
    dest->red   = MAXCOLORS - 1 - src->red;
    dest->blue  = MAXCOLORS - 1 - src->blue;
    dest->green = MAXCOLORS - 1 - src->green;
}

static void rgb_greyscale(const RGB *src, RGB *dest)
{
    int y;
    
    y = INTENSITY(src->red, src->green, src->blue);
    
    dest->red   = y;
    dest->blue  = y;
    dest->green = y;
}

static void rgb_srgb(const RGB *src, RGB *dest)
{
    fRGB fsrc, fdest;
    
    fsrc.red    = (double) src->red   / (MAXCOLORS - 1);
    fsrc.green  = (double) src->green / (MAXCOLORS - 1);
    fsrc.blue   = (double) src->blue  / (MAXCOLORS - 1);

    fdest.red   = fRGB2fSRGB(fsrc.red);
    fdest.green = fRGB2fSRGB(fsrc.green);
    fdest.blue  = fRGB2fSRGB(fsrc.blue);

    dest->red   = (int) rint(fdest.red   * (MAXCOLORS - 1));
    dest->green = (int) rint(fdest.green * (MAXCOLORS - 1));
    dest->blue  = (int) rint(fdest.blue  * (MAXCOLORS - 1));
}

static void rgb_bw(const RGB *src, RGB *dest)
{
    int y;
    
    if (src->red   < MAXCOLORS - 1 ||
        src->green < MAXCOLORS - 1 ||
        src->blue  < MAXCOLORS - 1) {
        y = 0;
    } else {
        y = MAXCOLORS - 1;
    }
    
    dest->red   = y;
    dest->blue  = y;
    dest->green = y;
}

void canvas_color_trans(Canvas *canvas, CMap_entry *cmap)
{
    switch (canvas->curdevice->color_trans) {
    case COLOR_TRANS_BW:
        rgb_bw(&cmap->rgb, &cmap->devrgb);
        break;
    case COLOR_TRANS_GREYSCALE:
        rgb_greyscale(&cmap->rgb, &cmap->devrgb);
        break;
    case COLOR_TRANS_NEGATIVE:
        rgb_invert(&cmap->rgb, &cmap->devrgb);
        break;
    case COLOR_TRANS_REVERSE:
        if (is_rgb_grey(&cmap->rgb)) {
            rgb_invert(&cmap->rgb, &cmap->devrgb);
        } else {
            cmap->devrgb = cmap->rgb;
        }
        break;
    case COLOR_TRANS_SRGB:
        rgb_srgb(&cmap->rgb, &cmap->devrgb);
        break;
    case COLOR_TRANS_NONE:
    default:
        cmap->devrgb = cmap->rgb;
        break;
    }
}

/*
 * add_color() adds a new entry to the colormap table
 */
int add_color(Canvas *canvas, const RGB *rgb, int ctype)
{
    int cindex;
    
    if (is_valid_color(rgb) != TRUE) {
        cindex = BAD_COLOR;
    } else if ((cindex = find_color(canvas, rgb)) != BAD_COLOR) {
        if (ctype == COLOR_MAIN && canvas->cmap[cindex].ctype != COLOR_MAIN) {
            canvas->cmap[cindex].ctype = COLOR_MAIN;
        }
    } else if (store_color(canvas, canvas->ncolors, rgb, ctype)
        != RETURN_SUCCESS) {
        cindex = BAD_COLOR;
    } else {
        cindex = canvas->ncolors - 1;
    }
    
    return (cindex);
}

/*
 * int delete_color(int cindex)
 * {
 * }
 */

int get_rgb(const Canvas *canvas, unsigned int cindex, RGB *rgb)
{
    if (rgb && cindex < canvas->ncolors) {
        *rgb = canvas->cmap[cindex].devrgb;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_frgb(const Canvas *canvas, unsigned int cindex, fRGB *frgb)
{
    RGB rgb;
    if (frgb && get_rgb(canvas, cindex, &rgb) == RETURN_SUCCESS) {
        return RGB2fRGB(&rgb, frgb);
    } else {
        return RETURN_FAILURE;
    }
}

CMap_entry *get_color_def(const Canvas *canvas, unsigned int cindex)
{
    if (cindex < canvas->ncolors) {
        return &canvas->cmap[cindex];
    } else {
        return NULL;
    }
}

int get_colortype(const Canvas *canvas, unsigned int cindex)
{
    if (cindex < canvas->ncolors) {
        return (canvas->cmap[cindex].ctype);
    } else {
        return (BAD_COLOR);
    }
}

double get_colorintensity(const Canvas *canvas, int cindex)
{
    RGB rgb;
    YIQ yiq;
    
    if (get_rgb(canvas, cindex, &rgb) == RETURN_SUCCESS &&
        RGB2YIQ(&rgb, &yiq)    == RETURN_SUCCESS) {
        return yiq.y;
    } else {
        return 0.0;
    }
}

int get_cmy(const Canvas *canvas, unsigned int cindex, CMY *cmy)
{
    RGB rgb;
    
    if (get_rgb(canvas, cindex, &rgb) == RETURN_SUCCESS &&
        RGB2CMY(&rgb, cmy)     == RETURN_SUCCESS) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_cmyk(const Canvas *canvas, unsigned int cindex, CMYK *cmyk)
{
    CMY cmy;
    
    if (get_cmy(canvas, cindex, &cmy) == RETURN_SUCCESS) {
        cmyk->black   = MIN3(cmy.cyan, cmy.magenta, cmy.yellow);
        cmyk->cyan    = cmy.cyan    - cmyk->black;
        cmyk->magenta = cmy.magenta - cmyk->black;
        cmyk->yellow  = cmy.yellow  - cmyk->black;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int get_fcmyk(const Canvas *canvas, unsigned int cindex, fCMYK *fcmyk)
{
    CMYK cmyk;
    
    if (get_cmyk(canvas, cindex, &cmyk) == RETURN_SUCCESS) {
        fcmyk->cyan    = (double) cmyk.cyan    /(MAXCOLORS - 1);
        fcmyk->magenta = (double) cmyk.magenta /(MAXCOLORS - 1);
        fcmyk->yellow  = (double) cmyk.yellow  /(MAXCOLORS - 1);
        fcmyk->black   = (double) cmyk.black   /(MAXCOLORS - 1);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int make_color_scale(Canvas *canvas,
    unsigned int fg, unsigned int bg,
    unsigned int ncolors, unsigned long *colors)
{
    unsigned int i;
    fRGB fg_frgb, bg_frgb, delta_frgb;
    CMap_entry *fg_color, *bg_color;

    if (ncolors < 2 || !colors) {
        return RETURN_FAILURE;
    }
    
    fg_color = get_color_def(canvas, fg);
    bg_color = get_color_def(canvas, bg);
    if (!bg_color || !fg_color) {
        return RETURN_FAILURE;
    }

    RGB2fRGB(&fg_color->rgb, &fg_frgb);
    RGB2fRGB(&bg_color->rgb, &bg_frgb);
    
    delta_frgb.red   = (fg_frgb.red   - bg_frgb.red)  /(ncolors - 1);
    delta_frgb.green = (fg_frgb.green - bg_frgb.green)/(ncolors - 1);
    delta_frgb.blue  = (fg_frgb.blue  - bg_frgb.blue) /(ncolors - 1);
    colors[0] = bg;
    for (i = 1; i < ncolors - 1; i++) {
    	fRGB frgb;
        RGB rgb;
        int c;
        
        frgb.red   = bg_frgb.red   + i*delta_frgb.red;
    	frgb.green = bg_frgb.green + i*delta_frgb.green;
    	frgb.blue  = bg_frgb.blue  + i*delta_frgb.blue;
    	fRGB2RGB(&frgb, &rgb);
    	c = add_color(canvas, &rgb, COLOR_AUX);
        colors[i] = (c == BAD_COLOR) ? 0:c;
    }
    colors[ncolors - 1] = fg;
    
    return RETURN_SUCCESS;
}


/* 
 * ------------------ Pattern routines ---------------
 */

static int realloc_patterns(Canvas *canvas, unsigned int n)
{
    unsigned int i;
    PMap_entry *p_tmp;
    
    for (i = n; i < canvas->npatterns; i++) {
        XCFREE(canvas->pmap[i].pattern.bits);
    }
    
    p_tmp = xrealloc(canvas->pmap, n*sizeof(PMap_entry));
    if (n > 0 && p_tmp == NULL) {
        return RETURN_FAILURE;
    } else {
        canvas->pmap = p_tmp;
        for (i = canvas->npatterns; i < n; i++) {
            memset(&canvas->pmap[i], 0, sizeof(PMap_entry));
        }
    }
    canvas->npatterns = n;
    
    return RETURN_SUCCESS;
}

unsigned int number_of_patterns(const Canvas *canvas)
{
    return canvas->npatterns;
}

int canvas_set_pattern(Canvas *canvas, unsigned int n, const Pattern *pat)
{
    unsigned int bitlen;
    PMap_entry *pe;
    
    if (n >= canvas->npatterns) {
        if (realloc_patterns(canvas, n + 1) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
    }
    
    pe = &canvas->pmap[n];
    
    bitlen = pat->width*pat->height/8;
    
    xfree(pe->pattern.bits);
    pe->pattern.bits = xmalloc(SIZEOF_INT*bitlen);
    memcpy(pe->pattern.bits, pat->bits, bitlen);
    pe->pattern.width  = pat->width;
    pe->pattern.height = pat->height;
    
    return RETURN_SUCCESS;
}

Pattern *canvas_get_pattern(const Canvas *canvas, unsigned int n)
{
    if (n < canvas->npatterns) {
        return &canvas->pmap[n].pattern;
    } else {
        return NULL;
    }
}

/*
 *  Initialize the patterns.
 */
void initialize_patterns(Canvas *canvas)
{
    unsigned int i, n;
    
    n = sizeof(patterns_init)/sizeof(Pattern);
    realloc_patterns(canvas, n);
    for (i = 0; i < n; i++) {
        canvas_set_pattern(canvas, i, &patterns_init[i]);
    }
}


/* 
 * ------------------ Line style routines ---------------
 */

unsigned int number_of_linestyles(const Canvas *canvas)
{
    return canvas->nlinestyles;
}

static int realloc_linestyles(Canvas *canvas, unsigned int n)
{
    unsigned int i;
    LMap_entry *p_tmp;
    
    for (i = n; i < canvas->nlinestyles; i++) {
        XCFREE(canvas->lmap[i].linestyle.array);
    }
    
    p_tmp = xrealloc(canvas->lmap, n*sizeof(LMap_entry));
    if (n != 0 && p_tmp == NULL) {
        return RETURN_FAILURE;
    } else {
        canvas->lmap = p_tmp;
        for (i = canvas->nlinestyles; i < n; i++) {
            memset(&canvas->lmap[i], 0, sizeof(LMap_entry));
        }
    }
    canvas->nlinestyles = n;
    
    return RETURN_SUCCESS;
}

int canvas_set_linestyle(Canvas *canvas, unsigned int n, const LineStyle *ls)
{
    LineStyle *pls;
    
    if (n >= canvas->nlinestyles) {
        if (realloc_linestyles(canvas, n + 1) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
    }
    
    xfree(canvas->lmap[n].linestyle.array);
    pls = &canvas->lmap[n].linestyle;
    pls->length = ls->length;
    pls->array = xmalloc(ls->length*SIZEOF_INT);
    memcpy(pls->array, ls->array, ls->length*SIZEOF_INT);
    
    return RETURN_SUCCESS;
}

LineStyle *canvas_get_linestyle(const Canvas *canvas, unsigned int n)
{
    if (n < canvas->nlinestyles) {
        return &canvas->lmap[n].linestyle;
    } else {
        return NULL;
    }
}

/*
 *  Initialize the linestyle patterns
 */
void initialize_linestyles(Canvas *canvas)
{
    unsigned int i, n;
    
    n = sizeof(linestyles_init)/sizeof(LineStyle);
    realloc_linestyles(canvas, n);
    for (i = 0; i < n; i++) {
        canvas_set_linestyle(canvas, i, &linestyles_init[i]);
    }
}

/*
 * ---------------- bbox utilities --------------------
 */

static const view invalid_view = {-1.0, -1.0, -1.0, -1.0};

void reset_bbox(Canvas *canvas, int type)
{
    view *vp;
    
    switch(type) {
    case BBOX_TYPE_GLOB:
        vp = &(canvas->bboxes[0].v);
        break;
    case BBOX_TYPE_TEMP:
        vp = &(canvas->bboxes[1].v);
        break;
    default:
        errmsg("Incorrect call of reset_bbox()");
        return;
    }
    *vp = invalid_view;
}

void reset_bboxes(Canvas *canvas)
{
    reset_bbox(canvas, BBOX_TYPE_GLOB);
    reset_bbox(canvas, BBOX_TYPE_TEMP);
}

void freeze_bbox(Canvas *canvas, int type)
{
    BBox_type *bbp;
    
    switch (type) {
    case BBOX_TYPE_GLOB:
        bbp = &canvas->bboxes[0];
        break;
    case BBOX_TYPE_TEMP:
        bbp = &canvas->bboxes[1];
        break;
    default:
        errmsg("Incorrect call of freeze_bbox()");
        return;
    }
    bbp->fv = bbp->v;
}

int get_bbox(const Canvas *canvas, int type, view *v)
{
    switch (type) {
    case BBOX_TYPE_GLOB:
        *v = canvas->bboxes[0].v;
        break;
    case BBOX_TYPE_TEMP:
        *v = canvas->bboxes[1].v;
        break;
    default:
        *v = invalid_view;
        errmsg("Incorrect call of get_bbox()");
        return RETURN_FAILURE;
    }
    return RETURN_SUCCESS;
}

int is_valid_bbox(const view *v)
{
    if ((v->xv1 == invalid_view.xv1) &&
        (v->xv2 == invalid_view.xv2) &&
        (v->yv1 == invalid_view.yv1) &&
        (v->yv2 == invalid_view.yv2)) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

int merge_bboxes(const view *v1, const view *v2, view *v)
{
    if (!is_valid_bbox(v1)) {
        if (is_valid_bbox(v2)) {
            *v = *v2;
            return RETURN_SUCCESS;
        } else {
            *v = invalid_view;
            return RETURN_FAILURE;
        }
    } else if (!is_valid_bbox(v2)) {
        *v = *v1;
        return RETURN_SUCCESS;
    } else {
        v->xv1 = MIN2(v1->xv1, v2->xv1);
        v->xv2 = MAX2(v1->xv2, v2->xv2);
        v->yv1 = MIN2(v1->yv1, v2->yv1);
        v->yv2 = MAX2(v1->yv2, v2->yv2);
        
        return RETURN_SUCCESS;
    }
}

void update_bbox(Canvas *canvas, int type, const VPoint *vp)
{
    BBox_type *bbp;
    
    switch (type) {
    case BBOX_TYPE_GLOB:
        /* Global bbox is updated only with real drawings */
        if (get_draw_mode(canvas) == FALSE) {
            return;
        }
        bbp = &canvas->bboxes[0];
        break;
    case BBOX_TYPE_TEMP:
        bbp = &canvas->bboxes[1];
        break;
    default:
        errmsg("Incorrect call of update_bbox()");
        return;
    }
    if (bbp->active == TRUE) {
        if (is_vpoint_inside(&bbp->v, vp, 0.0) == FALSE) {
            if (is_valid_bbox(&bbp->v)) {
                bbp->v.xv1 = MIN2(bbp->v.xv1, vp->x);
                bbp->v.xv2 = MAX2(bbp->v.xv2, vp->x);
                bbp->v.yv1 = MIN2(bbp->v.yv1, vp->y);
                bbp->v.yv2 = MAX2(bbp->v.yv2, vp->y);
            } else {
                bbp->v.xv1 = vp->x;
                bbp->v.xv2 = vp->x;
                bbp->v.yv1 = vp->y;
                bbp->v.yv2 = vp->y;
            }
        }
    }
}

void update_bboxes(Canvas *canvas, const VPoint *vp)
{
    update_bbox(canvas, BBOX_TYPE_GLOB, vp);
    update_bbox(canvas, BBOX_TYPE_TEMP, vp);
}

int melt_bbox(Canvas *canvas, int type)
{
    BBox_type *bbp;
    
    switch(type) {
    case BBOX_TYPE_GLOB:
        bbp = &canvas->bboxes[0];
        break;
    case BBOX_TYPE_TEMP:
        bbp = &canvas->bboxes[1];
        break;
    default:
        errmsg("Incorrect call of melt_bbox()");
        return RETURN_FAILURE;
    }
    
    return merge_bboxes(&bbp->v, &bbp->fv, &bbp->v);
}

void activate_bbox(Canvas *canvas, int type, int status)
{
    BBox_type *bbp;
    
    switch(type) {
    case BBOX_TYPE_GLOB:
        bbp = &canvas->bboxes[0];
        break;
    case BBOX_TYPE_TEMP:
        bbp = &canvas->bboxes[1];
        break;
    default:
        errmsg("Incorrect call of activate_bbox()");
        return;
    }
    bbp->active = status;
}

/* Extend all view boundaries with w */
int view_extend(view *v, double w)
{
    if (!v) {
        return RETURN_FAILURE;
    } else {
        v->xv1 -= w;
        v->xv2 += w;
        v->yv1 -= w;
        v->yv2 += w;
        return RETURN_SUCCESS;
    }
}

int update_bboxes_with_view(Canvas *canvas, const view *v)
{
    if (!v) {
        return RETURN_FAILURE;
    } else {
        VPoint vp;
        
        vp.x = v->xv1;
        vp.y = v->yv1;
        update_bboxes(canvas, &vp);
        vp.x = v->xv2;
        vp.y = v->yv2;
        update_bboxes(canvas, &vp);
        
        return RETURN_SUCCESS;
    }
}

int update_bboxes_with_vpoints(Canvas *canvas, const VPoint *vps, int n, double lw)
{
    if (!vps || n < 1) {
        return RETURN_FAILURE;
    } else {
        int i;
        double xmin, xmax, ymin, ymax;
        view v;
        
        xmin = xmax = vps[0].x;
        ymin = ymax = vps[0].y;
        
        for (i = 1; i < n; i++) {
            if (vps[i].x < xmin) {
                xmin = vps[i].x;
            } else
            if  (vps[i].x > xmax) {
                xmax = vps[i].x;
            }
            
            if (vps[i].y < ymin) {
                ymin = vps[i].y;
            } else
            if  (vps[i].y > ymax) {
                ymax = vps[i].y;
            }
        }
        
        v.xv1 = xmin;
        v.xv2 = xmax;
        v.yv1 = ymin;
        v.yv2 = ymax;
        
        view_extend(&v, lw/2);
        
        update_bboxes_with_view(canvas, &v);
        
        return RETURN_SUCCESS;
    }
}

void canvas_stats_reset(Canvas *canvas)
{
    unsigned int i;
    
    for (i = 0; i < canvas->ncolors; i++) {
        canvas->cmap[i].used = 0;
    }
    for (i = 0; i < canvas->npatterns; i++) {
        canvas->pmap[i].used = 0;
    }
    for (i = 0; i < canvas->nlinestyles; i++) {
        canvas->lmap[i].used = 0;
    }
    for (i = 0; i < canvas->nfonts; i++) {
        int j;
        canvas->FontDBtable[i].used = 0;
        for (j = 0; j < 256; j++) {
            canvas->FontDBtable[i].chars_used[j] = 0; 
        }
    }
}

static void canvas_stats_update(Canvas *canvas, int type)
{
    if (type & CANVAS_STATS_COLOR) {
        canvas->cmap[getcolor(canvas)].used = 1;
    }
    if (type & CANVAS_STATS_PATTERN) {
        canvas->pmap[getpattern(canvas)].used = 1;
    }
    if (type & CANVAS_STATS_LINESTYLE) {
        canvas->lmap[getlinestyle(canvas)].used = 1;
    }
}

static void canvas_char_stats_update(Canvas *canvas,
    int font, const char *s, int len)
{
    int j;
    
    canvas->FontDBtable[font].used = 1;
    for (j = 0; j < len; j++) {
        canvas->FontDBtable[font].chars_used[(unsigned char) s[j]] = 1; 
    }
}

CanvasStats *canvas_stats(const Canvas *canvas)
{
    CanvasStats *cstats = xmalloc(sizeof(CanvasStats));
    
    if (cstats) {
        unsigned int i, j;
        
        memset(cstats, 0, sizeof(CanvasStats));
        
        /* colors */
        for (i = 0; i < canvas->ncolors; i++) {
            if (canvas->cmap[i].used) {
                cstats->ncolors++;
            }
        }
        cstats->colors = xmalloc(cstats->ncolors*SIZEOF_INT);
        for (i = 0, j = 0; i < canvas->ncolors; i++) {
            if (canvas->cmap[i].used) {
                cstats->colors[j] = i;
                j++;
            }
        }

        /* patterns */
        for (i = 0; i < canvas->npatterns; i++) {
            if (canvas->pmap[i].used) {
                cstats->npatterns++;
            }
        }
        cstats->patterns = xmalloc(cstats->npatterns*SIZEOF_INT);
        for (i = 0, j = 0; i < canvas->npatterns; i++) {
            if (canvas->pmap[i].used) {
                cstats->patterns[j] = i;
                j++;
            }
        }

        /* linestyles */
        for (i = 0; i < canvas->nlinestyles; i++) {
            if (canvas->lmap[i].used) {
                cstats->nlinestyles++;
            }
        }
        cstats->linestyles = xmalloc(cstats->nlinestyles*SIZEOF_INT);
        for (i = 0, j = 0; i < canvas->nlinestyles; i++) {
            if (canvas->lmap[i].used) {
                cstats->linestyles[j] = i;
                j++;
            }
        }

        /* fonts */
        for (i = 0; i < canvas->nfonts; i++) {
            if (canvas->FontDBtable[i].used) {
                cstats->nfonts++;
            }
        }
        cstats->fonts = xmalloc(cstats->nfonts*sizeof(FontStats));
        for (i = 0, j = 0; i < canvas->nfonts; i++) {
            if (canvas->FontDBtable[i].used) {
                cstats->fonts[j].font = i;
                memcpy(&cstats->fonts[j].chars_used,
                    &canvas->FontDBtable[i].chars_used, 256);
                j++;
            }
        }
        
        /* BBox */
        get_bbox(canvas, BBOX_TYPE_GLOB, &cstats->bbox);
    }
    
    return cstats;
}

static void canvas_stats_free(CanvasStats *cstats)
{
    if (cstats) {
        xfree(cstats->colors);
        xfree(cstats->patterns);
        xfree(cstats->linestyles);
        xfree(cstats->fonts);
        xfree(cstats);
    }
}

int canvas_draw(Canvas *canvas, CanvasDrawProc dproc, void *data)
{
    unsigned int npasses, passno;
    CanvasStats *cstats;
    
    if (canvas->curdevice->twopass) {
        npasses = 2;
    } else {
        npasses = 1;
    }
    
    cstats = NULL;
    
    for (passno = 0; passno < npasses; passno++) {
        if (npasses == 2 && passno == 0) {
            canvas->drypass = TRUE;
        } else {
            canvas->drypass = FALSE;
        }
        
        reset_bboxes(canvas);
        activate_bbox(canvas, BBOX_TYPE_GLOB, FALSE);
        activate_bbox(canvas, BBOX_TYPE_TEMP, FALSE);
        canvas_stats_reset(canvas);
        
        if (!canvas->drypass) {
            if (initgraphics(canvas, cstats) != RETURN_SUCCESS) {
                errmsg("Device wasn't properly initialized");
                return RETURN_FAILURE;
            }
        }
        
        if (!canvas->curdevice->autocrop) {
            VPoint vp;
            
            activate_bbox(canvas, BBOX_TYPE_GLOB, TRUE);

            vp.x = vp.y = 0.0;
            update_bbox(canvas, BBOX_TYPE_GLOB, &vp);
            get_page_viewport(canvas, &vp.x, &vp.y);
            update_bbox(canvas, BBOX_TYPE_GLOB, &vp);
        }
        
        if (canvas->pagefill) {
            VPoint vp1, vp2;
            
            if (cstats && canvas->curdevice->autocrop) {
                vp1.x = cstats->bbox.xv1;
                vp1.y = cstats->bbox.yv1;
                vp2.x = cstats->bbox.xv2;
                vp2.y = cstats->bbox.yv2;
            } else {
                vp1.x = 0.0;
                vp1.y = 0.0;
                get_page_viewport(canvas, &vp2.x, &vp2.y);
            }
            
            setcolor(canvas, getbgcolor(canvas));
            setpattern(canvas, 1);
            setclipping(canvas, FALSE);
            FillRect(canvas, &vp1, &vp2);
        }

        activate_bbox(canvas, BBOX_TYPE_GLOB, TRUE);
        
        dproc(canvas, data);
        
        if (!cstats) {
            cstats = canvas_stats(canvas);
        }
        
        if (!canvas->drypass) {
            leavegraphics(canvas, cstats);
        }
    }
    
    canvas_stats_free(cstats);
    
    return RETURN_SUCCESS;
}
