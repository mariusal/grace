/*
 * grace_np - a library for interfacing with Grace using pipes
 * 
 * Copyright (c) 1997-98 Henrik Seidel
 * Copyright (c) 1999 Grace Development Team
 *
 *
 *                           All Rights Reserved
 * 
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 * 
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 * 
 *    You should have received a copy of the GNU Library General Public
 *    License along with this library; if not, write to the Free
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif
#ifdef HAVE_VFORK_H
#  include <vfork.h>
#endif
#include <limits.h>
#ifndef OPEN_MAX
#  define OPEN_MAX 256
#endif

#include "grace_np.h"

/* static global variables */
static char* buf = NULL;               /* global write buffer */
static int bufsize;                    /* size of the global write buffer */
static int bufsizeforce;               /* threshold for forcing a flush */
static int fd_pipe = -1;               /* file descriptor of the pipe */
static int broken_pipe = 0;            /* is the pipe broken ? */
static pid_t pid = (pid_t) -1;         /* pid of grace */

/*
 * notify grace when finished
 */
static void
#ifdef HAVE_ON_EXIT
notify_grace_on_exit(int status, void* arg)
#else
notify_grace_on_exit(void)
#endif
{
    if (fd_pipe != -1) {
        GraceClosePipe();
    }
}

/*
 * default function for reporting errors
 */
static void
GraceDefaultError(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

/*
 * variable holding user function for reporting errors
 */
static GraceErrorFunctionType error_function = GraceDefaultError;

/*
 * function for reporting system errors
 */
static void
GracePerror(const char *prefix)
{
    char msg[1024];

#ifdef HAVE_STRERROR
    sprintf(msg, "%s: %s", prefix, strerror(errno));
#else
# ifdef HAVE_SYS_ERRLIST_IN_STDIO_H
    sprintf(msg, "%s: %s", prefix, sys_errlist[errno]);
# else
    if (errno == EPIPE) {
        /* this one deserve special attention here */
        sprintf(msg, "%s: Broken pipe", prefix);
    } else {
        sprintf(msg, "%s: System error (errno = %d)", prefix, errno);
    }
# endif
#endif

    error_function(msg);

}

/*
 * try to send data to grace (on pass only)
 */
static int
GraceOneWrite(int left)
{
    int written;

    written = write(fd_pipe, buf, left);

    if (written > 0) {

        left -= written;

        if (left > 0) {
            /* move the remaining characters (and the final '\0') */
#ifdef HAVE_MEMMOVE
            memmove(buf, buf + written, left + 1);
#else
            bcopy(buf + written, buf, left + 1);
#endif
        } else {
            /* clear the buffer */
            *buf = '\0';
        }

    } else if (written < 0) {
        GracePerror("GraceOneWrite");
        if (errno == EPIPE) {
            /* Grace has closed the pipe : we cannot write anymore */
            broken_pipe = 1;
            GraceClose();
        }
        return (-1);
    }

    return (left);

}

/*
 * register an user function to report errors
 */
GraceErrorFunctionType
GraceRegisterErrorFunction(GraceErrorFunctionType f)
{
    GraceErrorFunctionType old = error_function;
    if (f != (GraceErrorFunctionType) NULL) {
        error_function = f;
    }
    return old;
}

int
GraceOpen(const int arg)
{
    int i, fd[2];
    int retval;
    char fd_number [4];

    if (fd_pipe != -1) {
        error_function("Grace subprocess already running");
        return (-1);
    }

    /* Set the buffer sizes according to arg */
    if (arg < 64) {
        error_function("The buffer size in GraceOpen should be >= 64");
        return (-1);
    }
    bufsize = arg;
    bufsizeforce = arg / 2;

    /* make sure the grace subprocess is notified at the end */
#ifdef HAVE_ON_EXIT
    on_exit(notify_grace_on_exit, NULL);
#else
    atexit(notify_grace_on_exit);
#endif

    /* Don't exit on SIGPIPE */
    signal(SIGPIPE, SIG_IGN);

    /* Don't exit when our child "grace" exits */
    signal(SIGCHLD, SIG_IGN);

    /* Make the pipe */
    if (pipe(fd)) {
        GracePerror("GraceOpen");
        return (-1);
    }

    /* Fork a subprocess for starting grace */
    pid = vfork();
    if (pid == (pid_t) (-1)) {
        GracePerror("GraceOpen");
        return (-1);
    }

    /* If we are the child, replace ourselves with grace */
    if (pid == (pid_t) 0) {
        for (i = 0; i < OPEN_MAX; i++) {
            /* we close everything except stdin, stdout, stderr
               and the read part of the pipe */
            if (i != fd[0] &&
                i != STDIN_FILENO && i != STDOUT_FILENO && i != STDERR_FILENO) {
                close(i);
            }
        }
        sprintf(fd_number, "%d", fd[0]);
        retval = execlp("xmgrace", "xmgrace", "-noask", "-dpipe",
                        fd_number, (char *) NULL);
        if (retval == -1) {
            fprintf(stderr, "GraceOpen: Could not start xmgrace\n");
            exit(1);
        }
    }

    /* We are the parent -> keep the write part of the pipe
       and allocate the write buffer */
    buf = (char *) malloc ((size_t) bufsize);
    if (buf == NULL) {
        error_function("GraceOpen: Not enough memory");
        close(fd[0]);
        close(fd[1]);
        return (-1);
    }
    *buf = '\0';

    close(fd[0]);
    fd_pipe = fd[1];
    broken_pipe = 0;

    return (0);

}

int
GraceIsOpen(void)
{
    return (fd_pipe >= 0) ? 1 : 0;
}

int
GraceClose(void)
{

    if (fd_pipe == -1) {
        error_function("No grace subprocess");
        return (-1);
    }

    /* Tell grace to close the pipe */
    if (! broken_pipe) {
        if (GraceCommand ("exit") == -1)
            return (-1);
        if (GraceFlush() != 0)
            return (-1);
    } else {
        kill(pid, SIGTERM);
    }

    /* Wait until grace exit */
    waitpid (pid, NULL, 0);

    /* Close the pipe and free the buffer */
    if (close(fd_pipe) == -1) {
        GracePerror("GraceClose");
	return (-1);
    }
    fd_pipe = -1;
    free (buf);

    return (0);

}

int
GraceClosePipe(void)
{

    if (fd_pipe == -1) {
        error_function("No grace subprocess");
        return (-1);
    }

    /* Tell grace to close the pipe */
    if (! broken_pipe) {
        if (GraceCommand ("close") == -1)
            return (-1);
        if (GraceFlush() != 0)
            return (-1);
    }

    /* Close the pipe and free the buffer */
    if (close(fd_pipe) == -1) {
        GracePerror("GraceClose");
	return (-1);
    }
    fd_pipe = -1;
    free (buf);

    return (0);

}

int
GraceFlush(void)
{
    int loop, left;

    if (fd_pipe == -1) {
        error_function("No grace subprocess");
        return (-1);
    }

    left = strlen(buf);

    for (loop = 0; loop < 30; loop++) {
        left = GraceOneWrite(left);
        if (left < 0) {
            return (-1);
        } else if (left == 0) {
            return (0);
        }
    }

    error_function("GraceFlush: ran into eternal loop");
    return (-1);

}

int
GracePrintf(const char* fmt, ...)
{
    va_list ap;
    char* str;
    int nchar;
    
    if (fd_pipe == -1) {
        error_function("No grace subprocess");
        return (0);
    }

    /* Allocate a new string buffer for the function arguments */
    str = (char *) malloc ((size_t) bufsize);
    if (str == (char *) NULL) {
        error_function("GracePrintf: Not enough memory");
        return (0);
    }
    /* Print to the string buffer according to the function arguments */
    va_start (ap, fmt);
#if defined(HAVE_VSNPRINTF)
    nchar = vsnprintf (str, bufsize - 2, fmt, ap);
#else
    nchar = vsprintf (str, fmt, ap);
#endif
    va_end (ap);
    nchar++;               /* This is for the appended "\n" */
    if (GraceCommand (str) == -1) {
        nchar = 0;
    }
    free (str);
    return (nchar);
}

int
GraceCommand(const char* cmd)
{
    int left;
    
    if (fd_pipe == -1) {
        error_function("No grace subprocess");
        return (-1);
    }

    /* Append the new string to the global write buffer */
    if (strlen(buf) + strlen(cmd) + 2 > bufsize) {
        error_function("GraceCommand: Buffer full");
        return (-1);
    }
    strcat(buf, cmd);
    strcat(buf, "\n");
    left = strlen(buf);
    
    /* Try to send the global write buffer to grace */
    left = GraceOneWrite(left);
    if (left >= bufsizeforce) {
        if (GraceFlush() != 0) {
            return (-1);
        }
    } else if (left < 0) {
        return (-1);
    }

    return (0);

}
