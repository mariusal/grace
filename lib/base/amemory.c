/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 2005 Grace Development Team
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
 * advanced memory management
 */

#include <stdlib.h>
#include <string.h>

#include "grace/baseP.h"

#ifdef HAVE_LIBUNDO
# include <undo.h>
#endif

static void *amem_simple_malloc(AMem *amem, size_t size)
{
    return xmalloc(size);
}

static void *amem_simple_realloc(AMem *amem, void *ptr, size_t size)
{
    return xrealloc(ptr, size);
}

static void amem_simple_free(AMem *amem, void *ptr)
{
    xfree(ptr);
}


#ifdef HAVE_LIBUNDO
static void *amem_undo_malloc(AMem *amem, size_t size)
{
    undo_set_session((UNDO *) amem->heap);
    return undo_malloc(size);
}

static void *amem_undo_realloc(AMem *amem, void *ptr, size_t size)
{
    undo_set_session((UNDO *) amem->heap);
    return undo_realloc(ptr, size);
}

static void amem_undo_free(AMem *amem, void *ptr)
{
    undo_set_session((UNDO *) amem->heap);
    undo_free(ptr);
}

static int amem_undo_snapshot(AMem *amem)
{
    undo_set_session((UNDO *) amem->heap);

    if (undo_snapshot() == UNDO_NOERROR) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int amem_undo_undo(AMem *amem)
{
    undo_set_session((UNDO *) amem->heap);

    if (undo_undo() == UNDO_NOERROR) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static int amem_undo_redo(AMem *amem)
{
    undo_set_session((UNDO *) amem->heap);

    if (undo_redo() == UNDO_NOERROR) {
        return RETURN_SUCCESS;
    } else {
        return RETURN_FAILURE;
    }
}

static unsigned int amem_undo_count(AMem *amem)
{
    undo_set_session((UNDO *) amem->heap);

    return undo_get_undo_count();
}

static unsigned int amem_redo_count(AMem *amem)
{
    undo_set_session((UNDO *) amem->heap);

    return undo_get_redo_count();
}
#endif


AMem *amem_amem_new(int model)
{
    AMem *amem;
    
    amem = xmalloc(sizeof(AMem));
    if (amem) {
        memset(amem, 0, sizeof(AMem));
        amem->model = model;
        
        switch (model) {
#ifdef HAVE_LIBUNDO
        case AMEM_MODEL_LIBUNDO:
            undo_new("grace");
            
            amem->heap            = undo_get_session();
            amem->undoable        = TRUE;
            
            amem->malloc_proc     = amem_undo_malloc;
            amem->realloc_proc    = amem_undo_realloc;
            amem->free_proc       = amem_undo_free;

            amem->snapshot_proc   = amem_undo_snapshot;
            amem->undo_proc       = amem_undo_undo;
            amem->redo_proc       = amem_undo_redo;
            amem->undo_count_proc = amem_undo_count;
            amem->redo_count_proc = amem_redo_count;
        
            break;
#endif
        case AMEM_MODEL_SIMPLE:
        default:
            amem->heap         = NULL;
            amem->undoable     = FALSE;
            
            amem->malloc_proc  = amem_simple_malloc;
            amem->realloc_proc = amem_simple_realloc;
            amem->free_proc    = amem_simple_free;
        
            break;
        }
    }
    
    return amem;
}

void amem_amem_free(AMem *amem)
{
    if (amem) {
        switch (amem->model) {
#ifdef HAVE_LIBUNDO
        case AMEM_MODEL_LIBUNDO:
            undo_set_session((UNDO *) amem->heap);
            undo_destroy();
            break;
#endif
        default:
            break;
        }
        xfree(amem);
    }
}

int amem_set_undo_limit(AMem *amem, size_t max_memory)
{
    switch (amem->model) {
#ifdef HAVE_LIBUNDO
    case AMEM_MODEL_LIBUNDO:
        undo_set_session((UNDO *) amem->heap);
        if (undo_set_memory_limit(max_memory) == UNDO_NOERROR) {
            return RETURN_SUCCESS;
        } else {
            return RETURN_FAILURE;
        }
        break;
#endif
    default:
        return RETURN_FAILURE;
        break;
    }
}

void *amem_malloc(AMem *amem, size_t size)
{
    return amem->malloc_proc(amem, size);
}

void *amem_realloc(AMem *amem, void *ptr, size_t size)
{
    return amem->realloc_proc(amem, ptr, size);
}

void amem_free(AMem *amem, void *ptr)
{
    amem->free_proc(amem, ptr);
}

void *amem_calloc(AMem *amem, size_t nmemb, size_t size)
{
    void *ptr = amem_malloc(amem, nmemb*size);
    if (ptr) {
        memset(ptr, 0, nmemb*size);
    }
    
    return ptr;
}

int amem_snapshot(AMem *amem)
{
    if (amem->undoable) {
        return amem->snapshot_proc(amem);
    } else {
        return RETURN_FAILURE;
    }
}

int amem_undo(AMem *amem)
{
    if (amem->undoable) {
        return amem->undo_proc(amem);
    } else {
        return RETURN_FAILURE;
    }
}

int amem_redo(AMem *amem)
{
    if (amem->undoable) {
        return amem->redo_proc(amem);
    } else {
        return RETURN_FAILURE;
    }
}

unsigned int amem_get_undo_count(AMem *amem)
{
    if (amem->undoable) {
        return amem->undo_count_proc(amem);
    } else {
        return 0;
    }
}

unsigned int amem_get_redo_count(AMem *amem)
{
    if (amem->undoable) {
        return amem->redo_count_proc(amem);
    } else {
        return 0;
    }
}


/* strings */
char *amem_strcpy(AMem *amem, char *dest, const char *src)
{
    if (src == dest) {
        ;
    } else if (src == NULL) {
        amem_free(amem, dest);
        dest = NULL;
    } else {
        dest = amem_realloc(amem, dest, (strlen(src) + 1)*SIZEOF_CHAR);
        if (dest) {
            strcpy(dest, src);
        }
    }
    
    return dest;
}

char *amem_strdup(AMem *amem, const char *s)
{
    return amem_strcpy(amem, NULL, s);
}
