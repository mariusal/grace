/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
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

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>

#include <qtinc.h>

extern "C" {
#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "defines.h"
#include "globals.h"

#include "utils.h"
#include "files.h"
#include "core_utils.h"

#include "motifinc.h"
#include "xprotos.h"
}

//extern XtAppContext app_con;

//extern Input_buffer *ib_tbl;
//extern int ib_tblsize;

//static GC gcxor;

//int x11_get_pixelsize(const GUI *gui)
//{
//    Screen *screen = DefaultScreenOfDisplay(gui->xstuff->disp);
//    int i, n;
//    XPixmapFormatValues *pmf;
//    int pixel_size = 0;

//    pmf = XListPixmapFormats(DisplayOfScreen(screen), &n);
//    if (pmf) {
//        for (i = 0; i < n; i++) {
//            if (pmf[i].depth == PlanesOfScreen(screen)) {
//                pixel_size = pmf[i].bits_per_pixel/8;
//                break;
//            }
//        }
//        XFree((char *) pmf);
//    }
    
//    return pixel_size;
//}

//long x11_allocate_color(GUI *gui, const RGB *rgb)
//{
//    X11Stuff *xstuff = gui->xstuff;
//    XColor xc;
    
//    xc.pixel = 0;
//    xc.flags = DoRed | DoGreen | DoBlue;
    
//    xc.red   = rgb->red   << (16 - CANVAS_BPCC);
//    xc.green = rgb->green << (16 - CANVAS_BPCC);
//    xc.blue  = rgb->blue  << (16 - CANVAS_BPCC);

//    if (XAllocColor(xstuff->disp, xstuff->cmap, &xc)) {
//        return xc.pixel;
//    } else
//    if (gui->install_cmap != CMAP_INSTALL_NEVER &&
//        gui->private_cmap == FALSE) {
//        xstuff->cmap = XCopyColormapAndFree(xstuff->disp, xstuff->cmap);
//        gui->private_cmap = TRUE;
        
//        /* try to allocate the same color in the private colormap */
//        return x11_allocate_color(gui, rgb);
//    } else {
//        return -1;
//    }
//}

//void set_wait_cursor(void)
//{
//    if (gapp->gui->xstuff->disp == NULL) {
//        return;
//    }
    
//    DefineDialogCursor(gapp->gui->xstuff->wait_cursor);
//}

//void unset_wait_cursor(void)
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    if (xstuff->disp == NULL) {
//        return;
//    }
    
//    UndefineDialogCursor();
//    if (xstuff->cur_cursor >= 0) {
//        set_cursor(gapp->gui, xstuff->cur_cursor);
//    }
//}

//void set_cursor(GUI *gui, int c)
//{
//    X11Stuff *xstuff = gui->xstuff;
//    if (xstuff->disp == NULL || xstuff->cur_cursor == c) {
//        return;
//    }

//    XUndefineCursor(xstuff->disp, xstuff->xwin);
//    xstuff->cur_cursor = c;
//    switch (c) {
//    case 0:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->line_cursor);
//        break;
//    case 1:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->find_cursor);
//        break;
//    case 2:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->text_cursor);
//        break;
//    case 3:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->kill_cursor);
//        break;
//    case 4:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->move_cursor);
//        break;
//    case 5:
//        XDefineCursor(xstuff->disp, xstuff->xwin, xstuff->drag_cursor);
//        break;
//    default:
//        xstuff->cur_cursor = -1;
//        break;
//    }
//    XFlush(xstuff->disp);
//}

//void init_cursors(GUI *gui)
//{
//    X11Stuff *xstuff = gui->xstuff;

//    xstuff->wait_cursor = XCreateFontCursor(xstuff->disp, XC_watch);
//    xstuff->line_cursor = XCreateFontCursor(xstuff->disp, XC_crosshair);
//    xstuff->find_cursor = XCreateFontCursor(xstuff->disp, XC_dotbox);
//    xstuff->text_cursor = XCreateFontCursor(xstuff->disp, XC_xterm);
//    xstuff->kill_cursor = XCreateFontCursor(xstuff->disp, XC_pirate);
//    xstuff->move_cursor = XCreateFontCursor(xstuff->disp, XC_fleur);
//    xstuff->drag_cursor = XCreateFontCursor(xstuff->disp, XC_hand2);
    
//    xstuff->cur_cursor = -1;
//}

///*
// *  Auxiliary routines for simultaneous drawing on display and pixmap
// */
//void aux_XDrawLine(GUI *gui, int x1, int y1, int x2, int y2)
//{
//    X11Stuff *xstuff = gui->xstuff;
//    XDrawLine(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XDrawLine(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
//    }
//}

//void aux_XDrawRectangle(GUI *gui, int x1, int y1, int x2, int y2)
//{
//    X11Stuff *xstuff = gui->xstuff;
//    XDrawRectangle(xstuff->disp, xstuff->xwin, gcxor, x1, y1, x2, y2);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XDrawRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x1, y1, x2, y2);
//    }
//}

//void aux_XFillRectangle(GUI *gui, int x, int y, unsigned int width, unsigned int height)
//{
//    X11Stuff *xstuff = gui->xstuff;
//    XFillRectangle(xstuff->disp, xstuff->xwin, gcxor, x, y, width, height);
//    if (xstuff->bufpixmap != (Pixmap) NULL) {
//        XFillRectangle(xstuff->disp, xstuff->bufpixmap, gcxor, x, y, width, height);
//    }
//}


///*
// * expose/resize proc
// */
//void expose_resize(Widget w, XtPointer client_data, XtPointer call_data)
//{
//    GraceApp *gapp = (GraceApp *) client_data;
//    static int inc = FALSE;
//    XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *) call_data;

//    /* HACK */
//    if (!gapp->gui->inwin) {
//        return;
//    }
    
//    if (!inc) {
//	inc = TRUE;
        
//        update_all();
//        xdrawgraph(gapp->gp);

//        return;
//    }
    
//    if (cbs->reason == XmCR_EXPOSE) {
//  	x11_redraw(cbs->window,
//            cbs->event->xexpose.x,
//            cbs->event->xexpose.y,
//            cbs->event->xexpose.width,
//            cbs->event->xexpose.height);
//    } else {
//        if (gui_is_page_free(gapp->gui)) {
//            sync_canvas_size(gapp);
//            update_all();
//            xdrawgraph(gapp->gp);
//        }
//    }
//}

    
//void xdrawgrid(X11Stuff *xstuff)
//{
//    int i, j;
//    double step;
//    XPoint xp;
//    unsigned long black = BlackPixel(xstuff->disp, xstuff->screennumber);
//    unsigned long white = WhitePixel(xstuff->disp, xstuff->screennumber);
    
//    XSetForeground(xstuff->disp, xstuff->gc, white);
//    XSetFillStyle(xstuff->disp, xstuff->gc, FillSolid);
//    XFillRectangle(xstuff->disp, xstuff->bufpixmap, xstuff->gc, 0, 0, xstuff->win_w, xstuff->win_h);
//    XSetForeground(xstuff->disp, xstuff->gc, black);
    
//    step = (double) (xstuff->win_scale)/10;
//    for (i = 0; i < xstuff->win_w/step; i++) {
//        for (j = 0; j < xstuff->win_h/step; j++) {
//            xp.x = rint(i*step);
//            xp.y = xstuff->win_h - rint(j*step);
//            XDrawPoint(xstuff->disp, xstuff->bufpixmap,
//                xstuff->gc, xp.x, xp.y);
//        }
//    }
    
//    XSetLineAttributes(xstuff->disp, xstuff->gc,
//        1, LineSolid, CapButt, JoinMiter);
//    XDrawRectangle(xstuff->disp, xstuff->bufpixmap,
//        xstuff->gc, 0, 0, xstuff->win_w - 1, xstuff->win_h - 1);
//}

//void x11_redraw(Window window, int x, int y, int width, int height)
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    if (gapp->gui->inwin == TRUE && xstuff->bufpixmap != (Pixmap) NULL) {
//        XCopyArea(xstuff->disp, xstuff->bufpixmap, window, xstuff->gc, x, y, width, height, x, y);
//    }
//}

//void x11_redraw_all()
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    x11_redraw(xstuff->xwin, 0, 0, xstuff->win_w, xstuff->win_h);
//}

//static void xmonitor_rti(XtPointer ib, int *ptrFd, XtInputId *ptrId)
//{
//    set_wait_cursor();
    
//    monitor_input(gapp, (Input_buffer *) ib, 1, 1);
    
//    unset_wait_cursor();
//}

//void xregister_rti(Input_buffer *ib)
//{
//    if (gapp->gui->inwin && ib) {
//        /* the screen has been initialized : we can register the buffer */
//        ib->id = (unsigned long) XtAppAddInput(app_con,
//                                               ib->fd,
//                                               (XtPointer) XtInputReadMask,
//                                               xmonitor_rti,
//                                               (XtPointer) ib);
//    }
//}

//void xunregister_rti(const Input_buffer *ib)
//{
//    if (gapp->gui->inwin && ib) {
//        /* the screen has been initialized : we can remove the buffer */
//        XtRemoveInput((XtInputId) ib->id);
//    }
//}

//void move_pointer(short x, short y)
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;

//    XWarpPointer(xstuff->disp, None, xstuff->xwin, 0, 0, 0, 0, x, y);
//}

//char *display_name(GUI *gui)
//{
//    return DisplayString(gui->xstuff->disp);
//}

//#define BUFSIZE 1024
//static int HandleXError(Display *dpy, XErrorEvent *event)
//{
//    char buffer[BUFSIZE], mesg[BUFSIZE], *output;

//    char *mtype = "XlibMessage";

//    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", buffer, BUFSIZE);
//    output = copy_string(NULL, buffer);
//    output = concat_strings(output, ":");
    
//    XGetErrorText(dpy, event->error_code, buffer, BUFSIZE);
//    output = concat_strings(output, buffer);
//    output = concat_strings(output, "\n");
    
//    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", mesg,
//                          BUFSIZE);
//    sprintf(buffer, mesg, event->request_code);
//    output = concat_strings(output, buffer);

//    if (event->request_code < 128) {
//        char number[32];
//        sprintf(number, "%d", event->request_code);
//        XGetErrorDatabaseText(dpy, "XRequest", number, "", mesg, BUFSIZE);
//        sprintf(buffer, " (%s)\n", mesg);
//        output = concat_strings(output, buffer);
//    }

//    switch (event->error_code) {
//    case (BadWindow):
//    case (BadPixmap):
//    case (BadCursor):
//    case (BadFont):
//    case (BadDrawable):
//    case (BadColor):
//    case (BadGC):
//    case (BadIDChoice):
//    case (BadValue):
//    case (BadAtom):
//        if (event->error_code == BadValue) {
//           XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x", mesg,
//                                 BUFSIZE);
//        } else if (event->error_code == BadAtom) {
//           XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x", mesg,
//                                 BUFSIZE);
//        } else {
//           XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
//                                 mesg, BUFSIZE);
//        }
//        output = concat_strings(output, "  ");
//        sprintf(buffer, mesg, event->resourceid);
//        output = concat_strings(output, buffer);
//        output = concat_strings(output, "\n");
//        break;
//    } /* switch() */

//    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", mesg,
//                           BUFSIZE);
//    sprintf(buffer, mesg, event->serial);
//    output = concat_strings(output, "  ");
//    output = concat_strings(output, buffer);
//    output = concat_strings(output, ".");
    
//    emergency_exit(gapp, TRUE, output);
//    xfree(output);
    
//    /* return value is ignored anyway */
//    return 0;
//}

///*
// * Interrupt handler for X IO errors
// */
//static int HandleXIOError(Display *d)
//{
//    char msg[BUFSIZE];
//    if (errno == EPIPE) {
//        sprintf(msg, "X connection to %s broken (server error - EPIPE).",
//               DisplayString(d));
//    } else {
//        sprintf(msg, "Fatal IO error on X server %s.", DisplayString(d));
//    }

//    emergency_exit(gapp, FALSE, msg);
    
//    /* Ideally, we don't reach this anyway ... */
//    return 1;
//}

//void installXErrorHandler(void)
//{
//    XSetErrorHandler(HandleXError);
//    XSetIOErrorHandler(HandleXIOError);
//}

//int x11_init(GraceApp *gapp)
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    XGCValues gc_val;
//    long mrsize;
//    int max_path_limit;
    
//    xstuff->screennumber = DefaultScreen(xstuff->disp);
//    xstuff->root = RootWindow(xstuff->disp, xstuff->screennumber);
 
//    xstuff->gc = DefaultGC(xstuff->disp, xstuff->screennumber);
    
//    xstuff->depth = DisplayPlanes(xstuff->disp, xstuff->screennumber);

//    /* init colormap */
//    xstuff->cmap = DefaultColormap(xstuff->disp, xstuff->screennumber);
//    /* redefine colormap, if needed */
//    if (gapp->gui->install_cmap == CMAP_INSTALL_ALWAYS) {
//        xstuff->cmap = XCopyColormapAndFree(xstuff->disp, xstuff->cmap);
//        gapp->gui->private_cmap = TRUE;
//    }
    
//    /* set GCs */
//    if (gapp->gui->invert) {
//        gc_val.function = GXinvert;
//    } else {
//        gc_val.function = GXxor;
//    }
//    gcxor = XCreateGC(xstuff->disp, xstuff->root, GCFunction, &gc_val);

//    /* XExtendedMaxRequestSize() appeared in X11R6 */
//#if XlibSpecificationRelease > 5
//    mrsize = XExtendedMaxRequestSize(xstuff->disp);
//#else
//    mrsize = 0;
//#endif
//    if (mrsize <= 0) {
//        mrsize = XMaxRequestSize(xstuff->disp);
//    }
//    max_path_limit = (mrsize - 3)/2;
//    if (max_path_limit < get_max_path_limit(grace_get_canvas(gapp->grace))) {
//        char buf[128];
//        sprintf(buf,
//            "Setting max drawing path length to %d (limited by the X server)",
//            max_path_limit);
//        errmsg(buf);
//        set_max_path_limit(grace_get_canvas(gapp->grace), max_path_limit);
//    }
    
//    xstuff->dpi = rint(MM_PER_INCH*DisplayWidth(xstuff->disp, xstuff->screennumber)/
//        DisplayWidthMM(xstuff->disp, xstuff->screennumber));

//    return RETURN_SUCCESS;
//}

//Pixmap char_to_pixmap(Widget w, int font, char c, int csize)
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;
//    CPixmap *pm;
//    Pixmap pixmap = 0;
//    int height, width, hshift, vshift;
//    float fsize = 0.8*(float)csize;
    
//    pm = canvas_raster_char(grace_get_canvas(gapp->grace),
//        font, c, fsize, &vshift, &hshift);
       
//    if (pm != NULL && pm->bits != NULL) {
//        long bg, fg;
//        Pixmap ptmp;
        
//        vshift = csize - vshift - 4;
//        height = pm->height;
//        width = pm->width;
        
//        ptmp = XCreateBitmapFromData(xstuff->disp, xstuff->root,
//                    pm->bits, width, height);
//        pixmap = XCreatePixmap(xstuff->disp, xstuff->root,
//            csize, csize, xstuff->depth);
        
//        XtVaGetValues(w, XmNbackground, &bg, XmNforeground, &fg, NULL);
//        XSetFillStyle(xstuff->disp, xstuff->gc, FillSolid);
//        XSetForeground(xstuff->disp, xstuff->gc, bg);
//        XFillRectangle(xstuff->disp, pixmap, xstuff->gc, 0, 0, csize, csize);
        
//        XSetBackground(xstuff->disp, xstuff->gc, bg);
//        XSetForeground(xstuff->disp, xstuff->gc, fg);
//        XCopyPlane(xstuff->disp, ptmp, pixmap, xstuff->gc, 0, 0,
//            width, height, hshift, vshift, 1);
        
//        XFreePixmap(xstuff->disp, ptmp);
//    }
//    canvas_cpixmap_free(pm);
    
//    return pixmap;
//}
Pixmap char_to_pixmap(Widget w, int font, char c, int csize)
{
    CPixmap *pm;
    QPixmap *pixmap;
    int height, width, hshift, vshift;
    float fsize = 0.8*(float)csize;

    pm = canvas_raster_char(grace_get_canvas(gapp->grace),
                            font, c, fsize, &vshift, &hshift);

    if (pm != NULL && pm->bits != NULL) {
        vshift = csize - vshift - 4;
        height = pm->height;
        width = pm->width;

        QBitmap bitmap = QBitmap::fromData(QSize(width, height),
                                           (const uchar*) pm->bits, QImage::Format_MonoLSB);

        pixmap = new QPixmap(csize, csize);
        pixmap->fill(Qt::white);

        QPainter painter(pixmap);
        painter.setPen(Qt::black);
        painter.drawPixmap(QRect(hshift, vshift, width, height),
                           bitmap,
                           QRect(0, 0, width, height));
    }
    canvas_cpixmap_free(pm);

    return pixmap;
}

//void init_xstream(X11stream *xstream)
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;

//    xstream->screen = DefaultScreenOfDisplay(xstuff->disp);
//    xstream->pixmap = xstuff->bufpixmap;
//}

//void create_pixmap(unsigned int w, unsigned int h)
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;

//    xstuff->bufpixmap = XCreatePixmap(xstuff->disp, xstuff->root, w, h, xstuff->depth);
//}

//void recreate_pixmap(unsigned int w, unsigned int h)
//{
//    X11Stuff *xstuff = gapp->gui->xstuff;

//    XFreePixmap(xstuff->disp, xstuff->bufpixmap);
//    create_pixmap(w, h);
//}

