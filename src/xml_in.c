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

#include "defines.h"
#include "utils.h"
#include "files.h"
#include "xfile.h"

#define BUFF_SIZE   8192


typedef struct _ParserData {
    int initialized;
    XStack *stack;
    int error;
} ParserData;

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
    
    if (!pdata || pdata->error) {
        return;
    }
    
    if (!pdata->initialized) {
        errmsg("Unrecognized document type");
        return;
    }
    
    if (xstack_increment(pdata->stack, name) != RETURN_SUCCESS) {
        pdata->error = TRUE;
        return;
    }

    printf("start\t[%d]: %s\n", pdata->stack->depth - 1, name);
}

static void xmlio_end_element_handler(void *userData, const XML_Char *name)
{
    ParserData *pdata = (ParserData *) userData;
    
    if (!pdata || pdata->error) {
        return;
    }
    
    if (!pdata->initialized) {
        errmsg("Unrecognized document type");
        return;
    }
    
    if (xstack_decrement(pdata->stack, name) != RETURN_SUCCESS) {
        pdata->error = TRUE;
        return;
    }
    
    printf("end\t[%d]: %s\n", pdata->stack->depth, name);
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
    XML_ParserFree(xp);
    grace_close(fp);
    
    if (err || pdata.error) {
        return RETURN_FAILURE;
    } else {
        return RETURN_SUCCESS;
    }
}
