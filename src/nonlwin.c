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
 * non linear curve fitting
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>

#include "globals.h"
#include "graphs.h"
#include "utils.h"
#include "files.h"
#include "plotone.h"
#include "protos.h"
#include "motifinc.h"

/* info strings
info = 0  improper input parameters.
info = 1  algorithm estimates that the relative error in the sum of squares is at most tol.
info = 2  algorithm estimates that the relative error between x and the solution is at most tol.
info = 3  conditions for info = 1 and info = 2 both hold.
info = 4  fvec is orthogonal to the columns of the jacobian to machine precision.
info = 5  number of calls to fcn has reached or exceeded the asked value (OK).
info = 6  tol is too small. no further reduction in the sum of squares is possible.
info = 7  tol is too small. no further improvement in the approximate solution x is possible.
*/

#define NONL_CANCEL 0
#define NONL_ACCEPT 1

static Widget nonl_frame = NULL;
static Widget nonl_panel;
Widget nonl_formula_item;
Widget nonl_title_item;
SetChoiceItem nonl_set_item;
static Widget nonl_load_item[3];
static Widget nonl_parm_item[MAXPARM];
static Widget nonl_value_item[MAXPARM];
static Widget nonl_constr_item[MAXPARM];
static Widget nonl_lowb_item[MAXPARM];
static Widget nonl_uppb_item[MAXPARM];
static Widget nonl_tol_item;
static Widget *nonl_nparm_item;
static Widget nonl_autol_item;
static Widget nonl_npts_item;
static Widget nonl_start_item, nonl_stop_item;
static Widget nonl_fload_rc;
static void do_nonl_proc(void *data);
static void do_nonl_toggle(int onoff, void *data);
static void do_constr_toggle(void *data);

static void update_nonl_frame_cb(void *data);
static void destroy_nonl_frame_cb(void *data);
static void reset_nonl_frame_cb(void *data);

void do_nparm_toggle(Widget w, XtPointer client_data, XtPointer call_data);
void create_openfit_popup(void *data);
void create_savefit_popup(void *data);
static int do_openfit_proc(char *filename, void *data);
static int do_savefit_proc(char *filename, void *data);

int nsteps;
int nlsetno;
int nlloadset = -1;

static char buf[256];

/* ARGSUSED */
void create_nonl_frame(void *data)
{
    int i;
    Widget sw, fr, title_fr, fr1, fr3, rc, rc1, rc2, rc3, lab, fitbut[4], but1[2];
    Widget menubar, menupane, submenupane;
    set_wait_cursor();
    if (nonl_frame == NULL) {
	char *fitlabel[4];
	char *blabel[2];

	fitlabel[0] = "Load as is";
	fitlabel[1] = "Run 5 steps";
	fitlabel[2] = "Run 20 steps";
	fitlabel[3] = "Run 100 steps";
	
	blabel[0] = "Accept";
	blabel[1] = "Cancel";

	nonl_frame = XmCreateDialogShell(app_shell, "Non-linear curve fitting", NULL, 0);
	handle_close(nonl_frame);
	nonl_panel = XmCreateForm(nonl_frame, "nonl_frame_rc", NULL, 0);

        menubar = CreateMenuBar(nonl_panel);
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Open...", 'O', create_openfit_popup, NULL);
        CreateMenuButton(menupane, "Save...", 'S', create_savefit_popup, NULL);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "Close", 'C', destroy_nonl_frame_cb, (void *) NONL_ACCEPT);

        menupane = CreateMenu(menubar, "Data", 'D', FALSE);

        CreateMenuButton(menupane, "Reset", 'R', reset_nonl_frame_cb, NULL);
        CreateMenuButton(menupane, "Update", 'U', update_nonl_frame_cb, NULL);

        menupane = CreateMenu(menubar, "Options", 'O', FALSE);
   
        submenupane = CreateMenu(menupane, "Load", 'L', FALSE);
    
        nonl_load_item[0] = CreateMenuToggle(submenupane, "Fitted values", 'v',
	    do_nonl_toggle, (XtPointer) 0);
        nonl_load_item[1] = CreateMenuToggle(submenupane, "Residuals", 'R',
	    do_nonl_toggle, (XtPointer) 1);
        nonl_load_item[2] = CreateMenuToggle(submenupane, "Function", 'F',
	    do_nonl_toggle, (XtPointer) 2);

        nonl_autol_item = CreateMenuToggle(menupane, "Autoload", 'A',
	    NULL, NULL);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);

        CreateMenuButton(menupane, "On fit", 'f', HelpCB, NULL);

        XtManageChild(menubar);
	XtVaSetValues(menubar,
		      XmNtopAttachment, XmATTACH_FORM,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      NULL);
        
        fr1 = CreateFrame(nonl_panel, NULL);
        nonl_set_item  = CreateSetSelector(fr1, "Apply to set:",
                                    SET_SELECT_ACTIVE,
                                    FILTER_SELECT_NONE,
                                    GRAPH_SELECT_CURRENT,
                                    SELECTION_TYPE_SINGLE);
	
	XtVaSetValues(fr1,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, menubar,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      NULL);
		      
	fr = CreateFrame(nonl_panel, NULL);
	rc = XmCreateRowColumn(fr, "nonl_rc", NULL, 0);
	
	title_fr = CreateFrame(rc, NULL);
	XtVaSetValues(title_fr, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
	nonl_title_item = XmCreateLabel(title_fr, nonl_opts.title, NULL, 0);
    	XtManageChild(nonl_title_item);
    	
	nonl_formula_item = CreateScrollTextItem2(rc, 2, "Formula:");
	rc1 = XmCreateRowColumn(rc, "nonl_rc", NULL, 0);
	XtVaSetValues(rc1, XmNorientation, XmHORIZONTAL, NULL);
	
	nonl_nparm_item = CreatePanelChoice(rc1,
				   "Number of parameters:",
				   12,
				   "0",
				   "1",
				   "2",
				   "3",
				   "4",
				   "5",
				   "6",
				   "7",
				   "8",
				   "9",
				  "10",
				   NULL, NULL);

	for (i = 0; i < MAXPARM + 1; i++) {
	    XtAddCallback(nonl_nparm_item[2 + i], XmNactivateCallback,
			(XtCallbackProc) do_nparm_toggle, (XtPointer) i);
	}
	
	nonl_tol_item = CreateTextItem2(rc1, 10, "Tolerance:");
	
	XtManageChild(rc1);
	
	sw = XtVaCreateManagedWidget("sw",
				     xmScrolledWindowWidgetClass, rc,
				     XmNheight, 180,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     NULL);

	rc2 = XmCreateRowColumn(sw, "rc2", NULL, 0);


	for (i = 0; i < MAXPARM; i++) {
	    nonl_parm_item[i] = XmCreateRowColumn(rc2, "rc1", NULL, 0);
	    XtVaSetValues(nonl_parm_item[i], XmNorientation, XmHORIZONTAL, NULL);
	    sprintf(buf, "A%1d: ", i);
	    nonl_value_item[i] = CreateTextItem2(nonl_parm_item[i], 10, buf);

	    nonl_constr_item[i] = XmCreateToggleButton(nonl_parm_item[i], "Bounds:", NULL, 0);
	    XtAddCallback(nonl_constr_item[i], XmNvalueChangedCallback, 
	    	    	    (XtCallbackProc) do_constr_toggle, (XtPointer) i);
	    XtManageChild(nonl_constr_item[i]);

	    nonl_lowb_item[i] = CreateTextItem2(nonl_parm_item[i], 6, "");
	    
	    sprintf(buf, "< A%1d < ", i);
	    lab = XmCreateLabel(nonl_parm_item[i], buf, NULL, 0);
    	    XtManageChild(lab);

	    nonl_uppb_item[i] = CreateTextItem2(nonl_parm_item[i], 6, "");
	}

	XtManageChild(rc2);
	XtManageChild(rc);

        XtVaSetValues(fr,
                      XmNtopAttachment, XmATTACH_WIDGET,
                      XmNtopWidget, fr1,
                      XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM,
                      NULL);
                      
	fr3 = CreateFrame(nonl_panel, NULL);
	rc3 = XmCreateRowColumn(fr3, "rc3", NULL, 0);
	
	CreateCommandButtons(rc3, 4, fitbut, fitlabel);
	AddButtonCB(fitbut[0], do_nonl_proc, (void *) 0);
	AddButtonCB(fitbut[1], do_nonl_proc, (void *) 5);
	AddButtonCB(fitbut[2], do_nonl_proc, (void *) 20);
	AddButtonCB(fitbut[3], do_nonl_proc, (void *) 100);
	
	CreateSeparator(rc3);	
	
	nonl_fload_rc = XmCreateRowColumn(rc3, "nonl_fload_rc", NULL, 0);
	XtVaSetValues(nonl_fload_rc, XmNorientation, XmHORIZONTAL, NULL);
	nonl_start_item = CreateTextItem2(nonl_fload_rc, 6, "Start load at:");
	nonl_stop_item = CreateTextItem2(nonl_fload_rc, 6, "Stop load at:");
	nonl_npts_item = CreateTextItem2(nonl_fload_rc, 4, "# of points:");
	XtManageChild(nonl_fload_rc);
	XtSetSensitive(nonl_fload_rc, False);

	CreateCommandButtons(rc3, 2, but1, blabel);
	AddButtonCB(but1[0], destroy_nonl_frame_cb, (void *) NONL_ACCEPT);
	AddButtonCB(but1[1], destroy_nonl_frame_cb, (void *) NONL_CANCEL);

	XtManageChild(rc3);
	XtVaSetValues(fr3,
	              XmNtopAttachment, XmATTACH_WIDGET,
                      XmNtopWidget, fr,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_FORM,
		      NULL);

	XtManageChild(nonl_panel);
    }
    update_nonl_frame();
    
    XtRaise(nonl_frame);
    
    unset_wait_cursor();
}

void do_nparm_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    int value = (int) client_data;
    for (i = 0; i < MAXPARM; i++) {
        if (i < value) {
            if (!XtIsManaged (nonl_parm_item[i])) {
                XtManageChild(nonl_parm_item[i]);
            }
        } else {
            if (XtIsManaged (nonl_parm_item[i])) {
                XtUnmanageChild(nonl_parm_item[i]);
            }
        }
    }
}

static void reset_nonl_frame_cb(void *data)
{
    reset_nonl();
    update_nonl_frame();
}

static void update_nonl_frame_cb(void *data)
{
    update_nonl_frame();
}

void update_nonl_frame(void)
{
    int i;
    
    if (nonl_frame) {
        XmString str = XmStringCreateLocalized(nonl_opts.title);
        XtVaSetValues(nonl_title_item, XmNlabelString, str, NULL);
/* 
 * If I define only XmALIGNMENT_CENTER (default!) then it's ignored - bug in Motif???
 */
    	XtVaSetValues(nonl_title_item, XmNalignment, XmALIGNMENT_BEGINNING, NULL);
        XtVaSetValues(nonl_title_item, XmNalignment, XmALIGNMENT_CENTER, NULL);
        XmStringFree(str);
        
        xv_setstr(nonl_formula_item, nonl_opts.formula);
        sprintf(buf, "%g", nonl_opts.tolerance);
        xv_setstr(nonl_tol_item, buf);
        SetChoice(nonl_nparm_item, nonl_opts.parnum);
        for (i = 0; i < MAXPARM; i++) {
            sprintf(buf, "%g", nonl_parms[i].value);
            xv_setstr(nonl_value_item[i], buf);
            XmToggleButtonSetState(nonl_constr_item[i], nonl_parms[i].constr, False);
            sprintf(buf, "%g", nonl_parms[i].min);
            xv_setstr(nonl_lowb_item[i], buf);
            XtSetSensitive(nonl_lowb_item[i], nonl_parms[i].constr);
            sprintf(buf, "%g", nonl_parms[i].max);
            xv_setstr(nonl_uppb_item[i], buf);
            XtSetSensitive(nonl_uppb_item[i], nonl_parms[i].constr);
            if (i < nonl_opts.parnum) {
                if (!XtIsManaged (nonl_parm_item[i])) {
                    XtManageChild(nonl_parm_item[i]);
                }
            } else {
                if (XtIsManaged (nonl_parm_item[i])) {
                    XtUnmanageChild(nonl_parm_item[i]);
                }
            }
        }
        
        XmToggleButtonSetState(nonl_autol_item, nonl_prefs.autoload, False);
        for (i = 0; i < 3; i++) {
	    XmToggleButtonSetState(nonl_load_item[i], False, False);
        }
        XmToggleButtonSetState(nonl_load_item[nonl_prefs.load], True, False);
        
        if (nonl_prefs.load == LOAD_FUNCTION) {
            XtSetSensitive(nonl_fload_rc, True);
        } else {
            XtSetSensitive(nonl_fload_rc, False);
        }
        
        sprintf(buf, "%g", nonl_prefs.start);
        xv_setstr(nonl_start_item, buf);
        sprintf(buf, "%g", nonl_prefs.stop);
        xv_setstr(nonl_stop_item, buf);
        sprintf(buf, "%d", nonl_prefs.npoints);
        xv_setstr(nonl_npts_item, buf);
    }

}

static void do_nonl_toggle(int onoff, void *data)
{
    
    int i;
    int value = (int) data;
    for (i = 0; i < 3; i++) {
	if (i != value) {
            XmToggleButtonSetState(nonl_load_item[i], False, False);
	} else {
            XmToggleButtonSetState(nonl_load_item[i], True, False);
	}
    }
    if (value == LOAD_FUNCTION) {
    	XtSetSensitive(nonl_fload_rc, True);
    } else {
    	XtSetSensitive(nonl_fload_rc, False);
    }
}

static void do_constr_toggle(void *data)
{
    int value = (int) data;
    if (XmToggleButtonGetState(nonl_constr_item[value])) {
    	XtSetSensitive(nonl_lowb_item[value], True);
    	XtSetSensitive(nonl_uppb_item[value], True);
    	nonl_parms[value].constr = TRUE;
    } else {
    	XtSetSensitive(nonl_lowb_item[value], False);
    	XtSetSensitive(nonl_uppb_item[value], False);
    	nonl_parms[value].constr = FALSE;
    }
}

/* ARGSUSED */
static void do_nonl_proc(void *data)
{
    int i, npts = 0, info;
    double delx, *xfit, *y, *yfit;
    int nsteps = (int) data;
    int cg = get_cg();
    
    set_wait_cursor();
    nlsetno = GetSelectedSet(nonl_set_item);
    if (nlsetno == SET_SELECT_ERROR) {
    	errmsg("No set selected");
    	unset_wait_cursor();
    	return;
    }
    
    nonl_opts.tolerance = atof((char *) xv_getstr(nonl_tol_item));
    nonl_opts.parnum = GetChoice(nonl_nparm_item);
    strcpy(nonl_opts.formula, (char *) xv_getstr(nonl_formula_item));
    for (i = 0; i < nonl_opts.parnum; i++) {
	strcpy(buf, (char *) xv_getstr(nonl_value_item[i]));
	if (sscanf(buf, "%lf", &nonl_parms[i].value) != 1) {
	    errmsg("Invalid input in parameter field");
	    unset_wait_cursor();
	    return;
	}
	
	nonl_parms[i].constr = XmToggleButtonGetState(nonl_constr_item[i]);
	if (nonl_parms[i].constr) {
	    strcpy(buf, (char *) xv_getstr(nonl_lowb_item[i]));
	    if (sscanf(buf, "%lf", &nonl_parms[i].min) != 1) {
	    	errmsg("Invalid input in low-bound field");
	    	unset_wait_cursor();
	    	return;
	    }
	    strcpy(buf, (char *) xv_getstr(nonl_uppb_item[i]));
	    if (sscanf(buf, "%lf", &nonl_parms[i].max) != 1) {
	    	errmsg("Invalid input in upper-bound field");
	    	unset_wait_cursor();
	    	return;
	    }
	    if ((nonl_parms[i].value < nonl_parms[i].min) || (nonl_parms[i].value > nonl_parms[i].max)) {
	    	errmsg("Initial values must be within bounds");
	    	unset_wait_cursor();
	    	return;
	    }
	}
    }
    
    nonl_prefs.autoload = XmToggleButtonGetState(nonl_autol_item);
    for (i = 0; i < 3; i++) {
        if (XmToggleButtonGetState(nonl_load_item[i])) {
            nonl_prefs.load = i;
            break;
        }
    }
    
    if (nonl_prefs.load == LOAD_FUNCTION) {
		if( xv_evalexpr(nonl_start_item, &nonl_prefs.start) != 
													GRACE_EXIT_SUCCESS) {
			errmsg("Invalid input in start field");
	    	unset_wait_cursor();
	    	return;
		}
		if( xv_evalexpr(nonl_stop_item, &nonl_prefs.stop) != 
													GRACE_EXIT_SUCCESS) {
			errmsg("Invalid input in start field");
	    	unset_wait_cursor();
	    	return;
		}
		if( xv_evalexpri(nonl_npts_item, &nonl_prefs.npoints) != 
													GRACE_EXIT_SUCCESS) {
			errmsg("Invalid input in start field");
	    	unset_wait_cursor();
	    	return;
		}
    }
    
    if (nsteps) { /* we are asked to fit */
    	sprintf(buf, "Fitting with formula: %s\n", nonl_opts.formula);
    	stufftext(buf, 0);
    	sprintf(buf, "Initial guesses:\n");
    	stufftext(buf, 0);
    	for (i = 0; i < nonl_opts.parnum; i++) {
	    sprintf(buf, "\ta%1d = %g\n", i, nonl_parms[i].value);
	    stufftext(buf, 0);
    	}
    	sprintf(buf, "Tolerance = %g\n", nonl_opts.tolerance);
    	stufftext(buf, 0);

/*
 * The fit itself!
 */    	

    	info = do_nonlfit(cg, nlsetno, nsteps);
    	if (info == -1) {
	    errmsg("Fatal error in do_nonlfit()");  
	    unset_wait_cursor();
	    return;  	
    	}
   	    	
    	for (i = 0; i < nonl_opts.parnum; i++) {
	    sprintf(buf, "%g", nonl_parms[i].value);
	    xv_setstr(nonl_value_item[i], buf);
    	}

    	if ((info > 0 && info < 4) || (info == 5)) {
	    sprintf(buf, "Computed values:\n");
	    stufftext(buf, 0);
	    for (i = 0; i < nonl_opts.parnum; i++) {
		sprintf(buf, "\ta%1d = %g\n", i, nonl_parms[i].value);
		stufftext(buf, 0);
	    }
    	}


    	if (info >= 0 && info <= 7) {
    	    char *s;
    	    switch (info) {
    	    case 0:
    		s = "Improper input parameters.\n";
    		break;
    	    case 1:
    		s = "Relative error in the sum of squares is at most tol.\n";
    		break;
    	    case 2:
    		s = "Relative error between A and the solution is at most tol.\n";
    		break;
    	    case 3:
    		s = "Relative error in the sum of squares and A and the solution is at most tol.\n";
    		break;
    	    case 4:
    		s = "Fvec is orthogonal to the columns of the jacobian to machine precision.\n";
    		break;
    	    case 5:
    		s = "\n";
    		break;
    	    case 6:
    		s = "Tol is too small. No further reduction in the sum of squares is possible.\n";
    		break;
    	    case 7:
    		s = "Tol is too small. No further improvement in the approximate solution A is possible.\n";
    		break;
    	    default:
    		s = "\n";
    		errmsg("Internal error in do_nonl_proc(), please report");
    		break;
    	    }
    	    stufftext(s, 0);
    	    stufftext("\n", 0);
    	}
    } /* endif (nsteps) */

/*
 * Select & activate a set to load results to
 */    
    if (!nsteps || nonl_prefs.autoload) {
    	/* check if the set is already allocated */
    	if ((nlloadset == -1) || (nlloadset == nlsetno) || !getsetlength(cg, nlloadset)) {
    	    nlloadset = nextset(cg);
    	    if (nlloadset == -1) {
    	      errmsg("No more sets!");
    	      unset_wait_cursor();
    	      return;
    	    } else {
    		activateset(cg, nlloadset);
    		setlength(cg, nlloadset, 1);
    	    }
    	}
    	    	
    	switch (nonl_prefs.load) {
    	case LOAD_VALUES:
    	  sprintf(buf, "Evaluating fitted values and loading result to set %d:\n", nlloadset);
    	  stufftext(buf, 0);
    	  npts = getsetlength(cg, nlsetno);
    	  setlength(cg, nlloadset, npts);
    	  copycol2(cg, nlsetno, cg, nlloadset, 0);
    	  break;
    	case LOAD_RESIDUALS:
    	  sprintf(buf, "Evaluating fitted values and loading residuals to set %d:\n", nlloadset);
    	  stufftext(buf, 0);
    	  npts = getsetlength(cg, nlsetno);
    	  setlength(cg, nlloadset, npts);
    	  copycol2(cg, nlsetno, cg, nlloadset, 0);
    	  break;
    	case LOAD_FUNCTION:
    	  sprintf(buf, "Computing fitting function and loading result to set %d:\n", nlloadset);
    	  stufftext(buf, 0);
    	  
    	  npts  = nonl_prefs.npoints;
    	  if (npts <= 1) {
    	      errmsg("Number of points must be > 1");
    	      unset_wait_cursor();
    	      return;
    	  }
    	  
    	  setlength(cg, nlloadset, npts);
    	  
    	  delx = (nonl_prefs.stop - nonl_prefs.start)/(npts - 1);
    	  xfit = getx(cg, nlloadset);
	  for (i = 0; i < npts; i++) {
	      xfit[i] = nonl_prefs.start + i * delx;
	  }
    	  break;
    	}
    	
    	setcomment(cg, nlloadset, nonl_opts.formula);
    	
    	do_compute(cg, nlloadset, cg, nlloadset, NULL, nonl_opts.formula);
    	
    	if (nonl_prefs.load == LOAD_RESIDUALS) { /* load residuals */
    	    y = gety(cg, nlsetno);
    	    yfit = gety(cg, nlloadset);
    	    for (i = 0; i < npts; i++) {
	      yfit[i] -= y[i];
	    }
    	}
    	
    	update_set_lists(cg);
    	drawgraph();
    }
    unset_wait_cursor();
}

static void destroy_nonl_frame_cb(void *data)
{
    int value = (int) data;
    
    if (value == NONL_CANCEL) {
    	if (nlloadset != -1) {
	    killset(get_cg(), nlloadset);
    	}
	update_all();
    	drawgraph();
    }
    
    nlloadset = -1;
    XtUnmanageChild(nonl_frame);
}

void create_openfit_popup(void *data)
{
    static FSBStructure *fsb = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Open fit parameter file", "*.fit");
	AddFileSelectionBoxCB(fsb, do_openfit_proc, NULL);
        XtManageChild(fsb->FSB);
    }
    
    XtRaise(fsb->dialog);

    unset_wait_cursor();
}

static int do_openfit_proc(char *filename, void *data)
{
    reset_nonl();
    getparms(filename);
    update_nonl_frame();
    
    return FALSE;
}


void create_savefit_popup(void *data)
{
    static FSBStructure *fsb = NULL;
    static Widget title_item = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        Widget fr;
        
        fsb = CreateFileSelectionBox(app_shell, "Save fit parameter file", "*.fit");
	fr = CreateFrame(fsb->rc, NULL);
	title_item = CreateTextItem2(fr, 25, "Title: ");
	AddFileSelectionBoxCB(fsb, do_savefit_proc, (void *) title_item);
        XtManageChild(fsb->FSB);
    }
    
    xv_setstr(title_item, nonl_opts.title);
    
    XtRaise(fsb->dialog);

    unset_wait_cursor();
}

static int do_savefit_proc(char *filename, void *data)
{
    FILE *pp;
    Widget title_item = (Widget) data;
    
    pp = grace_openw(filename);
    if (pp != NULL) {
        strcpy(nonl_opts.title, (char *) xv_getstr(title_item));
        put_fitparms(pp, 0);
        grace_close(pp);
    }
    return TRUE;
}
