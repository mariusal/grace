/*
 * Grace - Graphics for Exploratory Data Analysis
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 GRACE Development Team
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "missing.h"

/* To make ANSI C happy about non-empty file */
void _missing_c_dummy_func(void) {}

#ifndef HAVE_DRAND48
double drand48(void)
{
    return (double) (1.0/RAND_MAX)*rand();
}
#endif

#ifndef HAVE_POPEN
#  ifdef VMS

/* drop in popen() / pclose() for VMS
 * originally written for port of perl to vms
 */

/* (to aid porting) - how are errors dealt with */

#define ERROR(msg) { fprintf(stderr, "%s\nFile %s line %d\n", msg, __FILE__, __LINE__); }
#define FATAL(msg) { fprintf(stderr, "%s\nFile %s line %d\n", msg, __FILE__, __LINE__); exit(GRACE_EXIT_FAILURE); }

#include <unistd.h>
#include <string.h>
#include <descrip.h>
#include <starlet.h>
#include <lib$routines.h>
#include <dvidef.h>
#include <syidef.h>
#include <jpidef.h>
#include <ssdef.h>

#define _cksts(call) \
  if (!(sts=(call))&1) FATAL("Internal error") else {}

static void
create_mbx(unsigned short int *chan, struct dsc$descriptor_s *namdsc)
{
	static unsigned long int mbxbufsiz;
		long int syiitm = SYI$_MAXBUF, dviitm = DVI$_DEVNAM;
	unsigned long sts;  /* for _cksts */
  
  if (!mbxbufsiz) {
    /*
     * Get the SYSGEN parameter MAXBUF, and the smaller of it and the
     * preprocessor consant BUFSIZ from stdio.h as the size of the
     * 'pipe' mailbox.
     */

    _cksts(lib$getsyi(&syiitm, &mbxbufsiz, 0, 0, 0, 0));
    if (mbxbufsiz > BUFSIZ) mbxbufsiz = BUFSIZ; 
  }
  _cksts(sys$crembx(0,chan,mbxbufsiz,mbxbufsiz,0,0,0));

  _cksts(lib$getdvi(&dviitm, chan, NULL, NULL, namdsc, &namdsc->dsc$w_length));
  namdsc->dsc$a_pointer[namdsc->dsc$w_length] = '\0';

}  /* end of create_mbx() */

struct pipe_details
{
    struct pipe_details *next;
    FILE *fp;
    int pid;
    unsigned long int completion;
};

static struct pipe_details *open_pipes = NULL;
static $DESCRIPTOR(nl_desc, "NL:");
static int waitpid_asleep = 0;

static void
popen_completion_ast(unsigned long int unused)
{
  if (waitpid_asleep) {
    waitpid_asleep = 0;
    sys$wake(0,0);
  }
}

#pragma message save
#pragma message disable (ADDRCONSTEXT)
FILE *
popen(char *cmd, char *mode)
{
    char mbxname[64];
    unsigned short int chan;
    unsigned long int flags=1;  /* nowait - gnu c doesn't allow &1 */
    struct pipe_details *info;
    struct dsc$descriptor_s namdsc = {sizeof mbxname, DSC$K_DTYPE_T,
                                      DSC$K_CLASS_S, mbxname},
                            cmddsc = {0, DSC$K_DTYPE_T,
                                      DSC$K_CLASS_S, 0};
	unsigned long sts;                            

    if (!(info=malloc(sizeof(struct pipe_details))))
    {
    	ERROR("Cannot malloc space");
    	return NULL;
    }

    info->completion=0;  /* I assume this will remain 0 until terminates */
        
    /* create mailbox */
    create_mbx(&chan,&namdsc);

    /* open a FILE* onto it */
    info->fp=fopen(mbxname, mode);

    /* give up other channel onto it */
    _cksts(sys$dassgn(chan));

    if (!info->fp)
        return NULL;
        
    cmddsc.dsc$w_length=strlen(cmd);
    cmddsc.dsc$a_pointer=cmd;

    if (strcmp(mode,"r")==0) {
      _cksts(lib$spawn(&cmddsc, &nl_desc, &namdsc, &flags,
                     0  /* name */, &info->pid, &info->completion,
                     0, popen_completion_ast,0,0,0,0));
    }
    else {
      _cksts(lib$spawn(&cmddsc, &namdsc, 0 /* sys$output */, &flags,
                     0  /* name */, &info->pid, &info->completion));
    }

    info->next=open_pipes;  /* prepend to list */
    open_pipes=info;
        
    return info->fp;
}
#pragma message restore

int pclose(FILE *fp)
{
    struct pipe_details *info, *last = NULL;
    unsigned long int abort = SS$_TIMEOUT, retsts;
    unsigned long sts;
    
    for (info = open_pipes; info != NULL; last = info, info = info->next)
        if (info->fp == fp) break;

    if (info == NULL)
      /* get here => no such pipe open */
      /* FATAL("pclose() - no such pipe open ???"); too extreme, removed*/
		return -1;								/* standard behaviour */

    if (!info->completion) { /* Tap them gently on the shoulder . . .*/
      _cksts(sys$forcex(&info->pid,0,&abort));
      sleep(1);
    }
    if (!info->completion)  /* We tried to be nice . . . */
      _cksts(sys$delprc(&info->pid));
    
    fclose(info->fp);
    /* remove from list of open pipes */
    if (last) last->next = info->next;
    else open_pipes = info->next;
    retsts = info->completion;
    free(info);

    return retsts;
}  /* end of pclose() */


/* sort-of waitpid; use only with popen() */
/*{{{unsigned long int waitpid(unsigned long int pid, int *statusp, int flags)*/
unsigned long int
waitpid(unsigned long int pid, int *statusp, int flags)
{
    struct pipe_details *info;
    unsigned long int abort = SS$_TIMEOUT;
    unsigned long sts;
    
    for (info = open_pipes; info != NULL; info = info->next)
        if (info->pid == pid) break;

    if (info != NULL) {  /* we know about this child */
      while (!info->completion) {
        waitpid_asleep = 1;
        sys$hiber();
      }

      *statusp = info->completion;
      return pid;
    }
    else {  /* we haven't heard of this child */
      $DESCRIPTOR(intdsc,"0 00:00:01");
      unsigned long int ownercode = JPI$_OWNER, ownerpid, mypid;
      unsigned long int interval[2];

      _cksts(lib$getjpi(&ownercode,&pid,0,&ownerpid,0,0));
      _cksts(lib$getjpi(&ownercode,0,0,&mypid,0,0));
      if (ownerpid != mypid)
        FATAL("pid not a child");

      _cksts(sys$bintim(&intdsc,interval));
      while ((sts=lib$getjpi(&ownercode,&pid,0,&ownerpid,0,0)) & 1) {
        _cksts(sys$schdwk(0,0,interval,0));
        _cksts(sys$hiber());
      }
      _cksts(sts);

      /* There's no easy way to find the termination status a child we're
       * not aware of beforehand.  If we're really interested in the future,
       * we can go looking for a termination mailbox, or chase after the
       * accounting record for the process.
       */
      *statusp = 0;
      return pid;
    }
                    
}  /* end of waitpid() */

#  else /* not VMS */

/* temporary filename */
static char tfile[GR_MAXPATHLEN];
/* closing command    */
static char closecom[GR_MAXPATHLEN];
/* filter action */
static enum filtact { FILTER_NONE, FILTER_READ, FILTER_WRITE } filt_act;

FILE *popen(char *cmd, char *mode)
{
    switch (mode[0]) {
    case 'r':
        filt_act = FILTER_READ;
        strcpy(buf, cmd);
        strcat(buf, " > ");
        break;
    case 'w':
        filt_act = FILTER_WRITE;
        strcpy(buf, cmd);
        strcat(buf, " < ");
        break;
    default:
        filt_act = FILTER_NONE;
        return NULL;
    }
    
    strcat(buf, tmpnam(tfile));
    if (system(buf) == -1) {
        tfile[0] = '\0';
        filt_act = FILTER_NONE;
        return NULL;            
    } else {
        return fopen(tfile, mode);
    }
}

int pclose(FILE *fp)
{
    int result = -1;
    
    result = fclose( fp );
    switch( filt_act ) {
    case FILTER_READ:
        remove( tfile );
        tfile[0] = '\0';
        break;
    case FILTER_WRITE:
        system( closecom );
        remove( tfile );
        tfile[0] = '\0';
        break;
    default:
        break;
    }
    
    return result;
}

#  endif /* VMS */

#endif /* HAVE_POPEN */

#ifdef __EMX__
char *popen_path_translate(char *path)
{
    static char absfn[GR_MAXPATHLEN];
    
    _abspath(absfn, path, GR_MAXPATHLEN);
    
    return(absfn);
}
#endif
