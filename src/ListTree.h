/*-----------------------------------------------------------------------------
 *
 * ListTree	A list widget that displays a file manager style tree
 *
 * Copyright (c) 1996 Robert W. McMullen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Author: Rob McMullen <rwmcm@mail.ae.utexas.edu>
 *         http://www.ae.utexas.edu/~rwmcm
 */

#ifndef _ListTree_H
#define _ListTree_H

#ifndef QT_GUI
#include <X11/Core.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#endif

#define _ListTree_WIDGET_VERSION	3.0

#define XtNmargin		"margin"
#define XtNindent		"indent"
#define XtNspacing		"spacing"
#define XtNhorizontalSpacing	"horizontalSpacing"
#define XtNverticalSpacing	"verticalSpacing"
#define XtNlineWidth		"lineWidth"
#define XtNtopItemPosition      "topItemPosition"
#define XtNvisibleItemCount     "visibleItemCount"
#define XtNhighlightPath	"highlightPath"
#define XtNclickPixmapToOpen	"clickPixmapToOpen"
#define XtNdoIncrementalHighlightCallback	"incCallback"
#define XtNbranchPixmap		"branchPixmap"
#define XtNbranchOpenPixmap	"branchOpenPixmap"
#define XtNleafPixmap		"leafPixmap"
#define XtNleafOpenPixmap	"leafOpenPixmap"
#define XtNactivateCallback	"activateCallback"
#define XtNhighlightCallback	"highlightCallback"
#define XtNmenuCallback		"menuCallback"
#define XtNdestroyItemCallback	"destroyItemCallback"
#define XtNdropCallback		"dropCallback"

#define XtBRANCH	1
#define XtLEAF		2
#define XtMENU		3
#define XtDESTROY	4
#define XtDROP		5

#ifdef __cplusplus
extern "C" {
#endif

#ifndef QT_GUI
extern WidgetClass listtreeWidgetClass;

typedef struct _ListTreeClassRec *ListTreeWidgetClass;
typedef struct _ListTreeRec      *ListTreeWidget;
#endif

typedef enum _ListTreeItemType {
    ItemDetermineType = 0,
    ItemBranchType = XtBRANCH,
    ItemLeafType = XtLEAF
} ListTreeItemType;

typedef struct _ListTreeItem {
  Boolean	open;
  Boolean	highlighted;
  char		*text;
  int		length;
  int		x,y,ytext;
  int		count;
#ifndef QT_GUI
  Dimension	height;
#else
  Widget widget;
#endif
  ListTreeItemType type;
  struct _ListTreeItem 	*parent,*firstchild,*prevsibling,*nextsibling;
  Pixmap	openPixmap,closedPixmap;
  XtPointer	user_data;
} ListTreeItem;

typedef struct _ListTreeReturnStruct {
  int		reason;
  ListTreeItem	*item;
  ListTreeItem	**path;
  int		count;
  Boolean	open;
} ListTreeReturnStruct;

typedef struct _ListTreeMultiReturnStruct {
  ListTreeItem	**items;
  int		count;
} ListTreeMultiReturnStruct;

typedef struct _ListTreeActivateStruct {
  int		reason;
  ListTreeItem	*item;
  Boolean	open;
  ListTreeItem	**path;
  int		count;
} ListTreeActivateStruct;

typedef struct _ListTreeItemReturnStruct {
  int		reason;
  ListTreeItem	*item;
#ifndef QT_GUI
  XEvent	*event;
#endif
} ListTreeItemReturnStruct;

typedef struct _ListTreeDropStruct {
  int		reason;
  ListTreeItem	*item;
  char		ok;
  char		operation;
} ListTreeDropStruct;

/*
** Public function declarations
*/

/* ListTree.c */
void ListTreeRefresh (Widget w);
void ListTreeRefreshOff (Widget w);
void ListTreeRefreshOn (Widget w);
ListTreeItem *ListTreeAdd (Widget w, ListTreeItem *parent, char *string);
ListTreeItem *ListTreeAddType (Widget w, ListTreeItem *parent, char *string, ListTreeItemType type);
ListTreeItem *ListTreeAddBranch (Widget w, ListTreeItem *parent, char *string);
ListTreeItem *ListTreeAddLeaf (Widget w, ListTreeItem *parent, char *string);
void ListTreeSetItemPixmaps (Widget w, ListTreeItem *item, Pixmap openPixmap, Pixmap closedPixmap);
void ListTreeRenameItem (Widget w, ListTreeItem *item, char *string);
int ListTreeDelete (Widget w, ListTreeItem *item);
int ListTreeDeleteChildren (Widget w, ListTreeItem *item);
int ListTreeReparent (Widget w, ListTreeItem *item, ListTreeItem *newparent);
int ListTreeReparentChildren (Widget w, ListTreeItem *item, ListTreeItem *newparent);
int ListTreeOrderSiblings (Widget w, ListTreeItem *item);
int ListTreeOrderChildren (Widget w, ListTreeItem *item);
int ListTreeUserOrderChildren (Widget w, ListTreeItem *item, int (*func)(const void *, const void *));
int ListTreeUserOrderSiblings (Widget w, ListTreeItem *item, int (*func)(const void *, const void *));
ListTreeItem *ListTreeFindSiblingName (Widget w, ListTreeItem *item, char *name);
ListTreeItem *ListTreeFindChildName (Widget w, ListTreeItem *item, char *name);
void ListTreeHighlightItem (Widget w, ListTreeItem *item);
void ListTreeHighlightItemMultiple(Widget w, ListTreeItem * item);
ListTreeItem *ListTreeFirstItem (Widget w);
void ListTreeClearHighlighted(Widget w);
void ListTreeGetHighlighted(Widget w,ListTreeMultiReturnStruct *ret);
void ListTreeSetHighlighted(Widget w,ListTreeItem **items,
			    int count,Boolean clear);

void ListTreeSetPos(Widget w, ListTreeItem *item);
void ListTreeSetBottomPos(Widget aw, ListTreeItem *item);

#ifndef QT_GUI
Widget XmCreateScrolledListTree (Widget parent, char *name, Arg *args, Cardinal count);
#endif

#ifdef __cplusplus
};
#endif

#endif /* _ListTree_H */
