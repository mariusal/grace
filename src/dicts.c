/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2001-2005 Grace Development Team
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
 * Misc stuff using dictionaries
 */

#include <config.h>

#include "grace.h"
#include "defines.h"
#include "core_utils.h"
#include "dicts.h"
#include "xstrings.h"

int grace_rt_init_dicts(RunTime *rt)
{
    /* Dictionary tables */
    const DictEntry graph_type_defaults =
        {GRAPH_BAD, "unknown", "Unknown"};
    const DictEntry graph_type_entries[] = 
        {
            {GRAPH_XY,    "xy",    "XY"   },
            {GRAPH_CHART, "chart", "Chart"},
            {GRAPH_POLAR, "polar", "Polar"},
            {GRAPH_SMITH, "smith", "Smith"},
            {GRAPH_FIXED, "fixed", "Fixed"},
            {GRAPH_PIE,   "pie",   "Pie"  }
        };
    
    const DictEntry set_type_defaults =
        {SET_BAD, "unknown", "Unknown"};
    const DictEntry set_type_entries[] = 
        {
            {SET_XY,         "xy",         "XY"        },
            {SET_XYDX,       "xydx",       "XYdX"      },
            {SET_XYDY,       "xydy",       "XYdY"      },
            {SET_XYDXDX,     "xydxdx",     "XYdXdX"    },
            {SET_XYDYDY,     "xydydy",     "XYdYdY"    },
            {SET_XYDXDY,     "xydxdy",     "XYdXdY"    },
            {SET_XYDXDXDYDY, "xydxdxdydy", "XYdXdXdYdY"},
            {SET_BAR,        "bar",        "Bar"       },
            {SET_BARDY,      "bardy",      "BardY"     },
            {SET_BARDYDY,    "bardydy",    "BardYdY"   },
            {SET_XYHILO,     "xyhilo",     "XYHiLo"    },
            {SET_XYZ,        "xyz",        "XYZ"       },
            {SET_XYR,        "xyr",        "XYR"       },
            {SET_XYSIZE,     "xysize",     "XYSize"    },
            {SET_XYCOLOR,    "xycolor",    "XYColor"   },
            {SET_XYCOLPAT,   "xycolpat",   "XYColPat"  },
            {SET_XYVMAP,     "xyvmap",     "XYVMap"    },
            {SET_BOXPLOT,    "boxplot",    "BoxPlot"   }
        };

    const DictEntry object_type_defaults =
        {DO_NONE, VStrNone, "Unknown"};
    const DictEntry object_type_entries[] = 
        {
            {DO_LINE, VStrLine, "Line"},
            {DO_BOX,  VStrBox,  "Box" },
            {DO_ARC,  VStrArc,  "Arc" }
        };

    const DictEntry inout_placement_defaults =
        {TICKS_IN, VStrIn, "In"};
    const DictEntry inout_placement_entries[] = 
        {
            {TICKS_IN,   VStrIn,   "In"  },
            {TICKS_OUT,  VStrOut,  "Out" },
            {TICKS_BOTH, VStrBoth, "Both"}
        };

    const DictEntry side_placement_defaults =
        {PLACEMENT_NORMAL, VStrNormal, "Normal"};
    const DictEntry side_placement_entries[] = 
        {
            {PLACEMENT_NORMAL,   VStrNormal,   "Normal"  },
            {PLACEMENT_OPPOSITE, VStrOpposite, "Opposite"},
            {PLACEMENT_BOTH,     VStrBoth,     "Both"    }
        };

    const DictEntry axis_position_defaults =
        {AXIS_POS_NORMAL, VStrNormal, "Normal"};
    const DictEntry axis_position_entries[] = 
        {
            {AXIS_POS_NORMAL,   VStrNormal,   "Normal"  },
            {AXIS_POS_OPPOSITE, VStrOpposite, "Opposite"},
            {AXIS_POS_ZERO,     VStrZero,     "Zero"    }
        };

    const DictEntry spec_ticks_defaults =
        {TICKS_SPEC_NONE, VStrNone, "None"};
    const DictEntry spec_ticks_entries[] = 
        {
            {TICKS_SPEC_NONE,  VStrNone,  "None"                 },
            {TICKS_SPEC_MARKS, VStrTicks, "Tick marks"           },
            {TICKS_SPEC_BOTH,  VStrBoth,  "Tick marks and labels"}
        };

    const DictEntry region_type_defaults =
        {REGION_POLYGON, VStrPolygon, "Polygon"};
    const DictEntry region_type_entries[] = 
        {
            {REGION_POLYGON, VStrPolygon, "Polygon"},
            {REGION_BAND,    VStrBand,    "Band"   },
            {REGION_FORMULA, VStrFormula, "Formula"}
        };

    const DictEntry arrow_type_defaults =
        {ARROW_TYPE_LINE, VStrLine, "Line"};
    const DictEntry arrow_type_entries[] = 
        {
            {ARROW_TYPE_LINE,   VStrLine,   "Line"},
            {ARROW_TYPE_FILLED, VStrFilled, "Filled"},
            {ARROW_TYPE_CIRCLE, VStrCircle, "Circle"}
        };

    const DictEntry glocator_type_defaults =
        {GLOCATOR_TYPE_NONE, VStrNone, "None"};
    const DictEntry glocator_type_entries[] = 
        {
            {GLOCATOR_TYPE_NONE,  VStrNone,  "None" },
            {GLOCATOR_TYPE_XY,    VStrXY,    "XY"   },
            {GLOCATOR_TYPE_POLAR, VStrPolar, "Polar"}
        };

    const DictEntry sym_type_defaults =
        {SYM_NONE, VStrNone, "None"};
    const DictEntry sym_type_entries[] = 
        {
            {SYM_NONE,    VStrNone,          "None"          },
            {SYM_CIRCLE,  VStrCircle,        "Circle"        },
            {SYM_SQUARE,  VStrSquare,        "Square"        },
            {SYM_DIAMOND, VStrDiamond,       "Diamond"       },
            {SYM_TRIANG1, VStrTriangleUp,    "Triangle up"   },
            {SYM_TRIANG2, VStrTriangleLeft,  "Triangle left" },
            {SYM_TRIANG3, VStrTriangleDown,  "Triangle down" },
            {SYM_TRIANG4, VStrTriangleRight, "Triangle right"},
            {SYM_PLUS,    VStrPlus,          "Plus"          },
            {SYM_X,       VStrX,             "X"             },
            {SYM_SPLAT,   VStrSplat,         "Splat"         },
            {SYM_CHAR,    VStrChar,          "Character"     }
        };

    const DictEntry line_type_defaults =
        {LINE_TYPE_NONE, VStrNone, "None"};
    const DictEntry line_type_entries[] = 
        {
            {LINE_TYPE_NONE,       VStrNone,        "None"        },
            {LINE_TYPE_STRAIGHT,   VStrStraight,    "Straight"    },
            {LINE_TYPE_LEFTSTAIR,  VStrLeftStairs,  "Left stairs" },
            {LINE_TYPE_RIGHTSTAIR, VStrRightStairs, "Right stairs"},
            {LINE_TYPE_SEGMENT2,   VStrSegment,     "Segment"     }, 
            {LINE_TYPE_SEGMENT3,   VStrSegment3,    "3-Segment"   }
        };

    const DictEntry setfill_type_defaults =
        {SETFILL_NONE, VStrNone, "None"};
    const DictEntry setfill_type_entries[] = 
        {
            {SETFILL_NONE,     VStrNone,     "None"       },
            {SETFILL_POLYGON,  VStrPolygon,  "As polygon" },
            {SETFILL_BASELINE, VStrBaseline, "To baseline"}
        };

    const DictEntry baseline_type_defaults =
        {BASELINE_TYPE_0, VStrZero, "Zero"};
    const DictEntry baseline_type_entries[] = 
        {
            {BASELINE_TYPE_0,    VStrZero,     "Zero"     },
            {BASELINE_TYPE_SMIN, VStrSetMin,   "Set min"  },
            {BASELINE_TYPE_SMAX, VStrSetMax,   "Set max"  },
            {BASELINE_TYPE_GMIN, VStrGraphMin, "Graph min"},
            {BASELINE_TYPE_GMAX, VStrGraphMax, "Graph max"}
        };

    const DictEntry framedecor_type_defaults =
        {FRAME_DECOR_NONE, VStrNone, "None"};
    const DictEntry framedecor_type_entries[] = 
        {
            {FRAME_DECOR_NONE, VStrNone,      "None"},
            {FRAME_DECOR_LINE, VStrLine,      "Underline"},
            {FRAME_DECOR_RECT, VStrRectangle, "Rectangle"},
            {FRAME_DECOR_OVAL, VStrOval,      "Oval"}
        };

    const DictEntry scale_type_defaults =
        {SCALE_NORMAL, VStrNormal, "Normal"};
    const DictEntry scale_type_entries[] = 
        {
            {SCALE_NORMAL, VStrNormal,      "Normal"     },
            {SCALE_LOG,    VStrLogarithmic, "Logarithmic"},
            {SCALE_REC,    VStrReciprocal,  "Reciprocal" },
            {SCALE_LOGIT,  VStrLogit,       "Logit"      }
        };

    const DictEntry arrow_placement_defaults =
        {ARROW_AT_NONE, VStrNone, "None"};
    const DictEntry arrow_placement_entries[] = 
        {
            {ARROW_AT_NONE,      VStrNone,      "None"     },
            {ARROW_AT_BEGINNING, VStrBeginning, "Beginning"},
            {ARROW_AT_END,       VStrEnd,       "End"      },
            {ARROW_AT_BOTH,      VStrBoth,      "Both"     }
        };

    if (!(rt->graph_type_dict =
        DICT_NEW_STATIC(graph_type_entries, &graph_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->set_type_dict =
        DICT_NEW_STATIC(set_type_entries, &set_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->object_type_dict =
        DICT_NEW_STATIC(object_type_entries, &object_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->inout_placement_dict =
        DICT_NEW_STATIC(inout_placement_entries, &inout_placement_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->side_placement_dict =
        DICT_NEW_STATIC(side_placement_entries, &side_placement_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->axis_position_dict =
        DICT_NEW_STATIC(axis_position_entries, &axis_position_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->spec_ticks_dict =
        DICT_NEW_STATIC(spec_ticks_entries, &spec_ticks_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->region_type_dict =
        DICT_NEW_STATIC(region_type_entries, &region_type_defaults))) {
        return RETURN_FAILURE;
    }
    
    if (!(rt->arrow_type_dict =
        DICT_NEW_STATIC(arrow_type_entries, &arrow_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->glocator_type_dict =
        DICT_NEW_STATIC(glocator_type_entries, &glocator_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->sym_type_dict =
        DICT_NEW_STATIC(sym_type_entries, &sym_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->line_type_dict =
        DICT_NEW_STATIC(line_type_entries, &line_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->setfill_type_dict =
        DICT_NEW_STATIC(setfill_type_entries, &setfill_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->baseline_type_dict =
        DICT_NEW_STATIC(baseline_type_entries, &baseline_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->framedecor_type_dict =
        DICT_NEW_STATIC(framedecor_type_entries, &framedecor_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->scale_type_dict =
        DICT_NEW_STATIC(scale_type_entries, &scale_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(rt->arrow_placement_dict =
        DICT_NEW_STATIC(arrow_placement_entries, &arrow_placement_defaults))) {
        return RETURN_FAILURE;
    }

    return RETURN_SUCCESS;
}

void grace_rt_free_dicts(RunTime *rt)
{
    dict_free(rt->graph_type_dict);
    dict_free(rt->set_type_dict);
    dict_free(rt->object_type_dict);
    dict_free(rt->inout_placement_dict);
    dict_free(rt->side_placement_dict);
    dict_free(rt->axis_position_dict);
    dict_free(rt->spec_ticks_dict);
    dict_free(rt->region_type_dict);
    dict_free(rt->arrow_type_dict);
    dict_free(rt->glocator_type_dict);
    dict_free(rt->sym_type_dict);
    dict_free(rt->line_type_dict);
    dict_free(rt->setfill_type_dict);
    dict_free(rt->baseline_type_dict);
    dict_free(rt->framedecor_type_dict);
    dict_free(rt->scale_type_dict);
    dict_free(rt->arrow_placement_dict);
}

char *graph_types(RunTime *rt, GraphType it)
{
    char *s;
    
    dict_get_name_by_key(rt->graph_type_dict, it, &s);
    
    return s;
}

char *graph_type_descr(RunTime *rt, GraphType it)
{
    char *s;
    
    dict_get_descr_by_key(rt->graph_type_dict, it, &s);
    
    return s;
}

GraphType graph_get_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->graph_type_dict, name, &retval);
    
    return retval;
}


char *set_types(RunTime *rt, SetType it)
{
    char *s;
    
    dict_get_name_by_key(rt->set_type_dict, it, &s);
    
    return s;
}

char *set_type_descr(RunTime *rt, SetType it)
{
    char *s;
    
    dict_get_descr_by_key(rt->set_type_dict, it, &s);
    
    return s;
}

SetType get_settype_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->set_type_dict, name, &retval);
    
    return retval;
}

char *object_types(RunTime *rt, OType it)
{
    char *s;
    
    dict_get_name_by_key(rt->object_type_dict, it, &s);
    
    return s;
}

char *object_type_descr(RunTime *rt, OType it)
{
    char *s;
    
    dict_get_descr_by_key(rt->object_type_dict, it, &s);
    
    return s;
}

OType get_objecttype_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->object_type_dict, name, &retval);
    
    return retval;
}

char *inout_placement_name(RunTime *rt, int inout)
{
    char *s;
    
    dict_get_name_by_key(rt->inout_placement_dict, inout, &s);
    
    return s;
}

int get_inout_placement_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->inout_placement_dict, name, &retval);
    
    return retval;
}

char *side_placement_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->side_placement_dict, it, &s);
    
    return s;
}

int get_side_placement_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->side_placement_dict, name, &retval);
    
    return retval;
}

char *spec_tick_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->spec_ticks_dict, it, &s);
    
    return s;
}

int get_spec_tick_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->spec_ticks_dict, name, &retval);
    
    return retval;
}


char *region_types(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->region_type_dict, it, &s);
    
    return s;
}

int get_regiontype_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->region_type_dict, name, &retval);
    
    return retval;
}

char *axis_position_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->axis_position_dict, it, &s);
    
    return s;
}

int get_axis_position_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->axis_position_dict, name, &retval);
    
    return retval;
}

char *arrow_type_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->arrow_type_dict, it, &s);
    
    return s;
}

int get_arrow_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->arrow_type_dict, name, &retval);
    
    return retval;
}

char *glocator_type_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->glocator_type_dict, it, &s);
    
    return s;
}

int get_glocator_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->glocator_type_dict, name, &retval);
    
    return retval;
}

char *sym_type_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->sym_type_dict, it, &s);
    
    return s;
}

int get_sym_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->sym_type_dict, name, &retval);
    
    return retval;
}

char *line_type_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->line_type_dict, it, &s);
    
    return s;
}

int get_line_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->line_type_dict, name, &retval);
    
    return retval;
}

char *setfill_type_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->setfill_type_dict, it, &s);
    
    return s;
}

int get_setfill_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->setfill_type_dict, name, &retval);
    
    return retval;
}

char *baseline_type_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->baseline_type_dict, it, &s);
    
    return s;
}

int get_baseline_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->baseline_type_dict, name, &retval);
    
    return retval;
}

char *framedecor_type_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->framedecor_type_dict, it, &s);
    
    return s;
}

int get_framedecor_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->framedecor_type_dict, name, &retval);
    
    return retval;
}

char *scale_type_name(RunTime *rt, ScaleType it)
{
    char *s;
    
    dict_get_name_by_key(rt->scale_type_dict, it, &s);
    
    return s;
}

ScaleType get_scale_type_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->scale_type_dict, name, &retval);
    
    return retval;
}

char *arrow_placement_name(RunTime *rt, int it)
{
    char *s;
    
    dict_get_name_by_key(rt->arrow_placement_dict, it, &s);
    
    return s;
}

int get_arrow_placement_by_name(RunTime *rt, const char *name)
{
    int retval;
    
    dict_get_key_by_name(rt->arrow_placement_dict, name, &retval);
    
    return retval;
}

