/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2005 Grace Development Team
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
 * read/write data/parameter files
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#if defined(HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif
#include <sys/stat.h>

#ifndef QT_GUI
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/DrawingA.h>
#endif

#include "globals.h"
#include "core_utils.h"
#include "utils.h"
#include "files.h"
#include "motifinc.h"
#include "xprotos.h"

static int open_proc(FSBStructure *fsb, char *filename, void *data);
static int save_proc(FSBStructure *fsb, char *filename, void *data);

static int read_sets_proc(FSBStructure *fsb, char *filename, void *data);
static void set_load_proc(OptionStructure *opt, int value, void *data);
static void set_src_proc(OptionStructure *opt, int value, void *data);
static int write_ssd_proc(FSBStructure *fsb, char *filename, void *data);

void create_saveproject_popup(GProject *gp)
{
    static FSBStructure *fsb = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Save project");
        AddFileSelectionBoxCB(fsb, save_proc, gp);
#ifdef QT_GUI
        SetFileSelectionBoxPattern(fsb, "*.xgr");
#endif
        ManageChild(fsb->FSB);
    }
    
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
}

/*
 *  save project to a file
 */
static int save_proc(FSBStructure *fsb, char *filename, void *data)
{
    GProject *gp = (GProject *) data;

    if (save_project(gp, filename) == RETURN_SUCCESS) {
        update_all();
        return TRUE;
    } else {
        return FALSE;
    }
}

#ifndef QT_GUI

#define PREVIEW_WIDTH   250
#define PREVIEW_HEIGHT  200
#endif
typedef struct {
    FSBStructure *fsb;
#ifndef QT_GUI
    Widget canvasw;
    Pixmap pixmap;

    int idevice;

    int preview_ok;
    int x_offset, y_offset;
#endif
} openGUI;
#ifndef QT_GUI
static void select_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    openGUI *ui = (openGUI *) client_data;
    char *filename = XmTextGetString(w);
    struct stat statb;
    Canvas *canvas = grace_get_canvas(gapp->grace);

    XClearWindow(XtDisplay(ui->canvasw), XtWindow(ui->canvasw));

    ui->preview_ok = FALSE;
    
    if (stat(filename, &statb) == 0 && !S_ISDIR(statb.st_mode)) {

        GProject *gp = load_any_project(gapp, filename);

        if (gp) {
            Quark *project = gproject_get_top(gp);
            int wpp, hpp;
            float dpi;
            Device_entry *d = get_device_props(canvas, ui->idevice);
            Page_geometry *pg = &d->pg;
            X11stream xstream;
            
            project_get_page_dimensions(project, &wpp, &hpp);

            if (wpp > hpp) {
                dpi = 72.0*PREVIEW_WIDTH/wpp;
            } else {
                dpi = 72.0*PREVIEW_HEIGHT/hpp;
            }
            
            pg->dpi = dpi;
            pg->width  = MIN2((unsigned long) (wpp*dpi/72), PREVIEW_WIDTH);
            pg->height = MIN2((unsigned long) (hpp*dpi/72), PREVIEW_HEIGHT);
            ui->x_offset = (PREVIEW_WIDTH - pg->width)/2;
            ui->y_offset = (PREVIEW_HEIGHT - pg->height)/2;

            select_device(canvas, ui->idevice);

            xstream.screen = XtScreen(ui->canvasw);
            xstream.pixmap = ui->pixmap;
            canvas_set_prstream(canvas, &xstream);

            XSetForeground(XtDisplay(ui->canvasw),
                DefaultGCOfScreen(xstream.screen),
                WhitePixelOfScreen(xstream.screen));
            XSetFillStyle(XtDisplay(ui->canvasw),
                DefaultGCOfScreen(xstream.screen), FillSolid);
            XFillRectangle(XtDisplay(ui->canvasw), ui->pixmap,
                DefaultGCOfScreen(xstream.screen), 0, 0,
                PREVIEW_WIDTH, PREVIEW_HEIGHT);

	    gproject_render(gp);

            XCopyArea(XtDisplay(ui->canvasw), ui->pixmap, XtWindow(ui->canvasw),
                DefaultGCOfScreen(XtScreen(ui->canvasw)),
                0, 0, pg->width, pg->height, ui->x_offset, ui->y_offset);

            ui->preview_ok = TRUE;
            
            gproject_free(gp);
        }
    }
    XtFree(filename);
}

void exposeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    openGUI *ui = (openGUI *) client_data;
    XmDrawingAreaCallbackStruct *cbs =
        (XmDrawingAreaCallbackStruct *) call_data;
    if (ui->preview_ok) {
  	XCopyArea(XtDisplay(ui->canvasw), ui->pixmap, cbs->window,
            DefaultGCOfScreen(XtScreen(ui->canvasw)),
            cbs->event->xexpose.x - ui->x_offset,
            cbs->event->xexpose.y - ui->y_offset,
            cbs->event->xexpose.width,
            cbs->event->xexpose.height,
            cbs->event->xexpose.x,
            cbs->event->xexpose.y);
    }
}
#endif
void create_openproject_popup(void)
{
    static openGUI *ui = NULL;

    set_wait_cursor();

    if (ui == NULL) {
#ifndef QT_GUI
        X11Stuff *xstuff = gapp->gui->xstuff;
        Widget fr, text;
        Canvas *canvas = grace_get_canvas(gapp->grace);
#endif
        ui = xmalloc(sizeof(openGUI));
        memset(ui, 0, sizeof(openGUI));

#ifndef QT_GUI
        ui->idevice = register_x11_drv(canvas);

        device_set_aux(canvas, ui->idevice);

        ui->pixmap = XCreatePixmap(xstuff->disp, xstuff->root,
            PREVIEW_WIDTH, PREVIEW_HEIGHT, xstuff->depth);
#endif
        ui->fsb = CreateFileSelectionBox(app_shell, "Open project");
	AddFileSelectionBoxCB(ui->fsb, open_proc, NULL);
#ifdef QT_GUI
        SetFileSelectionBoxPattern(ui->fsb, "*.*gr");
#endif
#ifndef QT_GUI
        fr = CreateFrame(ui->fsb->rc, "Preview");

        ui->canvasw = XtVaCreateManagedWidget("canvas",
            xmDrawingAreaWidgetClass, fr,
            XmNwidth, PREVIEW_WIDTH,
            XmNheight, PREVIEW_HEIGHT,
            XmNresizePolicy, XmRESIZE_NONE,
            XmNbackground, WhitePixel(xstuff->disp, xstuff->screennumber),
            NULL);
        XtAddCallback(ui->canvasw, XmNexposeCallback, exposeCB, ui);

        text = XtNameToWidget(ui->fsb->FSB, "Text");
        XtAddCallback(text, XmNvalueChangedCallback,
           select_cb, (XtPointer) ui);
#endif
        ManageChild(ui->fsb->FSB);
    }
    RaiseWindow(ui->fsb->dialog);

    unset_wait_cursor();
}

/*
 *  open project from a file
 */
static int open_proc(FSBStructure *fsb, char *filename, void *data)
{
    unsigned int i;
    int is_open;
    char *epath;
    char *docname;

    is_open = FALSE;
    epath = grace_path(gapp->grace, filename);

    for (i = 0; i < gapp->gpcount; i++) {
        docname = gproject_get_docname(gapp->gplist[i]);
        if (epath && docname && !strcmp(epath, docname)) {
            is_open = TRUE;
            break;
        }
    }
    xfree(epath);

    if (is_open && gapp->gplist[i] != gapp->gp) {
        gapp_set_active_gproject(gapp, gapp->gplist[i]);
        xdrawgraph(gapp->gp);
        update_all();
    }

    if (is_open) {
        return TRUE;
    }

    if (load_project(gapp, filename) == RETURN_SUCCESS) {
        xdrawgraph(gapp->gp);
        update_all();
        return TRUE;
    } else {
        return FALSE;
    }
}


typedef struct {
    StorageStructure *graph_item;  /* graph choice item */
    OptionStructure *ftype_item;   /* set type choice item */
    OptionStructure *load_item;    /* load as single/nxy/block */
    OptionStructure *src_item;     /* normal/pipe */
    OptionStructure *auto_item;    /* autoscale on read */
    OptionStructure *datehint;
} rdataGUI;

void create_file_popup(Widget but, void *data)
{
    static FSBStructure *rdata_dialog = NULL;

    set_wait_cursor();

    if (rdata_dialog == NULL) {
        Widget rc, rc2, fr;
        rdataGUI *gui;
        OptionItem option_items[3];
        OptionItem opitems[4] = {
            {FMT_iso,      "ISO"     },
            {FMT_european, "European"},
            {FMT_us,       "US"      },
            {FMT_nohint,   "None"    }
        };
        
        gui = xmalloc(sizeof(rdataGUI));
        
	rdata_dialog = CreateFileSelectionBox(app_shell, "Read sets");
	AddFileSelectionBoxCB(rdata_dialog, read_sets_proc, (void *) gui);
#ifdef QT_GUI
        SetFileSelectionBoxPattern(rdata_dialog, "*.dat");
#endif

	fr = CreateFrame(rdata_dialog->rc, NULL);
	rc = CreateVContainer(fr);

	gui->graph_item = CreateGraphChoice(rc,
            "Read to graph:", LIST_TYPE_SINGLE);

	rc2 = CreateHContainer(rc);
 
	option_items[0].value = LOAD_SINGLE;
	option_items[0].label = "Single set";
	option_items[1].value = LOAD_NXY;
	option_items[1].label = "NXY";
	option_items[2].value = LOAD_BLOCK;
	option_items[2].label = "Block data";
	gui->load_item = CreateOptionChoice(rc2, "Load as:", 1, 3, option_items);
        AddOptionChoiceCB(gui->load_item, set_load_proc, (void *) gui);
        gui->ftype_item = CreateSetTypeChoice(rc2, "Set type:");

	rc2 = CreateHContainer(rc);
        gui->src_item = CreateOptionChoiceVA(rc2, "Data source:",
            "Disk", SOURCE_DISK,
            "Pipe", SOURCE_PIPE,
            NULL);
	AddOptionChoiceCB(gui->src_item, set_src_proc, (void *) gui);

	rc2 = CreateHContainer(rc);
	gui->auto_item = CreateASChoice(rc2, "Autoscale on read:");
        gui->datehint = CreateOptionChoice(rc2, "Date hint:", 0, 4, opitems);
	SetOptionChoice(gui->datehint, get_date_hint(gapp));

        ManageChild(rdata_dialog->FSB);
    }
    
    
    RaiseWindow(rdata_dialog->dialog);
    
    unset_wait_cursor();
}

static int read_sets_proc(FSBStructure *fsb, char *filename, void *data)
{
    Quark *gr;
    int load;
    
    rdataGUI *gui = (rdataGUI *) data;
    
    load = GetOptionChoice(gui->load_item);
    if (GetSingleStorageChoice(gui->graph_item, &gr) != RETURN_SUCCESS) {
        errmsg("Please select a single graph");
    } else {
        int settype;
        if (load == LOAD_SINGLE) {
            settype = GetOptionChoice(gui->ftype_item);
        } else {
            settype = SET_XY;
        }

        gapp->rt->autoscale_onread = GetOptionChoice(gui->auto_item);
        set_date_hint(gapp, GetOptionChoice(gui->datehint));
        
        getdata(gr, filename, settype, load);

        snapshot_and_update(gapp->gp, TRUE);
    }
    /* never close the popup */
    return FALSE;
}

static void set_src_proc(OptionStructure *opt, int value, void *data)
{
}

static void set_load_proc(OptionStructure *opt, int value, void *data)
{
    rdataGUI *gui = (rdataGUI *) data;
    
    if (value == LOAD_SINGLE) {
        SetSensitive(gui->ftype_item->menu, TRUE);
    } else {
        SetOptionChoice(gui->ftype_item, SET_XY);
        SetSensitive(gui->ftype_item->menu, FALSE);
    }
}


typedef struct {
    SSDColStructure *sel;
} wdataGUI;

void create_write_popup(Widget but, void *data)
{
    static FSBStructure *fsb = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        wdataGUI *gui;
        
	gui = xmalloc(sizeof(wdataGUI));
	
        fsb = CreateFileSelectionBox(app_shell, "Export data");
	AddFileSelectionBoxCB(fsb, write_ssd_proc, (void *) gui);
#ifdef QT_GUI
        SetFileSelectionBoxPattern(fsb, "*.dat");
#endif
	
        gui->sel = CreateSSDColSelector(fsb->rc, NULL, LIST_TYPE_MULTIPLE);

        ManageChild(fsb->FSB);
    }
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
}

/*
 *  write ssd columns to a file
 */
static int write_ssd_proc(FSBStructure *fsb, char *filename, void *data)
{
    wdataGUI *gui = (wdataGUI *) data;
    int ncols, res;
    Quark *ssd;
    int *cols;
    FILE *fp;
    
    ncols = GetSSDColChoices(gui->sel, &ssd, &cols);
    if (ncols < 1) {
        errmsg("No columns selected");
        return FALSE;
    }
    
    fp = gapp_openw(gapp, filename);
    if (fp == NULL) {
        return FALSE;
    }

    res = write_ssd(ssd, ncols, cols, fp);
    
    xfree(cols);
    
    gapp_close(fp);
    
    if (res == RETURN_SUCCESS) {
        return TRUE;
    } else {
        return FALSE;
    }
}
