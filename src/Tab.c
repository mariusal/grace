
/************************************************************************* 
 * Version 1.1  on  July-30-1997
 * (c) 1997 Pralay Dakua (pkanti@hotmail.com)
 *     
 * This is a free software and permission to use, modify, distribute,
 * selling and using for commercial purpose is hereby granted provided
 * that THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE
 * INCLUDED IN ALL COPIES AND THEIR SUPPORTING DOCUMENTATIONS.
 *
 * There is no warranty for this software. In no event Pralay Dakua
 * will be liable for merchantability and fitness of the software and 
 * damages due to this software.
 *
 * Author:
 * Pralay Dakua (pkanti@hotmail.com)
 *
 **************************************************************************/

#include "TabP.h"

static void Initialize(Widget,Widget,ArgList,Cardinal*);
static void Destroy(Widget);
static void Resize(Widget);
static void ReDisplay(Widget, XEvent*, Region);
static Boolean SetValues(Widget,Widget, Widget,ArgList,Cardinal*);

static XtGeometryResult QueryGeometry(Widget, XtWidgetGeometry*,
                                    XtWidgetGeometry*);
static XtGeometryResult GeometryManager(Widget, XtWidgetGeometry*,
                                    XtWidgetGeometry*);
static void ChangeManaged(Widget);
static void InsertChild(Widget);
static void DeleteChild(Widget);


static void Constraint_Initialize(Widget,Widget,ArgList,Cardinal*);
static Boolean Constraint_SetValues(Widget,Widget, Widget,ArgList,Cardinal*);
static void Constraint_Destroy(Widget);

static void Layout(XmTabWidget);

static void draw_tabs(XmTabWidget);
static void compute_tab_rects(XmTabWidget);
static Widget search_tab(XmTabWidget, XEvent*);
static void change_tab(XmTabWidget, Widget);
static void create_gc(XmTabWidget);
static void change_gc(XmTabWidget);
static void configure_children(XmTabWidget);
static void get_children_space(XmTabWidget, Dimension*, Dimension*, Dimension*);

void DrawShadowLines(Display*, Drawable, GC, XPoint*, int, Dimension);

#define offset(field) XtOffsetOf(XmTabRec, field)
#define ACTIONPROC(proc)  static void proc(Widget,XEvent*,String*,Cardinal*)

#define MANAGE(w)    w->core.managed = True
#define UNMANAGE(w)    w->core.managed = False

ACTIONPROC(ClickAction);

static XtResource resources[]={
	{
	XmNvalueChangedCallback,
	XmCValueChangedCallback,
	XtRCallback,
	sizeof(XtPointer),
	offset(tab.value_changed_callback),
	XtRCallback,
	NULL
	},
	{
	XmNfontList,
	XmCFontList,
	XmRFontList,
	sizeof(XmFontList),
	offset(tab.tab_font_list),
	XmRString,
	"fixed"
	},
	{
	XmNresizeChildren,
	XmCResizeChildren,
	XmRBoolean,
	sizeof(Boolean),
	offset(tab.resize_children),
	XmRImmediate,
	(XtPointer)True
	},
	{
	XmNshadowThickness,
	XmCShadowThickness,
	XmRDimension,
	sizeof(Dimension),
	offset(manager.shadow_thickness),
	XmRString,
	"2"
	},
	{
	XmNmarginWidth,
	XmCMarginWidth,
	XmRDimension,
	sizeof(Dimension),
	offset(tab.margin_width),
	XmRString,
	"3"
	},
	{
	XmNmarginHeight,
	XmCMarginHeight,
	XmRDimension,
	sizeof(Dimension),
	offset(tab.margin_height),
	XmRString,
	"3"
	}
};

static XtResource constraint_resources[]={
	{
	XmNtabLabel,
	XmCTabLabel,
	XmRXmString,
	sizeof(XmString),
	XtOffsetOf(XmTabConstraintRec, tab.tab_label),
	XmRImmediate,
	(XtPointer)NULL
	}
};

static XtActionsRec actions[] = {
	{"ClickAction",ClickAction}
};

static char translations[] =
	"<Btn1Down>: ClickAction()";


XmTabClassRec xmTabClassRec={
{ /***** core class ******/
(WidgetClass) &xmManagerClassRec,		/* super class */
"XmTab",					/* class name */
sizeof(XmTabRec),				/* widget size */
NULL,						/* class initialize */
NULL,						/* class part initialize */
False,						/* class inited */
(XtInitProc)Initialize,				/* initialize */
NULL,						/* initialize hook */
XtInheritRealize,				/* realize */
actions,					/* actions */
XtNumber(actions),				/* num actions */
resources,					/* resources */
XtNumber(resources),				/* num resources */
NULLQUARK,					/* xrm class */
True,						/* compress motion */
XtExposeCompressMultiple,			/* compress exposure */
False,						/* compress enter leave */
False,						/* visible interest */
(XtWidgetProc)Destroy,				/* destroy */
(XtWidgetProc)Resize,				/* resize */
(XtExposeProc)ReDisplay,			/* expose */
(XtSetValuesFunc)SetValues,			/* set values */
NULL,						/* set values hook */
XtInheritSetValuesAlmost,			/* set values almost */
NULL,						/* get values hook */
XtInheritAcceptFocus,			 	/* accept focus */
XtVersion,					/* version */
NULL,						/* callback private */
translations,					/* tm table */
(XtGeometryHandler)QueryGeometry,	 	/* query geometry */
XtInheritDisplayAccelerator,			/* display accelerator */
NULL						/* extension */
},
{ /***** composite class part *****/
(XtGeometryHandler)GeometryManager, 		/* geomerty manager */
(XtWidgetProc)ChangeManaged, 			/* change managed */
InsertChild, /* XtInheritInsertChild,	 insert child */
DeleteChild, /* XtInheritDeleteChild delete child */
NULL						/* extension */
},
{ /**** Constraint Class Part *****/
constraint_resources,				/* constraint resource list */
XtNumber(constraint_resources),			/* number of constraints in list */
sizeof(XmTabConstraintRec),			/* size of constraint record     */
(XtInitProc)Constraint_Initialize,		/* constraint initialization */
(XtWidgetProc)Constraint_Destroy,		/* constraint destroy proc */
(XtSetValuesFunc)Constraint_SetValues,		/* constraint set_values proc */
NULL						/* pointer to extension record */
},
{ /****** Manager Class Part *****/
NULL,						/* translations */
NULL,						/* syn_resources  */
0,						/* num_syn_resources */
NULL,						/* syn_constraint_resources */
0,					/* num_syn_constraint_resources */
XmInheritParentProcess,				/* parent_process */
NULL						/* extension */
},
{ /******* Tab Class Part ******/
NULL						/* extension */
}
};

WidgetClass xmTabWidgetClass=(WidgetClass) &xmTabClassRec;

static XmManagerClassRec *SuperClass = (XmManagerClassRec*) &xmManagerClassRec;

/*********************** Actions *******************************/


static void ClickAction(Widget w, XEvent *event,
                           String *params,Cardinal *nparams)
{
XmTabWidget wid = (XmTabWidget)w;
Widget tab_wid;
XmTabCallbackStruct call_data;
XmTabConstraintRec *tab_constraint;

	tab_wid = search_tab(wid, event);

	if(!tab_wid) return;

	change_tab(wid, tab_wid);
	draw_tabs(wid);

	tab_constraint = 
		(XmTabConstraintRec *) tab_wid->core.constraints;

	call_data.reason = XmCR_VALUE_CHANGED;
	call_data.tab_child = tab_wid;
	call_data.event = event;

	if( tab_constraint->tab.tab_label )
	{
		call_data.tab_label = 
			XmStringCopy(tab_constraint->tab.tab_label);
	}
	else
	{
		call_data.tab_label = 
			XmStringCreateLocalized(XtName(tab_wid));;
	}

	XtCallCallbacks(w, XmNvalueChangedCallback, &call_data);

	XmStringFree(call_data.tab_label);
}

/*********************** end of Actions **************************/

/********************** Core Class Methods **********************/

static void Initialize(Widget treq,Widget tnew,ArgList args,Cardinal *nargs)
{
XmTabWidget wid = (XmTabWidget) tnew;

	wid->tab.raise = 0;
	wid->tab.cut_size = 0;
	wid->tab.active_tab = NULL;
	create_gc(wid);
}

static void Resize(Widget w)
{
XmTabWidget wid = (XmTabWidget)w;

	Layout(wid);
	draw_tabs(wid);
}

static void ReDisplay(Widget w, XEvent *event, Region region)
{
XmTabWidget wid = (XmTabWidget)w;

	draw_tabs(wid);
}

static void Destroy(Widget w)
{
/*
 * XmTabWidget wid = (XmTabWidget)w;
 */
}

static Boolean SetValues(Widget current,Widget request, Widget new,
				ArgList args,Cardinal *nargs)
{
XmTabWidget curw = (XmTabWidget) current;
XmTabWidget neww = (XmTabWidget) new;
Boolean redraw = False;

	if((curw->tab.margin_width != neww->tab.margin_width)
		|| (curw->tab.margin_height != neww->tab.margin_height))
	{
		Layout(neww);
		redraw = True;
	}
	else if( curw->manager.shadow_thickness != neww->manager.shadow_thickness)
	{
		Layout(neww);
		redraw = True;
	}
	else if( curw->tab.tab_font_list != neww->tab.tab_font_list)
	{
		Layout(neww);
		redraw = True;
	}
	else if( curw->tab.resize_children != neww->tab.resize_children)
	{
		Layout(neww);
	}

	if(curw->manager.foreground != neww->manager.foreground)
	{
		change_gc(neww);
		redraw = True;
	}

return(redraw); 
}

static XtGeometryResult QueryGeometry(Widget w, 
			XtWidgetGeometry *request,
			XtWidgetGeometry *reply)
{
XmTabWidget wid = (XmTabWidget)w;
Dimension extra_width, extra_height;
Dimension width, height, rect_space;
Dimension own_width;
XtGeometryResult ret_val = XtGeometryNo;

	compute_tab_rects(wid);

	extra_width = 2*(wid->tab.margin_width + wid->manager.shadow_thickness +1);
	extra_height = wid->tab.tab_height +
		2*(wid->tab.margin_height + wid->manager.shadow_thickness +1);

	reply->request_mode = 0;

	if( request->request_mode & CWX )
		{
			reply->request_mode |= CWX;
			reply->x = request->x;
			ret_val = XtGeometryYes;
		}
	if( request->request_mode & CWY )
		{
			reply->request_mode |= CWY;
			reply->y = request->y;
			ret_val = XtGeometryYes;
		}

	if( request->request_mode & CWSibling )
		{
			reply->request_mode |= CWSibling;
			reply->sibling = request->sibling;
			ret_val = XtGeometryYes;
		}
	if( request->request_mode & CWBorderWidth )
		{
			reply->request_mode |= CWBorderWidth;
			reply->border_width = request->border_width;
			ret_val = XtGeometryYes;
		}
	if( request->request_mode & CWStackMode )
		{
			reply->request_mode |= CWStackMode;
			reply->stack_mode = request->stack_mode;
			ret_val = XtGeometryYes;
		}

	get_children_space(wid, &width, &height, &rect_space);

	rect_space = rect_space + wid->manager.shadow_thickness + 2*wid->tab.margin_width;

	own_width = width + extra_width;

	if(own_width < rect_space)
			own_width = rect_space;

	if( (request->request_mode & CWWidth) || (request->request_mode == 0))
		{
			reply->request_mode |= CWWidth;
			reply->width = own_width;
			ret_val = XtGeometryAlmost;
		}
	if( (request->request_mode & CWHeight) || (request->request_mode == 0))
		{
			reply->request_mode |= CWHeight;
			reply->height = height + extra_height;
			ret_val = XtGeometryAlmost;
		}

return(ret_val);
}

/********************** end of Core Class Methods ***************/

/********************** Composite Methods ***********************/

static XtGeometryResult GeometryManager(Widget w, 
			XtWidgetGeometry *request,
			XtWidgetGeometry *reply)
{
XmTabWidget wid = (XmTabWidget)w->core.parent;
XtGeometryResult ret_val = XtGeometryNo;
Dimension extra_width, extra_height;

	reply->request_mode = 0;

	if(request->request_mode & CWWidth)
	{
		extra_width = 2*(wid->tab.margin_width 
				+ wid->manager.shadow_thickness + 1);

		if(request->width > (wid->core.width - extra_width))
		{
			reply->width = wid->core.width - extra_width;
			ret_val = XtGeometryNo;
/*
 * 			reply->request_mode |= CWWidth;
 */
		}
		else
		{
			reply->width = request->width;
			ret_val = XtGeometryYes;
		}
			reply->request_mode |= CWWidth;
	}
	if(request->request_mode & CWHeight)
	{
		extra_height = wid->tab.tab_height + 2*(wid->tab.margin_height
				+ wid->manager.shadow_thickness + 1);

		if(request->height > (wid->core.height - extra_height))
		{
			reply->height = wid->core.height - extra_height;
			ret_val = XtGeometryNo;
		}
		else
		{
			reply->height = request->height;
			ret_val = XtGeometryYes;
		}
			reply->request_mode |= CWHeight;
	}

return(ret_val);
}

static void ChangeManaged(Widget w)
{
XmTabWidget wid = (XmTabWidget)w;
Widget child;
int i;

	for(i=0; i< wid->composite.num_children; i++)
	{
		child = wid->composite.children[i];
		child->core.mapped_when_managed = True;

		if(child == wid->tab.active_tab)
		{

			MANAGE(child);
		}
		else 
		{

			UNMANAGE(child);
		}
	}

Layout(wid);
draw_tabs(wid);
}

static void InsertChild(Widget w)
{
XmTabWidget wid = (XmTabWidget)w->core.parent;

	(*SuperClass->composite_class.insert_child)(w);

	if(wid->composite.num_children == 1)
		wid->tab.active_tab = w;
}

static void DeleteChild(Widget w)
{
XmTabWidget wid = (XmTabWidget)w->core.parent;

	(*SuperClass->composite_class.delete_child)(w);

	if( w == wid->tab.active_tab)
	{
		 if(wid->composite.num_children > 0)
		{
			change_tab(wid, wid->composite.children[0]);
		}
		else
		{
			wid->tab.active_tab = NULL;
		}
	}
}

static void Layout(XmTabWidget wid)
{
	compute_tab_rects(wid);
	configure_children(wid);
}

/********************** end of Composite Methods ***************/

/****************** Constraint Class Methods *******************/

static void Constraint_Initialize(Widget treq,Widget tnew,
			ArgList args,Cardinal *nargs)
{
XmTabConstraintRec *tab_const;
XmString temp_str;

 	tab_const = (XmTabConstraintRec *) tnew->core.constraints;

	if(tab_const->tab.tab_label)
	{
		temp_str = tab_const->tab.tab_label;
		tab_const->tab.tab_label = XmStringCopy(temp_str);
	}
}

static void Constraint_Destroy(Widget w)
{
XmTabConstraintRec *tab_const;

	tab_const = (XmTabConstraintRec *) w->core.constraints;

	if(tab_const->tab.tab_label)
		XmStringFree(tab_const->tab.tab_label);
}

static Boolean Constraint_SetValues(Widget current,Widget request, Widget new,
                                          ArgList args,Cardinal *nargs)
{
Boolean redraw = False; 
XmTabConstraintRec *tab_con_cur;
XmTabConstraintRec *tab_con_new;
XmString temp_str;
XmTabWidget tab_wid = (XmTabWidget)(new->core.parent);

 	tab_con_cur = 
		(XmTabConstraintRec *) current->core.constraints;
 	tab_con_new = 
		(XmTabConstraintRec *) new->core.constraints;

	if( !XmStringCompare(tab_con_cur->tab.tab_label,
				tab_con_new->tab.tab_label))
	{
		XmStringFree(tab_con_cur->tab.tab_label);
		temp_str = tab_con_new->tab.tab_label;
		tab_con_new->tab.tab_label = XmStringCopy(temp_str);
		Layout(tab_wid);
		redraw = True;
	}

return(redraw);
}

/****************** end of Constraint Class Methods ************/

/*************** local processing functions *****************/

static void configure_children(XmTabWidget wid)
{
int i;
Widget child;
Position x, y;
Dimension width, height;
Dimension own_width, own_height, own_width_ret, own_height_ret;
XtGeometryResult geo_result;
Dimension extra_width, extra_height;
Dimension rect_space;

	x = (Position)(wid->tab.margin_width 
				+ wid->manager.shadow_thickness + 1);
	y = (Position)(wid->tab.margin_height + wid->tab.tab_height 
					+ wid->manager.shadow_thickness + 1);

	for(i=0; i< wid->composite.num_children; i++)
	{
		child = wid->composite.children[i];

		XtMoveWidget(child, x, y);
	}

	get_children_space(wid, &width, &height, &rect_space);

	extra_width = 2*(wid->tab.margin_width 
					+ wid->manager.shadow_thickness + 1);
	extra_height = wid->tab.tab_height + 2*(wid->tab.margin_height 
					+ wid->manager.shadow_thickness + 1);
	own_width = own_width_ret = width + extra_width;
	own_height = own_height_ret = height + extra_height;

	rect_space = rect_space + wid->manager.shadow_thickness 
						+ 2*wid->tab.margin_width;

	if(rect_space > own_width)
	own_width = own_width_ret = rect_space;

	geo_result = XtMakeResizeRequest((Widget)wid, own_width, own_height,
					&own_width_ret, &own_height_ret);

	if(geo_result == XtGeometryYes)
	{
		width = ( own_width_ret > extra_width ) ? 
				(own_width_ret - extra_width) : 1;
		height = ( own_height_ret > extra_height ) ? 
				(own_height_ret - extra_height) : 1;
	}
	else
	{
		width = wid->core.width - extra_width;
		height = wid->core.height - extra_height;
	}


	if(wid->tab.resize_children)
	{
		for(i=0; i< wid->composite.num_children; i++)
		{
		child = wid->composite.children[i];
		XtResizeWidget(child, width, height, 0);
		}
	}
	else
	{
		for(i=0; i< wid->composite.num_children; i++)
		{
		child = wid->composite.children[i];
		if(child->core.width > width)
		XtResizeWidget(child, width, child->core.height, 0);
		if(child->core.height > height)
		XtResizeWidget(child, child->core.width, height, 0);
		}
	}
}

static void get_children_space(XmTabWidget wid, Dimension *width, Dimension *height, Dimension *rect_space)
{
XtGeometryResult geo_result;
XtWidgetGeometry reply;
int i;
Widget child;
XmTabConstraintRec *tab_const;

	*width = 1;
	*height = 1;
	*rect_space = 0;

	for(i=0; i< wid->composite.num_children; i++)
	{

		child = wid->composite.children[i];

		geo_result = XtQueryGeometry(child, NULL, &reply);

			if((reply.request_mode & CWWidth) 
						&& (reply.width > *width))
			{
				*width = reply.width;
			}
			if((reply.request_mode & CWHeight) 
						&& (reply.height > *height))
			{
				*height = reply.height;
			}

	 	tab_const = 
			(XmTabConstraintRec *) child->core.constraints;

		*rect_space = (Dimension)(tab_const->tab.tab_rect.x 
					+ tab_const->tab.tab_rect.width);

		XtResizeWidget(child, reply.width, reply.height, 0);
	}
}

static void change_gc(XmTabWidget wid)
{
	XtReleaseGC((Widget)wid, wid->tab.normal_gc);
	create_gc(wid);
}

static void create_gc(XmTabWidget wid)
{
    XGCValues gc_val;
    unsigned long gc_mask;
    XmFontContext font_context;
    XmFontType font_type;
    XmFontListEntry font_list_entry;
    XtPointer pointer;

    if (!XmFontListInitFontContext(&font_context, wid->tab.tab_font_list)) {
	XtAppErrorMsg(
	    XtWidgetToApplicationContext((Widget) wid),
	    "newFont", "badFont", "Tab",
	    "XmTab: XmFontListInitFontContext failed, bad fontList",
	    NULL, 0);
    }

    if ((font_list_entry = XmFontListNextEntry(font_context)) == NULL) {
	XtAppErrorMsg(
	    XtWidgetToApplicationContext((Widget) wid),
	    "newFont", "badFont", "Tab",
	    "XmTab: XmFontListNextEntry failed, no next fontList",
	    NULL, 0);
    }

    pointer = XmFontListEntryGetFont(font_list_entry, &font_type);

    gc_val.foreground = wid->manager.foreground;
    gc_val.background = wid->core.background_pixel;

    gc_mask = GCForeground | GCBackground;
    
    if (font_type == XmFONT_IS_FONT) {
        XFontStruct *font_struct = (XFontStruct *)pointer;
        gc_val.font = font_struct->fid;
	gc_mask |= GCFont;
    }

    wid->tab.normal_gc = XtGetGC((Widget)wid, gc_mask, &gc_val);

    XmFontListFreeFontContext(font_context);
}

static void change_tab(XmTabWidget wid, Widget new_tab)
{

	if(XtIsRealized(wid->tab.active_tab)) 
		XtUnmapWidget(wid->tab.active_tab);

	wid->tab.active_tab = new_tab;
	XtManageChild(new_tab);
}

static void compute_tab_rects(XmTabWidget wid)
{
Dimension str_width;
Dimension str_height;
short left_x, left_y;
unsigned short width, height;
int i;
XmTabConstraintRec *tab_constraint;
XmFontList font_list;
String name;
XmString xm_name;
Dimension greater_height;
short x_space, y_space;
XmString temp_str;
Widget child;

	left_x = wid->tab.margin_width;
	left_y = wid->tab.margin_height;

	font_list = wid->tab.tab_font_list;

	temp_str = XmStringCreateLocalized("0");	
	str_width = XmStringWidth(font_list, temp_str);
	str_height = XmStringHeight(font_list, temp_str);
	XmStringFree(temp_str);

	x_space = 2*str_width;
	y_space = str_height/2;

	greater_height = 0;

	for(i=0; i< wid->composite.num_children; i++)
	{
		child = wid->composite.children[i];

	 	tab_constraint = 
			(XmTabConstraintRec *) child->core.constraints;

		if(tab_constraint->tab.tab_label)
		{
			str_width = XmStringWidth(font_list, 
						tab_constraint->tab.tab_label);
			str_height = XmStringHeight(font_list, 
						tab_constraint->tab.tab_label);
		}
		else
		{
			name = XtName(child);
			xm_name = XmStringCreateLocalized(name);
			str_width = XmStringWidth(font_list, xm_name);
			str_height = XmStringHeight(font_list, xm_name);
			XmStringFree(xm_name);
		}


		width = (unsigned short)( str_width + x_space);
		height = (unsigned short)(str_height + y_space);

		  if( height > greater_height) greater_height = height;

		tab_constraint->tab.tab_rect.x = left_x;
		tab_constraint->tab.tab_rect.y = left_y;
		tab_constraint->tab.tab_rect.width = width;

		left_x += (short) width + wid->manager.shadow_thickness/2;
	}

		wid->tab.tab_height = greater_height;

	for(i=0; i< wid->composite.num_children; i++)
	{
		child = wid->composite.children[i];

	 	tab_constraint = 
			(XmTabConstraintRec *) child->core.constraints;

		tab_constraint->tab.tab_rect.height = greater_height;

	}

	wid->tab.cut_size = greater_height/8;
	wid->tab.raise = greater_height/10;

	if(wid->tab.raise > wid->tab.margin_height)
		wid->tab.raise = wid->tab.margin_height;
}

static void draw_tabs(XmTabWidget wid)
{
Widget w = (Widget)wid;
GC bright_gc, dark_gc;
XPoint point[4];
int i;
Widget child;
short x, y;
short width, height;
XmTabConstraintRec *tab_constraint;
Position str_x, str_y;
Dimension str_width;
Dimension str_height;
XmFontList font_list;
String name;
XmString xm_name = (XmString) NULL;

 if(!XtWindow(w)) return;

	   XClearWindow(XtDisplay(w), XtWindow(w));

	bright_gc = wid->manager.top_shadow_GC;
	dark_gc = wid->manager.bottom_shadow_GC;

	font_list = wid->tab.tab_font_list;

	for(i=0; i< wid->composite.num_children; i++)
	{
		child = wid->composite.children[i];

		if(wid->tab.active_tab == child)
		{
			continue;
		}

	 	tab_constraint = 
			(XmTabConstraintRec *) child->core.constraints;

		x = tab_constraint->tab.tab_rect.x;
		y = tab_constraint->tab.tab_rect.y;
		width = (short)tab_constraint->tab.tab_rect.width;
		height = (short)tab_constraint->tab.tab_rect.height;

		if(tab_constraint->tab.tab_label)
		{
			str_width = XmStringWidth(font_list, 
					tab_constraint->tab.tab_label);

			str_height = XmStringHeight(font_list, 
					tab_constraint->tab.tab_label);
		}
		else
		{
			name = XtName(child);
			xm_name = XmStringCreateLocalized(name);
			str_width = XmStringWidth(font_list, xm_name);
			str_height = XmStringHeight(font_list, xm_name);
		}

			str_x = x + wid->manager.shadow_thickness;
			str_y = y + (height - str_height)/2;
			str_width = width - 2*wid->manager.shadow_thickness;

		if(tab_constraint->tab.tab_label)
		{
			XmStringDraw(XtDisplay(wid),
					XtWindow(wid),
					font_list,
					tab_constraint->tab.tab_label,
					wid->tab.normal_gc,
					str_x, str_y,
					str_width,
					XmALIGNMENT_CENTER,
					XmSTRING_DIRECTION_L_TO_R,
					&(tab_constraint->tab.tab_rect));
		}
		else
		{
			XmStringDraw(XtDisplay(wid),
					XtWindow(wid),
					font_list,
					xm_name,
					wid->tab.normal_gc,
					str_x, str_y,
					str_width,
					XmALIGNMENT_CENTER,
					XmSTRING_DIRECTION_L_TO_R,
					&(tab_constraint->tab.tab_rect));
			XmStringFree(xm_name);
		}
		
		point[0].x = x;
		point[0].y = y + height;

		point[1].x = x;
		point[1].y = y + wid->tab.cut_size;

		point[2].x = x + wid->tab.cut_size;
		point[2].y = y;

		point[3].x = x + width - wid->tab.cut_size;
		point[3].y = y;

		DrawShadowLines(XtDisplay(w), XtWindow(w),
				bright_gc, point, 4,
				wid->manager.shadow_thickness); 

		point[0].x = x + width - wid->tab.cut_size;
		point[0].y = y;

		point[1].x = x + width;
		point[1].y = y + wid->tab.cut_size;

		point[2].x = x + width;
		point[2].y = y + height;

		DrawShadowLines(XtDisplay(w), XtWindow(w),
				dark_gc, point, 3,
				wid->manager.shadow_thickness); 
	}

	if(wid->composite.num_children == 0 || !wid->tab.active_tab) return;

	/*** draw the current active tab label 
		and its shade ***/

	child = wid->tab.active_tab;

	tab_constraint = 
		(XmTabConstraintRec *) child->core.constraints;

	x = tab_constraint->tab.tab_rect.x;
	y = tab_constraint->tab.tab_rect.y;
	width = (short)tab_constraint->tab.tab_rect.width;
	height = (short)tab_constraint->tab.tab_rect.height;

	if(tab_constraint->tab.tab_label)
	{
		str_width = XmStringWidth(font_list, 
				tab_constraint->tab.tab_label);

		str_height = XmStringHeight(font_list, 
				tab_constraint->tab.tab_label);
	}
	else
	{
		name = XtName(child);
		xm_name = XmStringCreateLocalized(name);
		str_width = XmStringWidth(font_list, xm_name);
		str_height = XmStringHeight(font_list, xm_name);
	}

		str_x = x + wid->manager.shadow_thickness;
		str_y = y + (height - str_height)/2;
		str_width = width - 2*wid->manager.shadow_thickness;

	if(tab_constraint->tab.tab_label)
	{
		XmStringDraw(XtDisplay(wid),
				XtWindow(wid),
				font_list,
				tab_constraint->tab.tab_label,
				wid->tab.normal_gc,
				str_x, str_y,
				str_width,
				XmALIGNMENT_CENTER,
				XmSTRING_DIRECTION_L_TO_R,
				&(tab_constraint->tab.tab_rect));
	}
	else
	{
		XmStringDraw(XtDisplay(wid),
				XtWindow(wid),
				font_list,
				xm_name,
				wid->tab.normal_gc,
				str_x, str_y,
				str_width,
				XmALIGNMENT_CENTER,
				XmSTRING_DIRECTION_L_TO_R,
				&(tab_constraint->tab.tab_rect));
		XmStringFree(xm_name);
	}
	
	point[0].x = x;
	point[0].y = y + height;

	point[1].x = x;
	point[1].y = y + wid->tab.cut_size - wid->tab.raise;

	point[2].x = x + wid->tab.cut_size;
	point[2].y = y - wid->tab.raise;

	point[3].x = x + width - wid->tab.cut_size;
	point[3].y = y - wid->tab.raise;

	DrawShadowLines(XtDisplay(w), XtWindow(w),
			bright_gc, point, 4,
			wid->manager.shadow_thickness); 

	point[0].x = x + width - wid->tab.cut_size;
	point[0].y = y - wid->tab.raise;

	point[1].x = x + width;
	point[1].y = y + wid->tab.cut_size - wid->tab.raise;

	point[2].x = x + width;
	point[2].y = y + height;

	DrawShadowLines(XtDisplay(w), XtWindow(w),
			dark_gc, point, 3,
			wid->manager.shadow_thickness); 

	/*** draw the border around the tab child ****/

	x = tab_constraint->tab.tab_rect.x;
	y = tab_constraint->tab.tab_rect.y;
	width = (short)tab_constraint->tab.tab_rect.width;
	height = (short)tab_constraint->tab.tab_rect.height;

	point[0].x = wid->tab.margin_width;
	point[0].y = wid->core.height - wid->tab.margin_height;

	point[1].x = wid->tab.margin_width;
	point[1].y = y + height;

	point[2].x = x;
	point[2].y = y + height;

	DrawShadowLines(XtDisplay(wid), XtWindow(wid),
			bright_gc, 
			point, 3,
			wid->manager.shadow_thickness);

	point[0].x = x + width;
	point[0].y = y + height;

	point[1].x = wid->core.width - wid->tab.margin_width;
	point[1].y = y + height;

	DrawShadowLines(XtDisplay(wid), XtWindow(wid),
			bright_gc, 
			point, 2,
			wid->manager.shadow_thickness);

	point[0].x = wid->core.width - wid->tab.margin_width;
	point[0].y = y + height;

	point[1].x = wid->core.width - wid->tab.margin_width;
	point[1].y = wid->core.height - wid->tab.margin_height;

	point[2].x = wid->tab.margin_width;
	point[2].y = wid->core.height - wid->tab.margin_height;

	DrawShadowLines(XtDisplay(wid), XtWindow(wid),
			dark_gc, 
			point, 3,
			wid->manager.shadow_thickness);

}

void DrawShadowLines(Display *display, Drawable drawable, GC gc,
			XPoint *points, int num_points,
			Dimension shad_thick)
{
XGCValues gc_val;
unsigned long gc_mask;
int line_width;

	/*** get the original linewidth and store it ***/
	gc_mask = GCLineWidth;
	XGetGCValues(display, gc, gc_mask, &gc_val);
	line_width = gc_val.line_width;
	
	/*** change the GC according to shadow thickness ***/
	gc_val.line_width = (int) shad_thick;
	XChangeGC(display, gc, gc_mask, &gc_val);

	/*** draw now ***/
	XDrawLines(display, drawable,
		gc, points, num_points, CoordModeOrigin); 

	/*** restore original line_width in GC ***/
        gc_val.line_width = line_width;
	XChangeGC(display, gc, gc_mask, &gc_val);
}

static Widget search_tab(XmTabWidget wid, XEvent *event) 
{
int i;
XmTabConstraintRec *tab_constraint;
int rect_x, rect_y;
int width, height;
int ev_x, ev_y;
Widget child;
	
	ev_x = event->xbutton.x;
	ev_y = event->xbutton.y;

	for(i=0; i< wid->composite.num_children; i++)
	{

		child = wid->composite.children[i];

		if( child == wid->tab.active_tab) continue;

	 	tab_constraint = 
			(XmTabConstraintRec *) child->core.constraints;

		rect_x = (int) tab_constraint->tab.tab_rect.x;
		rect_y = (int) tab_constraint->tab.tab_rect.y;
		width = (int) tab_constraint->tab.tab_rect.width;
		height = (int) tab_constraint->tab.tab_rect.height;

		if( ev_x >= rect_x 
			&& ev_y >= rect_y
			&& ev_x <= (rect_x + width)
			&& ev_y <= (rect_y + height))
		{
			if( ev_x < (rect_x + wid->tab.cut_size)
			   && ev_y < (rect_y + wid->tab.cut_size)
			   && (wid->tab.cut_size - (rect_x - ev_x)) < (ev_y - rect_y) )
			{
			return(child);
			}	
			else if( ev_x > (rect_x + width - wid->tab.cut_size)
			   && ev_y < (rect_y + wid->tab.cut_size)
			   && (wid->tab.cut_size - (rect_x + width - ev_x)) < (ev_y - rect_y) )
			{
			return(child);
			}
			else
			{
			return(child);
			}
		}
	}
	
return(NULL);
}

/************************* Public methods *********************************/

extern void XmTabSetTabWidget(Widget tab, Widget child, Boolean notify)
{
XmTabWidget wid = (XmTabWidget) tab;
int i;
Widget tab_wid;
XmTabCallbackStruct call_data;
XmTabConstraintRec *tab_constraint;

	/**** first be sure the mentioned widget is a valid child ***/

	tab_wid = NULL;

	for(i=0; i< wid->composite.num_children; i++)
	{
		if(wid->composite.children[i] == child)
		{
			tab_wid = child;
			break;
		}
	}

	if(!tab_wid) return;

	if(tab_wid == wid->tab.active_tab) return;
	
		change_tab(wid, tab_wid);
		draw_tabs(wid);

	if(notify)
	{
		call_data.reason = XmCR_VALUE_CHANGED;
		call_data.tab_child = tab_wid;
		call_data.event = NULL;

		tab_constraint = 
			(XmTabConstraintRec *) child->core.constraints;

		if( tab_constraint->tab.tab_label )
		{
		call_data.tab_label = 
			XmStringCopy(tab_constraint->tab.tab_label);
		}
		else
		{
		call_data.tab_label = 
			XmStringCreateLocalized(XtName(child));;
		}

		XtCallCallbacks((Widget)wid, XmNvalueChangedCallback, 
							&call_data);

		XmStringFree(call_data.tab_label);
	}
}

extern Widget XmTabGetTabWidget(Widget tab)
{
XmTabWidget wid = (XmTabWidget) tab;

	return(wid->tab.active_tab);
}

/************************* end of file **********************************/

