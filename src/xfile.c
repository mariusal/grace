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
 * XML output and related stuff
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "utils.h"
#include "files.h"
#include "xfile.h"


#define DEFAULT_INDENT_STRING   "  "

#define ATTR_CHUNK_SIZE  16

#define XSTACK_CHUNK_SIZE   16

XStack *xstack_new(void)
{
    XStack *xs = xmalloc(sizeof(XStack));
    
    if (xs) {
        xs->size  = 0;
        xs->depth = 0;
        xs->stack = NULL;
    }
    
    return xs;
}

void xstack_free(XStack *xs)
{
    if (xs) {
        while (xs->depth) {
            xs->depth--;
            xfree(xs->stack[xs->depth]);
        }
        
        xfree(xs);
    }
}

int xstack_increment(XStack *xs, const char *name)
{
    if (xs->size <= xs->depth) {
        int new_size = xs->size + XSTACK_CHUNK_SIZE;
        char **p = xrealloc(xs->stack, new_size*SIZEOF_VOID_P);
        if (!p) {
            return RETURN_FAILURE;
        } else {
            xs->stack = p;
            xs->size = new_size;
        }
    }
    xs->stack[xs->depth] = copy_string(NULL, name);
    xs->depth++;
    
    return RETURN_SUCCESS;
}

int xstack_decrement(XStack *xs, const char *name)
{
    if (xs->depth < 1) {
        return RETURN_FAILURE;
    } else if (!compare_strings(name, xs->stack[xs->depth - 1])) {
        return RETURN_FAILURE;
    } else {
        xfree(xs->stack[xs->depth - 1]);
        xs->depth--;
        
        return RETURN_SUCCESS;
    }
}

int xstack_get_first(XStack *xs, char **name)
{
    if (xs && xs->depth > 0) {
        *name = xs->stack[0];
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}


Attributes *attributes_new(void)
{
    Attributes *attrs;
    
    attrs = xmalloc(sizeof(Attributes));
    if (attrs) {
        attrs->count = 0;
        attrs->size  = 0;
        attrs->args  = NULL;
    }
    
    return attrs;
}

void attributes_free(Attributes *attrs)
{
    if (attrs) {
        int i;
        for (i = 0; i < attrs->size; i++) {
            xfree(attrs->args[i].name);
            xfree(attrs->args[i].value);
        }
        xfree(attrs->args);
        xfree(attrs);
    }
}

static int attributes_resize(Attributes *attrs, int count)
{
    if (count > attrs->size) {
        int newsize = attrs->size + ATTR_CHUNK_SIZE;
        ElementAttribute *p =
            xrealloc(attrs->args, newsize*sizeof(ElementAttribute));
        if (!p) {
            return RETURN_FAILURE;
        } else {
            int i;
            attrs->args = p;
            for (i = attrs->size; i < newsize; i++) {
                attrs->args[i].name  = NULL;
                attrs->args[i].value = NULL;
            }
            attrs->size = newsize;
        }
    }
    return RETURN_SUCCESS;
}

int attributes_set_sval(Attributes *attrs, const char *name, const char *value)
{
    if (attrs->count >= attrs->size) {
        if (attributes_resize(attrs, attrs->count + 1) != RETURN_SUCCESS) {
            return RETURN_FAILURE;
        }
    }
    
    attrs->args[attrs->count].name  =
        copy_string(attrs->args[attrs->count].name, name);
    attrs->args[attrs->count].value =
        copy_string(attrs->args[attrs->count].value, value);
    
    attrs->count++;
    
    return RETURN_SUCCESS;
}

int attributes_set_bval(Attributes *attrs, const char *name, int bval)
{
    char *value = bval ? "yes":"no";
    
    return attributes_set_sval(attrs, name, value);
}

int attributes_set_ival(Attributes *attrs, const char *name, int ival)
{
    char value[32];
    
    sprintf(value, "%d", ival);
    
    return attributes_set_sval(attrs, name, value);
}

int attributes_set_ival_formatted(Attributes *attrs, const char *name,
    int ival, char *format)
{
    if (format) {
        char value[128];
        
        sprintf(value, format, ival);

        return attributes_set_sval(attrs, name, value);
    } else {
        return attributes_set_ival(attrs, name, ival);
    }
}

int attributes_set_dval(Attributes *attrs, const char *name, double dval)
{
    char value[32];
    
    sprintf(value, "%g", dval);
    
    return attributes_set_sval(attrs, name, value);
}

int attributes_set_dval_formatted(Attributes *attrs, const char *name,
    double dval, char *format)
{
    if (format) {
        char value[128];
        
        sprintf(value, format, dval);

        return attributes_set_sval(attrs, name, value);
    } else {
        return attributes_set_dval(attrs, name, dval);
    }
}

int attributes_reset(Attributes *attrs)
{
    attrs->count = 0;
    
    return RETURN_SUCCESS;
}


void xfile_free(XFile *xf)
{
    if (xf) {
        xfree(xf->fname);
        xstack_free(xf->tree);
        xfree(xf->indstr);
        grace_close(xf->fp);
        xfree(xf);
    }
}

XFile *xfile_new(char *fname)
{
    XFile *xf;

    xf = xmalloc(sizeof(XFile));
    if (xf) {
        xf->tree   = xstack_new();
        xf->indent = 0;
        xf->curpos = 0;
        xf->indstr = copy_string(NULL, DEFAULT_INDENT_STRING);
        xf->fname  = copy_string(NULL, fname);
        xf->fp     = grace_openw(xf->fname);
        
        if (!xf->tree || !xf->indstr || !xf->fname || !xf->fp) {
            xfile_free(xf);
            xf = NULL;
        }
    }
    
    return xf;
}

static int xfile_output(XFile *xf, char *str)
{
    int i, nb, len;
    
    if (xf->curpos == 0) {
        for (i = 0; i < xf->indent; i++) {
            if ((nb = fputs(xf->indstr, xf->fp)) >= 0) {
                xf->curpos += strlen(xf->indstr);
            } else {
                return RETURN_FAILURE;
            }
        }
    }
    
    nb = fputs(str, xf->fp);
    if (nb >= 0) {
        len = strlen(str);
        if (len && str[len - 1] == '\n') {
            xf->curpos = 0;
        } else {
            xf->curpos += nb;
        }
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int xfile_indent_increment(XFile *xf)
{
    if (xf->curpos > 0) {
        xfile_output(xf, "\n");
    }
    xf->indent++;
    return RETURN_SUCCESS;
}

static int xfile_indent_decrement(XFile *xf)
{
    if (xf->indent > 0) {
        if (xf->curpos > 0) {
            xfile_output(xf, "\n");
        }
        xf->indent--;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static void xfile_output_attributes(XFile *xf, Attributes *attrs)
{
    if (attrs) {
        unsigned int i;
        for (i = 0; i < attrs->count; i++) {
            if (attrs->args[i].name) {
                /* FIXME: escape special chars */
                xfile_output(xf, " ");
                xfile_output(xf, attrs->args[i].name);
                xfile_output(xf, "=\"");
                xfile_output(xf, PSTRING(attrs->args[i].value));
                xfile_output(xf, "\"");
            }
        }
    }
}

int xfile_processing_instruction(XFile *xf, Attributes *attrs)
{
    xfile_output(xf, "<?xml");
    xfile_output_attributes(xf, attrs);
    xfile_output(xf, "?>\n");
    
    return RETURN_SUCCESS;
}

int xfile_begin_element(XFile *xf, char *name, Attributes *attrs)
{
    xfile_output(xf, "<");
    xfile_output(xf, name);
    xfile_output_attributes(xf, attrs);
    xfile_output(xf, ">\n");
    xfile_indent_increment(xf);
    
    if (xstack_increment(xf->tree, name) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    return RETURN_SUCCESS;
}

int xfile_end_element(XFile *xf, char *name)
{
    /* FIXME: check that the current element corresponds to name */
    xfile_indent_decrement(xf);
    xfile_output(xf, "</");
    xfile_output(xf, name);
    xfile_output(xf, ">\n");
    
    if (xstack_decrement(xf->tree, name) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    return RETURN_SUCCESS;
}

int xfile_empty_element(XFile *xf, char *name, Attributes *attrs)
{
    xfile_output(xf, "<");
    xfile_output(xf, name);
    xfile_output_attributes(xf, attrs);
    xfile_output(xf, "/>\n");
    
    return RETURN_SUCCESS;
}

int xfile_begin(XFile *xf, char *encoding, int standalone,
    char *dtd_name, char *dtd_uri, char *root, Attributes *root_attrs)
{
    Attributes *attrs;
    
    attrs = attributes_new();
    if (!attrs) {
        return RETURN_FAILURE;
    }
    
    attributes_set_sval(attrs, "version", "1.0");

    if (encoding) {
        attributes_set_sval(attrs, "encoding", encoding);
    }
    
    attributes_set_bval(attrs, "standalone", standalone);
    
    xfile_processing_instruction(xf, attrs);
    
    attributes_free(attrs);
    
    if (!standalone) {
        xfile_output(xf, "<!DOCTYPE ");
        xfile_output(xf, root);
        xfile_output(xf, " ");
        xfile_output(xf, dtd_name ? "PUBLIC":"SYSTEM");
        xfile_output(xf, " ");
        if (dtd_name) {
            xfile_output(xf, "\"");
            xfile_output(xf, dtd_name);
            xfile_output(xf, "\" ");
        }
        xfile_output(xf, "\"");
        xfile_output(xf, dtd_uri);
        xfile_output(xf, "\">\n");
    }
    
    xfile_begin_element(xf, root, root_attrs);
    
    return RETURN_SUCCESS;
}

int xfile_end(XFile *xf)
{
    char *root;
    
    if (xstack_get_first(xf->tree, &root) == RETURN_SUCCESS) {
        return xfile_end_element(xf, root);
    } else {
        return RETURN_FAILURE;
    }
}

int xfile_comment(XFile *xf, char *comment)
{
    xfile_output(xf, "<!-- ");
    xfile_output(xf, comment);
    xfile_output(xf, " -->\n");
    
    return RETURN_SUCCESS;
}

int xfile_cdata(XFile *xf, char *cdata)
{
    xfile_output(xf, "<![CDATA[\n");
    
    xfile_indent_increment(xf);
    xfile_output(xf, cdata);
    xfile_indent_decrement(xf);
    
    xfile_output(xf, "]]>\n");
    
    return RETURN_SUCCESS;
}

int xfile_pcdata(XFile *xf, char *pcdata)
{
    /* FIXME: escape special chars */
    return xfile_output(xf, pcdata);
}

