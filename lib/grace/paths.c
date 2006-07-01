/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1996-2002 Grace Development Team
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
 * Path translations for I/O operations
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

#include "grace/graceP.h"

char *grace_get_userhome(const Grace *grace)
{
    return grace->userhome;
}

/* If path begins with '~', replace it with the user's home dir.
   If expanding is impossible, return NULL */
static char *grace_expand_tilde(const Grace *grace, const char *path)
{
    size_t uname_len;
    char *sep_ptr, *epath;
    
    if (!path || path[0] != '~') {
        return NULL;
    }
    
    /* position of the first path separator */
    sep_ptr = strchr(path, '/');
    
    if (sep_ptr) {
        uname_len = sep_ptr - path - 1;
    } else {
        uname_len = 0;
    }
    
    /* get the home dir path first */
    if (!uname_len) {
        epath = copy_string(NULL, grace->userhome);
    } else {
	char *uname;
        struct passwd *pent;
        
        uname = xmalloc(uname_len + 1);
        if (uname) {
            strncpy(uname, path + 1, uname_len);
            uname[uname_len] = '\0';
        }
        
	pent = getpwnam(uname);
        xfree(uname);
        
        if (pent) {
	    epath = copy_string(NULL, pent->pw_dir);
        } else {
	    errmsg("No user by that name");
            return NULL;
        }
    }
    
    /* append the rest of the path */
    epath = concat_strings(epath, sep_ptr);
    
    return epath;
}

char *grace_path(const Grace *grace, const char *path)
{
    char *epath;
    struct stat statb;

    epath = grace_expand_tilde(grace, path);
    if (epath) {
        return epath;
    }
    
    if (string_is_empty(path)       ||
        path[0] == '/'              ||
        strstr(path, "./")  == path ||
        strstr(path, "../") == path) {
    
        return copy_string(NULL, path);
    }
        
    /* if we arrived here, the path is relative */
    if (stat(path, &statb) == 0) {
        /* ok, we found it */
        return copy_string(NULL, path);
    }

    /* second try: in .grace/ in the current dir */
    epath = copy_string(NULL, ".grace/");
    epath = concat_strings(epath, path);
    if (stat(epath, &statb) == 0) {
        return epath;
    }

    /* third try: in $HOME/.grace/ */
    epath = copy_string(epath, grace->userhome);
    epath = concat_strings(epath, "/.grace/");
    epath = concat_strings(epath, path);
    if (stat(epath, &statb) == 0) {
        return epath;
    }

    /* the last attempt: in $GRACE_HOME */
    epath = copy_string(epath, grace->grace_home);
    epath = concat_strings(epath, "/");
    epath = concat_strings(epath, path);
    if (stat(epath, &statb) == 0) {
        return epath;
    }

    /* giving up... */
    return copy_string(NULL, path);
}

char *grace_path2(const Grace *grace, const char *prefix, const char *path)
{
    char *s, *epath;
    
    s = copy_string(NULL, prefix);
    if (!s) {
        return NULL;
    }
    
    if (s[strlen(s) - 1] != '/') {
        s = concat_strings(s, "/");
    }
    
    s = concat_strings(s, path);
    
    epath = grace_path(grace, s);
    
    xfree(s);
    
    return epath;
}
