/* buildinfo.c */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_UNAME
#  include <sys/utsname.h>
#endif
#include <time.h>
#if !defined(NONE_GUI) && !defined(QT_GUI)
#  include <Xm/Xm.h>
#  include <Xbae/patchlevel.h>
#endif

#include <t1lib.h>

#ifndef GRACE_HOME
#  define GRACE_HOME "/usr/local/gapp"
#endif

#ifndef GRACE_PRINT_CMD
#  define GRACE_PRINT_CMD ""
#endif

#ifndef GRACE_EDITOR
#  define GRACE_EDITOR "xterm -e vi"
#endif

#ifndef GRACE_HELPVIEWER
#  define GRACE_HELPVIEWER "mozilla -remote openURL\\\\(%s,new-window\\\\) >>/dev/null 2>&1 || mozilla %s"
#endif


static void VersionInfo(FILE *outfile)
{

#ifdef HAVE_UNAME
    struct utsname u_info;
#endif
    time_t time_info;
    char *ctime_string;

    fprintf(outfile, "#define BI_VERSION_ID %d\n",
            MAJOR_REV*10000 + MINOR_REV*100 + PATCHLEVEL);
#ifdef BETA_VER
    fprintf(outfile, "#define BI_VERSION \"Grace-%d.%d.%d %s\"\n",
	    MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER);
#else
    fprintf(outfile, "#define BI_VERSION \"Grace-%d.%d.%d\"\n",
	    MAJOR_REV, MINOR_REV, PATCHLEVEL);
#endif

/* We don't want to reproduce the complete config.h,
   but those settings which may be related to problems at runtime */

#ifdef NONE_GUI
    fprintf(outfile, "#define BI_GUI \"none\"\n");
#else
#  ifdef QT_GUI
    fprintf(outfile, "#define BI_GUI \"Qt\"\n");
#  else
    fprintf(outfile, "#define BI_GUI \"%s\"\n", XmVERSION_STRING);
    fprintf(outfile, "#define BI_GUI_XBAE \"%i\"\n", XbaeVersion);
#  endif
#endif
    
    fprintf(outfile, "#define BI_T1LIB \"%s\"\n",
        T1_GetLibIdent());

    fprintf(outfile, "#define BI_CCOMPILER \"%s\"\n",
        CCOMPILER);
    
#ifdef HAVE_UNAME
    uname(&u_info);
    fprintf(outfile, "#define BI_SYSTEM \"%s %s %s\"\n",
        u_info.sysname, u_info.release, u_info.machine);
#else
    fprintf(outfile, "#define BI_SYSTEM \"a non-POSIX OS\"\n");
#endif

    time_info = time(NULL);
    ctime_string = ctime(&time_info);
    if (ctime_string[strlen(ctime_string) - 1] == '\n') {
        ctime_string[strlen(ctime_string) - 1] = '\0';
    }
    fprintf(outfile, "#define BI_DATE \"%s\"\n", ctime_string);

    fprintf(outfile, "\n");

    fprintf(outfile, "#define GRACE_HOME \"%s\"\n", GRACE_HOME);
    fprintf(outfile, "#define GRACE_EDITOR \"%s\"\n", GRACE_EDITOR);
    fprintf(outfile, "#define GRACE_PRINT_CMD \"%s\"\n", GRACE_PRINT_CMD);
    fprintf(outfile, "#define GRACE_HELPVIEWER \"%s\"\n", GRACE_HELPVIEWER);

    return;
}


int main(int argc, char *argv[])
{
    FILE *outfile;

    if (argc == 1) {
	outfile = stdout;
    } else {
        if (!(outfile = fopen(argv[1], "w"))) {
            fprintf(stderr, "Failed to open %s for writing!\a\n", argv[1]);
            exit(1);
        }
    }

    VersionInfo(outfile);
   
    if (outfile != stdout) {
        fclose(outfile);
    }
    exit(0);
}
