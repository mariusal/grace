/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2006 Grace Development Team
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

#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_CUPS
#  include <cups/cups.h>
#  include <cups/ppd.h>
#endif

#include "defines.h"
#include "graceapp.h"
#include "utils.h"
#include "core_utils.h"
#include "xprotos.h"

GUI *gui_new(GraceApp *gapp)
{
    GUI *gui;
    
    gui = xmalloc(sizeof(GUI));
    if (!gui) {
        return NULL;
    }
    memset(gui, 0, sizeof(GUI));
    
    gui->P = gapp;

    gui->zoom            = 1.0;

    gui->inwin           = FALSE;
    gui->invert          = TRUE;
    gui->focus_policy    = FOCUS_CLICK;
    gui->draw_focus_flag = TRUE;

    gui->crosshair_cursor = FALSE;

    gui->noask           = FALSE;

    gui->instant_update  = FALSE;
    gui->toolbar         = TRUE;
    gui->statusbar       = TRUE;
    gui->locbar          = TRUE;

    gui->install_cmap    = CMAP_INSTALL_AUTO;
    gui->private_cmap    = FALSE;

#if defined WITH_XMHTML
    gui->force_external_viewer = FALSE;
#else
    gui->force_external_viewer = TRUE;
#endif
    
    return gui;
}

void gui_free(GUI *gui)
{
    if (gui) {
        xfree(gui->xstuff);
        xfree(gui);
    }
}


RunTime *runtime_new(GraceApp *gapp)
{
    RunTime *rt;
    char *s;

    rt = xmalloc(sizeof(RunTime));
    if (!rt) {
        return NULL;
    }
    memset(rt, 0, sizeof(RunTime));

    rt->P = gapp;
    
    /* allocatables */
    rt->print_dests  = NULL;
    rt->print_cmd    = NULL;
    rt->gapp_editor  = NULL;
    rt->help_viewer  = NULL;
    rt->workingdir   = NULL;
    
#ifdef HAVE_CUPS
    rt->use_cups = TRUE;
#else
    rt->use_cups = FALSE;
#endif

    /* print command */
    if ((s = getenv("GRACE_PRINT_CMD")) == NULL) {
	s = bi_print_cmd();
    }
    rt->print_cmd = copy_string(NULL, s);
    if (rt->use_cups) {
        rt->ptofile = FALSE;
    } else
    /* if no print command defined, print to file by default */
    if (string_is_empty(rt->print_cmd)) {
        rt->ptofile = TRUE;
    } else {
        rt->ptofile = FALSE;
    }

    /* editor */
    if ((s = getenv("GRACE_EDITOR")) == NULL) {
	s = bi_editor();
    }
    rt->gapp_editor = copy_string(NULL, s);

    /* html viewer */
    if ((s = getenv("GRACE_HELPVIEWER")) == NULL) {
	s = bi_helpviewer();
    }
    rt->help_viewer = copy_string(NULL, s);
    if (!strstr(rt->help_viewer, "%s")) {
        rt->help_viewer = concat_strings(rt->help_viewer, " %s");
    }

    /* working directory */
    rt->workingdir = xmalloc(GR_MAXPATHLEN);
    if (!getcwd(rt->workingdir, GR_MAXPATHLEN - 1)) {
        runtime_free(rt);
        return NULL;
    }
    if (rt->workingdir[strlen(rt->workingdir)-1] != '/') {
        rt->workingdir = concat_strings(rt->workingdir, "/");
    }

    if (!rt->print_cmd    ||
        !rt->gapp_editor ||
        !rt->help_viewer  ||
        !rt->workingdir) {
        runtime_free(rt);
        return NULL;
    }
    
    if (gapp_init_print(rt) != RETURN_SUCCESS) {
        runtime_free(rt);
        return NULL;
    }

    rt->print_file[0] = '\0';

    rt->tdevice = 0;
    rt->hdevice = 0;

    rt->date_hint = FMT_nohint;
    
    rt->timer_delay = 200;

    rt->autoscale_onread = AUTOSCALE_XY;


    rt->scrollper = 0.05;
    rt->shexper = 0.05;
    
    rt->resfp = NULL;

    rt->emergency_save = FALSE;
    rt->interrupts = 0;

    return rt;
}

void runtime_free(RunTime *rt)
{
    int i;
    
    if (!rt) {
        return;
    }
    
    for (i = 0; i < rt->num_print_dests; i++) {
        PrintDest *pd = &rt->print_dests[i];
        int j;
        xfree(pd->name);
        xfree(pd->inst);
        xfree(pd->printer);
        for (j = 0; j < pd->nogroups; j++) {
            PrintOptGroup *og = &pd->ogroups[j];
            int k;
            xfree(og->name);
            xfree(og->text);
            for (k = 0; k < og->nopts; k++) {
                PrintOption *po = &og->opts[k];
                xfree(po->name);
                xfree(po->text);
                dict_free(po->choices);
            }
            xfree(og->opts);
        }
        xfree(pd->ogroups);
    }
    xfree(rt->print_dests);
    
    xfree(rt->print_cmd);
    xfree(rt->gapp_editor);
    xfree(rt->help_viewer);
    xfree(rt->workingdir);

    if (rt->resfp) {
        gapp_close(rt->resfp);
    }
    
    xfree(rt);
}

static void eval_proc(GVarType type, GVarData vardata, void *udata)
{
    unsigned int i;
    DArray *da;
    char buf[64];
    
    switch (type) {
    case GVarNil:
        stufftext("(nil)\n");
        break;
    case GVarNum:
        sprintf(buf, "%g\n", vardata.num);
        stufftext(buf);
        break;
    case GVarBool:
        stufftext(vardata.boolval ? "true":"false");
        stufftext("\n");
        break;
    case GVarArr:
        da = vardata.arr;
        stufftext("{");
        for (i = 0; da && i < da->size; i++) {
            sprintf(buf, " %g ", da->x[i]);
            stufftext(buf);
        }
        stufftext("}\n");
        break;
    case GVarStr:
        stufftext(vardata.str);
        stufftext("\n");
        break;
    default:
        errmsg("unknown data type");
        break;
    }
}

GraceApp *gapp_new(void)
{
    GraceApp *gapp;
    
    gapp = xmalloc(sizeof(GraceApp));
    if (!gapp) {
        return NULL;
    }
    memset(gapp, 0, sizeof(GraceApp));
    
    if (grace_init() != RETURN_SUCCESS) {
        gapp_free(gapp);
        return NULL;
    }

    gapp->grace = grace_new(bi_home());
    if (!gapp->grace) {
        gapp_free(gapp);
        return NULL;
    }
    graal_set_eval_proc(grace_get_graal(gapp->grace), eval_proc);

    grace_set_udata(gapp->grace, gapp);
    
    gapp->rt = runtime_new(gapp);
    if (!gapp->rt) {
        gapp_free(gapp);
        return NULL;
    }
    
    gapp->gui = gui_new(gapp);
    if (!gapp->gui) {
        gapp_free(gapp);
        return NULL;
    }

    gapp->pc = container_new(grace_get_qfactory(gapp->grace), AMEM_MODEL_SIMPLE);
    if (!gapp->pc) {
        gapp_free(gapp);
        return NULL;
    }

    return gapp;
}

void gapp_free(GraceApp *gapp)
{
    unsigned int i;

    if (!gapp) {
        return;
    }
    
    for (i = 0; i < gapp->gpcount; i++) {
        gproject_free(gapp->gplist[i]);
    }
    xfree(gapp->gplist);

    quark_free(gapp->pc);
    gui_free(gapp->gui);
    runtime_free(gapp->rt);
    grace_free(gapp->grace);
    
    xfree(gapp);
}

GraceApp *gapp_from_quark(const Quark *q)
{
    GraceApp *gapp = NULL;
    if (q) {
        Grace *grace = grace_from_quark(q);
        gapp = grace_get_udata(grace);
    }
    
    return gapp;
}

RunTime *rt_from_quark(const Quark *q)
{
    GraceApp *gapp = gapp_from_quark(q);
    if (gapp) {
        return gapp->rt;
    } else {
        return NULL;
    }
}

GUI *gui_from_quark(const Quark *q)
{
    GraceApp *gapp = gapp_from_quark(q);
    if (gapp) {
        return gapp->gui;
    } else {
        return NULL;
    }
}

GProject *gproject_from_quark(const Quark *q)
{
    unsigned int i;
    GraceApp *gapp;
    GProject *gp;
    Quark *project = get_parent_project(q);

    if (!project) {
        return NULL;
    }

    gapp = gapp_from_quark(project);

    for (i = 0; i < gapp->gpcount; i++) {
        gp = gapp->gplist[i];
        if (gproject_get_top(gp) == project) {
            return gp;
        }
    }

    return NULL;
}

int gapp_add_gproject(GraceApp *gapp, GProject *gp)
{
    void *p;

    if (!gapp || !gp) {
        return RETURN_FAILURE;
    }

    p = xrealloc(gapp->gplist, (gapp->gpcount + 1)*sizeof(GProject));
    if (!p) {
        return RETURN_FAILURE;
    }

    gapp->gplist = p;
    gapp->gplist[gapp->gpcount] = gp;
    gapp->gpcount++;

    return RETURN_SUCCESS;
}

int gapp_delete_gproject(GraceApp *gapp, GProject *gp)
{
    unsigned int i, j = 0;
    GProject **p;
    GProject *gpr;

    if (!gapp || !gp) {
        return RETURN_FAILURE;
    }

    p = xmalloc((gapp->gpcount - 1)*sizeof(GProject));
    if (!p) {
        return RETURN_FAILURE;
    }

    for (i = 0; i < gapp->gpcount; i++) {
        gpr = gapp->gplist[i];

        if (gpr != gp) {
            p[j] = gpr;
            j++;
        }
    }

    xfree(gapp->gplist);

    gapp->gplist = p;
    gapp->gpcount--;

    if (gapp->gp == gp) {
        gapp->gp = NULL;
    }

    gproject_free(gp);

    return RETURN_SUCCESS;
}

int gapp_set_active_gproject(GraceApp *gapp, GProject *gp)
{
    if (!gapp || !gp) {
        return RETURN_FAILURE;
    }

    if (gapp->gp) {
        quark_set_active2(gproject_get_top(gapp->gp), FALSE);
    }
    quark_set_active2(gproject_get_top(gp), TRUE);

    gapp->gp = gp;
    /* reset graal ? */

    /* Set dimensions of all devices */
    grace_sync_canvas_devices(gp);

    /* Reset set autocolorization index */
    gapp->rt->setcolor = 0;

    /* Request update of color selectors */
    gapp->gui->need_colorsel_update = TRUE;

    /* Request update of font selectors */
    gapp->gui->need_fontsel_update = TRUE;

    clean_graph_selectors(NULL, QUARK_ETYPE_DELETE, NULL);
    clean_frame_selectors(NULL, QUARK_ETYPE_DELETE, NULL);

    return RETURN_SUCCESS;
}

int gapp_set_gproject_id(GraceApp *gapp, GProject *gp, int id)
{
    Quark *q = gproject_get_top(gp);

    return quark_move2(q, quark_parent_get(q), id);
}

int gapp_get_gproject_id(GraceApp *gapp, GProject *gp)
{
    Quark *q = gproject_get_top(gp);

    return quark_get_id(q);
}

/*
 * flag to indicate destination of hardcopy output,
 * ptofile = 0 means print to printer, otherwise print to file
 */
void set_ptofile(GraceApp *gapp, int flag)
{
    gapp->rt->ptofile = flag;
}

int get_ptofile(const GraceApp *gapp)
{
    return gapp->rt->ptofile;
}

/*
 * set the current print device
 */
int set_printer(GraceApp *gapp, int device)
{
    Canvas *canvas = grace_get_canvas(gapp->grace);
    Device_entry *d = get_device_props(canvas, device);
    if (!d || d->type == DEVICE_TERM) {
        return RETURN_FAILURE;
    } else {
        gapp->rt->hdevice = device;
	if (d->type != DEVICE_PRINT) {
            set_ptofile(gapp, TRUE);
        }
        return RETURN_SUCCESS;
    }
}

int set_printer_by_name(GraceApp *gapp, const char *dname)
{
    Canvas *canvas = grace_get_canvas(gapp->grace);
    int device;
    
    device = get_device_by_name(canvas, dname);
    
    return set_printer(gapp, device);
}

int set_page_dimensions(GraceApp *gapp, int wpp, int hpp, int rescale)
{
    if (wpp <= 0 || hpp <= 0 || !gapp || !gapp->gp) {
        return RETURN_FAILURE;
    } else {
        int wpp_old, hpp_old;
        Project *pr = project_get_data(gproject_get_top(gapp->gp));
	if (!pr) {
            return RETURN_FAILURE;
        }
        
        wpp_old = pr->page_wpp;
	hpp_old = pr->page_hpp;
        
        pr->page_wpp = wpp;
	pr->page_hpp = hpp;
        if (rescale) {
            if (hpp*wpp_old - wpp*hpp_old != 0) {
                /* aspect ratio changed */
                double ext_x, ext_y;
                double old_aspectr, new_aspectr;
                
                old_aspectr = (double) wpp_old/hpp_old;
                new_aspectr = (double) wpp/hpp;
                if (old_aspectr >= 1.0 && new_aspectr >= 1.0) {
                    ext_x = new_aspectr/old_aspectr;
                    ext_y = 1.0;
                } else if (old_aspectr <= 1.0 && new_aspectr <= 1.0) {
                    ext_x = 1.0;
                    ext_y = old_aspectr/new_aspectr;
                } else if (old_aspectr >= 1.0 && new_aspectr <= 1.0) {
                    ext_x = 1.0/old_aspectr;
                    ext_y = 1.0/new_aspectr;
                } else {
                    ext_x = new_aspectr;
                    ext_y = old_aspectr;
                }

                rescale_viewport(gproject_get_top(gapp->gp), ext_x, ext_y);
            } 
        }

        grace_sync_canvas_devices(gapp->gp);

        return RETURN_SUCCESS;
    }
}

#ifdef HAVE_CUPS
static void parse_group(PrintDest *pd, ppd_group_t *group)
{
    int           i, j;           /* Looping vars */
    ppd_option_t  *option;        /* Current option */
    ppd_choice_t  *choice;        /* Current choice */
    ppd_group_t   *subgroup;      /* Current subgroup */
    PrintOptGroup *og;
    PrintOption   *po;

    pd->ogroups = xrealloc(pd->ogroups, (pd->nogroups + 1)*sizeof(PrintOptGroup));
    if (!pd->ogroups) {
        return;
    }
    
    og = &pd->ogroups[pd->nogroups];
    pd->nogroups++;
    
    og->name = copy_string(NULL, group->name);
    og->text = copy_string(NULL, group->text);
    
    og->opts = xcalloc(group->num_options, sizeof(PrintOption));
    if (!og->opts) {
        return;
    }
    og->nopts = 0;

    for (i = 0, option = group->options, po = og->opts; i < group->num_options; i++, option++) {
        po->name = copy_string(NULL, option->keyword);
        po->text = copy_string(NULL, option->text);

        po->choices = dict_new();
        po->selected = -1;
        dict_resize(po->choices, option->num_choices);
        for (j = 0, choice = option->choices; j < option->num_choices; j++, choice++) {
            DictEntry de;
            
            de.key   = j;
            de.name  = choice->choice;
            de.descr = choice->text;
            dict_entry_copy(&po->choices->entries[j], &de);

            if (choice->marked) {
                po->selected = de.key;
                dict_entry_copy(&po->choices->defaults, &de);
            }
        }
        if (po->selected != -1) {
            og->nopts++;
            po++;
            pd->nopts++;
        } else {
            xfree(po->name);
            xfree(po->text);
            dict_free(po->choices);
        }
    }

    for (i = 0, subgroup = group->subgroups; i < group->num_subgroups; i++, subgroup++) {
        parse_group(pd, subgroup);
    }
}
#endif

int gapp_init_print(RunTime *rt)
{
#ifdef HAVE_CUPS
    int i;
    cups_dest_t *dests;
    
    rt->print_dest = 0;
    
    rt->num_print_dests = cupsGetDests(&dests);
    if (!rt->num_print_dests) {
        /* no CUPS printers defined or CUPS not running */
        rt->use_cups = FALSE;
    } else {
        rt->print_dests = xcalloc(rt->num_print_dests, sizeof(PrintDest));
        if (rt->print_dests == NULL) {
            return RETURN_FAILURE;
        }
    }
    
    for (i = 0; i < rt->num_print_dests; i++) {
        cups_dest_t *dest = &dests[i];
        PrintDest *pd = &rt->print_dests[i];

        char *printer;
        int           j;
        const char    *filename;        /* PPD filename */
        ppd_file_t    *ppd;             /* PPD data */
        ppd_group_t   *group;           /* Current group */
        
        printer = copy_string(NULL, dest->name);
        if (dest->instance) {
            printer = concat_strings(printer, "/");
            printer = concat_strings(printer, dest->instance);
        }
        
        pd->name    = copy_string(NULL, dest->name);
        pd->inst    = copy_string(NULL, dest->instance);
        pd->printer = printer;
        
        if (dest->is_default) {
            rt->print_dest = i;
        }
        
        if ((filename = cupsGetPPD(dest->name)) == NULL) {
            continue;
        }

        if ((ppd = ppdOpenFile(filename)) == NULL) {
            remove(filename);
            continue;
        }

        ppdMarkDefaults(ppd);
        cupsMarkOptions(ppd, dest->num_options, dest->options);

        for (j = 0, group = ppd->groups; j < ppd->num_groups; j++, group++) {
            parse_group(pd, group);
        }

        ppdClose(ppd);
        remove(filename);
    }
    cupsFreeDests(rt->num_print_dests, dests);
#else
    rt->print_dest = 0;
    rt->num_print_dests = 0;
#endif

    return RETURN_SUCCESS;
}

int gapp_print(const GraceApp *gapp, const char *fname)
{
    char tbuf[128];
    int retval = RETURN_SUCCESS;
#ifdef HAVE_CUPS
    if (gapp->rt->use_cups) {
        PrintDest *pd = &gapp->rt->print_dests[gapp->rt->print_dest];
        int i, j;
        int jobid;
        int num_options = 0;
        cups_option_t *options = NULL;

        for (i = 0; i < pd->nogroups; i++) {
            PrintOptGroup *og = &pd->ogroups[i];

            for (j = 0; j < og->nopts; j++) {
                PrintOption *po = &og->opts[j];
                char *value;

                if (po->selected != po->choices->defaults.key) {
                    dict_get_name_by_key(po->choices, po->selected, &value);
                    num_options = cupsAddOption(po->name, value, num_options, &options);
                }
            }
        }
        
        jobid = cupsPrintFile(pd->name, fname, "Grace", num_options, options);
        if (jobid == 0) {
            errmsg(ippErrorString(cupsLastError()));
            retval = RETURN_FAILURE;
        }
    } else
#endif
    {
        sprintf(tbuf, "%s %s", get_print_cmd(gapp), fname);
        system_wrap(tbuf);
    }
    
    return retval;
}

#define VP_EPSILON  0.001

/*
 * If writing to a file, check to see if it exists
 */
void do_hardcopy(const GProject *gp)
{
    Quark *project = gproject_get_top(gp);
    GraceApp *gapp = gapp_from_quark(project);
    RunTime *rt;
    Canvas *canvas;
    char fname[GR_MAXPATHLEN];
    view v;
    double vx, vy;
    int truncated_out, res;
    FILE *prstream;
    
    if (!gapp) {
        return;
    }
    
    rt = gapp->rt;
    canvas = grace_get_canvas(gapp->grace);
    
    if (get_ptofile(gapp)) {
        if (string_is_empty(rt->print_file)) {
            Device_entry *dev = get_device_props(canvas, rt->hdevice);
            sprintf(rt->print_file, "%s.%s",
                QIDSTR(project), dev->fext);
        }
        strcpy(fname, rt->print_file);
        prstream = gapp_openw(gapp, fname);
    } else {
        strcpy(fname, "gappXXXXXX");
        prstream = gapp_tmpfile(fname);
    }
    
    if (prstream == NULL) {
        return;
    }
    
    canvas_set_prstream(canvas, prstream); 
    
    select_device(canvas, rt->hdevice);
    
    res = gproject_render(gp);
    
    gapp_close(prstream);
    
    if (res != RETURN_SUCCESS) {
        return;
    }
    
    get_bbox(canvas, BBOX_TYPE_GLOB, &v);
    project_get_viewport(project, &vx, &vy);
    if (v.xv1 < 0.0 - VP_EPSILON || v.xv2 > vx + VP_EPSILON ||
        v.yv1 < 0.0 - VP_EPSILON || v.yv2 > vy + VP_EPSILON) {
        truncated_out = TRUE;
    } else {
        truncated_out = FALSE;
    }
    
    if (get_ptofile(gapp) == FALSE) {
        if (truncated_out == FALSE ||
            yesno("Printout is truncated. Continue?", NULL, NULL, NULL)) {
            gapp_print(gapp, fname);
#ifndef PRINT_CMD_UNLINKS
            remove(fname);
#endif
        }
    } else {
        if (truncated_out == TRUE) {
            errmsg("Output is truncated - tune device dimensions");
        }
    }
}

int gui_is_page_free(const GUI *gui)
{
    return gui->page_free;
}

void gui_set_page_free(GUI *gui, int onoff)
{
    if (gui->page_free == onoff) {
        return;
    }
    
    if (gui->inwin) {
        errmsg("Can not change layout after initialization of GUI");
        return;
    } else {
        gui->page_free = onoff;
    }
}

void gui_set_barebones(GUI *gui)
{
    gui->locbar    = FALSE;
    gui->toolbar   = FALSE;
    gui->statusbar = FALSE;
}
