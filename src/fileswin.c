/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>

#include "globals.h"
#include "graphs.h"
#include "graphutils.h"
#include "plotone.h"
#include "utils.h"
#include "files.h"
#include "motifinc.h"
#include "protos.h"

static Widget rdata_dialog;	/* read data popup */
static ListStructure *read_graph_item;	/* graph choice item */
static OptionStructure *read_ftype_item;	/* set type choice item */
static Widget read_nxy_item;	/* nxy "set type" */
static Widget wparam_frame;	/* write params popup */
static Widget wparam_panel;
static Widget *wparam_choice_item;
static Widget save_format_item;

static void set_src_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void rdata_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_rparams_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void wparam_apply_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_write_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);

static Widget rparams_dialog;	/* read params popup */

static Widget block_dialog;	/* read data popup */

static void block_proc(Widget w, XtPointer client_data, XtPointer call_data);


static void set_src_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int which = (int) client_data;
    XmToggleButtonCallbackStruct *state = (XmToggleButtonCallbackStruct *) call_data;

    if (state->set) {
        cursource = which;
    }
}

static void rdata_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int graphno;
    int n, *values;
    char *s;
    
    XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
	errmsg("Error converting XmString to char string");
	return;
    }

    set_wait_cursor();

    n = GetListChoices(read_graph_item, &values);
    if (n != 1) {
        errmsg("Please select a single graph");
    } else {
        graphno = values[0];
        if (GetToggleButtonState(read_nxy_item)) {
            curtype = SET_NXY;
        } else {
            curtype = GetOptionChoice(read_ftype_item);
        }
        getdata(graphno, s, cursource, curtype);

        update_all();
        drawgraph();
    }

    XtFree(s);
    if (n > 0) {
        free(values);
    }

    unset_wait_cursor();
}

void create_file_popup(Widget wid, XtPointer client_data, XtPointer call_data)
{
    int i;
    Widget lab, rc, rc2, fr, rb, w[3];
    XmString dirmask;

    set_wait_cursor();

    if (rdata_dialog == NULL) {
	rdata_dialog = XmCreateFileSelectionDialog(app_shell, "rdata_dialog", NULL, 0);
	XtVaSetValues(XtParent(rdata_dialog), XmNtitle, "Read sets", NULL);
	XtAddCallback(rdata_dialog, XmNcancelCallback, (XtCallbackProc) destroy_dialog, rdata_dialog);
	XtAddCallback(rdata_dialog, XmNokCallback, rdata_proc, 0);
	XtAddCallback(rdata_dialog, XmNhelpCallback, (XtCallbackProc) HelpCB, 
	              (XtPointer) "file.html#readsets");

	curtype = SET_XY;

	rc = XmCreateRowColumn(rdata_dialog, "Read data main RC", NULL, 0);

	fr = XmCreateFrame(rc, "frame_1", NULL, 0);
	rc2 = XmCreateRowColumn(fr, "Read data main RC", NULL, 0);
	XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
	read_ftype_item = CreateSetTypeChoice(rc2, "Set type:");
	read_nxy_item = CreateToggleButton(rc2, "NXY");

	XtManageChild(rc2);
	XtManageChild(fr);

	fr = XmCreateFrame(rc, "frame_2", NULL, 0);
	rc2 = XmCreateRowColumn(fr, "Read data main RC", NULL, 0);
	XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
	lab = XmCreateLabel(rc2, "Data source:", NULL, 0);
	rb = XmCreateRadioBox(rc2, "radio_box_2", NULL, 0);
	XtVaSetValues(rb, XmNorientation, XmHORIZONTAL, NULL);
	w[0] = XmCreateToggleButton(rb, "Disk", NULL, 0);
	w[1] = XmCreateToggleButton(rb, "Pipe", NULL, 0);
	for (i = 0; i < 2; i++) {
	    XtAddCallback(w[i], XmNvalueChangedCallback, set_src_proc, (XtPointer) i);
	}
	XtManageChild(lab);
	XtManageChild(rb);
	XtManageChildren(w, 2);
	XtManageChild(rc2);
	XtManageChild(fr);
	XmToggleButtonSetState(w[0], True, False);

	fr = XmCreateFrame(rc, "frame_3", NULL, 0);
	rc2 = XmCreateRowColumn(fr, "Read data main RC", NULL, 0);
	read_graph_item = CreateGraphChoice(rc2,
                                            "Read to graph:", LIST_TYPE_SINGLE);
	XtManageChild(rc2);
	XtManageChild(fr);
	XtManageChild(rc);
    }
    XtManageChild(rdata_dialog);
    XtRaise(XtParent(rdata_dialog));
    
    dirmask = XmStringCreateSimple(get_workingdir());
    XmFileSelectionDoSearch(rdata_dialog, dirmask);
    XmStringFree(dirmask);
    
    unset_wait_cursor();
}

static void do_rparams_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
	errwin("Error converting XmString to char string");
	return;
    }
    set_wait_cursor();
    getparms(s);
    unset_wait_cursor();
    XtFree(s);
}

void create_rparams_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    XmString dirmask;
    
    set_wait_cursor();
    if (rparams_dialog == NULL) {
	rparams_dialog = XmCreateFileSelectionDialog(app_shell, "rparams_dialog", NULL, 0);
	XtVaSetValues(XtParent(rparams_dialog), XmNtitle, "Read parameters", NULL);
	XtAddCallback(rparams_dialog, XmNcancelCallback, (XtCallbackProc) destroy_dialog, rparams_dialog);
	XtAddCallback(rparams_dialog, XmNokCallback, (XtCallbackProc) do_rparams_proc, 0);
	XtAddCallback(rparams_dialog, XmNhelpCallback, (XtCallbackProc) HelpCB, 
	              (XtPointer) "file.html#readpars");
    }
    
    XtManageChild(rparams_dialog);
    XtRaise(XtParent(rparams_dialog));

    dirmask = XmStringCreateSimple(get_workingdir());
    XmFileSelectionDoSearch(rparams_dialog, dirmask);
    XmStringFree(dirmask);

    unset_wait_cursor();
}

/*
 * Create the wparam Frame and the wparam Panel
 */
void create_wparam_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget fr;
    XmString dirmask;
    
    set_wait_cursor();
    if (wparam_frame == NULL) {
	wparam_frame = XmCreateFileSelectionDialog(app_shell, "wparam_frame", NULL, 0);
	XtVaSetValues(XtParent(wparam_frame), XmNtitle, "Write plot parameters", NULL);
	XtAddCallback(wparam_frame, XmNcancelCallback, (XtCallbackProc) destroy_dialog, wparam_frame);
	XtAddCallback(wparam_frame, XmNokCallback, (XtCallbackProc) wparam_apply_notify_proc, 0);
	XtAddCallback(wparam_frame, XmNhelpCallback, (XtCallbackProc) HelpCB, 
	              (XtPointer) "file.html#writeparams");

/* may not be needed
	handle_close(wparam_frame);
*/

	fr = XmCreateFrame(wparam_frame, "fr", NULL, 0);
	wparam_panel = XmCreateRowColumn(fr, "wparam_rc", NULL, 0);
	wparam_choice_item = CreatePanelChoice(wparam_panel,
                                               "Write parameters from graph:",
                                               3,
                                               "Current",
                                               "All",
                                               NULL,
                                               NULL);

	XtManageChild(wparam_panel);
	XtManageChild(fr);
    }
    
    XtManageChild(wparam_frame);
    XtRaise(XtParent(wparam_frame));

    dirmask = XmStringCreateSimple(get_workingdir());
    XmFileSelectionDoSearch(wparam_frame, dirmask);
    XmStringFree(dirmask);

    unset_wait_cursor();
}

static void wparam_apply_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *fname;
    int gno;
    FILE *pp;

    XmFileSelectionBoxCallbackStruct *cbs =
                            (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &fname)) {
		errwin("Error converting XmString to char string");
		return;
    }

    if (GetChoice(wparam_choice_item) == 0) {
	gno = get_cg();
    } else {
	gno = ALL_GRAPHS;
    }
    
    pp = grace_openw(fname);
    if (pp != NULL) {
        set_wait_cursor();
        putparms(gno, pp, 0);
        grace_close(pp);
        unset_wait_cursor();
    }
    
    XtFree(fname);
}

static Widget workingd_dialog;

static Widget dir_item;

static void workingdir_apply_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char buf[GR_MAXPATHLEN];
    char *s;
    XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
	errwin("Error converting XmString to char string");
	return;
    }
    strcpy(buf, s);
    XtFree(s);

    if (set_workingdir(buf) == GRACE_EXIT_SUCCESS) {
	XmFileSelectionDoSearch(workingd_dialog, NULL);
    } else {
	errmsg("Can't change to directory");
    }
    XtUnmanageChild(workingd_dialog);
}

static void select_dir(Widget w, XtPointer cd, XmListCallbackStruct * cbs)
{
    char buf[GR_MAXPATHLEN], *str;

    XmStringGetLtoR(cbs->item, charset, &str);
    strcpy(buf, str);
    XtFree(str);

    xv_setstr(dir_item, buf);
    XmFileSelectionDoSearch(workingd_dialog, NULL);
}

void create_workingdir_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    XmString str;

    set_wait_cursor();
    if (workingd_dialog == NULL) {
	workingd_dialog = XmCreateFileSelectionDialog(app_shell, "workingd_dialog", NULL, 0);
	XtVaSetValues(XtParent(workingd_dialog), XmNtitle, "Set working directory", NULL);
	XtAddCallback(workingd_dialog, XmNcancelCallback, (XtCallbackProc) destroy_dialog, (XtPointer) workingd_dialog);
	XtAddCallback(workingd_dialog, XmNokCallback, (XtCallbackProc) workingdir_apply_notify_proc, (XtPointer) 0);
	XtAddCallback(workingd_dialog, XmNhelpCallback, (XtCallbackProc) HelpCB, 
	              (XtPointer) "options.html#workdir");

/* unmanage unneeded items */
	w = XmFileSelectionBoxGetChild(workingd_dialog, XmDIALOG_LIST);
	XtUnmanageChild(XtParent(w));
	w = XmFileSelectionBoxGetChild(workingd_dialog, XmDIALOG_LIST_LABEL);
	XtUnmanageChild(w);
	w = XmFileSelectionBoxGetChild(workingd_dialog, XmDIALOG_FILTER_LABEL);
	XtUnmanageChild(w);
	w = XmFileSelectionBoxGetChild(workingd_dialog, XmDIALOG_FILTER_TEXT);
	XtUnmanageChild(w);
	w = XmFileSelectionBoxGetChild(workingd_dialog, XmDIALOG_APPLY_BUTTON);
	XtUnmanageChild(w);

/* save the name of the text item used for definition */
	dir_item = XmFileSelectionBoxGetChild(workingd_dialog, XmDIALOG_TEXT);

/* Add a callback to the dir list */
	w = XmFileSelectionBoxGetChild(workingd_dialog, XmDIALOG_DIR_LIST);
	XtAddCallback(w, XmNsingleSelectionCallback, (XtCallbackProc) select_dir, (XtPointer) 0);
	XtVaSetValues(w, XmNselectionPolicy, XmSINGLE_SELECT, NULL);
    }
    xv_setstr(dir_item, get_workingdir());
    XtVaSetValues(workingd_dialog, XmNdirectory,
		  str = XmStringCreateLtoR(get_workingdir(), charset), NULL);
    XmFileSelectionDoSearch(workingd_dialog, NULL);
    XmStringFree(str);
    XtManageChild(workingd_dialog);
    XtRaise(XtParent(workingd_dialog));
    unset_wait_cursor();
}

#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)

#include "netcdf.h"

/*
 *
 * netcdf reader
 *
 */

extern int readcdf;		/* declared in main.c */

extern char netcdf_name[], xvar_name[], yvar_name[];

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
    int setno;
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
    setno = -1;
    strcpy(fname, xv_getstr(netcdf_file_item));
    if (XmListGetSelectedPos(netcdf_listx_item, &pos_list, &pos_cnt)) {
	XtVaGetValues(netcdf_listx_item,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	cs = XmStringCopy(*s);
	if (XmStringGetLtoR(cs, charset, &cstr)) {
	    strcpy(xvar, cstr);
	    XtFree(cstr);
	}
	XmStringFree(cs);
    } else {
	errwin("Need to select X, either variable name or INDEX");
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
	if (XmStringGetLtoR(cs, charset, &cstr)) {
	    strcpy(yvar, cstr);
	    XtFree(cstr);
	}
	XmStringFree(cs);
    } else {
	errwin("Need to select Y");
	unset_wait_cursor();
	return;
    }
    if (strcmp(xvar, "INDEX") == 0) {
	retval = readnetcdf(get_cg(), setno, fname, NULL, yvar, -1, -1, 1);
    } else {
	retval = readnetcdf(get_cg(), setno, fname, xvar, yvar, -1, -1, 1);
    }
    if (retval) {
	drawgraph();
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
	xms = XmStringCreateLtoR("INDEX", charset);
	XmListAddItemUnselected(netcdf_listx_item, xms, 0);
	XmStringFree(xms);

	if (strlen(fname) < 2) {
	    unset_wait_cursor();
	    return;
	}
	if ((cdfid = ncopen(fname, NC_NOWRITE)) == -1) {
	    errwin("Can't open file.");
	    unset_wait_cursor();
	    return;
	}
	ncinquire(cdfid, &ndims, &nvars, &ngatts, &recdim);
/*
    printf("%d %d %d %d\n", ndims, nvars, ngatts, recdim);
*/
	for (i = 0; i < ndims; i++) {
	    ncdiminq(cdfid, i, NULL, &dimlen[i]);
	}
	for (i = 0; i < nvars; i++) {
	    ncvarinq(cdfid, i, varname, &datatype, &ndims, dim, &natts);
	    if ((var_id = ncvarid(cdfid, varname)) == -1) {
		char ebuf[256];
		sprintf(ebuf, "update_netcdfs(): No such variable %s", varname);
		errwin(ebuf);
		continue;
	    }
	    if (ndims != 1) {
		continue;
	    }
	    ncdiminq(cdfid, dim[0], (char *) NULL, &len);
	    sprintf(buf, "%s", varname);
	    xms = XmStringCreateLtoR(buf, charset);
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

void create_netcdfs_popup(Widget w, XtPointer client_data, XtPointer call_data)
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
#ifdef HAVE_MFHDF
	top = XmCreateDialogShell(app_shell, "netCDF/HDF", NULL, 0);
#else
#ifdef HAVE_NETCDF
	top = XmCreateDialogShell(app_shell, "netCDF", NULL, 0);
#endif

#endif
	handle_close(top);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

/*
	form = XmCreateForm(dialog, "form", NULL, 0);
	form = XmCreateRowColumn(dialog, "form", NULL, 0);
	XtVaSetValues(form,
		      XmNpacking, XmPACK_COLUMN,
		      XmNnumColumns, 1,
		      XmNorientation, XmHORIZONTAL,
		      XmNisAligned, True,
		      XmNadjustLast, False,
		      XmNentryAlignment, XmALIGNMENT_END,
		      NULL);
*/

	XtSetArg(args[0], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);
	XtSetArg(args[1], XmNvisibleItemCount, 5);

	lab = XmCreateLabel(dialog, "Select set X:", NULL, 0);
	XtManageChild(lab);
	netcdf_listx_item = XmCreateScrolledList(dialog, "list", args, 2);
	XtManageChild(netcdf_listx_item);

	lab = XmCreateLabel(dialog, "Select set Y:", NULL, 0);
	XtManageChild(lab);
	netcdf_listy_item = XmCreateScrolledList(dialog, "list", args, 2);
	XtManageChild(netcdf_listy_item);

	netcdf_file_item = CreateTextItem2(dialog, 30, "netCDF file:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

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

	XtManageChild(dialog);
	netcdf_frame = top;
	if (strlen(netcdf_name)) {
	    xv_setstr(netcdf_file_item, netcdf_name);
	}
    }
    update_netcdfs();
    XtRaise(top);
    unset_wait_cursor();
}

static void do_netcdffile_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog = (Widget) client_data;
    char *s;
    XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
	errwin("Error converting XmString to char string");
	return;
    }
    xv_setstr(netcdf_file_item, s);
    XtFree(s);
    XtUnmanageChild(dialog);
    update_netcdfs();
}

void create_netcdffiles_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget top;
    XmString  dirmask;
    
    set_wait_cursor();
    if (top == NULL) {
	top = XmCreateFileSelectionDialog(app_shell, "netcdfs", NULL, 0);
	XtVaSetValues(XtParent(top), XmNtitle, "Select netCDF file", NULL);

	XtAddCallback(top, XmNokCallback, (XtCallbackProc) do_netcdffile_proc, (XtPointer) top);
	XtAddCallback(top, XmNcancelCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);
    }       
    XtRaise(top);

    dirmask = XmStringCreateSimple(get_workingdir());
    XmFileSelectionDoSearch(top, dirmask);
    XmStringFree(dirmask);
    
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

/*
 * TODO, lots of declared, but unused variables here
 */
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
	errwin("Can't open file.");
	unset_wait_cursor();
	return;
    }
    if (XmListGetSelectedPos(netcdf_listx_item, &pos_list, &pos_cnt)) {
	XtVaGetValues(netcdf_listx_item,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	cs = XmStringCopy(*s);
	if (XmStringGetLtoR(cs, charset, &cstr)) {
	    strcpy(xvar, cstr);
	    XtFree(cstr);
	}
	XmStringFree(cs);
    } else {
	errwin("Need to select X, either variable name or INDEX");
	goto out1;
    }
    if (XmListGetSelectedPos(netcdf_listy_item, &pos_list, &pos_cnt)) {
	XtVaGetValues(netcdf_listy_item,
		      XmNselectedItemCount, &cnt,
		      XmNselectedItems, &s,
		      NULL);
	cs = XmStringCopy(*s);
	if (XmStringGetLtoR(cs, charset, &cstr)) {
	    strcpy(yvar, cstr);
	    XtFree(cstr);
	}
	XmStringFree(cs);
    } else {
	errwin("Need to select Y");
	goto out1;
    }
    if (strcmp(xvar, "INDEX") == 0) {
	stufftext("X is the index of the Y variable\n", STUFF_START);
    } else {
	if ((x_id = ncvarid(cdfid, xvar)) == -1) {
	    char ebuf[256];
	    sprintf(ebuf, "do_query(): No such variable %s for X", xvar);
	    errwin(ebuf);
	    goto out1;
	}
	ncvarinq(cdfid, x_id, NULL, &xdatatype, &xndims, xdim, &xnatts);
	ncdiminq(cdfid, xdim[0], NULL, &nx);
	sprintf(buf, "X is %s, data type %s \t length [%ld]\n", xvar, getcdf_type(xdatatype), nx);
	stufftext(buf, STUFF_TEXT);
	sprintf(buf, "\t%d Attributes:\n", xnatts);
	stufftext(buf, STUFF_TEXT);
	for (i = 0; i < xnatts; i++) {
	    atcharval[0] = 0;
	    ncattname(cdfid, x_id, i, attname);
	    ncattinq(cdfid, x_id, attname, &datatype, &atlen);
	    switch (datatype) {
	    case NC_CHAR:
		ncattget(cdfid, x_id, attname, (void *) atcharval);
		atcharval[atlen] = 0;
		sprintf(buf, "\t\t%s: %s\n", attname, atcharval);
		stufftext(buf, STUFF_TEXT);
		break;
	    case NC_FLOAT:
		ncattget(cdfid, x_id, attname, (void *) &f);
		sprintf(buf, "\t\t%s: %f\n", attname, f);
		stufftext(buf, STUFF_TEXT);
		break;
	    case NC_DOUBLE:
		ncattget(cdfid, x_id, attname, (void *) &d);
		sprintf(buf, "\t\t%s: %f\n", attname, d);
		stufftext(buf, STUFF_TEXT);
		break;
	       default:
                break;
            }
	}
    }
    if ((y_id = ncvarid(cdfid, yvar)) == -1) {
	char ebuf[256];
	sprintf(ebuf, "do_query(): No such variable %s for Y", yvar);
	errwin(ebuf);
	goto out1;
    }
    ncvarinq(cdfid, y_id, NULL, &ydatatype, &yndims, ydim, &ynatts);
    ncdiminq(cdfid, ydim[0], NULL, &ny);
    sprintf(buf, "Y is %s, data type %s \t length [%ld]\n", yvar, getcdf_type(ydatatype), ny);
    stufftext(buf, STUFF_TEXT);
    sprintf(buf, "\t%d Attributes:\n", ynatts);
    stufftext(buf, STUFF_TEXT);
    for (i = 0; i < ynatts; i++) {
	atcharval[0] = 0;
	ncattname(cdfid, y_id, i, attname);
	ncattinq(cdfid, y_id, attname, &datatype, &atlen);
	switch (datatype) {
	case NC_CHAR:
	    ncattget(cdfid, y_id, attname, (void *) atcharval);
	    atcharval[atlen] = 0;
	    sprintf(buf, "\t\t%s: %s\n", attname, atcharval);
	    stufftext(buf, STUFF_TEXT);
	    break;
	case NC_FLOAT:
	    ncattget(cdfid, y_id, attname, (void *) &f);
	    sprintf(buf, "\t\t%s: %f\n", attname, f);
	    stufftext(buf, STUFF_TEXT);
	    break;
	case NC_DOUBLE:
	    ncattget(cdfid, y_id, attname, (void *) &d);
	    sprintf(buf, "\t\t%s: %f\n", attname, d);
	    stufftext(buf, STUFF_TEXT);
	    break;
          default:
            break;
	}
    }

  out1:;
    ncclose(cdfid);
    stufftext("\n", STUFF_STOP);
    unset_wait_cursor();
}

#endif

/*
 * Save the current project
 */
typedef struct _Save_ui {
    Widget top;
} Save_ui;

static Save_ui sui;

/*
 * TODO need to do err checking of the file name before
 * copying the name to docname
 */
static void save_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    int done_ok;
    
    Save_ui *ui = (Save_ui *) client_data;
    XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
	errwin("Error converting XmString to char string");
	return;
    }
    XtUnmanageChild(ui->top);
    set_wait_cursor();
    
    strcpy(sformat, (char *) xv_getstr(save_format_item));
    if (save_project(s) == GRACE_EXIT_SUCCESS) {
        done_ok = TRUE;
    } else {
        done_ok = FALSE;
    }
    
    if (done_ok) {
    	strcpy(docname, s);

    	clear_dirtystate();
    	set_title(mybasename(docname));
    	drawgraph();
    }
    XtFree(s);
    unset_wait_cursor();
}


void create_saveproject_popup(void)
{
    Widget fr, dialog;
    XmString dirmask;
    
    set_wait_cursor();

    if (sui.top == NULL) {
	sui.top = XmCreateFileSelectionDialog(app_shell, "sui.top", NULL, 0);
	XtVaSetValues(XtParent(sui.top), XmNtitle, "Save project", NULL);
	XtAddCallback(sui.top, XmNcancelCallback, (XtCallbackProc) destroy_dialog, sui.top);
	XtAddCallback(sui.top, XmNokCallback, save_proc, (XtPointer) & sui);
	XtAddCallback(sui.top, XmNhelpCallback, (XtCallbackProc) HelpCB, 
	              (XtPointer) "file.html#save");

	fr = XmCreateFrame(sui.top, "fr", NULL, 0);
	dialog = XmCreateRowColumn(fr, "dialog_rc", NULL, 0);

	save_format_item = CreateTextItem2(dialog, 15, "Format: ");

	XtManageChild(dialog);
	XtManageChild(fr);
    }
    XtManageChild(sui.top);
    XtRaise(XtParent(sui.top));

    xv_setstr(save_format_item, sformat);
    
    dirmask = XmStringCreateSimple(get_workingdir());
    XmFileSelectionDoSearch(sui.top, dirmask);
    XmStringFree(dirmask);

    unset_wait_cursor();
}

/*
 * Open a project
 */
typedef struct _Open_ui {
    Widget top;
} Open_ui;

static Open_ui oui;

static void open_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    int done_ok;
    
    XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
	errwin("Error converting XmString to char string");
	return;
    }
    
    if (wipeout()){
    	return;
    }
    set_wait_cursor();
    
    lock_dirtystate();
    
    if (getdata(get_cg(), s, cursource, SET_XY) == GRACE_EXIT_SUCCESS) {
    	done_ok = TRUE;
    } else {
    	done_ok = FALSE;
    }

    if (done_ok) {
    	strcpy(docname, s);
    	clear_dirtystate();
    	set_title(mybasename(docname));
    } else {
        set_dirtystate();
    }
    
    XtFree(s);
    unset_wait_cursor();
    update_all();
    drawgraph();
}


void create_openproject_popup(void)
{
    XmString dirmask;
    set_wait_cursor();

    if (oui.top == NULL) {
	oui.top = XmCreateFileSelectionDialog(app_shell, "oui.top", NULL, 0);
	XtVaSetValues(XtParent(oui.top), XmNtitle, "Open project", NULL);
	XtAddCallback(oui.top, XmNcancelCallback, (XtCallbackProc) destroy_dialog, oui.top);
	XtAddCallback(oui.top, XmNokCallback, open_proc, (XtPointer) & oui);
	XtAddCallback(oui.top, XmNhelpCallback, (XtCallbackProc) HelpCB, 
	              (XtPointer) "file.html#open");
    }
    XtManageChild(oui.top);
    XtRaise(XtParent(oui.top));

    dirmask = XmStringCreateSimple(get_workingdir());
    XmFileSelectionDoSearch(oui.top, dirmask);
    XmStringFree(dirmask);

    unset_wait_cursor();
}


/* Add comment capability to project file */

static Widget describe_frame, describe_panel;
static Widget text_w;
static XFontStruct *f;
static XmFontList xmf;
extern Display *disp;

void update_descript(Widget w, XtPointer client_data, XtPointer call_data)
{
	char *s;
	s = (char *)XmTextGetString( text_w );
	strcpy( description, s );
	XtFree( s );
}

void update_describe_popup (void)
{
    if (describe_frame != NULL && text_w != NULL) {
        XmTextSetString( text_w, description );
    }
}

void create_describe_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_wait_cursor();
    if (describe_frame == NULL) {
    	Widget buts[2];
    	char *label[2];
    	label[0] = "Accept";
    	label[1] = "Close";
		f = (XFontStruct *) XLoadQueryFont(disp, "fixed");
		xmf = XmFontListCreate(f, charset);
		describe_frame = XmCreateDialogShell(app_shell, "Description", NULL, 0);
		handle_close(describe_frame);
		describe_panel = XmCreateRowColumn(describe_frame, "describe_form", NULL, 0);
		text_w = XmCreateScrolledText(describe_panel, "text_w", NULL, 0);
		XtVaSetValues(text_w,
		      XmNrows, 10,
		      XmNcolumns, 80,
		      XmNeditMode, XmMULTI_LINE_EDIT,
		      XmNfontList, xmf,
		      XmNwordWrap, True,
		      NULL);
		XtManageChild(text_w);

		XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, describe_panel, 
															NULL);
		CreateCommandButtons(describe_panel, 2, buts, label);
		XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)
							update_descript, (XtPointer) text_w);
		XtAddCallback(buts[1], XmNactivateCallback, 
					(XtCallbackProc) destroy_dialog, (XtPointer) describe_frame);

		XtManageChild(describe_panel);
    }
    XmTextSetString( text_w, description );
    XtRaise(describe_frame);
    unset_wait_cursor();
}

typedef struct _Write_ui {
    Widget top;
    ListStructure *sel;
/*
 *     Widget *graph_item;
 *     Widget embed_item;
 * #if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
 *     Widget netcdf_item;
 * #endif
 */
    Widget format_item;
} Write_ui;

Write_ui wui;


/*
 *  write a set or sets to a file
 */
static void do_write_sets_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selset, cd, i;
    int gno, setno;
    char *fn;
    char format[32];
    FILE *cp;
    
    Write_ui *ui = (Write_ui *) client_data;
    XmFileSelectionBoxCallbackStruct *cbs =
        (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &fn)) {
        errwin("Error converting XmString to char string");
        return;
    }

    cp = grace_openw(fn);
    if (cp == NULL) {
        XtFree(fn);
        return;
    }

    cd = GetListChoices(ui->sel, &selset);
    if (cd < 1) {
        errwin("No set selected");
    } else {
        gno = get_cg();
        strncpy(format, xv_getstr(ui->format_item), 31);
        set_wait_cursor();
        for(i = 0; i < cd; i++) {
            setno = selset[i];
            write_set(gno, setno, cp, format, TRUE);
        }
        free(selset);
    }
    XtFree(fn);
    grace_close(cp);
    unset_wait_cursor();
}


void create_write_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;
    Widget fr;
    XmString dirmask;

    set_wait_cursor();
    if (wui.top == NULL) {
	wui.top = XmCreateFileSelectionDialog(app_shell, "write_sets", NULL, 0);
	XtVaSetValues(XtParent(wui.top), XmNtitle, "Write sets", NULL);

	XtAddCallback(wui.top, XmNokCallback, (XtCallbackProc) do_write_sets_proc, (XtPointer) & wui);
	XtAddCallback(wui.top, XmNcancelCallback, (XtCallbackProc) destroy_dialog, (XtPointer) wui.top);

	XtAddCallback(wui.top, XmNhelpCallback, (XtCallbackProc) HelpCB, 
	              (XtPointer) "file.html#writesets");
	
	fr = XmCreateFrame(wui.top, "fr", NULL, 0);
	dialog = XmCreateRowColumn(fr, "dialog_rc", NULL, 0);

        wui.sel = CreateSetChoice(dialog, "Write set(s):",
                                                LIST_TYPE_MULTIPLE, TRUE);
	wui.format_item = CreateTextItem2(dialog, 15, "Format: ");

	XtManageChild(dialog);
	XtManageChild(fr);
    }
    xv_setstr(wui.format_item, sformat);
    XtManageChild(wui.top);
    XtRaise(XtParent(wui.top));

    dirmask = XmStringCreateSimple(get_workingdir());
    XmFileSelectionDoSearch(wui.top, dirmask);
    XmStringFree(dirmask);

    unset_wait_cursor();
}


static void block_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char *s;
    XmFileSelectionBoxCallbackStruct *cbs =
        (XmFileSelectionBoxCallbackStruct *) call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
        errwin("Error converting XmString to char string");
        return;
    }
    if (getdata(get_cg(), s, cursource, SET_BLOCK) == GRACE_EXIT_SUCCESS) {
	if (blocklen == 0) {
	    errwin("Block data length = 0");
	} else if (blockncols == 0) {
	    errwin("Number of columns in block data = 0");
	} else {
	    XtUnmanageChild(block_dialog);
	    create_eblock_frame(get_cg());
	}
    }
    XtFree(s);
}

void create_block_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    Widget lab, rc, fr, rb, rw[5];
    XmString dirmask;

    set_wait_cursor();
    if (block_dialog == NULL) {
	block_dialog = XmCreateFileSelectionDialog(app_shell, "read_block_data", NULL, 0);
	XtVaSetValues(XtParent(block_dialog), XmNtitle, "Read block data", NULL);

	XtAddCallback(block_dialog, XmNcancelCallback, (XtCallbackProc) destroy_dialog, block_dialog);
	XtAddCallback(block_dialog, XmNokCallback, (XtCallbackProc) block_proc, 0);
	
	XtAddCallback(block_dialog, XmNhelpCallback, (XtCallbackProc) HelpCB, 
	              (XtPointer) "file.html#readblock");

	fr = XmCreateFrame(block_dialog, "frame", NULL, 0);

	rc = XmCreateRowColumn(fr, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);

	lab = XmCreateLabel(rc, "Data source:", NULL, 0);
	XtManageChild(lab);

	rb = XmCreateRadioBox(rc, "rb", NULL, 0);
	XtVaSetValues(rb, XmNorientation, XmHORIZONTAL, NULL);

	rw[0] = XmCreateToggleButton(rb, "Disk", NULL, 0);
	rw[1] = XmCreateToggleButton(rb, "Pipe", NULL, 0);
	for (i = 0; i < 2; i++) {
	    XtAddCallback(rw[i], XmNvalueChangedCallback, (XtCallbackProc) set_src_proc, (XtPointer) i);
	}

	XtManageChildren(rw, 2);
	XtManageChild(rb);
	XtManageChild(rc);
	XtManageChild(fr);
	XmToggleButtonSetState(rw[0], True, False);
    }
    XtManageChild(block_dialog);
    XtRaise(XtParent(block_dialog));

    dirmask = XmStringCreateSimple(get_workingdir());
    XmFileSelectionDoSearch(block_dialog, dirmask);
    XmStringFree(dirmask);

    unset_wait_cursor();
}
