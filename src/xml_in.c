/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000 Grace Development Team
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
 * XML project input
 */

#include <config.h>

#include <stdio.h>

#include <expat.h>

#include "globals.h"
#include "defines.h"
#include "utils.h"
#include "objutils.h"
#include "device.h"
#include "files.h"
#include "xfile.h"
#include "xstrings.h"
#include "protos.h"

#define BUFF_SIZE   8192


typedef struct _ParserData {
    int initialized;
    
    XStack *stack;
    
    char *cbuffer;
    int cbufsize;
    int cbuflen;
    
    int error;
} ParserData;

static int atobool(const char *a)
{
    return compare_strings(a, "yes");
}

static void xmlio_start_doctype_handler(void *userData,
    const XML_Char *doctypeName, const XML_Char *sysid, const XML_Char *pubid,
    int has_internal_subset)
{
    ParserData *pdata = (ParserData *) userData;
    
    if (!pdata || pdata->error) {
        return;
    }
    
    if (compare_strings(doctypeName, "grace")) {
        pdata->initialized = TRUE;
    } else {
        pdata->error = TRUE;
        errmsg("Unrecognized document type");
    }
}

static void xmlio_start_element_handler(void *userData,
    const XML_Char *name, const XML_Char **atts)
{
    ParserData *pdata = (ParserData *) userData;
    XML_Char **ap;
    char *pname;
    void *context, *pcontext;
    
    if (!pdata || pdata->error) {
        return;
    }
    
    if (!pdata->initialized) {
        pdata->error = TRUE;
        return;
    }
    
    if (xstack_is_empty(pdata->stack)) {
        pname = NULL;
        pcontext = grace;
    } else {
        if (xstack_get_last(pdata->stack, &pname, &pcontext) != RETURN_SUCCESS) {
            pdata->error = TRUE;
            return;
        }
        
        if (!pname || !pcontext) {
            /* shouldn't happen */
            pdata->error = TRUE;
            return;
        }
    }
    
    if (compare_strings(name, EStrGrace) &&
        pname == NULL) {
        Project *pr = grace->project;
        context = pr;
        
        ap = (XML_Char **) atts;
        while (*ap) {
            char *aname, *aval;
            aname = *ap; ap++;
            aval  = *ap; ap++;
            if (compare_strings(aname, AStrVersion)) {
                int version_id = atoi(aval);
                project_set_version_id(pr, version_id);
            } else {
                errmsg("Unknown attribute, skipping");
            }
        }
    } else
    if (compare_strings(name, EStrDescription) &&
        compare_strings(pname, EStrGrace)) {
        context = pcontext;
    } else
    if (compare_strings(name, EStrDefinitions) &&
        compare_strings(pname, EStrGrace)) {
        /* FIXME */
        context = pcontext;
    } else
    if (compare_strings(name, EStrColormap) &&
        compare_strings(pname, EStrDefinitions)) {
        /* FIXME */
        context = pcontext;
    } else
    if (compare_strings(name, EStrColorDef) &&
        compare_strings(pname, EStrColormap)) {
        /* FIXME */
        context = pcontext;
    } else
    if (compare_strings(name, EStrFontmap) &&
        compare_strings(pname, EStrDefinitions)) {
        /* FIXME */
        context = pcontext;
    } else
    if (compare_strings(name, EStrFontDef) &&
        compare_strings(pname, EStrFontmap)) {
        /* FIXME */
        context = pcontext;
    } else
    if (compare_strings(name, EStrPage) &&
        compare_strings(pname, EStrGrace)) {
        int wpp = 0, hpp = 0;
        /* FIXME */
        context = pcontext;
        
        ap = (XML_Char **) atts;
        while (*ap) {
            char *aname, *aval;
            aname = *ap; ap++;
            aval  = *ap; ap++;
            if (compare_strings(aname, AStrWidth)) {
                wpp = atoi(aval);
            } else
            if (compare_strings(aname, AStrHeight)) {
                hpp = atoi(aval);
            } else {
                errmsg("Unknown attribute, skipping");
            }
        }
        set_page_dimensions(wpp, hpp, FALSE);
    } else
    if (compare_strings(name, EStrDataFormats) &&
        compare_strings(pname, EStrGrace)) {
        /* FIXME */
        context = pcontext;
    } else
    if (compare_strings(name, EStrDates) &&
        compare_strings(pname, EStrDataFormats)) {
        /* FIXME */
        context = pcontext;
        
        ap = (XML_Char **) atts;
        while (*ap) {
            char *aname, *aval;
            aname = *ap; ap++;
            aval  = *ap; ap++;
            if (compare_strings(aname, AStrReference)) {
                double refdate = atof(aval);
                set_ref_date(refdate);
            } else
            if (compare_strings(aname, AStrWrap)) {
                int enable_wrap = atobool(aval);
                allow_two_digits_years(enable_wrap);
            } else
            if (compare_strings(aname, AStrWrapYear)) {
                int wrap_year = atoi(aval);
                set_wrap_year(wrap_year);
            } else {
                errmsg("Unknown attribute, skipping");
            }
        }
    } else
    if (compare_strings(name, EStrText)) {
        /* nothing to do here */
        context = pcontext;
    } else {
        pdata->error = TRUE;
        return;
    }
    
    if (xstack_increment(pdata->stack, name, context) != RETURN_SUCCESS) {
        pdata->error = TRUE;
        return;
    }

    /* flush char buffer */
    pdata->cbuflen = 0;
}

static void xmlio_end_element_handler(void *userData, const XML_Char *name)
{
    ParserData *pdata = (ParserData *) userData;
    
    if (!pdata || pdata->error) {
        return;
    }
    
    if (!pdata->initialized) {
        pdata->error = TRUE;
        return;
    }
    
    if (xstack_decrement(pdata->stack, name) != RETURN_SUCCESS) {
        pdata->error = TRUE;
        return;
    }
    
    if (pdata->stack->depth < 1) {
        /* we're done */
        return;
    }
    
    /* use the char buffer if needed */
    if (pdata->cbuflen && compare_strings(name, EStrText)) {
        char *pname;
        void *pcontext;
        
        /* see which is our pname */
        if (xstack_get_last(pdata->stack, &pname, &pcontext) != RETURN_SUCCESS) {
            pdata->error = TRUE;
            return;
        }

        if (compare_strings(pname, EStrDescription)) {
            Project *pr = (Project *) pcontext;
            project_set_description(pr, pdata->cbuffer);
        } else 
        if (compare_strings(pname, EStrAxislabel)   ||
            compare_strings(pname, EStrLegendEntry)) {
            /* FIXME */
        } else 
        if (compare_strings(pname, EStrTitle) ||
            compare_strings(pname, EStrSubtitle)) {
            plotstr *ps = (plotstr *) pcontext;
            set_plotstr_string(ps, pdata->cbuffer);
        } else 
        if (compare_strings(pname, EStrStringData)) {
            DOStringData *sd = (DOStringData *) pcontext;
            sd->s = copy_string(sd->s, pdata->cbuffer);
        } else {
            /* shouldn't happen */
            pdata->error = TRUE;
            return;
        }
    }

    /* NB: we don't free the buffer; it will be reused later */
    pdata->cbuflen = 0;
}

static void xmlio_char_data_handler(void *userData, const XML_Char *s, int len)
{
    ParserData *pdata = (ParserData *) userData;
    int new_len;
    
    if (!pdata || pdata->error) {
        return;
    }
    
    if (!pdata->initialized) {
        pdata->error = TRUE;
        return;
    }
    
    new_len = pdata->cbuflen + len;
    
    if (new_len >= pdata->cbufsize) {
        pdata->cbuffer = xrealloc(pdata->cbuffer, (new_len + 1)*SIZEOF_CHAR);
        pdata->cbufsize = new_len + 1;
    }
    
    memcpy(pdata->cbuffer + pdata->cbuflen, s, len*SIZEOF_CHAR);
    pdata->cbuffer[new_len] = '\0';
    pdata->cbuflen = new_len;
}
  
int load_xgr_project(char *fn)
{
    XML_Parser xp;
    XML_Memory_Handling_Suite ms;
    ParserData pdata;
    FILE *fp;
    char *uri_base;
    int err, done;
    
    err = FALSE;

    fp = grace_openr(fn, SOURCE_DISK);
    if (fp == NULL) {
	return RETURN_FAILURE;
    }
    
    ms.malloc_fcn  = xmalloc;
    ms.realloc_fcn = xrealloc;
    ms.free_fcn    = xfree;
    
    xp = XML_ParserCreate_MM(NULL, &ms, NULL);
    
    if (!xp) {
        errmsg("Couldn't allocate memory for parser");
        grace_close(fp);
        return RETURN_FAILURE;
    }
    
    /* Set user data */
    pdata.error = FALSE;
    
    pdata.initialized = FALSE;
    
    pdata.stack = xstack_new();
    
    pdata.cbuffer  = NULL;
    pdata.cbufsize = 0;
    pdata.cbuflen  = 0;
    
    XML_SetUserData(xp, (void *) &pdata);
                 
    /* Set base for parsing local DTD */
    uri_base = copy_string(NULL, get_grace_home());
    uri_base = concat_strings(uri_base, "/dtd/");
    XML_SetBase(xp, uri_base);
    xfree(uri_base);

    /* Set the DOCTYPE handler */
    XML_SetStartDoctypeDeclHandler(xp, xmlio_start_doctype_handler);
               
    /* Set the element handler */
    XML_SetElementHandler(xp,
        xmlio_start_element_handler, xmlio_end_element_handler);

    /* Set the char data handler */
    XML_SetCharacterDataHandler(xp, xmlio_char_data_handler) ;
       
    done = FALSE;
    while (!done && !err && !pdata.error) {
        void *buff;
        int nbytes;
        
        buff = XML_GetBuffer(xp, BUFF_SIZE);
        if (buff == NULL) {
            errmsg("Couldn't allocate buffer for parser");
            err = TRUE;
        }

        nbytes = fread(buff, 1, BUFF_SIZE, fp);
        if (ferror(fp)) {
            errmsg("Read error");
            err = TRUE;
            break;
        }
        done = feof(fp);
              
        if (!XML_ParseBuffer(xp, nbytes, done)) {
            char buf[256];
            sprintf(buf, "Parse error at line %d:%s",
                  XML_GetCurrentLineNumber(xp),
                  XML_ErrorString(XML_GetErrorCode(xp)));
            errmsg(buf);
            err = TRUE;
        }
    }

    xstack_free(pdata.stack);
    xfree(pdata.cbuffer);
    
    XML_ParserFree(xp);
    grace_close(fp);
    
    if (err || pdata.error) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}
