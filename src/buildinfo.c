/* buildinfo.c */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#ifndef NONE_GUI
#  include <Xm/Xm.h>
#endif

#include <t1lib.h>

#include "patchlevel.h"

static void VersionInfo(FILE *outfile)
{

    struct utsname u_info;
    time_t time_info;

    fprintf(outfile, "Version: Grace-%d.%d.%d %s\n",
	    MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER);

/* We don't want to reproduce the complete config.h,
   but those settings which may be related to problems on runtime */

#ifdef NONE_GUI
    fprintf(outfile, "GUI: none\n");
#else
    fprintf(outfile, "GUI: %s\n", XmVERSION_STRING);
#endif
    
    fprintf(outfile, "T1lib: %s\n", T1_GetLibIdent());
    
#ifdef WITH_DEBUG
    fprintf(outfile, "Debugging is enabled\n");
#else
    fprintf(outfile, "Debugging is disabled\n");
#endif

    uname(&u_info);
    fprintf(outfile, "Built on: %s %s %s, %s \n", u_info.sysname, u_info.release,
	    u_info.version, u_info.machine);
    time_info = time(NULL);
    fprintf(outfile, "Build time: %s", ctime(&time_info));

    return;
}

int main(int argc, char *argv[])
{
    FILE *outfile;

    if (argc == 1) {
	outfile = stdout;
    } else {
	outfile = fopen(argv[1], "w");
        fprintf(stderr, "Failed to open file %s for writing!\a\n", argv[1]);
    }
    if (!outfile) {
        exit(1);
    }

    VersionInfo(outfile);

    fclose(outfile);
    exit(0);
}
