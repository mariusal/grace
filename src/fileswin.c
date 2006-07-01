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

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/DrawingA.h>

#include "globals.h"
#include "core_utils.h"
#include "utils.h"
#include "files.h"
#include "devlist.h"
#include "motifinc.h"
#include "xprotos.h"

static int open_proc(FSBStructure *fsb, char *filename, void *data);
static int save_proc(FSBStructure *fsb, char *filename, void *data);

static int read_sets_proc(FSBStructure *fsb, char *filename, void *data);
static void set_load_proc(OptionStructure *opt, int value, void *data);
static void set_src_proc(OptionStructure *opt, int value, void *data);
static int write_ssd_proc(FSBStructure *fsb, char *filename, void *data);

void create_saveproject_popup(void)
{
    static FSBStructure *fsb = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Save project");
	AddFileSelectionBoxCB(fsb, save_proc, NULL);
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
    if (save_project(gapp->project, filename) == RETURN_SUCCESS) {
        update_all();
        return TRUE;
    } else {
        return FALSE;
    }
}

#define PREVIEW_WIDTH   250
#define PREVIEW_HEIGHT  200

typedef struct {
    FSBStructure *fsb;
    Widget canvasw;
    Pixmap pixmap;

    int idevice;

    int preview_ok;
    int x_offset, y_offset;
} openGUI;

static void select_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    openGUI *ui = (openGUI *) client_data;
    char *filename = XmTextGetString(w);
    struct stat statb;
    Canvas *canvas = grace_get_canvas(gapp->grace);
    
    XClearWindow(XtDisplay(ui->canvasw), XtWindow(ui->canvasw));

    ui->preview_ok = FALSE;
    
    if (stat(filename, &statb) == 0 && !S_ISDIR(statb.st_mode)) {

        Quark *project = load_any_project(gapp, filename);

        if (project) {
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

	    grace_render(gapp->grace, project);

            XCopyArea(XtDisplay(ui->canvasw), ui->pixmap, XtWindow(ui->canvasw),
                DefaultGCOfScreen(XtScreen(ui->canvasw)),
                0, 0, pg->width, pg->height, ui->x_offset, ui->y_offset);
            
            ui->preview_ok = TRUE;
            
            quark_free(project);
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

void create_openproject_popup(void)
{
    static openGUI *ui = NULL;

    set_wait_cursor();

    if (ui == NULL) {
        X11Stuff *xstuff = gapp->gui->xstuff;
        Widget fr, text;
        Canvas *canvas = grace_get_canvas(gapp->grace);
        
        ui = xmalloc(sizeof(openGUI));
        memset(ui, 0, sizeof(openGUI));

        ui->idevice = register_x11_drv(canvas);
        device_set_aux(canvas, ui->idevice);
        
        ui->pixmap = XCreatePixmap(xstuff->disp, xstuff->root,
            PREVIEW_WIDTH, PREVIEW_HEIGHT, xstuff->depth);
        
        ui->fsb = CreateFileSelectionBox(app_shell, "Open project");
	AddFileSelectionBoxCB(ui->fsb, open_proc, NULL);

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
    if (load_project(gapp, filename) == RETURN_SUCCESS) {
        xdrawgraph(gapp->project);
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

        snapshot_and_update(gr, TRUE);
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
        SetSensitive(gui->ftype_item->menu, True);
    } else {
        SetOptionChoice(gui->ftype_item, SET_XY);
        SetSensitive(gui->ftype_item->menu, False);
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


#ifdef HAVE_NETCDF

#include <netcdf.h>

/*
 *
 * netcdf reader
 *
 */

static Widget netcdf_frame = (Widget) NULL;

static Widget netcdf_listx_item;
static Widget netcdf_listy_item;
static Widget netcdf_file_item;

void create_netcdffiles_popup(Widget w, XtPointer client_data, XtPointer call_data);

static void do_netcdfquery_proc(Widget w, XtPointer client_data, XtPointer call_data);

void update_netcdfs(void);

int getnetcdfvars(void);

static void do_netcdf_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char fname[256];
    char xvar[256], yvar[256];
    XmString *s, cs;
    int *pos_list;
    int j, pos_cnt, cnt, retval;
    char *cstr;

    set_wait_cursor();

/*
 * setno == -1, then next set
 */
    strcpy(fname, xv_getstr(netcdf_file_item));
    if (XmListGetSelectedPos(netcdf_listx_item, &pos_list, &pos_cnt)) {
	XtVaGetValues(netcdf_listx_item,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	cs = XmStringCopy(*s);
	if ((cstr = GetStringSimple(cs))) {
	    strcpy(xvar, cstr);
	    XtFree(cstr);
	}
	XmStringFree(cs);
    } else {
	errmsg("Need to select X, either variable name or INDEX");
	unset_wait_cursor();
	return;
    }
    if (XmListGetSelectedPos(netcdf_listy_item, &pos_list, &pos_cnt)) {
	j = pos_list[0];
	XtVaGetValues(netcdf_listy_item,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	cs = XmStringCopy(*s);
	if ((cstr = GetStringSimple(cs))) {
	    strcpy(yvar, cstr);
	    XtFree(cstr);
	}
	XmStringFree(cs);
    } else {
	errmsg("Need to select Y");
	unset_wait_cursor();
	return;
    }
    if (strcmp(xvar, "INDEX") == 0) {
	retval = readnetcdf(NULL, fname, NULL, yvar, -1, -1, 1);
    } else {
	retval = readnetcdf(NULL, fname, xvar, yvar, -1, -1, 1);
    }
    if (retval) {
	xdrawgraph(gapp->project);
    }
    unset_wait_cursor();
}

void update_netcdfs(void)
{
    int i;
    char buf[256], fname[512];
    XmString xms;
    int cdfid;			/* netCDF id */
    int ndims, nvars, ngatts, recdim;
    int var_id;
    char varname[256];
    nc_type datatype = 0;
    int dim[100], natts;
    long dimlen[100];
    long len;

    ncopts = 0;			/* no crash on error */

    if (netcdf_frame != NULL) {
	strcpy(fname, xv_getstr(netcdf_file_item));
	set_wait_cursor();
	XmListDeleteAllItems(netcdf_listx_item);
	XmListDeleteAllItems(netcdf_listy_item);
	xms = XmStringCreateLocalized("INDEX");
	XmListAddItemUnselected(netcdf_listx_item, xms, 0);
	XmStringFree(xms);

	if (strlen(fname) < 2) {
	    unset_wait_cursor();
	    return;
	}
	if ((cdfid = ncopen(fname, NC_NOWRITE)) == -1) {
	    errmsg("Can't open file.");
	    unset_wait_cursor();
	    return;
	}
	ncinquire(cdfid, &ndims, &nvars, &ngatts, &recdim);
	for (i = 0; i < ndims; i++) {
	    ncdiminq(cdfid, i, NULL, &dimlen[i]);
	}
	for (i = 0; i < nvars; i++) {
	    ncvarinq(cdfid, i, varname, &datatype, &ndims, dim, &natts);
	    if ((var_id = ncvarid(cdfid, varname)) == -1) {
		char ebuf[256];
		sprintf(ebuf, "update_netcdfs(): No such variable %s", varname);
		errmsg(ebuf);
		continue;
	    }
	    if (ndims != 1) {
		continue;
	    }
	    ncdiminq(cdfid, dim[0], (char *) NULL, &len);
	    sprintf(buf, "%s", varname);
	    xms = XmStringCreateLocalized(buf);
	    XmListAddItemUnselected(netcdf_listx_item, xms, 0);
	    XmListAddItemUnselected(netcdf_listy_item, xms, 0);
	    XmStringFree(xms);
	}
	ncclose(cdfid);
	
	unset_wait_cursor();
    }
}

static void do_netcdfupdate_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_wait_cursor();
    update_netcdfs();
    unset_wait_cursor();
}

void create_netcdfs_popup(Widget but, void *data)
{
    static Widget top, dialog;
    Widget lab;
    Arg args[3];

    set_wait_cursor();
    if (top == NULL) {
	char *label1[5];
	Widget but1[5];

	label1[0] = "Accept";
	label1[1] = "Files...";
	label1[2] = "Update";
	label1[3] = "Query";
	label1[4] = "Close";
	top = XmCreateDialogShell(app_shell, "netCDF", NULL, 0);
	handle_close(top);
	dialog = CreateVContainer(top);

	XtSetArg(args[0], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);
	XtSetArg(args[1], XmNvisibleItemCount, 5);

	lab = CreateLabel(dialog, "Select set X:");
	netcdf_listx_item = XmCreateScrolledList(dialog, "list", args, 2);
	ManageChild(netcdf_listx_item);

	lab = CreateLabel(dialog, "Select set Y:");
	netcdf_listy_item = XmCreateScrolledList(dialog, "list", args, 2);
	ManageChild(netcdf_listy_item);

	netcdf_file_item = CreateTextItem(dialog, 30, "netCDF file:");

	CreateSeparator(dialog);

	CreateCommandButtons(dialog, 5, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_netcdf_proc,
		      (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) create_netcdffiles_popup,
		      (XtPointer) NULL);
	XtAddCallback(but1[2], XmNactivateCallback, (XtCallbackProc) do_netcdfupdate_proc,
		      (XtPointer) NULL);
	XtAddCallback(but1[3], XmNactivateCallback, (XtCallbackProc) do_netcdfquery_proc,
		      (XtPointer) NULL);
	XtAddCallback(but1[4], XmNactivateCallback, (XtCallbackProc) destroy_dialog,
		      (XtPointer) top);

	ManageChild(dialog);
	netcdf_frame = top;
    }
    update_netcdfs();
    RaiseWindow(top);
    unset_wait_cursor();
}

static int do_netcdffile_proc(FSBStructure *fsb, char *filename, void *data)
{
    xv_setstr(netcdf_file_item, filename);
    update_netcdfs();
    
    return TRUE;
}

void create_netcdffiles_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    static FSBStructure *fsb = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Select netCDF file");
	AddFileSelectionBoxCB(fsb, do_netcdffile_proc, NULL);
        ManageChild(fsb->FSB);
    }
    
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
}

char *getcdf_type(nc_type datatype)
{
    switch (datatype) {
    case NC_SHORT:
	return "NC_SHORT";
	break;
    case NC_LONG:
	return "NC_LONG";
	break;
    case NC_FLOAT:
	return "NC_FLOAT";
	break;
    case NC_DOUBLE:
	return "NC_DOUBLE";
	break;
    default:
	return "UNKNOWN (can't read this)";
	break;
    }
}

static void do_netcdfquery_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char xvar[256], yvar[256];
    char buf[256], fname[512];
    XmString *s, cs;
    int *pos_list;
    int i, pos_cnt, cnt;
    char *cstr;

    int cdfid;			/* netCDF id */
    nc_type datatype = 0;
    float f;
    double d;

    int x_id, y_id;
    nc_type xdatatype = 0;
    nc_type ydatatype = 0;
    int xndims, xdim[10], xnatts;
    int yndims, ydim[10], ynatts;
    long nx, ny;

    int atlen;
    char attname[256];
    char atcharval[256];

    ncopts = 0;			/* no crash on error */

    set_wait_cursor();

    strcpy(fname, xv_getstr(netcdf_file_item));

    if ((cdfid = ncopen(fname, NC_NOWRITE)) == -1) {
	errmsg("Can't open file.");
	unset_wait_cursor();
	return;
    }
    if (XmListGetSelectedPos(netcdf_listx_item, &pos_list, &pos_cnt)) {
	XtVaGetValues(netcdf_listx_item,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	cs = XmStringCopy(*s);
	if ((cstr = GetStringSimple(cs))) {
	    strcpy(xvar, cstr);
	    XtFree(cstr);
	}
	XmStringFree(cs);
    } else {
	errmsg("Need to select X, either variable name or INDEX");
	goto out1;
    }
    if (XmListGetSelectedPos(netcdf_listy_item, &pos_list, &pos_cnt)) {
	XtVaGetValues(netcdf_listy_item,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	cs = XmStringCopy(*s);
	if ((cstr = GetStringSimple(cs))) {
	    strcpy(yvar, cstr);
	    XtFree(cstr);
	}
	XmStringFree(cs);
    } else {
	errmsg("Need to select Y");
	goto out1;
    }
    if (strcmp(xvar, "INDEX") == 0) {
	stufftext("X is the index of the Y variable\n");
    } else {
	if ((x_id = ncvarid(cdfid, xvar)) == -1) {
	    char ebuf[256];
	    sprintf(ebuf, "do_query(): No such variable %s for X", xvar);
	    errmsg(ebuf);
	    goto out1;
	}
	ncvarinq(cdfid, x_id, NULL, &xdatatype, &xndims, xdim, &xnatts);
	ncdiminq(cdfid, xdim[0], NULL, &nx);
	sprintf(buf, "X is %s, data type %s \t length [%ld]\n", xvar, getcdf_type(xdatatype), nx);
	stufftext(buf);
	sprintf(buf, "\t%d Attributes:\n", xnatts);
	stufftext(buf);
	for (i = 0; i < xnatts; i++) {
	    atcharval[0] = 0;
	    ncattname(cdfid, x_id, i, attname);
	    ncattinq(cdfid, x_id, attname, &datatype, &atlen);
	    switch (datatype) {
	    case NC_CHAR:
		ncattget(cdfid, x_id, attname, (void *) atcharval);
		atcharval[atlen] = 0;
		sprintf(buf, "\t\t%s: %s\n", attname, atcharval);
		stufftext(buf);
		break;
	    case NC_FLOAT:
		ncattget(cdfid, x_id, attname, (void *) &f);
		sprintf(buf, "\t\t%s: %f\n", attname, f);
		stufftext(buf);
		break;
	    case NC_DOUBLE:
		ncattget(cdfid, x_id, attname, (void *) &d);
		sprintf(buf, "\t\t%s: %f\n", attname, d);
		stufftext(buf);
		break;
	       default:
                break;
            }
	}
    }
    if ((y_id = ncvarid(cdfid, yvar)) == -1) {
	char ebuf[256];
	sprintf(ebuf, "do_query(): No such variable %s for Y", yvar);
	errmsg(ebuf);
	goto out1;
    }
    ncvarinq(cdfid, y_id, NULL, &ydatatype, &yndims, ydim, &ynatts);
    ncdiminq(cdfid, ydim[0], NULL, &ny);
    sprintf(buf, "Y is %s, data type %s \t length [%ld]\n", yvar, getcdf_type(ydatatype), ny);
    stufftext(buf);
    sprintf(buf, "\t%d Attributes:\n", ynatts);
    stufftext(buf);
    for (i = 0; i < ynatts; i++) {
	atcharval[0] = 0;
	ncattname(cdfid, y_id, i, attname);
	ncattinq(cdfid, y_id, attname, &datatype, &atlen);
	switch (datatype) {
	case NC_CHAR:
	    ncattget(cdfid, y_id, attname, (void *) atcharval);
	    atcharval[atlen] = 0;
	    sprintf(buf, "\t\t%s: %s\n", attname, atcharval);
	    stufftext(buf);
	    break;
	case NC_FLOAT:
	    ncattget(cdfid, y_id, attname, (void *) &f);
	    sprintf(buf, "\t\t%s: %f\n", attname, f);
	    stufftext(buf);
	    break;
	case NC_DOUBLE:
	    ncattget(cdfid, y_id, attname, (void *) &d);
	    sprintf(buf, "\t\t%s: %f\n", attname, d);
	    stufftext(buf);
	    break;
          default:
            break;
	}
    }

  out1:;
    ncclose(cdfid);
    stufftext("\n");
    unset_wait_cursor();
}

#endif
