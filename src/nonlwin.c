/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
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
#include <Xm/ScrolledW.h>

#include "globals.h"
#include "defines.h"
#include "graphs.h"
#include "utils.h"
#include "files.h"
#include "parser.h"
#include "motifinc.h"
#include "protos.h"

/* nonlprefs.load possible values */
#define LOAD_VALUES         0
#define LOAD_RESIDUALS      1
#define LOAD_FUNCTION       2

#define  WEIGHT_NONE    0
#define  WEIGHT_Y       1
#define  WEIGHT_Y2      2
#define  WEIGHT_DY      3
#define  WEIGHT_CUSTOM  4

/* prefs for non-linear fit */
typedef struct {
    int autoload;       /* do autoload */
    int load;           /* load to... */
    int npoints;        /* # of points to evaluate function at */
    double start;       /* start... */
    double stop;        /* stop ... */
} nonlprefs;

typedef struct {
    TransformStructure *tdialog;
    TextStructure *formula_item;
    Widget title_item;
    Widget parm_item[MAXPARM];
    Widget value_item[MAXPARM];
    Widget constr_item[MAXPARM];
    Widget lowb_item[MAXPARM];
    Widget uppb_item[MAXPARM];
    Widget tol_item;
    OptionStructure *nparm_item;
    SpinStructure *nsteps_item;
    OptionStructure *load_item;
    Widget autol_item;
    Widget npts_item;
    Widget start_item, stop_item;
    Widget fload_rc;
    RestrictionStructure *restr_item;
    OptionStructure *weigh_item;
    Widget wfunc_item;
    
    NLFit nlfit;
} NonL_ui;

static nonlprefs nonl_prefs = {TRUE, LOAD_VALUES, 10, 0.0, 1.0};

static int do_nonl_proc(void *data);
static void do_nonl_toggle(int onoff, void *data);
static void nonl_wf_cb(int value, void *data);
static void do_constr_toggle(int onoff, void *data);

static void update_nonl_frame(NonL_ui *ui);

static void update_nonl_frame_cb(void *data);
static void reset_frame_cb(void *data);

static void do_nparm_toggle(int value, void *data);
static void create_openfit_popup(void *data);
static void create_savefit_popup(void *data);
static int do_openfit_proc(char *filename, void *data);
static int do_savefit_proc(char *filename, void *data);


/* ARGSUSED */
void create_nonl_frame(void *data)
{
    static NonL_ui *ui = NULL;
    
    set_wait_cursor();
    
    if (!ui) {
        int i;
        char buf[256];
        OptionItem np_option_items[MAXPARM + 1], option_items[5];
        Widget frame, menubar, menupane;
        Widget nonl_tab, nonl_main, nonl_advanced;
        Widget sw, title_fr, fr3, rc1, rc2, rc3, lab;
        
        ui = xmalloc(sizeof(NonL_ui));
        memset(ui, 0, sizeof(NonL_ui));

        ui->nlfit.title   = NULL;
        ui->nlfit.formula = NULL;
        reset_nonl(&ui->nlfit);

	ui->tdialog = CreateTransformDialogForm(app_shell,
            "Non-linear curve fitting", LIST_TYPE_SINGLE);
        frame = ui->tdialog->form;

        menubar = ui->tdialog->menubar;
        
        menupane = CreateMenu(menubar, "File", 'F', FALSE);
        CreateMenuButton(menupane, "Open...", 'O', create_openfit_popup, (void *) ui);
        CreateMenuButton(menupane, "Save...", 'S', create_savefit_popup, (void *) ui);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "Close", 'C', destroy_dialog_cb, GetParent(frame));

        menupane = CreateMenu(menubar, "Edit", 'E', FALSE);

        CreateMenuButton(menupane, "Reset fit parameters", 'R', reset_frame_cb, NULL);

        menupane = CreateMenu(menubar, "View", 'V', FALSE);
   
        ui->autol_item = CreateMenuToggle(menupane, "Autoload", 'A',
	    NULL, NULL);
        CreateMenuSeparator(menupane);
        CreateMenuButton(menupane, "Update", 'U', update_nonl_frame_cb, NULL);

        menupane = CreateMenu(menubar, "Help", 'H', TRUE);

        CreateMenuHelpButton(menupane, "On fit", 'f',
            frame, "doc/UsersGuide.html#non-linear-fit");

        ManageChild(menubar);
        
	title_fr = CreateFrame(frame, NULL);
	XtVaSetValues(title_fr, XmNshadowType, XmSHADOW_ETCHED_OUT, NULL);
	ui->title_item = CreateLabel(title_fr, NULL);
        AddDialogFormChild(frame, title_fr);

        /* ------------ Tabs --------------*/

        nonl_tab = CreateTab(frame);        


        /* ------------ Main tab --------------*/
        
        nonl_main = CreateTabPage(nonl_tab, "Main");
    	
	ui->formula_item = CreateTextInput(nonl_main, "Formula:");
	rc1 = CreateHContainer(nonl_main);
	
	for (i = 0; i < MAXPARM + 1; i++) {
	    np_option_items[i].value = i;
            sprintf(buf, "%d", i);
	    np_option_items[i].label = copy_string(NULL, buf);
        }
	ui->nparm_item = CreateOptionChoice(rc1,
            "Parameters:", 1, MAXPARM + 1, np_option_items);
        AddOptionChoiceCB(ui->nparm_item, do_nparm_toggle, ui);
        
	ui->tol_item = CreateTextItem2(rc1, 8, "Tolerance:");
        
	ui->nsteps_item = CreateSpinChoice(rc1, "Iterations:", 3,
            SPIN_TYPE_INT, 0.0, 500.0, 5.0);
	SetSpinChoice(ui->nsteps_item, 5.0);
        
	sw = XtVaCreateManagedWidget("sw",
				     xmScrolledWindowWidgetClass, nonl_main,
				     XmNheight, 180,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     NULL);

	rc2 = CreateVContainer(sw);
	for (i = 0; i < MAXPARM; i++) {
	    ui->parm_item[i] = CreateHContainer(rc2);
	    sprintf(buf, "A%1d: ", i);
	    ui->value_item[i] = CreateTextItem2(ui->parm_item[i], 10, buf);

	    ui->constr_item[i] = CreateToggleButton(ui->parm_item[i], "Bounds:");
	    AddToggleButtonCB(ui->constr_item[i], do_constr_toggle, (void *) i);

	    ui->lowb_item[i] = CreateTextItem2(ui->parm_item[i], 6, "");
	    
	    sprintf(buf, "< A%1d < ", i);
	    lab = CreateLabel(ui->parm_item[i], buf);

	    ui->uppb_item[i] = CreateTextItem2(ui->parm_item[i], 6, "");
	}

        /* ------------ Advanced tab --------------*/

        nonl_advanced = CreateTabPage(nonl_tab, "Advanced");

	ui->restr_item =
            CreateRestrictionChoice(nonl_advanced, "Source data filtering");

	fr3 = CreateFrame(nonl_advanced, "Weighting");
        rc3 = CreateHContainer(fr3);
        option_items[0].value = WEIGHT_NONE;
        option_items[0].label = "None";
        option_items[1].value = WEIGHT_Y;
        option_items[1].label = "1/Y";
        option_items[2].value = WEIGHT_Y2;
        option_items[2].label = "1/Y^2";
        option_items[3].value = WEIGHT_DY;
        option_items[3].label = "1/dY^2";
        option_items[4].value = WEIGHT_CUSTOM;
        option_items[4].label = "Custom";
	ui->weigh_item = CreateOptionChoice(rc3, "Weights", 1, 5, option_items);
	ui->wfunc_item = CreateTextItem2(rc3, 30, "Function:");
	AddOptionChoiceCB(ui->weigh_item, nonl_wf_cb, (void *) ui->wfunc_item);

	fr3 = CreateFrame(nonl_advanced, "Load options");
        rc3 = CreateVContainer(fr3);
        option_items[0].value = LOAD_VALUES;
        option_items[0].label = "Fitted values";
        option_items[1].value = LOAD_RESIDUALS;
        option_items[1].label = "Residuals";
        option_items[2].value = LOAD_FUNCTION;
        option_items[2].label = "Function";
	ui->load_item = CreateOptionChoice(rc3, "Load", 1, 3, option_items);
	ui->fload_rc = CreateHContainer(rc3);
	ui->start_item = CreateTextItem2(ui->fload_rc, 6, "Start load at:");
	ui->stop_item = CreateTextItem2(ui->fload_rc, 6, "Stop load at:");
	ui->npts_item = CreateTextItem2(ui->fload_rc, 4, "# of points:");
        AddOptionChoiceCB(ui->load_item, do_nonl_toggle, (void *) ui->fload_rc);

	CreateAACDialog(frame, nonl_tab, do_nonl_proc, (void *) ui);
    }
    
    update_nonl_frame(ui);
    
    RaiseWindow(GetParent(ui->tdialog->form));
    
    unset_wait_cursor();
}

static void do_nparm_toggle(int value, void *data)
{
    NonL_ui *ui = (NonL_ui *) data;
    int i;
    for (i = 0; i < MAXPARM; i++) {
        if (i < value) {
            ManageChild(ui->parm_item[i]);
        } else {
            UnmanageChild(ui->parm_item[i]);
        }
    }
}

static void reset_frame_cb(void *data)
{
    NonL_ui *ui = (NonL_ui *) data;
    reset_nonl(&ui->nlfit);
    update_nonl_frame(ui);
}

static void update_nonl_frame_cb(void *data)
{
    NonL_ui *ui = (NonL_ui *) data;
    update_nonl_frame(ui);
}

static void update_nonl_frame(NonL_ui *ui)
{
    int i;
    
    if (ui) {
        char buf[256];
        NLFit *nlfit = &ui->nlfit;
        SetLabel(ui->title_item, nlfit->title);
/* 
 * If I define only XmALIGNMENT_CENTER (default!) then it's ignored - bug in Motif???
 */
    	XtVaSetValues(ui->title_item, XmNalignment, XmALIGNMENT_BEGINNING, NULL);
        XtVaSetValues(ui->title_item, XmNalignment, XmALIGNMENT_CENTER, NULL);
        
        SetTextString(ui->formula_item, nlfit->formula);
        sprintf(buf, "%g", nlfit->tolerance);
        xv_setstr(ui->tol_item, buf);
        SetOptionChoice(ui->nparm_item, nlfit->parnum);
        for (i = 0; i < MAXPARM; i++) {
            nonlparm *nlp = &nlfit->parms[i];
            sprintf(buf, "%g", nlp->value);
            xv_setstr(ui->value_item[i], buf);
            SetToggleButtonState(ui->constr_item[i], nlp->constr);
            sprintf(buf, "%g", nlp->min);
            xv_setstr(ui->lowb_item[i], buf);
            SetSensitive(ui->lowb_item[i], nlp->constr);
            sprintf(buf, "%g", nlp->max);
            xv_setstr(ui->uppb_item[i], buf);
            SetSensitive(ui->uppb_item[i], nlp->constr);
            if (i < nlfit->parnum) {
                if (!XtIsManaged (ui->parm_item[i])) {
                    ManageChild(ui->parm_item[i]);
                }
            } else {
                if (XtIsManaged (ui->parm_item[i])) {
                    UnmanageChild(ui->parm_item[i]);
                }
            }
        }
        
        SetToggleButtonState(ui->autol_item, nonl_prefs.autoload);
        SetOptionChoice(ui->load_item, nonl_prefs.load);
        
        if (nonl_prefs.load == LOAD_FUNCTION) {
            SetSensitive(ui->fload_rc, True);
        } else {
            SetSensitive(ui->fload_rc, False);
        }

        if (GetOptionChoice(ui->weigh_item) == WEIGHT_CUSTOM) {
            SetSensitive(GetParent(ui->wfunc_item), True);
        } else {
            SetSensitive(GetParent(ui->wfunc_item), False);
        }
        
        sprintf(buf, "%g", nonl_prefs.start);
        xv_setstr(ui->start_item, buf);
        sprintf(buf, "%g", nonl_prefs.stop);
        xv_setstr(ui->stop_item, buf);
        sprintf(buf, "%d", nonl_prefs.npoints);
        xv_setstr(ui->npts_item, buf);
    }
}

static int do_nonl_proc(void *data)
{
    NonL_ui *ui = (NonL_ui *) data;
    NLFit *nlfit = &ui->nlfit;
    int i;
    int nsteps;
    int resno;
    char *fstr;
    int nlen, wlen;
    int weight_method;
    double *ytmp, *warray;
    int restr_type, restr_negate;
    char *rarray;
    int nssrc;
    Quark *psrc, *pdest, **srcsets, **destsets;
    char buf[256];
    
    if (GetTransformDialogSettings(ui->tdialog, TRUE, &nssrc, &srcsets, &destsets)
        != RETURN_SUCCESS) {
    	return RETURN_FAILURE;
    }
    
    psrc  = srcsets[0];
    pdest = destsets[0];
    
    nlfit->formula = copy_string(nlfit->formula, GetTextString(ui->formula_item));
    nsteps = (int) GetSpinChoice(ui->nsteps_item);
    nlfit->tolerance = atof(xv_getstr(ui->tol_item));
    
    nlfit->parnum = GetOptionChoice(ui->nparm_item);
    for (i = 0; i < nlfit->parnum; i++) {
	nonlparm *nlp = &nlfit->parms[i];
        strcpy(buf, xv_getstr(ui->value_item[i]));
	if (sscanf(buf, "%lf", &nlp->value) != 1) {
	    errmsg("Invalid input in parameter field");
	    return RETURN_FAILURE;
	}
	
	nlp->constr = GetToggleButtonState(ui->constr_item[i]);
	if (nlp->constr) {
	    strcpy(buf, xv_getstr(ui->lowb_item[i]));
	    if (sscanf(buf, "%lf", &nlp->min) != 1) {
	    	errmsg("Invalid input in low-bound field");
	    	return RETURN_FAILURE;
	    }
	    strcpy(buf, xv_getstr(ui->uppb_item[i]));
	    if (sscanf(buf, "%lf", &nlp->max) != 1) {
	    	errmsg("Invalid input in upper-bound field");
	    	return RETURN_FAILURE;
	    }
	    if ((nlp->value < nlp->min) || (nlp->value > nlp->max)) {
	    	errmsg("Initial values must be within bounds");
	    	return RETURN_FAILURE;
	    }
	}
    }

    nonl_prefs.autoload = GetToggleButtonState(ui->autol_item);
    nonl_prefs.load = GetOptionChoice(ui->load_item);
    
    if (nonl_prefs.load == LOAD_FUNCTION) {
	if (xv_evalexpr(ui->start_item, &nonl_prefs.start) != RETURN_SUCCESS) {
	    errmsg("Invalid input in start field");
	    return RETURN_FAILURE;
	}
	if (xv_evalexpr(ui->stop_item, &nonl_prefs.stop) != RETURN_SUCCESS) {
	    errmsg("Invalid input in stop field");
	    return RETURN_FAILURE;
	}
	if (xv_evalexpri(ui->npts_item, &nonl_prefs.npoints) != RETURN_SUCCESS) {
	    errmsg("Invalid input in npoints field");
	    return RETURN_FAILURE;
	}
    	if (nonl_prefs.npoints <= 1) {
    	    errmsg("Number of points must be > 1");
	    return RETURN_FAILURE;
    	}
    }
    
    
    for (i = 0; i < nlfit->parnum; i++) {
        nonlparm *nlp = &nlfit->parms[i];
        double *var;
        
        var = define_parser_scalar(nlp->name);
        if (var) {
            *var = nlp->value;
        }
    }
    
    if (nsteps) {
        /* apply weigh function */
    	nlen = getsetlength(psrc);
	weight_method = GetOptionChoice(ui->weigh_item);
        switch (weight_method) {
        case WEIGHT_Y:
        case WEIGHT_Y2:
            ytmp = getcol(psrc, DATA_Y);
            for (i = 0; i < nlen; i++) {
                if (ytmp[i] == 0.0) {
	            errmsg("Divide by zero while calculating weights");
                    return RETURN_FAILURE;
                }
            }
            warray = xmalloc(nlen*SIZEOF_DOUBLE);
            if (warray == NULL) {
	        errmsg("xmalloc failed in do_nonl_proc()");
                return RETURN_FAILURE;
            }
            for (i = 0; i < nlen; i++) {
                if (weight_method == WEIGHT_Y) {
                    warray[i] = 1/ytmp[i];
                } else {
                    warray[i] = 1/(ytmp[i]*ytmp[i]);
                }
            }
            break;
        case WEIGHT_DY:
            ytmp = getcol(psrc, DATA_Y1);
            if (ytmp == NULL) {
	        errmsg("The set doesn't have dY data column");
                return RETURN_FAILURE;
            }
            for (i = 0; i < nlen; i++) {
                if (ytmp[i] == 0.0) {
	            errmsg("Divide by zero while calculating weights");
                    return RETURN_FAILURE;
                }
            }
            warray = xmalloc(nlen*SIZEOF_DOUBLE);
            if (warray == NULL) {
	        errmsg("xmalloc failed in do_nonl_proc()");
            }
            for (i = 0; i < nlen; i++) {
                warray[i] = 1/(ytmp[i]*ytmp[i]);
            }
            break;
        case WEIGHT_CUSTOM:
            if (set_parser_setno(psrc) != RETURN_SUCCESS) {
                errmsg("Bad set");
                return RETURN_FAILURE;
            }
            
            fstr = xv_getstr(ui->wfunc_item);
            if (v_scanner(fstr, &wlen, &warray) != RETURN_SUCCESS) {
                errmsg("Error evaluating expression for weights");
                return RETURN_FAILURE;
            }
            if (wlen != nlen) {
                errmsg("The array of weights has different length");
                xfree(warray);
                return RETURN_FAILURE;
            }
            break;
        default:
            warray = NULL;
            break;
        }

        /* apply restriction */
        restr_type = GetOptionChoice(ui->restr_item->r_sel);
        restr_negate = GetToggleButtonState(ui->restr_item->negate);
        resno = get_restriction_array(psrc,
            restr_type, restr_negate, &rarray);
	if (resno != RETURN_SUCCESS) {
	    errmsg("Error in restriction evaluation");
	    xfree(warray);
            return RETURN_FAILURE;
	}

        /* The fit itself! */
    	resno = do_nonlfit(psrc, nlfit, warray, rarray, nsteps);
	xfree(warray);
	xfree(rarray);
    	if (resno != RETURN_SUCCESS) {
	    errmsg("Fatal error in do_nonlfit()");  
	    return RETURN_FAILURE;  	
    	}
   	    	
    	for (i = 0; i < nlfit->parnum; i++) {
	    sprintf(buf, "%g", nlfit->parms[i].value);
	    xv_setstr(ui->value_item[i], buf);
    	}
    }

/*
 * Select & activate a set to load results to
 */    
    if (nonl_prefs.autoload) {
        int npts = 0;
        double delx, *xfit, *y, *yfit;
    	
        switch (nonl_prefs.load) {
    	case LOAD_VALUES:
    	case LOAD_RESIDUALS:
    	    npts = getsetlength(psrc);
    	    setlength(pdest, npts);
    	    copycol2(psrc, pdest, DATA_X);
    	    break;
    	case LOAD_FUNCTION:
    	    npts  = nonl_prefs.npoints;
 
    	    setlength(pdest, npts);
 
    	    delx = (nonl_prefs.stop - nonl_prefs.start)/(npts - 1);
    	    xfit = getx(pdest);
	    for (i = 0; i < npts; i++) {
	        xfit[i] = nonl_prefs.start + i * delx;
	    }
    	    break;
    	}
    	
    	setcomment(pdest, nlfit->formula);
    	
    	do_compute(pdest, pdest, NULL, nlfit->formula);
    	
    	if (nonl_prefs.load == LOAD_RESIDUALS) { /* load residuals */
    	    y = gety(psrc);
    	    yfit = gety(pdest);
    	    for (i = 0; i < npts; i++) {
	        yfit[i] -= y[i];
	    }
    	}
    	
    	update_set_lists(pdest->parent);
    	xdrawgraph();
    }
    
    return RETURN_SUCCESS;
}

static void nonl_wf_cb(int value, void *data)
{
    Widget rc = GetParent((Widget) data);
    
    if (value == WEIGHT_CUSTOM) {
    	SetSensitive(rc, True);
    } else {
    	SetSensitive(rc, False);
    }
}

static void do_nonl_toggle(int value, void *data)
{
    Widget rc = (Widget) data;
    
    if (value == LOAD_FUNCTION) {
    	SetSensitive(rc, True);
    } else {
    	SetSensitive(rc, False);
    }
}

static void do_constr_toggle(int onoff, void *data)
{
#if 0
    int value = (int) data;
    if (onoff) {
    	SetSensitive(ui->lowb_item[value], True);
    	SetSensitive(ui->uppb_item[value], True);
    	nlfit.parms[value].constr = TRUE;
    } else {
    	SetSensitive(ui->lowb_item[value], False);
    	SetSensitive(ui->uppb_item[value], False);
    	nlfit.parms[value].constr = FALSE;
    }
#endif
}


static void create_openfit_popup(void *data)
{
    static FSBStructure *fsb = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        fsb = CreateFileSelectionBox(app_shell, "Open fit parameter file");
	AddFileSelectionBoxCB(fsb, do_openfit_proc, NULL);
        ManageChild(fsb->FSB);
    }
    
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
}

static int do_openfit_proc(char *filename, void *data)
{
    NonL_ui *ui = (NonL_ui *) data;
    reset_nonl(&ui->nlfit);
    errwin("Not implemented yet");
    update_nonl_frame(ui);
    
    return FALSE;
}


static void create_savefit_popup(void *data)
{
    NonL_ui *ui = (NonL_ui *) data;
    static FSBStructure *fsb = NULL;
    static Widget title_item = NULL;

    set_wait_cursor();

    if (fsb == NULL) {
        Widget fr;
        
        fsb = CreateFileSelectionBox(app_shell, "Save fit parameter file");
	fr = CreateFrame(fsb->rc, NULL);
	title_item = CreateTextItem2(fr, 25, "Title: ");
	AddFileSelectionBoxCB(fsb, do_savefit_proc, (void *) title_item);
        ManageChild(fsb->FSB);
    }
    
    xv_setstr(title_item, ui->nlfit.title);
    
    RaiseWindow(fsb->dialog);

    unset_wait_cursor();
}

static int do_savefit_proc(char *filename, void *data)
{
    FILE *pp;
    Widget title_item = (Widget) data;
    
    pp = grace_openw(filename);
    if (pp != NULL) {
#if 0
        nlfit.title = copy_string(nlfit.title, xv_getstr(title_item));
#endif
        errwin("Not implemented yet");
        /* FIXME */;
        grace_close(pp);
    }
    return TRUE;
}
