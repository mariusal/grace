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
 * Page/Device setup
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include "globals.h"
#include "device.h"
#include "utils.h"
#include "plotone.h"

#include "motifinc.h"
#include "protos.h"


extern char print_file[];

static int current_page_units = 0;

static Widget psetup_frame;
static Widget psetup_rc;
static Widget device_opts_item;
static Widget *printto_item;
static Widget print_fileing_item;
static Widget rc_filesel;
static Widget printfile_item;
static Widget pdev_rc;
static Widget *devices_item;
static Widget current_dev_item;
static Widget *page_orient_item;
static Widget *page_format_item;
static Widget page_x_item;
static Widget page_y_item;
static Widget *page_size_unit_item;
static Widget dev_x_res_item;
static Widget dev_y_res_item;
static Widget fontaa_item;
static Widget devfont_item;

static void do_pr_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void do_format_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void do_orient_toggle(Widget w, XtPointer client_data, XtPointer call_data);

static void set_printer_proc(Widget w, XtPointer client_data, XtPointer call_data);
void create_printfiles_popup(Widget, XtPointer, XtPointer call_data);
void create_devopts_popup(Widget, XtPointer, XtPointer call_data);

static void do_device_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void do_units_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void update_printer_setup(int device_id);
static void update_device_setup(int device_id);

void create_printer_setup(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    Widget device_panel, rc, rc1, fr, wbut;
    
    set_wait_cursor();
    
    if (psetup_frame == NULL) {
	psetup_frame = XmCreateDialogShell(app_shell, "Device setup", NULL, 0);
	handle_close(psetup_frame);
        device_panel = XtVaCreateWidget("device_panel", xmFormWidgetClass, 
                                        psetup_frame, NULL, 0);
	psetup_rc = XmCreateRowColumn(device_panel, "psetup_rc", NULL, 0);

        fr = CreateFrame(psetup_rc, "Device setup");
        rc1 = XmCreateRowColumn(fr, "rc", NULL, 0);
	pdev_rc = XmCreateRowColumn(rc1, "pdev_rc", NULL, 0);
        XtVaSetValues(pdev_rc, XmNorientation, XmHORIZONTAL, NULL);

	devices_item = CreatePanelChoice(pdev_rc, "Device: ",
					 number_of_devices() + 1,
					 "Display",
					 "PostScript",
					 "EPS",
					 "Metafile",
#ifdef HAVE_LIBPDF
					 "PDF",
#endif
#ifdef HAVE_LIBGD
					 "GD",
					 "GIF",
					 "PNM",
#endif
					 0, 0);
	for (i = 0; i < number_of_devices(); i++) {
	    XtAddCallback(devices_item[2 + i], XmNactivateCallback,
			(XtCallbackProc) do_device_toggle, (XtPointer) i);
	}
        
        device_opts_item = XtVaCreateManagedWidget("Device options...",
                                                xmPushButtonWidgetClass, 
                                                pdev_rc, NULL);
	XtAddCallback(device_opts_item, XmNactivateCallback, 
                            (XtCallbackProc) create_devopts_popup,
                            (XtPointer) NULL);
        XtManageChild(pdev_rc);
        
	current_dev_item = CreateToggleButton(rc1, "Set as current print device");
        XtManageChild(rc1);
        
        fr = CreateFrame(psetup_rc, "Print options");
        rc1 = XmCreateRowColumn(fr, "rc", NULL, 0);
	printto_item = CreatePanelChoice(rc1, "Print to: ",
					 3,
					 "Printer",
					 "File", 0, 0);
	for (i = 0; i < 2; i++) {
	    XtAddCallback(printto_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_pr_toggle, (XtPointer) i);
	}

	print_fileing_item = CreateTextItem2(rc1, 25, "Print command:");

	rc_filesel = XmCreateRowColumn(rc1, "rc", NULL, 0);
        XtVaSetValues(rc_filesel, XmNorientation, XmHORIZONTAL, NULL);

	printfile_item = CreateTextItem2(rc_filesel, 20, "File name:");

	wbut = XtVaCreateManagedWidget("Browse...", xmPushButtonWidgetClass,
                                        rc_filesel, NULL);
	XtAddCallback(wbut, XmNactivateCallback, 
                            (XtCallbackProc) create_printfiles_popup,
                            (XtPointer) NULL);
	XtManageChild(rc_filesel);

	XtManageChild(rc1);
	
        fr = CreateFrame(psetup_rc, "Page");
        rc1 = XmCreateRowColumn(fr, "rc", NULL, 0);
        
	rc = XmCreateRowColumn(rc1, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        page_orient_item = CreatePanelChoice(rc, "Orientation: ",
					 3,
					 "Landscape",
					 "Portrait",
					 0, 0);
	for (i = 0; i < 2; i++) {
	    XtAddCallback(page_orient_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_orient_toggle, (XtPointer) i);
	}
	page_format_item = CreatePanelChoice(rc, "Size: ",
			      4,
			      "Custom",
			      "Letter",
			      "A4",
			      0, 0);
	for (i = 0; i < 3; i++) {
	    XtAddCallback(page_format_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_format_toggle, (XtPointer) i);
	}
        XtManageChild(rc);

	rc = XmCreateRowColumn(rc1, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        page_x_item = CreateTextItem2(rc, 6, "Dimensions:");
        page_y_item = CreateTextItem2(rc, 6, "x ");
	page_size_unit_item = CreatePanelChoice(rc, " ",
			                        4,
			                        "pix",
			                        "in",
			                        "cm",
			                        0, 0);
	for (i = 0; i < 3; i++) {
	    XtAddCallback(page_size_unit_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_units_toggle, (XtPointer) i);
	}
        SetChoice(page_size_unit_item, current_page_units);
        XtManageChild(rc);

	rc = XmCreateRowColumn(rc1, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        dev_x_res_item = CreateTextItem2(rc, 4, "Resolution (dpi):");
        dev_y_res_item = CreateTextItem2(rc, 4, "x ");
        XtManageChild(rc);

        XtManageChild(rc1);

        fr = CreateFrame(psetup_rc, "Fonts");
        rc1 = XmCreateRowColumn(fr, "rc", NULL, 0);
	fontaa_item = CreateToggleButton(rc1, "Enable font antialiasing");
	devfont_item = CreateToggleButton(rc1, "Use device fonts");
        XtManageChild(rc1);
        
	CreateSeparator(psetup_rc);

	CreateAACButtons(psetup_rc, device_panel, set_printer_proc);
	XtManageChild(psetup_rc);
	XtManageChild(device_panel);
    }
    
    XtRaise(psetup_frame);
    update_printer_setup(hdevice);
    unset_wait_cursor();
}

static void update_printer_setup(int device_id)
{
    if (psetup_frame) {
        SetChoice(devices_item, device_id);
        update_device_setup(device_id);
    }
}

static void update_device_setup(int device_id)
{
    char buf[GR_MAXPATHLEN];
    int buflen;
    int page_units;
    double page_x, page_y;
    
    Page_geometry pg;
    Device_entry dev;
    
    if (psetup_frame) {
	dev = get_device_props(device_id);
        pg = dev.pg;
        	
        if (dev.setup == NULL) {
            XtSetSensitive(device_opts_item, False);
        } else {
            XtSetSensitive(device_opts_item, True);
        }
        if (device_id == hdevice) {
            SetToggleButtonState(current_dev_item, TRUE);
            XtSetSensitive(current_dev_item, False);
        } else {
            SetToggleButtonState(current_dev_item, FALSE);
            XtSetSensitive(current_dev_item, True);
        }

   	strcpy(buf, mybasename(docname)); 
   	buflen = 0;
        while (buf[buflen] != '\0' && buf[buflen] != '.') {
            buflen++;
        }
        
        buf[buflen] = '.';
        buf[buflen + 1] = '\0';
        
        if (print_file == NULL || print_file[0] == '\0' || strstr(print_file, buf)) {
            strcat(buf, dev.fext);
            xv_setstr(printfile_item, buf);
        } else {
            xv_setstr(printfile_item, print_file);
        }
                
        xv_setstr(print_fileing_item, get_print_cmd());
        
        switch (dev.type) {
        case DEVICE_TERM:
            XtSetSensitive(current_dev_item, False);
            XtSetSensitive(printto_item[0], False);
            XtSetSensitive(XtParent(print_fileing_item), False);
            XtSetSensitive(rc_filesel, False);
            break;
        case DEVICE_FILE:
            SetChoice(printto_item, TRUE);
            XtSetSensitive(printto_item[0], False);
            XtSetSensitive(XtParent(print_fileing_item), False);
            XtSetSensitive(rc_filesel, True);
            break;
        case DEVICE_PRINT:
            SetChoice(printto_item, ptofile);
            XtSetSensitive(printto_item[0], True);
            if (ptofile == TRUE) {
                XtSetSensitive(rc_filesel, True);
                XtSetSensitive(XtParent(print_fileing_item), False);
            } else {
                XtSetSensitive(rc_filesel, False);
                XtSetSensitive(XtParent(print_fileing_item), True);
            }
            break;
        }
        
        if (pg.width < pg.height) {
            SetChoice(page_orient_item, PAGE_ORIENT_PORTRAIT);
        } else {
            SetChoice(page_orient_item, PAGE_ORIENT_LANDSCAPE);
        }
        
        /* Always assume custom */
        SetChoice(page_format_item, PAGE_FORMAT_CUSTOM); 
        XtSetSensitive(page_orient_item[0], False);
        
        sprintf (buf, "%.0f", pg.dpi_x); 
        xv_setstr(dev_x_res_item, buf);
        sprintf (buf, "%.0f", pg.dpi_y); 
        xv_setstr(dev_y_res_item, buf);
        
        page_units = GetChoice(page_size_unit_item);
        
        switch (page_units) {
        case 0:     /* pixels */
            page_x = (float) pg.width;
            page_y = (float) pg.height;
            break;
        case 1:      /* inches */
            page_x = (float) pg.width / pg.dpi_x;
            page_y = (float) pg.height / pg.dpi_y;
            break;
        case 2:      /* cm */ 
            page_x = (float) CM_PER_INCH * pg.width / pg.dpi_x;
            page_y = (float) CM_PER_INCH * pg.height / pg.dpi_y;
            break;
        default:
            errmsg("Internal error");
            return;
        }
        
        sprintf (buf, "%.2f", page_x); 
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", page_y); 
        xv_setstr(page_y_item, buf);
        
        SetToggleButtonState(fontaa_item, dev.fontaa);
        
        SetToggleButtonState(devfont_item, dev.devfonts);
    }
}

static void set_printer_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int aac_mode;
    int seldevice;
    double page_x, page_y;
    double dpi_x, dpi_y;
    int page_units;
    Device_entry dev;
    Page_geometry pg;
    
    aac_mode = (int) client_data;
    
    if (aac_mode == AAC_CLOSE) {
        XtUnmanageChild(psetup_frame);
        return;
    }
    
    seldevice = (int) GetChoice(devices_item);

    if (GetToggleButtonState(current_dev_item)) {
        hdevice = seldevice;
        ptofile = (int) GetChoice(printto_item);
        if (ptofile) {
            strcpy(print_file, xv_getstr(printfile_item));
        } else {
            set_print_cmd(xv_getstr(print_fileing_item));
        }
        XtSetSensitive(current_dev_item, False);
    }
    
    dev = get_device_props(seldevice);
    dev.devfonts = GetToggleButtonState(devfont_item);
    dev.fontaa = GetToggleButtonState(fontaa_item);
    
    if (xv_evalexpr(page_x_item, &page_x) != GRACE_EXIT_SUCCESS || 
        xv_evalexpr(page_y_item, &page_y) != GRACE_EXIT_SUCCESS  ) {
        errmsg("Invalid page dimension(s)");
        return;
    }

    if (xv_evalexpr(dev_x_res_item, &dpi_x) != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(dev_y_res_item, &dpi_y) != GRACE_EXIT_SUCCESS ) {
        errmsg("Invalid dpi");
        return;
    }

    page_units = GetChoice(page_size_unit_item);

    switch (page_units) {
    case 0: 
        pg.width =  (long) page_x;
        pg.height = (long) page_y;
        break;
    case 1: 
        pg.width =  (long) (page_x * dpi_x);
        pg.height = (long) (page_y * dpi_y);
        break;
    case 2: 
        pg.width =  (long) (page_x * dpi_x / CM_PER_INCH);
        pg.height = (long) (page_y * dpi_y / CM_PER_INCH);
        break;
    default:
        errmsg("Internal error");
        return;
    }
    
    pg.dpi_x = dpi_x;
    pg.dpi_y = dpi_y;
    
    dev.pg = pg;
    
    set_device_props(seldevice, dev);
    
    if (aac_mode == AAC_ACCEPT) {
        XtUnmanageChild(psetup_frame);
    }
    
    if (seldevice == tdevice) {
        drawgraph();
    }
}


/*
 * set the print options
 */
static void do_device_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{ 
    int device_id = (int) client_data;
    update_device_setup(device_id);
}

static void do_pr_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int value = (int) client_data;
    
    if (value == TRUE) {
        XtSetSensitive(rc_filesel, True);
        XtSetSensitive(XtParent(print_fileing_item), False);
    } else {
        XtSetSensitive(rc_filesel, False);
        XtSetSensitive(XtParent(print_fileing_item), True);
    }
}

static void do_format_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int value = (int) client_data;
    int orientation;
    int x, y;
    double px, py;
    int page_units;
    double dpi_x, dpi_y;
    char buf[32];
    
    if (value == PAGE_FORMAT_CUSTOM) {
        XtSetSensitive(page_x_item, True);
        XtSetSensitive(page_y_item, True);
        XtSetSensitive(page_orient_item[0], False);
    } else {
        XtSetSensitive(page_x_item, False);
        XtSetSensitive(page_y_item, False);
        XtSetSensitive(page_orient_item[0], True);
    }
    
    
    switch (value) {
    case PAGE_FORMAT_USLETTER:
        x = 612;
        y = 792;
        break;
    case PAGE_FORMAT_A4:
        x = 595;
        y = 842;
        break;
    case PAGE_FORMAT_CUSTOM:
    default:
        return;
    }

    
    page_units = GetChoice(page_size_unit_item);
    
    switch (page_units) {
    case 0:      /* pixels */
        if (xv_evalexpr(dev_x_res_item, &dpi_x) != GRACE_EXIT_SUCCESS ||
            xv_evalexpr(dev_y_res_item, &dpi_y) != GRACE_EXIT_SUCCESS ) {
            errmsg("Invalid dpi");
            return;
        }
        px = (float) x*dpi_x/72.0;
        py = (float) y*dpi_y/72.0;
        break;
    case 1:      /* inches */
        px = (float) x/72.0;
        py = (float) y/72.0;
        break;
    case 2:      /* cm */ 
        px = (float) x/72.0*CM_PER_INCH;
        py = (float) y/72.0*CM_PER_INCH;
        break;
    default:
        errmsg("Internal error");
        return;
    }
    
    orientation = GetChoice(page_orient_item);
    
    if ((orientation == PAGE_ORIENT_LANDSCAPE && px > py) ||
        (orientation == PAGE_ORIENT_PORTRAIT  && px < py) ) {
        sprintf (buf, "%.2f", px);
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", py);
        xv_setstr(page_y_item, buf);
    } else {
        sprintf (buf, "%.2f", py);
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", px);
        xv_setstr(page_y_item, buf);
    }
}

static void do_orient_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int orientation = (int) client_data;
    double px, py;
    char buf[32];

    if (xv_evalexpr(page_x_item, &px) != GRACE_EXIT_SUCCESS || 
        xv_evalexpr(page_y_item, &py) != GRACE_EXIT_SUCCESS ) {
        errmsg("Invalid page dimension(s)");
        return;
    }
    
    if ((orientation == PAGE_ORIENT_LANDSCAPE && px > py) ||
        (orientation == PAGE_ORIENT_PORTRAIT  && px < py) ) {
        sprintf (buf, "%.2f", px);
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", py);
        xv_setstr(page_y_item, buf);
    } else {
        sprintf (buf, "%.2f", py);
        xv_setstr(page_x_item, buf);
        sprintf (buf, "%.2f", px);
        xv_setstr(page_y_item, buf);
    }
}

static void do_prfilesel_proc(Widget w, XtPointer client_data,
                                        XtPointer call_data)
{
    Widget dialog = (Widget) client_data;
    char *s;
    XmFileSelectionBoxCallbackStruct *cbs =
        (XmFileSelectionBoxCallbackStruct *) call_data;

    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
        errwin("do_prfilesel_proc(): Error converting XmString to char string");
        return;
    }
    xv_setstr(printfile_item, s);
    strcpy(print_file, s);
    XtVaSetValues(printfile_item, XmNcursorPosition, strlen(s), NULL);
    XtFree(s);
    XtUnmanageChild(dialog);
}


void create_printfiles_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget top;
    int device;
    Device_entry dev;
    char buf[16];

    set_wait_cursor();
    if (top == NULL) {
        top = XmCreateFileSelectionDialog(app_shell, "prfilsel", NULL, 0);
        XtVaSetValues(XtParent(top), XmNtitle, "Select print file", NULL);
        XtAddCallback(top, XmNokCallback, (XtCallbackProc) do_prfilesel_proc,
                                                    (XtPointer) top);
        XtAddCallback(top, XmNcancelCallback, (XtCallbackProc) destroy_dialog,
                                                    (XtPointer) top);
    }
    
    device = GetChoice(devices_item);
    dev = get_device_props(device);
    sprintf(buf, "*.%s", dev.fext);
    XtVaSetValues(top, XmNdirMask, XmStringCreate(buf, charset), NULL );
    XtRaise(top);
    unset_wait_cursor();
}

void create_devopts_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    int device_id;
    Device_entry dev;
    
    device_id = GetChoice(devices_item);
    dev = get_device_props(device_id);
    if (dev.setup == NULL) {
        /* Should never come to here */
        errmsg("No options can be set for this device");
    } else {
        (dev.setup)();
    }
}

static void do_units_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    char buf[32];
    double page_x, page_y;
    double dev_x_res, dev_y_res;
    int page_units = (int) client_data;
    
    if (xv_evalexpr(page_x_item, &page_x) != GRACE_EXIT_SUCCESS || 
        xv_evalexpr(page_y_item, &page_y) != GRACE_EXIT_SUCCESS ) {
        errmsg("Invalid page dimension(s)");
        return;
    }
    
    if (xv_evalexpr(dev_x_res_item, &dev_x_res) != GRACE_EXIT_SUCCESS ||
        xv_evalexpr(dev_y_res_item, &dev_y_res) != GRACE_EXIT_SUCCESS ) {
        errmsg("Invalid device resolution(s)");
        return;
    }
    
    if (dev_x_res <= 0.0 || dev_y_res <= 0.0) {
        errmsg("Device resolution(s) <= 0");
        return;
    }
    
    if (current_page_units == page_units) {
        ;
    } else if (current_page_units == 0 && page_units == 1) {
        page_x /= dev_x_res;
        page_y /= dev_y_res;
    } else if (current_page_units == 0 && page_units == 2) {
        page_x /= (dev_x_res/CM_PER_INCH);
        page_y /= (dev_y_res/CM_PER_INCH);
    } else if (current_page_units == 1 && page_units == 0) {
        page_x *= dev_x_res;
        page_y *= dev_y_res;
    } else if (current_page_units == 1 && page_units == 2) {
        page_x *= CM_PER_INCH;
        page_y *= CM_PER_INCH;
    } else if (current_page_units == 2 && page_units == 0) {
        page_x *= (dev_x_res/CM_PER_INCH);
        page_y *= (dev_y_res/CM_PER_INCH);
    } else if (current_page_units == 2 && page_units == 1) {
        page_x /= CM_PER_INCH;
        page_y /= CM_PER_INCH;
    } else {
        errmsg("Internal error");
        return;
    }
        
    current_page_units = page_units;
    
    sprintf (buf, "%.2f", page_x); 
    xv_setstr(page_x_item, buf);
    sprintf (buf, "%.2f", page_y); 
    xv_setstr(page_y_item, buf);
}
