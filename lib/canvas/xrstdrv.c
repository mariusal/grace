/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2002 Grace Development Team
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
 * Raster meta driver for Grace
 *
 */

#include <config.h>

#ifdef HAVE_LIBXMI

/* for abs() */
#include <stdlib.h>

#include <string.h>

#include "grace/baseP.h"
#include "xrstdrv.h"

static int xrst_parser_wrapper(const Canvas *canvas, void *data, const char *s)
{
    Xrst_data *ddata = (Xrst_data *) data;
    return ddata->parser(canvas, ddata->data, s);
}

static void xrst_setup_wrapper(const Canvas *canvas, void *data)
{
    Xrst_data *ddata = (Xrst_data *) data;
    ddata->setup(canvas, ddata->data);
}

int register_xrst_device(Canvas *canvas, const XrstDevice_entry *xdev)
{
    Device_entry *d;
    Xrst_data *ddata;

    ddata = xmalloc(sizeof(Xrst_data));
    if (!ddata) {
        return -1;
    }
    memset(ddata, 0, sizeof(Xrst_data));
    
    ddata->dump   = xdev->dump;
    ddata->data   = xdev->data;
    ddata->setup  = xdev->setup;
    ddata->parser = xdev->parser;
    
    d = device_new(xdev->name, xdev->type, FALSE, (void *) ddata);
    if (!d) {
        xfree(ddata);
        return -1;
    }

    device_set_fext(d, xdev->fext);
    
    device_set_procs(d,
        xrst_initgraphics,
        xrst_leavegraphics,
        xdev->parser ? xrst_parser_wrapper:NULL,
        xdev->setup  ? xrst_setup_wrapper:NULL,
        NULL,
        xrst_drawpixel,
        xrst_drawpolyline,
        xrst_fillpolygon,
        xrst_drawarc,
        xrst_fillarc,
        xrst_putpixmap,
        NULL);
    
    device_set_fontrast(d, xdev->fontaa ? FONT_RASTER_AA_LOW:FONT_RASTER_MONO);

    return register_device(canvas, d);
}

static void VPoint2miPoint(const Xrst_data *ddata, const VPoint *vp, miPoint *mp)
{
    mp->x = (int) rint(ddata->page_scale*vp->x);
    mp->y = (int) rint(ddata->height - ddata->page_scale*vp->y);
}

int xrst_initgraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    Xrst_data *ddata    = (Xrst_data *) data;
    Page_geometry *pg   = get_page_geometry(canvas);
    
    ddata->color        = BAD_COLOR;
    ddata->bgcolor      = BAD_COLOR;
    ddata->patno        = -1;
    ddata->linewidth    = -1;
    ddata->linestyle    = -1;
    ddata->arcfillmode  = -1;
    ddata->arcfillmode  = -1;
    ddata->linecap      = -1;
    ddata->linejoin     = -1;
    
    ddata->width        = pg->width;
    ddata->height       = pg->height;
    ddata->page_scale    = (ddata->height < ddata->width) ? ddata->height:ddata->width;
    
    ddata->gc           = NULL;
    ddata->paintedSet   = miNewPaintedSet();
    ddata->mcanvas      = miNewCanvas(ddata->width, ddata->height, 0);
    
    return RETURN_SUCCESS;
}


static void xrst_flush_buffer(const Xrst_data *ddata)
{
    static const miPoint offset = {0, 0};

    miCopyPaintedSetToCanvas(ddata->paintedSet, ddata->mcanvas, offset);
    miClearPaintedSet(ddata->paintedSet);
}


static void xrst_free_pixmap(miPixmap *bm)
{
    if (bm) {
        if (bm->pixmap) {
            int i;
            for (i = 0; i < bm->width; i++) {
                xfree(bm->pixmap[i]);
            }
            xfree(bm->pixmap);
        }
        xfree(bm);
    }
}

static miPixmap *xrst_create_pixmap_from_pattern(const Pattern *pat,
    miPixel bg, miPixel fg)
{
    miPixmap *pm;
    
    pm = xmalloc(sizeof(miPixmap));
    if (pm) {
        pm->width  = pat->width;
        pm->height = pat->height;
        pm->pixmap = xmalloc(pat->width*SIZEOF_VOID_P);
        if (pm->pixmap) {
            int i, j, k;
            unsigned char p;
            
            for (i = 0; i < pm->width; i++) {
                pm->pixmap[i] = xmalloc(pat->height*sizeof(miPixel));
                if (!pm->pixmap[i]) {
                    xrst_free_pixmap(pm);
                    pm = NULL;
                    break;
                }
            }
            
            /* FIXME: general code for other but 16x16 bitmaps! */
            for (k = 0; k < 16; k++) {
                for (j = 0; j < 2; j++) {
                    for (i = 0; i < 8; i++) {
                        p = pat->bits[k*2+j];
                        if ((p >> i) & 0x01) {
                            pm->pixmap[k][8*j + i] = fg;
                        } else {
                            pm->pixmap[k][8*j + i] = bg;
                        }
                    }
                }
            }
        } else {
            xrst_free_pixmap(pm);
            pm = NULL;
        }
    }
    
    return pm;
}

static void xrst_setpen(const Canvas *canvas, Xrst_data *ddata)
{
    int fg, bg, p;
    Pen pen;
    static const miPoint origin = {0, 0};
    miPixmap *texture;
    miPixel xrst_colors[2];
    
    bg = getbgcolor(canvas);
    getpen(canvas, &pen);
    fg = pen.color;
    p = pen.pattern;
    
    if ((fg == ddata->color) && (bg == ddata->bgcolor) && (p == ddata->patno)) {
        return;
    }
        
    ddata->color = fg;
    ddata->bgcolor = bg;
    ddata->patno = p;
    
    xrst_colors[0] = bg;
    xrst_colors[1] = fg;
    if (!ddata->gc) {
        ddata->gc = miNewGC(2, xrst_colors);
    } else {
        miSetGCPixels(ddata->gc, 2, xrst_colors);
    }
    
    if (p == 1) {
        texture = NULL;
    } else {
        Pattern *pat = canvas_get_pattern(canvas, p);
        texture = xrst_create_pixmap_from_pattern(pat, bg, fg);
    }
    miSetCanvasTexture(ddata->mcanvas, texture, origin);
    
    xrst_free_pixmap(texture);
}

static void xrst_setdrawbrush(const Canvas *canvas, Xrst_data *ddata)
{
    unsigned int iw;
    int style;
    int lc, lj;
    int i, scale, darr_len;
    unsigned int *xdarr;

    xrst_setpen(canvas, ddata);
    
    iw = (unsigned int) rint(getlinewidth(canvas)*ddata->page_scale);
    if (iw == 1) {
        iw = 0;
    }
    style = getlinestyle(canvas);
    lc = getlinecap(canvas);
    lj = getlinejoin(canvas);
    
    switch (lc) {
    case LINECAP_BUTT:
        lc = MI_CAP_BUTT;
        break;
    case LINECAP_ROUND:
        lc = MI_CAP_ROUND;
        break;
    case LINECAP_PROJ:
        lc = MI_CAP_PROJECTING;
        break;
    }

    switch (lj) {
    case LINEJOIN_MITER:
        lj = MI_JOIN_MITER;
        break;
    case LINEJOIN_ROUND:
        lj = MI_JOIN_ROUND;
        break;
    case LINEJOIN_BEVEL:
        lj = MI_JOIN_BEVEL;
        break;
    }

    if (iw != ddata->linewidth || style != ddata->linestyle ||
        lc != ddata->linecap   || lj    != ddata->linejoin) {
        
        miSetGCAttrib(ddata->gc, MI_GC_LINE_WIDTH, iw);
        miSetGCAttrib(ddata->gc, MI_GC_JOIN_STYLE, lj);
        miSetGCAttrib(ddata->gc, MI_GC_CAP_STYLE, lc);
        
        if (style > 1) {
            LineStyle *linestyle = canvas_get_linestyle(canvas, style);
            darr_len = linestyle->length;
            xdarr = xmalloc(darr_len*SIZEOF_INT);
            if (xdarr == NULL) {
                return;
            }
            scale = MAX2(1, iw);
            for (i = 0; i < darr_len; i++) {
                xdarr[i] = scale*linestyle->array[i];
            }
            miSetGCAttrib(ddata->gc, MI_GC_LINE_STYLE, MI_LINE_ON_OFF_DASH);
            miSetGCDashes(ddata->gc, darr_len, xdarr, 0);
            xfree(xdarr);
        } else if (style == 1) {
            miSetGCAttrib(ddata->gc, MI_GC_LINE_STYLE, MI_LINE_SOLID);
        }
 
        ddata->linestyle = style;
        ddata->linewidth = iw;
        ddata->linecap   = lc;
        ddata->linejoin  = lj;
    }
    return;
}

static void miDrawPoint(miPaintedSet *paintedSet, const miGC *pGC,
    const miPoint *pPt)
{
    miDrawPoints(paintedSet, pGC, MI_COORD_MODE_ORIGIN, 1, pPt);
}

void xrst_drawpixel(const Canvas *canvas, void *data,
    const VPoint *vp)
{
    Xrst_data *ddata = (Xrst_data *) data;
    miPoint mp;
    
    VPoint2miPoint(ddata, vp, &mp);
    xrst_setpen(canvas, ddata);
    miDrawPoint(ddata->paintedSet, ddata->gc, &mp);

    xrst_flush_buffer(ddata);
}

void xrst_drawpolyline(const Canvas *canvas, void *data,
    const VPoint *vps, int n, int mode)
{
    Xrst_data *ddata = (Xrst_data *) data;
    int i, xn = n;
    miPoint *p;
    
    if (mode == POLYLINE_CLOSED) {
        xn++;
    }
    
    p = xmalloc(xn*sizeof(miPoint));
    if (p == NULL) {
        return;
    }
    
    for (i = 0; i < n; i++) {
        VPoint2miPoint(ddata, &vps[i], &p[i]);
    }
    if (mode == POLYLINE_CLOSED) {
        p[n] = p[0];
    }
    
    xrst_setdrawbrush(canvas, ddata);
    
    miDrawLines(ddata->paintedSet, ddata->gc, MI_COORD_MODE_ORIGIN, xn, p);
    
    xfree(p);

    xrst_flush_buffer(ddata);
}


void xrst_fillpolygon(const Canvas *canvas, void *data,
    const VPoint *vps, int nc)
{
    Xrst_data *ddata = (Xrst_data *) data;
    int i;
    int fr;
    miPoint *p;
    
    p = (miPoint *) xmalloc(nc*sizeof(miPoint));
    if (p == NULL) {
        return;
    }
    
    for (i = 0; i < nc; i++) {
        VPoint2miPoint(ddata, &vps[i], &p[i]);
    }
    
    xrst_setpen(canvas, ddata);

    fr = getfillrule(canvas);
    if (fr != ddata->fillrule) {
        ddata->fillrule = fr;
        if (fr == FILLRULE_WINDING) {
            miSetGCAttrib(ddata->gc, MI_GC_FILL_RULE, MI_WINDING_RULE);
        } else {
            miSetGCAttrib(ddata->gc, MI_GC_FILL_RULE, MI_EVEN_ODD_RULE);
        }
    }

    miFillPolygon(ddata->paintedSet, ddata->gc,
        MI_SHAPE_GENERAL, MI_COORD_MODE_ORIGIN, nc, p);
    
    xfree(p);

    xrst_flush_buffer(ddata);
}

/*
 *  xrst_drawarc
 */
void xrst_drawarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2)
{
    Xrst_data *ddata = (Xrst_data *) data;
    miPoint mp1, mp2;
    
    VPoint2miPoint(ddata, vp1, &mp1);
    VPoint2miPoint(ddata, vp2, &mp2);
    
    xrst_setdrawbrush(canvas, ddata);
    
    if (mp1.x != mp2.x || mp1.y != mp2.y) {
        miArc arc;
        
        arc.x      = MIN2(mp1.x, mp2.x);
        arc.y      = MIN2(mp1.y, mp2.y);
        arc.width  = abs(mp2.x - mp1.x);
        arc.height = abs(mp2.y - mp1.y);
        arc.angle1 = (int) rint(64*a1);
        arc.angle2 = (int) rint(64*a2);

        miDrawArcs(ddata->paintedSet, ddata->gc, 1, &arc);
    } else { /* zero radius */
        miDrawPoint(ddata->paintedSet, ddata->gc, &mp1);
    }
    
    xrst_flush_buffer(ddata);
}

/*
 *  xrst_fillarc
 */
void xrst_fillarc(const Canvas *canvas, void *data,
    const VPoint *vp1, const VPoint *vp2, double a1, double a2, int mode)
{
    Xrst_data *ddata = (Xrst_data *) data;
    miPoint mp1, mp2;
    
    VPoint2miPoint(ddata, vp1, &mp1);
    VPoint2miPoint(ddata, vp2, &mp2);
    
    xrst_setpen(canvas, ddata);
    
    if (mp1.x != mp2.x || mp1.y != mp2.y) {
        miArc arc;
        
        arc.x      = MIN2(mp1.x, mp2.x);
        arc.y      = MIN2(mp1.y, mp2.y);
        arc.width  = abs(mp2.x - mp1.x);
        arc.height = abs(mp2.y - mp1.y);
        arc.angle1 = (int) rint(64*a1);
        arc.angle2 = (int) rint(64*a2);
        
        if (ddata->arcfillmode != mode) {
            ddata->arcfillmode = mode;
            if (mode == ARCFILL_CHORD) {
                miSetGCAttrib(ddata->gc, MI_GC_ARC_MODE, MI_ARC_CHORD);
            } else {
                miSetGCAttrib(ddata->gc, MI_GC_ARC_MODE, MI_ARC_PIE_SLICE);
            }
        }
        
        miFillArcs(ddata->paintedSet, ddata->gc, 1, &arc);
    } else { /* zero radius */
        miDrawPoint(ddata->paintedSet, ddata->gc, &mp1);
    }
    
    xrst_flush_buffer(ddata);
}


void xrst_putpixmap(const Canvas *canvas, void *data,
    const VPoint *vp, int width, int height, char *databits,
    int pixmap_bpp, int bitmap_pad, int pixmap_type)
{
    Xrst_data *ddata = (Xrst_data *) data;
    int cindex, bg;
    int color;
    
    int	i, k, j;
    long paddedW;
    
    miPoint mp;
    int x, y, xleft, ytop, xright, ybottom;
    
    bg = getbgcolor(canvas);
    
    VPoint2miPoint(ddata, vp, &mp);
    
    MI_GET_CANVAS_DRAWABLE_BOUNDS(ddata->mcanvas, xleft, ytop, xright, ybottom)
    
    y = mp.y;
    if (pixmap_bpp == 1) {
        color = getcolor(canvas);
        paddedW = PAD(width, bitmap_pad);
        for (k = 0; k < height; k++) {
            x = mp.x;
            y++;
            for (j = 0; j < paddedW/bitmap_pad; j++) {
                for (i = 0; i < bitmap_pad && j*bitmap_pad + i < width; i++) {
                    x++;
                    /* bound checking */
                    if (x < xleft || x > xright ||
                        y < ytop  || y > ybottom) {
                        continue;
                    }
                    if (bin_dump(&(databits)[k*paddedW/bitmap_pad+j], i, bitmap_pad)) {
                        MI_SET_CANVAS_DRAWABLE_PIXEL(ddata->mcanvas, x, y, color);
                    } else {
                        if (pixmap_type == PIXMAP_OPAQUE) {
                            MI_SET_CANVAS_DRAWABLE_PIXEL(ddata->mcanvas, x, y, bg);
                        }
                    }
                }
            }
        }
    } else {
        for (k = 0; k < height; k++) {
            x = mp.x;
            y++;
            for (j = 0; j < width; j++) {
                x++;
                /* bound checking */
                if (x < xleft || x > xright ||
                    y < ytop  || y > ybottom) {
                    continue;
                }
                cindex = (databits)[k*width+j];
                if (cindex != bg || pixmap_type == PIXMAP_OPAQUE) {
                    color = cindex;
                    MI_SET_CANVAS_DRAWABLE_PIXEL(ddata->mcanvas, x, y, color);
                }
            }
        }
    }
}

void xrst_leavegraphics(const Canvas *canvas, void *data,
    const CanvasStats *cstats)
{
    Xrst_data *ddata = (Xrst_data *) data;
    Xrst_pixmap pixmap;
    view v;
    VPoint luvp, rlvp;
    miPoint lump, rlmp;

    v = cstats->bbox;
    
    /* left upper corner */
    luvp.x = v.xv1;
    luvp.y = v.yv2;
    VPoint2miPoint(ddata, &luvp, &lump);
    /* right lower corner */
    rlvp.x = v.xv2;
    rlvp.y = v.yv1;
    VPoint2miPoint(ddata, &rlvp, &rlmp);
    /* make sure the edge pixel lines aren't cut off */
    lump.x -= 1;
    lump.y -= 1;
    rlmp.x += 1;
    rlmp.y += 1;
    /* ... yet we're within the canvas still */
    lump.x = MAX2(0, lump.x);
    lump.y = MAX2(0, lump.y);
    rlmp.x = MIN2(ddata->mcanvas->drawable->width, rlmp.x);
    rlmp.y = MIN2(ddata->mcanvas->drawable->height, rlmp.y);
    
    pixmap.width   = rlmp.x - lump.x;
    pixmap.height  = rlmp.y - lump.y;
    pixmap.matrix  = xmalloc(pixmap.height*SIZEOF_VOID_P);
    if (pixmap.matrix) {
        int i;
        for (i = 0; i < pixmap.height; i++) {
            pixmap.matrix[i] = &ddata->mcanvas->drawable->pixmap[i + lump.y][lump.x];
        }
        
        ddata->dump(canvas, ddata->data,
            cstats->ncolors, cstats->colors, &pixmap);
    
        xfree(pixmap.matrix);
    }
    
    /* clean up */
    miDeleteCanvas(ddata->mcanvas);
    miDeleteGC(ddata->gc);
    miDeletePaintedSet(ddata->paintedSet);
}

#else /* No XMI library */
void _xrstdrv_c_dummy_func(void) {}
#endif
