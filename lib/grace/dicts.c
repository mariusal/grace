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

/*
 * Misc stuff using dictionaries
 */

#include "grace/grace.h"
#include "xstrings.h"

int grace_init_dicts(Grace *grace)
{
    /* Dictionary tables */
    const DictEntry graph_type_defaults =
        {GRAPH_BAD, "unknown", "Unknown"};
    const DictEntry graph_type_entries[] = 
        {
            {GRAPH_XY,    "xy",    "XY"   },
            {GRAPH_CHART, "chagrace", "Chagrace"},
            {GRAPH_POLAR, "polar", "Polar"},
            {GRAPH_SMITH, "smith", "Smith"},
            {GRAPH_FIXED, "fixed", "Fixed"},
            {GRAPH_PIE,   "pie",   "Pie"  }
        };
    
    const DictEntry set_type_defaults =
        {SET_BAD, "unknown", "Unknown"};
    const DictEntry set_type_entries[] = 
        {
            {SET_XY,         "xy",         "XY"      },
            {SET_BAR,        "bar",        "Bar"     },
            {SET_XYHILO,     "xyhilo",     "XYHiLo"  },
            {SET_XYR,        "xyr",        "XYR"     },
            {SET_XYSIZE,     "xysize",     "XYSize"  },
            {SET_XYCOLOR,    "xycolor",    "XYColor" },
            {SET_XYCOLPAT,   "xycolpat",   "XYColPat"},
            {SET_XYVMAP,     "xyvmap",     "XYVMap"  },
            {SET_BOXPLOT,    "boxplot",    "BoxPlot" }
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

    const DictEntry arcclosure_type_defaults =
        {ARCCLOSURE_CHORD, VStrChord, "Chord"};
    const DictEntry arcclosure_type_entries[] = 
        {
            {ARCCLOSURE_CHORD,    VStrChord,    "Chord"    },
            {ARCCLOSURE_PIESLICE, VStrPieSlice, "Pie slice"}
        };

    const DictEntry format_type_defaults =
        {FORMAT_GENERAL, VStrGeneral, "General"};
    const DictEntry format_type_entries[] = 
        {
            {FORMAT_DECIMAL,        VStrDecimal,        "Decimal"            },
            {FORMAT_EXPONENTIAL,    VStrExponential,    "Exponential"        },
            {FORMAT_GENERAL,        VStrGeneral,        "General"            },
            {FORMAT_POWER,          VStrPower,          "Power"              },
            {FORMAT_SCIENTIFIC,     VStrScientific,     "Scientific"         },
            {FORMAT_ENGINEERING,    VStrEngineering,    "Engineering"        },
            {FORMAT_DATETIME,       VStrDateTime,       "DateTime"           },
            {FORMAT_GEOGRAPHIC,     VStrGeographic,     "Geographic"         }
        };

    const DictEntry frame_type_defaults =
        {FRAME_TYPE_CLOSED, VStrClosed, "Closed"};
    const DictEntry frame_type_entries[] = 
        {
            {FRAME_TYPE_CLOSED,      VStrClosed,      "Closed"      },
            {FRAME_TYPE_HALFOPEN,    VStrHalfOpen,    "Half open"   },
            {FRAME_TYPE_BREAKTOP,    VStrBreakTop,    "Break top"   },
            {FRAME_TYPE_BREAKBOTTOM, VStrBreakBottom, "Break bottom"},
            {FRAME_TYPE_BREAKLEFT,   VStrBreakLeft,   "Break left"  },
            {FRAME_TYPE_BREAKRIGHT,  VStrBreakRight,  "Break right" }
        };

    const DictEntry dataset_col_defaults =
        {DATA_BAD, "?", "?"};
    const DictEntry dataset_col_entries[] = 
        {
            {DATA_X,  "x",  "X" },
            {DATA_Y,  "y",  "Y" },
            {DATA_Y1, "y1", "Y1"},
            {DATA_Y2, "y2", "Y2"},
            {DATA_Y3, "y3", "Y3"},
            {DATA_Y4, "y4", "Y4"}
        };

    if (!(grace->graph_type_dict =
        DICT_NEW_STATIC(graph_type_entries, &graph_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->set_type_dict =
        DICT_NEW_STATIC(set_type_entries, &set_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->object_type_dict =
        DICT_NEW_STATIC(object_type_entries, &object_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->inout_placement_dict =
        DICT_NEW_STATIC(inout_placement_entries, &inout_placement_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->axis_position_dict =
        DICT_NEW_STATIC(axis_position_entries, &axis_position_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->spec_ticks_dict =
        DICT_NEW_STATIC(spec_ticks_entries, &spec_ticks_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->region_type_dict =
        DICT_NEW_STATIC(region_type_entries, &region_type_defaults))) {
        return RETURN_FAILURE;
    }
    
    if (!(grace->arrow_type_dict =
        DICT_NEW_STATIC(arrow_type_entries, &arrow_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->glocator_type_dict =
        DICT_NEW_STATIC(glocator_type_entries, &glocator_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->sym_type_dict =
        DICT_NEW_STATIC(sym_type_entries, &sym_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->line_type_dict =
        DICT_NEW_STATIC(line_type_entries, &line_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->setfill_type_dict =
        DICT_NEW_STATIC(setfill_type_entries, &setfill_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->baseline_type_dict =
        DICT_NEW_STATIC(baseline_type_entries, &baseline_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->framedecor_type_dict =
        DICT_NEW_STATIC(framedecor_type_entries, &framedecor_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->scale_type_dict =
        DICT_NEW_STATIC(scale_type_entries, &scale_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->arrow_placement_dict =
        DICT_NEW_STATIC(arrow_placement_entries, &arrow_placement_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->arcclosure_type_dict =
        DICT_NEW_STATIC(arcclosure_type_entries, &arcclosure_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->format_type_dict =
        DICT_NEW_STATIC(format_type_entries, &format_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->frame_type_dict =
        DICT_NEW_STATIC(frame_type_entries, &frame_type_defaults))) {
        return RETURN_FAILURE;
    }
    if (!(grace->dataset_col_dict =
        DICT_NEW_STATIC(dataset_col_entries, &dataset_col_defaults))) {
        return RETURN_FAILURE;
    }

    return RETURN_SUCCESS;
}

void grace_free_dicts(Grace *grace)
{
    dict_free(grace->graph_type_dict);
    dict_free(grace->set_type_dict);
    dict_free(grace->object_type_dict);
    dict_free(grace->inout_placement_dict);
    dict_free(grace->axis_position_dict);
    dict_free(grace->spec_ticks_dict);
    dict_free(grace->region_type_dict);
    dict_free(grace->arrow_type_dict);
    dict_free(grace->glocator_type_dict);
    dict_free(grace->sym_type_dict);
    dict_free(grace->line_type_dict);
    dict_free(grace->setfill_type_dict);
    dict_free(grace->baseline_type_dict);
    dict_free(grace->framedecor_type_dict);
    dict_free(grace->scale_type_dict);
    dict_free(grace->arrow_placement_dict);
    dict_free(grace->arcclosure_type_dict);
    dict_free(grace->format_type_dict);
    dict_free(grace->frame_type_dict);
    dict_free(grace->dataset_col_dict);
}

char *graph_types(Grace *grace, GraphType it)
{
    char *s;
    
    dict_get_name_by_key(grace->graph_type_dict, it, &s);
    
    return s;
}

char *graph_type_descr(Grace *grace, GraphType it)
{
    char *s;
    
    dict_get_descr_by_key(grace->graph_type_dict, it, &s);
    
    return s;
}

GraphType graph_get_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->graph_type_dict, name, &retval);
    
    return retval;
}


char *set_types(Grace *grace, SetType it)
{
    char *s;
    
    dict_get_name_by_key(grace->set_type_dict, it, &s);
    
    return s;
}

char *set_type_descr(Grace *grace, SetType it)
{
    char *s;
    
    dict_get_descr_by_key(grace->set_type_dict, it, &s);
    
    return s;
}

SetType get_settype_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->set_type_dict, name, &retval);
    
    return retval;
}

char *object_types(Grace *grace, OType it)
{
    char *s;
    
    dict_get_name_by_key(grace->object_type_dict, it, &s);
    
    return s;
}

char *object_type_descr(Grace *grace, OType it)
{
    char *s;
    
    dict_get_descr_by_key(grace->object_type_dict, it, &s);
    
    return s;
}

OType get_objecttype_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->object_type_dict, name, &retval);
    
    return retval;
}

char *inout_placement_name(Grace *grace, int inout)
{
    char *s;
    
    dict_get_name_by_key(grace->inout_placement_dict, inout, &s);
    
    return s;
}

int get_inout_placement_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->inout_placement_dict, name, &retval);
    
    return retval;
}

char *spec_tick_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->spec_ticks_dict, it, &s);
    
    return s;
}

int get_spec_tick_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->spec_ticks_dict, name, &retval);
    
    return retval;
}


char *region_types(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->region_type_dict, it, &s);
    
    return s;
}

int get_regiontype_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->region_type_dict, name, &retval);
    
    return retval;
}

char *axis_position_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->axis_position_dict, it, &s);
    
    return s;
}

int get_axis_position_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->axis_position_dict, name, &retval);
    
    return retval;
}

char *arrow_type_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->arrow_type_dict, it, &s);
    
    return s;
}

int get_arrow_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->arrow_type_dict, name, &retval);
    
    return retval;
}

char *glocator_type_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->glocator_type_dict, it, &s);
    
    return s;
}

int get_glocator_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->glocator_type_dict, name, &retval);
    
    return retval;
}

char *sym_type_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->sym_type_dict, it, &s);
    
    return s;
}

int get_sym_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->sym_type_dict, name, &retval);
    
    return retval;
}

char *line_type_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->line_type_dict, it, &s);
    
    return s;
}

int get_line_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->line_type_dict, name, &retval);
    
    return retval;
}

char *setfill_type_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->setfill_type_dict, it, &s);
    
    return s;
}

int get_setfill_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->setfill_type_dict, name, &retval);
    
    return retval;
}

char *baseline_type_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->baseline_type_dict, it, &s);
    
    return s;
}

int get_baseline_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->baseline_type_dict, name, &retval);
    
    return retval;
}

char *framedecor_type_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->framedecor_type_dict, it, &s);
    
    return s;
}

int get_framedecor_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->framedecor_type_dict, name, &retval);
    
    return retval;
}

char *scale_type_name(Grace *grace, ScaleType it)
{
    char *s;
    
    dict_get_name_by_key(grace->scale_type_dict, it, &s);
    
    return s;
}

ScaleType get_scale_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->scale_type_dict, name, &retval);
    
    return retval;
}

char *arrow_placement_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->arrow_placement_dict, it, &s);
    
    return s;
}

int get_arrow_placement_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->arrow_placement_dict, name, &retval);
    
    return retval;
}

char *arcclosure_type_name(Grace *grace, int it)
{
    char *s;
    
    dict_get_name_by_key(grace->arcclosure_type_dict, it, &s);
    
    return s;
}

int get_arcclosure_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->arcclosure_type_dict, name, &retval);
    
    return retval;
}

char *format_type_name(Grace *grace, FormatType it)
{
    char *s;
    
    dict_get_name_by_key(grace->format_type_dict, it, &s);
    
    return s;
}

FormatType get_format_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->format_type_dict, name, &retval);
    
    return retval;
}

char *format_type_descr(Grace *grace, FormatType it)
{
    char *s;
    
    dict_get_descr_by_key(grace->format_type_dict, it, &s);
    
    return s;
}

char *frame_type_name(Grace *grace, FrameType it)
{
    char *s;
    
    dict_get_name_by_key(grace->frame_type_dict, it, &s);
    
    return s;
}

FrameType get_frame_type_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->frame_type_dict, name, &retval);
    
    return retval;
}

char *frame_type_descr(Grace *grace, FrameType it)
{
    char *s;
    
    dict_get_descr_by_key(grace->frame_type_dict, it, &s);
    
    return s;
}

char *dataset_col_name(Grace *grace, DataColumn it)
{
    char *s;
    
    dict_get_name_by_key(grace->dataset_col_dict, it, &s);
    
    return s;
}

DataColumn get_dataset_col_by_name(Grace *grace, const char *name)
{
    int retval;
    
    dict_get_key_by_name(grace->dataset_col_dict, name, &retval);
    
    return retval;
}
