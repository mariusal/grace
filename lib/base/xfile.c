/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2000-2006 Grace Development Team
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
 * XML output and related stuff
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grace/baseP.h"


#define DEFAULT_INDENT_STRING   "  "

#define ATTR_CHUNK_SIZE  16

#define XSTACK_CHUNK_SIZE   16

/* Types of text convert logics */
#define CONVERT_NONE        0
#define CONVERT_PCDATA      1
#define CONVERT_CDATA       2
#define CONVERT_COMMENTS    3

XStack *xstack_new(void)
{
    XStack *xs = xmalloc(sizeof(XStack));
    
    if (xs) {
        xs->size    = 0;
        xs->depth   = 0;
        xs->entries = NULL;
    }
    
    return xs;
}

void xstack_free(XStack *xs)
{
    if (xs) {
        while (xs->depth) {
            xs->depth--;
            xfree(xs->entries[xs->depth].name);
        }
        
        xfree(xs->entries);
        xfree(xs);
    }
}

int xstack_increment(XStack *xs, const char *name, void *data)
{
    if (xs->size <= xs->depth) {
        int new_size = xs->size + XSTACK_CHUNK_SIZE;
        XStackEntry *p = xrealloc(xs->entries, new_size*sizeof(XStackEntry));
        if (!p) {
            return RETURN_FAILURE;
        } else {
            xs->entries = p;
            xs->size = new_size;
        }
    }
    xs->entries[xs->depth].data = data;
    xs->entries[xs->depth].name = copy_string(NULL, name);
    xs->depth++;
    
    return RETURN_SUCCESS;
}

int xstack_decrement(XStack *xs, const char *name)
{
    if (xs->depth < 1) {
        return RETURN_FAILURE;
    } else if (!strings_are_equal(name, xs->entries[xs->depth - 1].name)) {
        return RETURN_FAILURE;
    } else {
        xfree(xs->entries[xs->depth - 1].name);
        xs->depth--;
        
        return RETURN_SUCCESS;
    }
}

int xstack_get_first(XStack *xs, char **name, void **data)
{
    if (xs && xs->depth > 0) {
        *data = xs->entries[0].data;
        *name = xs->entries[0].name;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int xstack_get_last(XStack *xs, char **name, void **data)
{
    if (xs && xs->depth > 0) {
        *data = xs->entries[xs->depth - 1].data;
        *name = xs->entries[xs->depth - 1].name;
        
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

int xstack_is_empty(XStack *xs)
{
    return !(xs->depth);
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
        unsigned int i;
        for (i = 0; i < attrs->size; i++) {
            xfree(attrs->args[i].name);
            xfree(attrs->args[i].value);
        }
        xfree(attrs->args);
        xfree(attrs);
    }
}

static int attributes_resize(Attributes *attrs, unsigned int count)
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

int attributes_set_ns(Attributes *attrs, const char *ns, const char *uri)
{
    int retval;
    char *buf;
    
    buf = copy_string(NULL, "xmlns");
    if (ns) {
        buf = concat_strings(buf, ":");
        buf = concat_strings(buf, ns);
    }
    
    retval = attributes_set_sval(attrs, buf, uri);
    
    xfree(buf);
    
    return retval;
}

int attributes_reset(Attributes *attrs)
{
    attrs->count = 0;
    
    return RETURN_SUCCESS;
}


void xfile_free(XFile *xf)
{
    if (xf) {
        xstack_free(xf->tree);
        xfree(xf->indstr);
        xfree(xf->ns_prefix);
        xfree(xf->ns_uri);
        xfree(xf);
    }
}

XFile *xfile_new(FILE *fp)
{
    XFile *xf;

    xf = xmalloc(sizeof(XFile));
    if (xf) {
        memset(xf, 0, sizeof(XFile));
        
        xf->tree    = xstack_new();
        xf->convert = CONVERT_NONE;
        xf->indstr  = copy_string(NULL, DEFAULT_INDENT_STRING);
        xf->fp      = fp;
        
        if (!xf->tree || !xf->indstr || !xf->fp) {
            xfile_free(xf);
            xf = NULL;
        }
    }
    
    return xf;
}

int xfile_set_indstr(XFile *xf, const char *indstr)
{
    if (xf && indstr) {
        xf->indstr = copy_string(xf->indstr, indstr);
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static void xfile_convert(XFile *xf, int flag)
{
    xf->convert = flag;
}

static int xfile_output(XFile *xf, char *str)
{
    unsigned int i;
    
    if (!str) {
        return RETURN_SUCCESS;
    }
    
    if (xf->curpos == 0) {
        for (i = 0; i < xf->indent; i++) {
            if (fputs(xf->indstr, xf->fp) < 0) {
                return RETURN_FAILURE;
            } else {
                xf->curpos += strlen(xf->indstr);
            }
        }
    }
    
    if (xf->convert) {
        for (i = 0; i < strlen(str); i++) {
            char buf[8];
            unsigned char uc = (unsigned char) str[i];
            if (uc & 0x80) {
                /* 2-byte UTF-8 */
                buf[0] = 0xC0 | (uc >> 6);
                buf[1] = 0x80 | (uc & 0x3F);
                buf[2] = '\0';
            } else {
                if (xf->convert == CONVERT_PCDATA && uc == '"') {
                    strcpy(buf, "&quot;");
                } else
                if (xf->convert == CONVERT_PCDATA && uc == '&') {
                    strcpy(buf, "&amp;");
                } else
                if (xf->convert == CONVERT_PCDATA && uc == '<') {
                    strcpy(buf, "&lt;");
                } else
                if (uc == '>' &&
                    i > 1 && str[i - 1] == ']' && str[i - 2] == ']') {
                    strcpy(buf, "&gt;");
                } else
                if (xf->convert == CONVERT_COMMENTS &&
                    uc == '-' && str[i + 1] == '-') {
                    continue;
                } else {
                    buf[0] = uc;
                    buf[1] = '\0';
                }
            }
            
            if (fputs(buf, xf->fp) < 0) {
                return RETURN_FAILURE;
            } else {
                xf->curpos += strlen(buf);
            }
        }
        
        return RETURN_SUCCESS;
    } else {
        if (fputs(str, xf->fp) < 0) {
            return RETURN_FAILURE;
        } else {
            xf->curpos += strlen(str);
            return RETURN_SUCCESS;
        }
    }
}

static int xfile_crlf(XFile *xf)
{
    if (fputc('\n', xf->fp) != EOF) {
        xf->curpos = 0;
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int xfile_indent_increment(XFile *xf)
{
    if (xf->curpos > 0) {
        xfile_crlf(xf);
    }
    xf->indent++;
    return RETURN_SUCCESS;
}

static int xfile_indent_decrement(XFile *xf)
{
    if (xf->indent > 0) {
        if (xf->curpos > 0) {
            xfile_crlf(xf);
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
                xfile_output(xf, " ");
                xfile_output(xf, attrs->args[i].name);
                xfile_output(xf, "=\"");
                
                xfile_convert(xf, CONVERT_PCDATA);
                xfile_output(xf, attrs->args[i].value);
                xfile_convert(xf, CONVERT_NONE);
                
                xfile_output(xf, "\"");
            }
        }
    }
}

int xfile_processing_instruction(XFile *xf, Attributes *attrs)
{
    xfile_output(xf, "<?xml");
    xfile_output_attributes(xf, attrs);
    xfile_output(xf, "?>");
    xfile_crlf(xf);
    
    return RETURN_SUCCESS;
}

static int xfile_output_element_name(XFile *xf, char *name)
{
    if (xf->ns_force && xf->ns_prefix) {
        xfile_output(xf, xf->ns_prefix);
        xfile_output(xf, ":");
    }
    xfile_output(xf, name);
    
    return RETURN_SUCCESS;
}

int xfile_begin_element(XFile *xf, char *name, Attributes *attrs)
{
    xfile_output(xf, "<");
    xfile_output_element_name(xf, name);
    xfile_output_attributes(xf, attrs);
    xfile_output(xf, ">");
    xfile_indent_increment(xf);
    
    if (xstack_increment(xf->tree, name, NULL) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    return RETURN_SUCCESS;
}

int xfile_end_element(XFile *xf, char *name)
{
    xfile_indent_decrement(xf);
    xfile_output(xf, "</");
    xfile_output_element_name(xf, name);
    xfile_output(xf, ">");
    xfile_crlf(xf);
    
    if (xstack_decrement(xf->tree, name) != RETURN_SUCCESS) {
        return RETURN_FAILURE;
    }
    
    return RETURN_SUCCESS;
}

int xfile_empty_element(XFile *xf, char *name, Attributes *attrs)
{
    xfile_output(xf, "<");
    xfile_output_element_name(xf, name);
    xfile_output_attributes(xf, attrs);
    xfile_output(xf, "/>");
    xfile_crlf(xf);
    
    return RETURN_SUCCESS;
}

int xfile_text_element(XFile *xf,
    char *name, Attributes *attrs, char *text, int cdata)
{
    if (string_is_empty(text)) {
        return xfile_empty_element(xf, name, attrs);
    } else {
        xfile_output(xf, "<");
        xfile_output_element_name(xf, name);
        xfile_output_attributes(xf, attrs);
        xfile_output(xf, ">");
        
        if (cdata) {
            xfile_output(xf, "<![CDATA[");
            xfile_convert(xf, CONVERT_CDATA);
        } else {
            xfile_convert(xf, CONVERT_PCDATA);
        }

        xfile_output(xf, text);
        xfile_convert(xf, CONVERT_NONE);

        if (cdata) {
            xfile_output(xf, "]]>");
        }

        xfile_output(xf, "</");
        xfile_output_element_name(xf, name);
        xfile_output(xf, ">");
        xfile_crlf(xf);

        return RETURN_SUCCESS;
    }
}

int xfile_set_ns(XFile *xf, const char *ns, const char *uri, int force)
{
    if (xf->ns_uri) {
        return RETURN_FAILURE;
    }
    xf->ns_uri    = copy_string(NULL, uri);
    xf->ns_prefix = copy_string(NULL, ns);
    xf->ns_force  = force;
    
    return RETURN_SUCCESS;
}

int xfile_begin(XFile *xf, char *root, Attributes *root_attrs)
{
    Attributes *attrs;
    
    attrs = attributes_new();
    if (!attrs) {
        return RETURN_FAILURE;
    }
    
    attributes_set_sval(attrs, "version", "1.0");

    xfile_processing_instruction(xf, attrs);
    
    attributes_free(attrs);
    
    attributes_set_ns(root_attrs, xf->ns_prefix, xf->ns_uri);
    xfile_begin_element(xf, root, root_attrs);
    
    return RETURN_SUCCESS;
}

int xfile_end(XFile *xf)
{
    char *root;
    void *dummy;
    
    if (xstack_get_first(xf->tree, &root, &dummy) == RETURN_SUCCESS) {
        return xfile_end_element(xf, root);
    } else {
        return RETURN_FAILURE;
    }
}

int xfile_comment(XFile *xf, char *comment)
{
    xfile_output(xf, "<!-- ");
    
    xfile_convert(xf, CONVERT_COMMENTS);
    xfile_output(xf, comment);
    xfile_convert(xf, CONVERT_NONE);
    
    xfile_output(xf, " -->");
    xfile_crlf(xf);
    
    return RETURN_SUCCESS;
}
