/* buildinfo.c */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_UTSNAME_H
#  include <sys/utsname.h>
#endif
#ifdef HAVE_TIME_H
#  include <time.h>
#endif
#ifndef NONE_GUI
#  include <Xm/Xm.h>
#endif
#include "patchlevel.h"

#if !defined (EXIT_SUCCESS)
#define EXIT_SUCCESS 0
#endif
#if !defined (EXIT_FAILURE)
#define EXIT_FAILURE 1
#endif

char pBuildString;


void VersionInfo(FILE *outfile) {

#ifdef HAVE_SYS_UTSNAME_H
  struct utsname u_info;
#endif
#ifdef HAVE_TIME_H
  time_t time_info;
#endif

  fprintf(outfile, "Grace-%d.%d.%d %s\n",  
           MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER);

#ifdef HAVE_SYS_UTSNAME_H
  uname(&u_info);
  fprintf(outfile, "%s %s %s on %s \n", u_info.sysname, u_info.version,
                    u_info.release, u_info.machine);
#endif
#ifdef HAVE_TIME_H
  time_info = time(NULL);
  fprintf(outfile, "Buildtime: %s\n", ctime(&time_info));
#endif

/* We don't want to reproduce the complete config.h,
   but those settings which may be related to problems on runtime */

#ifdef NONE_GUI
  fprintf(outfile, "GUI: none\n");
#else
#  ifdef HAVE_LESSTIF
  fprintf(outfile, "GUI: %s\n", LesstifVERSION_STRING);
#  else
  fprintf(outfile, "GUI: %s\n", XmVERSION_STRING);
#  endif

#endif

#ifdef HAVE_LIBT1
  fprintf(outfile, "T1lib: installed version\n");
#else
  fprintf(outfile, "T1lib: bundled version\n");
#endif
#ifdef WITH_DEBUG
  fprintf(outfile, "Debugging is enabled\n");
#else
  fprintf(outfile, "Debugging is disabled\n");
#endif
 
  return;
}

int main (int argc, char *argv[]) {
    FILE *outfile;

    if (argc==1) 
        outfile=stdout;
    else {
        outfile = fopen(argv[1], "w");
    }
    if (!outfile) exit(EXIT_FAILURE);

    VersionInfo(outfile);

    fclose(outfile);
    exit(EXIT_SUCCESS);
}
