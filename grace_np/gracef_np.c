#include <config.h>

#if defined(HAVE_F77)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grace_np.h"

typedef void (*GraceFortranFunctionType) (const char *str, int len);
static GraceFortranFunctionType fortran_error = (GraceFortranFunctionType) 0;

static void GraceFortranWrapper(const char *str)
{
    if (fortran_error == (GraceFortranFunctionType) 0) {
        fprintf(stderr, "%s\n", str);
    } else {
        fortran_error(str, strlen(str));
    }
}

void
#ifdef NEED_F77_UNDERSCORE
graceregistererrorfunctionf_ (GraceFortranFunctionType f)
#else
graceregistererrorfunctionf (GraceFortranFunctionType f)
#endif
{
    fortran_error = f;
    GraceRegisterErrorFunction(GraceFortranWrapper);
}

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
graceisopenf_ (void)
#else
graceisopenf (void)
#endif
{
    return (GraceIsOpen ());
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

