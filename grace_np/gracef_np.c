#include <config.h>

#if defined(HAVE_F77)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grace_np.h"

int
#ifdef NEED_F77_UNDERSCORE
graceopenf_ (const int *arg)
#else
graceopenf (const int *arg)
#endif
{
    return (GraceOpen (*arg));
}

int
#ifdef NEED_F77_UNDERSCORE
graceclosef_ (void)
#else
graceclosef (void)
#endif
{
    return (GraceClose ());
}

int
#ifdef NEED_F77_UNDERSCORE
graceflushf_ (void)
#else
graceflushf (void)
#endif
{
    return (GraceFlush ());
}


int
#ifdef NEED_F77_UNDERSCORE
gracecommandf_ (const char* arg, int length)
#else
gracecommandf (const char* arg, int length)
#endif
{
    char* str;
    int res;

    str = (char*) malloc ((size_t) (length + 1));
    if (str == NULL) {
        fprintf (stderr, "GraceCommandf: Not enough memory\n");
        return (-1);
    }
    strncpy (str, arg, length);
    str[length] = 0;
    res = GraceCommand (str);
    free (str);
    return (res);
}

#else /* no Fortran */

/* To make ANSI C happy about non-empty file */
void _gracef_np_c_dummy_func(void) {}

#endif /* HAVE_F77 */

