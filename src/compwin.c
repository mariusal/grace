/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2002 Grace Development Team
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
 * transformations, curve fitting, etc.
 *
 * formerly, this was all one big popup, now it is several.
 * All are created as needed
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include "graphs.h"
#include "utils.h"
#include "ssdata.h"
#include "motifinc.h"
#include "protos.h"

static int compute_aac(void *data);
static int do_histo_proc(void *data);
static int do_fourier_proc(void *data);
static int do_interp_proc(void *data);
static int do_runavg_proc(void *data);
static int do_differ_proc(void *data);
static int do_int_proc(void *data);
static int do_linearc_proc(void *data);
static int do_xcor_proc(void *data);
static int do_sample_proc(void *data);
static int do_prune_proc(void *data);

typedef struct _Eval_ui {
    Widget top;
    SrcDestStructure *srcdest;
    Widget formula_item;
    RestrictionStructure *restr_item;
} Eval_ui;

void create_eval_frame(void *data)
{
    static Eval_ui *eui = NULL;
    
    set_wait_cursor();
    
    if (eui == NULL) {
        Widget rc_trans;

	eui = xmalloc(sizeof(Eval_ui));
        
        eui->top = CreateDialogForm(app_shell, "Evaluate expression");
        SetDialogFormResizable(eui->top, TRUE);

        eui->srcdest = CreateSrcDestSelector(eui->top, LIST_TYPE_MULTIPLE);
        AddDialogFormChild(eui->top, eui->srcdest->form);

	rc_trans = CreateVContainer(eui->top);

	eui->formula_item = CreateScrollTextItem2(rc_trans, 3, "Formula:");

        eui->restr_item =
            CreateRestrictionChoice(rc_trans, "Source data filtering");

        CreateAACDialog(eui->top, rc_trans, compute_aac, (void *) eui);
    }
    
    RaiseWindow(GetParent(eui->top));
    
    unset_wait_cursor();
}

/*
 * evaluate a formula
 */
static int compute_aac(void *data)
{
    int error, resno;
    int i, g1_ok, g2_ok, ns1, ns2, *svalues1, *svalues2,
        gno1, gno2, setno1, setno2;
    char fstr[256];
    int restr_type, restr_negate;
    char *rarray;
    Eval_ui *eui = (Eval_ui *) data;

    restr_type = GetOptionChoice(eui->restr_item->r_sel);
    restr_negate = GetToggleButtonState(eui->restr_item->negate);
    
    g1_ok = GetSingleListChoice(eui->srcdest->src->graph_sel, &gno1);
    g2_ok = GetSingleListChoice(eui->srcdest->dest->graph_sel, &gno2);
    ns1 = GetListChoices(eui->srcdest->src->set_sel, &svalues1);
    ns2 = GetListChoices(eui->srcdest->dest->set_sel, &svalues2);
    
    error = FALSE;
    if (g1_ok == RETURN_FAILURE || g2_ok == RETURN_FAILURE) {
        error = TRUE;
        errmsg("Please select single source and destination graphs");
    } else if (ns1 == 0) {
        error = TRUE;
        errmsg("No source sets selected");
    } else if (ns1 != ns2 && ns2 != 0) {
        error = TRUE;
        errmsg("Different number of source and destination sets");
    } else {
        strcpy(fstr, xv_getstr(eui->formula_item));
        for (i = 0; i < ns1; i++) {
	    setno1 = svalues1[i];
	    if (ns2 != 0) {
                setno2 = svalues2[i];
            } else {
                setno2 = nextset(gno2);
                set_set_hidden(gno2, setno2, FALSE);
            }
	    
            resno = get_restriction_array(gno1, setno1,
                restr_type, restr_negate, &rarray);
	    if (resno != RETURN_SUCCESS) {
	        errmsg("Error in evaluation restriction");
	        break;
	    }
            
            resno = do_compute(gno1, setno1, gno2, setno2, rarray, fstr);
	    XCFREE(rarray);
	    if (resno != RETURN_SUCCESS) {
	        errmsg("Error in do_compute(), check formula");
                break;
	    }
        }
    }
    
    if (ns1 > 0) {
        xfree(svalues1);
    }
    if (ns2 > 0) {
        xfree(svalues2);
    }
    if (error == FALSE) {
        if (gno1 != gno2) {
            update_set_lists(gno1);
            update_set_lists(gno2);
        } else {
            update_set_lists(gno1);
        }
        xdrawgraph();
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


#define SAMPLING_MESH   0
#define SAMPLING_SET    1

/* interpolation */

typedef struct _Interp_ui {
    TransformStructure *tdialog;
    OptionStructure *method;
    OptionStructure *sampling;
    Widget strict;
    Widget mrc;
    Widget mstart;
    Widget mstop;
    Widget mlength;
    ListStructure *sset_sel;
} Interp_ui;


static void sampling_cb(int value, void *data)
{
    Interp_ui *ui = (Interp_ui *) data;
    
    if (value == SAMPLING_MESH) {
        SetSensitive(ui->mrc, True);
        SetSensitive(ui->sset_sel->list, False);
    } else {
        SetSensitive(ui->mrc, False);
        SetSensitive(ui->sset_sel->list, True);
    }
}

void create_interp_frame(void *data)
{
    static Interp_ui *interpui = NULL;
    
    set_wait_cursor();

    if (interpui == NULL) {
        Widget fr, rc, rc2;
        OptionItem opitems[3];
        
        interpui = xmalloc(sizeof(Interp_ui));
        interpui->tdialog = CreateTransformDialogForm(app_shell,
            "Interpolation", LIST_TYPE_MULTIPLE);
        fr = CreateFrame(interpui->tdialog->form, NULL);
        rc = CreateVContainer(fr);
        
        rc2 = CreateHContainer(rc);
        opitems[0].value = INTERP_LINEAR;
        opitems[0].label = "Linear";
        opitems[1].value = INTERP_SPLINE;
        opitems[1].label = "Cubic spline";
        opitems[2].value = INTERP_ASPLINE;
        opitems[2].label = "Akima spline";
        interpui->method = CreateOptionChoice(rc2, "Method:", 0, 3, opitems);
        
        interpui->strict =
            CreateToggleButton(rc2, "Strict (within source set bounds)");
        
        CreateSeparator(rc);
        
        opitems[0].value = SAMPLING_MESH;
        opitems[0].label = "Linear mesh";
        opitems[1].value = SAMPLING_SET;
        opitems[1].label = "Abscissas of another set";
        interpui->sampling = CreateOptionChoice(rc, "Sampling:", 0, 2, opitems);
        AddOptionChoiceCB(interpui->sampling, sampling_cb, interpui);

        interpui->mrc = CreateHContainer(rc);
	interpui->mstart  = CreateTextItem2(interpui->mrc, 10, "Start at:");
	interpui->mstop   = CreateTextItem2(interpui->mrc, 10, "Stop at:");
	interpui->mlength = CreateTextItem2(interpui->mrc, 6, "Length:");
        
        interpui->sset_sel = CreateSetChoice(rc,
            "Sampling set", LIST_TYPE_SINGLE, TRUE);
        SetSensitive(interpui->sset_sel->list, False);
        
        CreateAACDialog(interpui->tdialog->form, fr, do_interp_proc, interpui);
    }
    
    RaiseWindow(GetParent(interpui->tdialog->form));
    unset_wait_cursor();
}


static int do_interp_proc(void *data)
{
    int error, res;
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int method, sampling, strict;
    int i, meshlen;
    double *mesh = NULL;
    Interp_ui *ui = (Interp_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest,
        &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }

    error = FALSE;
    
    method = GetOptionChoice(ui->method);
    sampling = GetOptionChoice(ui->sampling);
    strict = GetToggleButtonState(ui->strict);

    if (sampling == SAMPLING_SET) {
        int gsampl, setnosampl;
        gsampl = get_cg();
        res = GetSingleListChoice(ui->sset_sel, &setnosampl);
        if (res != RETURN_SUCCESS) {
            errmsg("Please select single sampling set");
            error = TRUE;
        } else {
            meshlen = getsetlength(gsampl, setnosampl);
            mesh = getcol(gsampl, setnosampl, DATA_X);
        }
    } else {
        double start, stop;
        if (xv_evalexpr(ui->mstart, &start)     != RETURN_SUCCESS ||
            xv_evalexpr(ui->mstop,  &stop)      != RETURN_SUCCESS ||
            xv_evalexpri(ui->mlength, &meshlen) != RETURN_SUCCESS ) {
             errmsg("Can't parse mesh settings");
             error = TRUE;
        } else {
            mesh = allocate_mesh(start, stop, meshlen);
            if (mesh == NULL) {
	        errmsg("Can't allocate mesh");
                error = TRUE;
            }
        }
    }
    
    if (error) {
        xfree(svaluessrc);
        if (nsdest > 0) {
            xfree(svaluesdest);
        }
        return RETURN_FAILURE;
    }

    for (i = 0; i < nssrc; i++) {
	int setnosrc, setnodest;
        setnosrc = svaluessrc[i];
	if (nsdest != 0) {
            setnodest = svaluesdest[i];
        } else {
            setnodest = NEW_SET;
        }
        
        res = do_interp(gsrc, setnosrc, gdest, setnodest,
            mesh, meshlen, method, strict);
	
        if (res != RETURN_SUCCESS) {
	    errmsg("Error in do_interp()");
	    error = TRUE;
            break;
	}
    }
    
    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    if (sampling == SAMPLING_MESH) {
        xfree(mesh);
    }

    update_set_lists(gdest);
    xdrawgraph();
    
    if (error) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}


/* histograms */

typedef struct _Histo_ui {
    TransformStructure *tdialog;
    Widget cumulative;
    Widget normalize;
    OptionStructure *sampling;
    Widget mrc;
    Widget mstart;
    Widget mstop;
    Widget mlength;
    ListStructure *sset_sel;
} Histo_ui;

static void binsampling_cb(int value, void *data)
{
    Interp_ui *ui = (Interp_ui *) data;
    
    if (value == SAMPLING_MESH) {
        SetSensitive(ui->mrc, True);
        SetSensitive(ui->sset_sel->list, False);
    } else {
        SetSensitive(ui->mrc, False);
        SetSensitive(ui->sset_sel->list, True);
    }
}

void create_histo_frame(void *data)
{
    static Histo_ui *histoui = NULL;

    set_wait_cursor();

    if (histoui == NULL) {
        Widget fr, rc, rc2;
        OptionItem opitems[2];
        
        histoui = xmalloc(sizeof(Histo_ui));
        histoui->tdialog = CreateTransformDialogForm(app_shell,
            "Histograms", LIST_TYPE_MULTIPLE);
        fr = CreateFrame(histoui->tdialog->form, NULL);
        rc = CreateVContainer(fr);
        
        rc2 = CreateHContainer(rc);
        histoui->cumulative = CreateToggleButton(rc2, "Cumulative histogram");
        histoui->normalize = CreateToggleButton(rc2, "Normalize");
        
        CreateSeparator(rc);
        
        opitems[0].value = SAMPLING_MESH;
        opitems[0].label = "Linear mesh";
        opitems[1].value = SAMPLING_SET;
        opitems[1].label = "Abscissas of another set";
        histoui->sampling = CreateOptionChoice(rc, "Bin sampling:", 0, 2, opitems);
        AddOptionChoiceCB(histoui->sampling, binsampling_cb, histoui);

        histoui->mrc = CreateHContainer(rc);
	histoui->mstart  = CreateTextItem2(histoui->mrc, 10, "Start at:");
	histoui->mstop   = CreateTextItem2(histoui->mrc, 10, "Stop at:");
	histoui->mlength = CreateTextItem2(histoui->mrc, 6, "# of bins");
        
        histoui->sset_sel = CreateSetChoice(rc,
            "Sampling set", LIST_TYPE_SINGLE, TRUE);
        SetSensitive(histoui->sset_sel->list, False);
        
        CreateAACDialog(histoui->tdialog->form, fr, do_histo_proc, histoui);
    }
    
    RaiseWindow(GetParent(histoui->tdialog->form));
    unset_wait_cursor();
}


static int do_histo_proc(void *data)
{
    int error, res;
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int cumulative, normalize, sampling;
    int i, nbins;
    double *bins = NULL;
    Histo_ui *ui = (Histo_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest,
        &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }

    error = FALSE;
    
    cumulative = GetToggleButtonState(ui->cumulative);
    normalize  = GetToggleButtonState(ui->normalize);
    sampling   = GetOptionChoice(ui->sampling);

    if (sampling == SAMPLING_SET) {
        int gsampl, setnosampl;
        gsampl = get_cg();
        res = GetSingleListChoice(ui->sset_sel, &setnosampl);
        if (res != RETURN_SUCCESS) {
            errmsg("Please select single sampling set");
            error = TRUE;
        } else {
            nbins = getsetlength(gsampl, setnosampl) - 1;
            bins = getcol(gsampl, setnosampl, DATA_X);
        }
    } else {
        double start, stop;
        if (xv_evalexpr(ui->mstart, &start)   != RETURN_SUCCESS ||
            xv_evalexpr(ui->mstop,  &stop)    != RETURN_SUCCESS ||
            xv_evalexpri(ui->mlength, &nbins) != RETURN_SUCCESS ){
            errmsg("Can't parse mesh settings");
            error = TRUE;
        } else {
            bins = allocate_mesh(start, stop, nbins + 1);
            if (bins == NULL) {
	        errmsg("Can't allocate mesh");
                error = TRUE;
            }
        }
    }
    
    if (error) {
        xfree(svaluessrc);
        if (nsdest > 0) {
            xfree(svaluesdest);
        }
        return RETURN_FAILURE;
    }

    for (i = 0; i < nssrc; i++) {
	int setnosrc, setnodest;
        setnosrc = svaluessrc[i];
	if (nsdest != 0) {
            setnodest = svaluesdest[i];
        } else {
            setnodest = NEW_SET;
        }
        
        res = do_histo(gsrc, setnosrc, gdest, setnodest,
            bins, nbins, cumulative, normalize);
	
        if (res != RETURN_SUCCESS) {
	    errmsg("Error in do_histo()");
	    error = TRUE;
            break;
	}
    }
    
    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    if (sampling == SAMPLING_MESH) {
        xfree(bins);
    }

    update_set_lists(gdest);
    xdrawgraph();
    
    if (error) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}

/* DFTs */

typedef struct _Four_ui {
    TransformStructure *tdialog;
    Widget inverse;
    OptionStructure *xscale;
    OptionStructure *norm;
    Widget complexin;
    Widget dcdump;
    SpinStructure *oversampling;
    Widget round2n;
    OptionStructure *window;
    SpinStructure *winpar;
    Widget halflen;
    OptionStructure *output;
} Four_ui;

static void toggle_inverse_cb(int onoff, void *data)
{
    Four_ui *ui = (Four_ui *) data;
    if (onoff) {
        SetToggleButtonState(ui->halflen, FALSE);
        SetSensitive(ui->halflen, FALSE);
        
        SetToggleButtonState(ui->dcdump, FALSE);
        SetSensitive(ui->dcdump, FALSE);

        SetOptionChoice(ui->window, FFT_WINDOW_NONE);
        SetSensitive(ui->window->menu, FALSE);
    } else {
        SetToggleButtonState(ui->halflen, TRUE);
        SetSensitive(ui->halflen, TRUE);
        
        SetSensitive(ui->dcdump, TRUE);

        SetSensitive(ui->window->menu, TRUE);
    }
}

static void toggle_complex_cb(int onoff, void *data)
{
    Four_ui *ui = (Four_ui *) data;
    if (onoff) {
        SetToggleButtonState(ui->halflen, FALSE);
        SetSensitive(ui->halflen, FALSE);
    } else {
        SetToggleButtonState(ui->halflen, TRUE);
        SetSensitive(ui->halflen, TRUE);
    }
}

static void option_window_cb(int value, void *data)
{
    Four_ui *ui = (Four_ui *) data;
    SetSensitive(ui->winpar->rc, value == FFT_WINDOW_KAISER);
}

void create_fourier_frame(void *data)
{
    static Four_ui *fui = NULL;

    set_wait_cursor();

    if (fui == NULL) {
        Widget rc, fr, rc1, rc2;
        OptionItem window_opitems[] = {
            {FFT_WINDOW_NONE,       "None (Rectangular)"},
            {FFT_WINDOW_TRIANGULAR, "Triangular"        },
            {FFT_WINDOW_PARZEN,     "Parzen"            },
            {FFT_WINDOW_WELCH,      "Welch"             },
            {FFT_WINDOW_HANNING,    "Hanning"           },
            {FFT_WINDOW_HAMMING,    "Hamming"           },
            {FFT_WINDOW_FLATTOP,    "Flat top"          },
            {FFT_WINDOW_BLACKMAN,   "Blackman"          },
            {FFT_WINDOW_KAISER,     "Kaiser"            }
        };
        OptionItem output_opitems[] = {
            {FFT_OUTPUT_MAGNITUDE, "Magnitude"       },
            {FFT_OUTPUT_PHASE,     "Phase"           },
            {FFT_OUTPUT_RE,        "Real part"       },
            {FFT_OUTPUT_IM,        "Imaginary part"  },
            {FFT_OUTPUT_REIM,      "Complex"         },
            {FFT_OUTPUT_APHI,      "Complex (A, Phi)"}
        };
        OptionItem xscale_opitems[] = {
            {FFT_XSCALE_INDEX, "Index"         },
            {FFT_XSCALE_NU,    "Frequency"     },
            {FFT_XSCALE_OMEGA, "Ang. frequency"}
        };
        OptionItem norm_opitems[] = {
            {FFT_NORM_NONE,      "None"     },
            {FFT_NORM_SYMMETRIC, "Symmetric"},
            {FFT_NORM_FORWARD,   "Forward"  },
            {FFT_NORM_BACKWARD,  "Backward" }
        };
        
        fui = xmalloc(sizeof(Four_ui));
        
	fui->tdialog = CreateTransformDialogForm(app_shell,
            "Fourier transform", LIST_TYPE_MULTIPLE);

	rc = CreateVContainer(fui->tdialog->form);

        fr = CreateFrame(rc, "General");
	rc1 = CreateVContainer(fr);
	fui->inverse = CreateToggleButton(rc1, "Perform backward transform");
        AddToggleButtonCB(fui->inverse, toggle_inverse_cb, (void *) fui);
	rc2 = CreateHContainer(rc1);
	fui->xscale = CreateOptionChoice(rc2, "X scale:", 0, 3, xscale_opitems);
	fui->norm = CreateOptionChoice(rc2, "Normalize:", 0, 4, norm_opitems);
        
        fr = CreateFrame(rc, "Input");
	rc1 = CreateVContainer(fr);
	fui->complexin = CreateToggleButton(rc1, "Complex data");
        AddToggleButtonCB(fui->complexin, toggle_complex_cb, (void *) fui);
	fui->dcdump = CreateToggleButton(rc1, "Dump DC component");
	rc2 = CreateHContainer(rc1);
	fui->window = CreateOptionChoice(rc2,
            "Apply window:", 0, 9, window_opitems);
        AddOptionChoiceCB(fui->window, option_window_cb, (void *) fui);
        fui->winpar = CreateSpinChoice(rc2,
            "Parameter", 2, SPIN_TYPE_FLOAT, 0.0, 99.0, 1.0);
	rc2 = CreateHContainer(rc1);
        fui->oversampling = CreateSpinChoice(rc2,
            "Zero padding", 2, SPIN_TYPE_FLOAT, 1.0, 99.0, 1.0);
	fui->round2n = CreateToggleButton(rc2, "Round to 2^N");

        fr = CreateFrame(rc, "Output");
	rc1 = CreateHContainer(fr);
        fui->output = CreateOptionChoice(rc1,
            "Load:", 0, 6, output_opitems);
	fui->halflen = CreateToggleButton(rc1, "Half length");

	CreateAACDialog(fui->tdialog->form, rc, do_fourier_proc, (void *) fui);
        
        /* Default values */
        SetOptionChoice(fui->xscale, FFT_XSCALE_NU);
        SetOptionChoice(fui->norm, FFT_NORM_FORWARD);
        SetSpinChoice(fui->winpar, 1.0);
        SetSensitive(fui->winpar->rc, FALSE);
        SetToggleButtonState(fui->halflen, TRUE);
        SetSpinChoice(fui->oversampling, 1.0);
#ifndef HAVE_FFTW
        SetToggleButtonState(fui->round2n, TRUE);
#endif
    }
    
    RaiseWindow(GetParent(fui->tdialog->form));
    
    unset_wait_cursor();
}

/*
 * Fourier
 */
static int do_fourier_proc(void *data)
{
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int i, res, err = FALSE;
    int invflag, xscale, norm;
    int complexin, dcdump, window, round2n, halflen, output;
    double oversampling, beta;
    Four_ui *ui = (Four_ui *) data;
    
    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest, &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }

    invflag   = GetToggleButtonState(ui->inverse);
    xscale    = GetOptionChoice(ui->xscale);
    norm      = GetOptionChoice(ui->norm);
    
    complexin = GetToggleButtonState(ui->complexin);
    dcdump    = GetToggleButtonState(ui->dcdump);
    oversampling   = GetSpinChoice(ui->oversampling);
    round2n   = GetToggleButtonState(ui->round2n);
    window    = GetOptionChoice(ui->window);
    beta      = GetSpinChoice(ui->winpar);
    
    halflen   = GetToggleButtonState(ui->halflen);
    output    = GetOptionChoice(ui->output);
    
    for (i = 0; i < nssrc; i++) {
	int setfrom, setto;
        setfrom = svaluessrc[i];
	if (nsdest) {
            setto = svaluesdest[i];
        } else {
            setto = nextset(gdest);
        }
	if (do_fourier(gsrc, setfrom, gdest, setto,
            invflag, xscale, norm, complexin, dcdump, oversampling, round2n,
            window, beta, halflen, output) != RETURN_SUCCESS) {
            err = TRUE;
        }
    }

    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    
    update_set_lists(gdest);
    xdrawgraph();
    
    if (err) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}

/* finite differencing */

typedef struct _Diff_ui {
    TransformStructure *tdialog;
    OptionStructure *type;
    OptionStructure *xplace;
    SpinStructure *period;
} Diff_ui;

#define DIFF_TYPE_PLAIN         0
#define DIFF_TYPE_DERIVATIVE    1

void create_diff_frame(void *data)
{
    static Diff_ui *dui = NULL;

    set_wait_cursor();
    
    if (dui == NULL) {
        Widget rc;
        OptionItem topitems[] = {
            {DIFF_TYPE_PLAIN,      "Plain differences"},
            {DIFF_TYPE_DERIVATIVE, "Derivative"       }
        };
        OptionItem xopitems[] = {
            {DIFF_XPLACE_LEFT,   "Left"  },
            {DIFF_XPLACE_CENTER, "Center"},
            {DIFF_XPLACE_RIGHT,  "Right" }
        };
	
        dui = xmalloc(sizeof(Diff_ui));
        
        dui->tdialog = CreateTransformDialogForm(app_shell,
            "Differences", LIST_TYPE_MULTIPLE);
	
        rc = CreateVContainer(dui->tdialog->form);
        dui->type   = CreateOptionChoice(rc, "Type:", 0, 2, topitems);
        dui->xplace = CreateOptionChoice(rc, "X placement:", 0, 3, xopitems);
        dui->period = CreateSpinChoice(rc, "Period", 6, SPIN_TYPE_INT,
            (double) 1, (double) 999999, (double) 1);
        SetSpinChoice(dui->period, (double) 1);
	
        CreateAACDialog(dui->tdialog->form, rc, do_differ_proc, (void *) dui);
    }
    
    RaiseWindow(GetParent(dui->tdialog->form));
    unset_wait_cursor();
}

/*
 * finite differences
 */
static int do_differ_proc(void *data)
{
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int i, res, err = FALSE;
    int type, xplace, period;
    Diff_ui *ui = (Diff_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest, &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }

    type   = GetOptionChoice(ui->type);
    xplace = GetOptionChoice(ui->xplace);
    period = GetSpinChoice(ui->period);
    
    for (i = 0; i < nssrc; i++) {
	int setfrom, setto;
        setfrom = svaluessrc[i];
	if (nsdest) {
            setto = svaluesdest[i];
        } else {
            setto = nextset(gdest);
        }
	res = do_differ(gsrc, setfrom, gdest, setto,
            type == DIFF_TYPE_DERIVATIVE, xplace, period);
        if (res != RETURN_SUCCESS) {
            err = TRUE;
        }
    }

    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    
    update_set_lists(gdest);
    xdrawgraph();
    
    if (err) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}


/* running averages */

#define RUN_TYPE_CUSTOM     0
#define RUN_TYPE_AVERAGE    1
#define RUN_TYPE_STDDEV     2
#define RUN_TYPE_MIN        3
#define RUN_TYPE_MAX        4

typedef struct _Run_ui {
    TransformStructure *tdialog;
    SpinStructure *length;
    TextStructure *formula;
    OptionStructure *xplace;
} Run_ui;

static void run_type_cb(int value, void *data)
{
    Run_ui *ui = (Run_ui *) data;
    char *formula;
    
    switch (value) {
    case RUN_TYPE_AVERAGE:
        formula = "AVG($t)";
        break;
    case RUN_TYPE_STDDEV:
        formula = "SD($t)";
        break;
    case RUN_TYPE_MIN:
        formula = "MIN($t)";
        break;
    case RUN_TYPE_MAX:
        formula = "MAX($t)";
        break;
    default:
        formula = NULL;
        break;
    }
    
    if (formula) {
        SetTextString(ui->formula, formula);
        SetTextEditable(ui->formula, FALSE);
    } else {
        SetTextEditable(ui->formula, TRUE);
    }
}

void create_run_frame(void *data)
{
    static Run_ui *rui = NULL;
    
    set_wait_cursor();

    if (rui == NULL) {
        Widget rc;
        OptionStructure *type;
        OptionItem topitems[] = {
            {RUN_TYPE_CUSTOM,  "Custom"   },
            {RUN_TYPE_AVERAGE, "Average"  },
            {RUN_TYPE_STDDEV,  "Std. dev."},
            {RUN_TYPE_MIN,     "Minimum"  },
            {RUN_TYPE_MAX,     "Maximum"  }
        };
        OptionItem xopitems[] = {
            {RUN_XPLACE_LEFT,    "Left"   },
            {RUN_XPLACE_AVERAGE, "Average"},
            {RUN_XPLACE_RIGHT,   "Right"  }
        };
	
        rui = xmalloc(sizeof(Run_ui));
        
        rui->tdialog = CreateTransformDialogForm(app_shell,
            "Running properties", LIST_TYPE_MULTIPLE);

	rc = CreateVContainer(rui->tdialog->form);
        type = CreateOptionChoice(rc, "Type:", 0, 5, topitems);
        AddOptionChoiceCB(type, run_type_cb, (void *) rui);
	rui->formula = CreateTextInput(rc, "Formula:");
	rui->length = CreateSpinChoice(rc, "Length of sample", 6, SPIN_TYPE_INT,
            (double) 1, (double) 999999, (double) 1);
        rui->xplace = CreateOptionChoice(rc, "X placement:", 0, 3, xopitems);
        
        /* default settings */
        SetSpinChoice(rui->length, 1);
        SetOptionChoice(rui->xplace, RUN_XPLACE_AVERAGE);
        
        CreateAACDialog(rui->tdialog->form, rc, do_runavg_proc, (void *) rui);
    }
    
    RaiseWindow(GetParent(rui->tdialog->form));
    unset_wait_cursor();
}

/*
 * evaluation of running properties
 */
static int do_runavg_proc(void *data)
{
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int i, res, err = FALSE;
    int length, xplace;
    char *formula;
    Run_ui *ui = (Run_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest, &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    length = (int) GetSpinChoice(ui->length);
    formula = GetTextString(ui->formula);
    xplace = GetOptionChoice(ui->xplace);
    
    for (i = 0; i < nssrc; i++) {
	int setfrom, setto;
        setfrom = svaluessrc[i];
	if (nsdest) {
            setto = svaluesdest[i];
        } else {
            setto = nextset(gdest);
        }
	res = do_runavg(gsrc, setfrom, gdest, setto, length, formula, xplace);
        if (res != RETURN_SUCCESS) {
            err = TRUE;
        }
    }
    
    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    
    update_set_lists(gdest);
    xdrawgraph();
    
    if (err) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}

/* numerical integration */

typedef struct _Int_ui {
    TransformStructure *tdialog;
    Widget disponly;
} Int_ui;


void create_int_frame(void *data)
{
    static Int_ui *iui = NULL;

    set_wait_cursor();
    
    if (iui == NULL) {
        Widget rc;
        
        iui = xmalloc(sizeof(Run_ui));
        iui->tdialog = CreateTransformDialogForm(app_shell,
            "Running properties", LIST_TYPE_MULTIPLE);

	rc = CreateVContainer(iui->tdialog->form);

	iui->disponly = CreateToggleButton(rc, "Display integral value only");

        CreateAACDialog(iui->tdialog->form, rc, do_int_proc, (void *) iui);

    }
    
    RaiseWindow(GetParent(iui->tdialog->form));
    unset_wait_cursor();
}

/*
 * numerical integration
 */
static int do_int_proc(void *data)
{
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int i, res, err = FALSE;
    int disponly;
    double sum;
    Int_ui *ui = (Int_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest, &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    disponly = GetToggleButtonState(ui->disponly);
    
    for (i = 0; i < nssrc; i++) {
	int setfrom, setto;
        setfrom = svaluessrc[i];
	if (nsdest) {
            setto = svaluesdest[i];
        } else {
            setto = nextset(gdest);
        }
	res = do_int(gsrc, setfrom, gdest, setto, disponly, &sum);
        if (res != RETURN_SUCCESS) {
            err = TRUE;
        } else {
            char buf[64];
            sprintf(buf, "Integral of set G%d.S%d: %g\n", gsrc, setfrom, sum);
            stufftext(buf);
        }
    }
    
    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    
    if (!disponly) {
        update_set_lists(gdest);
        xdrawgraph();
    }
    
    if (err) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}

/* linear convolution */

typedef struct _Lconv_ui {
    TransformStructure *tdialog;
    GraphSetStructure *convsel;
} Lconv_ui;

void create_lconv_frame(void *data)
{
    static Lconv_ui *ui = NULL;

    set_wait_cursor();

    if (ui == NULL) {
        Widget rc;

        ui = xmalloc(sizeof(Lconv_ui));
        
        ui->tdialog = CreateTransformDialogForm(app_shell,
            "Linear convolution", LIST_TYPE_MULTIPLE);

	rc = CreateVContainer(ui->tdialog->form);
	ui->convsel = CreateGraphSetSelector(rc,
            "Convolve with:", LIST_TYPE_SINGLE);

        CreateAACDialog(ui->tdialog->form, rc, do_linearc_proc, (void *) ui);
    }
    
    RaiseWindow(GetParent(ui->tdialog->form));
    
    unset_wait_cursor();
}

/*
 * linear convolution
 */
static int do_linearc_proc(void *data)
{
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int i, res, err = FALSE;
    int gconv, setconv;
    Lconv_ui *ui = (Lconv_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest, &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    if (GetSingleListChoice(ui->convsel->graph_sel, &gconv) != RETURN_SUCCESS ||
        GetSingleListChoice(ui->convsel->set_sel, &setconv) != RETURN_SUCCESS) {
        errmsg("Please select a single set to be convoluted with");
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < nssrc; i++) {
	int setfrom, setto;
        setfrom = svaluessrc[i];
	if (nsdest) {
            setto = svaluesdest[i];
        } else {
            setto = nextset(gdest);
        }
        res = do_linearc(gsrc, setfrom, gdest, setto, gconv, setconv);
        if (res != RETURN_SUCCESS) {
            err = TRUE;
        }
    }

    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    
    update_set_lists(gdest);
    xdrawgraph();

    if (err) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}


/* cross correlation */

typedef struct _Cross_ui {
    TransformStructure *tdialog;
    Widget autocor;
    GraphSetStructure *corsel;
    SpinStructure *maxlag;
    Widget covar;
} Cross_ui;

static void xcor_self_toggle(int onoff, void *data)
{
    Cross_ui *ui = (Cross_ui *) data;
    
    SetSensitive(ui->corsel->frame, !onoff);
}

void create_xcor_frame(void *data)
{
    static Cross_ui *ui = NULL;

    set_wait_cursor();
    
    if (ui == NULL) {
        Widget rc;

        ui = xmalloc(sizeof(Cross_ui));
        
        ui->tdialog = CreateTransformDialogForm(app_shell,
            "Correlation/covariance", LIST_TYPE_MULTIPLE);

	rc = CreateVContainer(ui->tdialog->form);
	ui->autocor = CreateToggleButton(rc, "Auto-correlation");
        AddToggleButtonCB(ui->autocor, xcor_self_toggle, (void *) ui);
        ui->corsel = CreateGraphSetSelector(rc,
            "Correlate with:", LIST_TYPE_SINGLE);
	ui->maxlag = CreateSpinChoice(rc, "Maximum lag:", 6, SPIN_TYPE_INT,
            (double) 1, (double) 999999, (double) 1);
        ui->covar = CreateToggleButton(rc, "Calculate covariance");
        
        /* default settings */
        SetSpinChoice(ui->maxlag, 1);

        CreateAACDialog(ui->tdialog->form, rc, do_xcor_proc, (void *) ui);
    }
    
    RaiseWindow(GetParent(ui->tdialog->form));
    
    unset_wait_cursor();
}

/*
 * cross correlation
 */
static int do_xcor_proc(void *data)
{
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int i, res, err = FALSE;
    int gcor, setcor;
    int maxlag, autocor, covar;
    Cross_ui *ui = (Cross_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest, &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    maxlag  = (int) GetSpinChoice(ui->maxlag);
    autocor = GetToggleButtonState(ui->autocor);
    covar   = GetToggleButtonState(ui->covar);
    
    if (!autocor &&
        (GetSingleListChoice(ui->corsel->graph_sel, &gcor) != RETURN_SUCCESS ||
         GetSingleListChoice(ui->corsel->set_sel, &setcor) != RETURN_SUCCESS)) {
        errmsg("Please select a single set to be correlated with");
        return RETURN_FAILURE;
    }
    
    for (i = 0; i < nssrc; i++) {
	int setfrom, setto;
        setfrom = svaluessrc[i];
	if (nsdest) {
            setto = svaluesdest[i];
        } else {
            setto = nextset(gdest);
        }
        if (autocor) {
            gcor = gsrc;
            setcor = setfrom;
        }
        res = do_xcor(gsrc, setfrom, gdest, setto, gcor, setcor, maxlag, covar);
        if (res != RETURN_SUCCESS) {
            err = TRUE;
        }
    }

    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    
    update_set_lists(gdest);
    xdrawgraph();

    if (err) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}


/* sample a set */

typedef struct _Samp_ui {
    TransformStructure *tdialog;
    TextStructure *formula;
} Samp_ui;

void create_samp_frame(void *data)
{
    static Samp_ui *ui = NULL;

    set_wait_cursor();

    if (ui == NULL) {
        Widget rc;
	
        ui = xmalloc(sizeof(Samp_ui));
        
        ui->tdialog = CreateTransformDialogForm(app_shell,
            "Sample points", LIST_TYPE_MULTIPLE);

	rc = CreateVContainer(ui->tdialog->form);
	ui->formula = CreateTextInput(rc, "Logical expression:");

        CreateAACDialog(ui->tdialog->form, rc, do_sample_proc, (void *) ui);
    }
    
    RaiseWindow(GetParent(ui->tdialog->form));
    
    unset_wait_cursor();
}

/*
 * sample a set by a logical expression
 */
static int do_sample_proc(void *data)
{
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int i, res, err = FALSE;
    char *formula;
    Samp_ui *ui = (Samp_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest,
        &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    formula = GetTextString(ui->formula);

    for (i = 0; i < nssrc; i++) {
	int setfrom, setto;
        
        setfrom = svaluessrc[i];
	if (nsdest) {
            setto = svaluesdest[i];
        } else {
            setto = nextset(gdest);
        }
        
        res = do_sample(gsrc, setfrom, gdest, setto, formula);
        
        if (res != RETURN_SUCCESS) {
            err = TRUE;
        }
    }

    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    
    update_set_lists(gdest);
    xdrawgraph();

    if (err) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}

/* Prune data */

/* for data pruning */
#define PRUNE_TYPE_POINTS       0
#define PRUNE_TYPE_INTERP       1

#define PRUNE_AREA_RECTANGLE    0
#define PRUNE_AREA_ELLIPSE      1

#define PRUNE_DELTA_ABSOLUTE    0
#define PRUNE_DELTA_RELATIVE    1

typedef struct _Prune_ui {
    TransformStructure *tdialog;
    OptionStructure *type;
    OptionStructure *area;
    Widget dx;
    OptionStructure *dxtype;
    Widget dy;
    OptionStructure *dytype;
} Prune_ui;

void create_prune_frame(void *data)
{
    static Prune_ui *ui;
    
    set_wait_cursor();

    if (ui == NULL) {
	Widget rc, rc2;
        OptionItem topitems[] = {
            {PRUNE_TYPE_POINTS, "Points"       },
            {PRUNE_TYPE_INTERP, "Interpolation"}
        };
        OptionItem aopitems[] = {
            {PRUNE_AREA_RECTANGLE, "Rectangle"},
            {PRUNE_AREA_ELLIPSE,   "Ellipse"  }
        };
        OptionItem dopitems[] = {
            {PRUNE_DELTA_ABSOLUTE, "Absolute"},
            {PRUNE_DELTA_RELATIVE, "Relative"}
        };
        
        ui = xmalloc(sizeof(Prune_ui));

        ui->tdialog = CreateTransformDialogForm(app_shell,
            "Prune data", LIST_TYPE_MULTIPLE);

	rc = CreateVContainer(ui->tdialog->form);

        ui->type = CreateOptionChoice(rc, "Prune type:", 0, 2, topitems);
        ui->area = CreateOptionChoice(rc, "Prune area:", 0, 2, aopitems);

	rc2 = CreateHContainer(rc);
	ui->dx = CreateTextItem4(rc2, 16, "DX:");
	ui->dxtype = CreateOptionChoice(rc2, "Type:", 0, 2, dopitems);
	
	rc2 = CreateHContainer(rc);
	ui->dy = CreateTextItem4(rc2, 16, "DY:");
	ui->dytype = CreateOptionChoice(rc2, "Type:", 0, 2, dopitems);

        CreateAACDialog(ui->tdialog->form, rc, do_prune_proc, (void *) ui);
    }
    
    RaiseWindow(GetParent(ui->tdialog->form));
    
    unset_wait_cursor();
}


/*
 * Prune data
 */
static int do_prune_proc(void *data)
{
    int nssrc, nsdest, *svaluessrc, *svaluesdest, gsrc, gdest;
    int i, res, err = FALSE;
    int type, area, dxtype, dytype;
    double dx, dy;
    Prune_ui *ui = (Prune_ui *) data;

    res = GetTransformDialogSettings(ui->tdialog, TRUE,
        &gsrc, &gdest,
        &nssrc, &svaluessrc, &nsdest, &svaluesdest);
    
    if (res != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }

    type   = GetOptionChoice(ui->type);
    area   = GetOptionChoice(ui->area);
    dxtype = GetOptionChoice(ui->dxtype);
    dytype = GetOptionChoice(ui->dytype);

    if (xv_evalexpr(ui->dx, &dx) != RETURN_SUCCESS) {
        errmsg("Can't parse value for X");
        return RETURN_FAILURE;
    }
    if (xv_evalexpr(ui->dy, &dy) != RETURN_SUCCESS) {
        errmsg("Can't parse value for Y");
        return RETURN_FAILURE;
    }

    for (i = 0; i < nssrc; i++) {
	int setfrom, setto;
        
        setfrom = svaluessrc[i];
	if (nsdest) {
            setto = svaluesdest[i];
        } else {
            setto = nextset(gdest);
        }
        
	res = do_prune(gsrc, setfrom, gdest, setto,
            type == PRUNE_TYPE_INTERP, area == PRUNE_AREA_ELLIPSE,
            dx, dxtype, dy, dytype);
        
        if (res != RETURN_SUCCESS) {
            err = TRUE;
        }
    }

    xfree(svaluessrc);
    if (nsdest > 0) {
        xfree(svaluesdest);
    }
    
    update_set_lists(gdest);
    xdrawgraph();

    if (err) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}


typedef struct _Featext_ui {
    Widget top;
    GraphSetStructure *src;
    GraphSetStructure *dest;
    TextStructure *formula;
} Featext_ui;

static int do_fext_proc(void *data)
{
    char *formula;
    int nsrc, ndest, *ssids, *dsids, sdest, gsrc, gdest;

    Featext_ui *ui = (Featext_ui *) data;

    formula = GetTextString(ui->formula);
    
    if (GetSingleListChoice(ui->src->graph_sel, &gsrc) != RETURN_SUCCESS) {
        errmsg("Please select a single source graph");
        return RETURN_FAILURE;
    }
    if (GetSingleListChoice(ui->dest->graph_sel, &gdest) != RETURN_SUCCESS) {
        errmsg("Please select a single source graph");
        return RETURN_FAILURE;
    }
    
    nsrc = GetListChoices(ui->src->set_sel, &ssids);
    if (nsrc < 1) {
        errmsg("No source sets selected");
        return RETURN_FAILURE;
    }

    ndest = GetListChoices(ui->dest->set_sel, &dsids);
    if (ndest == 0) {
        sdest = nextset(gdest);
    } else if (ndest == 1) {
        sdest = dsids[0];
        xfree(dsids);
    } else {
        errmsg("Please select a single or none destination set");
        if (ndest > 0) {
            xfree(dsids);
        }
        if (nsrc > 0) {
            xfree(ssids);
        }
        return RETURN_FAILURE;
    }

    featext(gsrc, ssids, nsrc, gdest, sdest, formula); 

    if (nsrc > 0) {
        xfree(ssids);
    }
    
    update_set_lists(gdest);
    xdrawgraph();
    
    return RETURN_SUCCESS;
}

void create_featext_frame(void *data)
{
    static Featext_ui *feui = NULL;
 
    set_wait_cursor();
    
    if (!feui) {
        Widget grid;

        feui = xmalloc(sizeof(Featext_ui));
        if (!feui) {
            unset_wait_cursor();
            return;
        }
        
        feui->top = CreateDialogForm(app_shell, "Feature extraction");
        grid = CreateGrid(feui->top, 2, 1);
        AddDialogFormChild(feui->top, grid);
	feui->src = CreateGraphSetSelector(grid,
            "Extract from:", LIST_TYPE_MULTIPLE);
	feui->dest = CreateGraphSetSelector(grid,
            "Extract to:", LIST_TYPE_SINGLE);
        PlaceGridChild(grid, feui->src->frame, 0, 0);
        PlaceGridChild(grid, feui->dest->frame, 1, 0);

	feui->formula = CreateTextInput(feui->top, "Formula:");

	CreateAACDialog(feui->top, feui->formula->form, do_fext_proc, (void *) feui);
    }
    
    RaiseWindow(GetParent(feui->top));
    
    unset_wait_cursor();
}

typedef struct _Cumulative_ui {
    Widget top;
    GraphSetStructure *src;
    GraphSetStructure *dest;
} Cumulative_ui;

static int do_cumulative_proc(void *data)
{
    int nsrc, ndest, *ssids, *dsids, sdest, gsrc, gdest;

    Featext_ui *ui = (Featext_ui *) data;

    if (GetSingleListChoice(ui->src->graph_sel, &gsrc) != RETURN_SUCCESS) {
        errmsg("Please select a single source graph");
        return RETURN_FAILURE;
    }
    if (GetSingleListChoice(ui->dest->graph_sel, &gdest) != RETURN_SUCCESS) {
        errmsg("Please select a single source graph");
        return RETURN_FAILURE;
    }
    
    nsrc = GetListChoices(ui->src->set_sel, &ssids);
    if (nsrc < 1) {
        errmsg("No source sets selected");
        return RETURN_FAILURE;
    }

    ndest = GetListChoices(ui->dest->set_sel, &dsids);
    if (ndest == 0) {
        sdest = nextset(gdest);
    } else if (ndest == 1) {
        sdest = dsids[0];
        xfree(dsids);
    } else {
        errmsg("Please select a single or none destination set");
        if (ndest > 0) {
            xfree(dsids);
        }
        if (nsrc > 0) {
            xfree(ssids);
        }
        return RETURN_FAILURE;
    }

    cumulative(gsrc, ssids, nsrc, gdest, sdest); 

    if (nsrc > 0) {
        xfree(ssids);
    }
    
    update_set_lists(gdest);
    xdrawgraph();
    
    return RETURN_SUCCESS;
}

void create_cumulative_frame(void *data)
{
    static Cumulative_ui *ui = NULL;
 
    set_wait_cursor();
    
    if (!ui) {
        Widget grid;

        ui = xmalloc(sizeof(Cumulative_ui));
        if (!ui) {
            unset_wait_cursor();
            return;
        }
        
        ui->top = CreateDialogForm(app_shell, "Cumulative properties");
        grid = CreateGrid(ui->top, 2, 1);
	ui->src = CreateGraphSetSelector(grid,
            "Source group:", LIST_TYPE_MULTIPLE);
	ui->dest = CreateGraphSetSelector(grid,
            "Destination:", LIST_TYPE_SINGLE);
        PlaceGridChild(grid, ui->src->frame, 0, 0);
        PlaceGridChild(grid, ui->dest->frame, 1, 0);

	CreateAACDialog(ui->top, grid, do_cumulative_proc, (void *) ui);
    }
    
    RaiseWindow(GetParent(ui->top));
    
    unset_wait_cursor();
}
