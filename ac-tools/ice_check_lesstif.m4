dnl ICE_CHECK_LESSTIF
dnl -----------------
dnl
dnl Define `HAVE_LESSTIF' if the Motif library is actually a LessTif library
dnl
AC_DEFUN(ICE_CHECK_LESSTIF,
[
AC_MSG_CHECKING(whether the Motif library is actually a LessTif library)
AC_CACHE_VAL(ice_cv_have_lesstif,
AC_EGREP_CPP(yes,
[#include <Xm/Xm.h>
#ifdef LesstifVersion
yes
#endif
], ice_cv_have_lesstif=yes, ice_cv_have_lesstif=no))
AC_MSG_RESULT($ice_cv_have_lesstif)
if test "$ice_cv_have_lesstif" = yes; then
AC_DEFINE(HAVE_LESSTIF)
fi
])dnl
