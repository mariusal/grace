dnl ICE_FIND_MOTIF
dnl --------------
dnl
dnl Find Motif libraries and headers
dnl Put Motif include directory in motif_includes,
dnl put Motif library directory in motif_libraries,
dnl and add appropriate flags to X_CFLAGS and X_LIBS.
dnl
dnl
AC_DEFUN(ICE_FIND_MOTIF,
[
AC_REQUIRE([AC_PATH_XTRA])
motif_includes=
motif_libraries=
AC_ARG_WITH(motif,
[  --without-motif         do not use Motif widgets])
dnl Treat --without-motif like
dnl --without-motif-includes --without-motif-libraries.
if test "$with_motif" = "no"
then
motif_includes=no
motif_libraries=no
fi
AC_ARG_WITH(motif-includes,
[  --with-motif-includes=DIR    Motif include files are in DIR],
motif_includes="$withval")
AC_ARG_WITH(motif-libraries,
[  --with-motif-libraries=DIR   Motif libraries are in DIR],
motif_libraries="$withval")
AC_MSG_CHECKING(for Motif)
#
#
# Search the include files.
#
if test "$motif_includes" = ""; then
AC_CACHE_VAL(ice_cv_motif_includes,
[
ice_motif_save_LIBS="$LIBS"
ice_motif_save_CFLAGS="$CFLAGS"
ice_motif_save_CPPFLAGS="$CPPFLAGS"
ice_motif_save_LDFLAGS="$LDFLAGS"
#
LIBS="$X_PRE_LIBS -lXm -lXt -lX11 $X_EXTRA_LIBS $LIBS"
CFLAGS="$X_CFLAGS $CFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"
#
AC_TRY_COMPILE([#include <Xm/Xm.h>],[int a;],
[
# Xm/Xm.h is in the standard search path.
ice_cv_motif_includes=
],
[
# Xm/Xm.h is not in the standard search path.
# Locate it and put its directory in `motif_includes'
#
# /usr/include/Motif* are used on HP-UX (Motif).
# /usr/include/X11* are used on HP-UX (X and Athena).
# /usr/dt is used on Solaris (Motif).
# /usr/openwin is used on Solaris (X and Athena).
# Other directories are just guesses.
for dir in "$x_includes" "${prefix}/include" /usr/include /usr/local/include \
           /usr/include/Motif2.0 /usr/include/Motif1.2 /usr/include/Motif1.1 \
           /usr/include/X11R6 /usr/include/X11R5 /usr/include/X11R4 \
           /usr/dt/include /usr/openwin/include \
           /usr/dt/*/include /opt/*/include /usr/include/Motif* \
           "${prefix}"/*/include /usr/*/include /usr/local/*/include \
           "${prefix}"/include/* /usr/include/* /usr/local/include/*; do
if test -f "$dir/Xm/Xm.h"; then
ice_cv_motif_includes="$dir"
break
fi
done
])
#
LIBS="$ice_motif_save_LIBS"
CFLAGS="$ice_motif_save_CFLAGS"
CPPFLAGS="$ice_motif_save_CPPFLAGS"
LDFLAGS="$ice_motif_save_LDFLAGS"
])
motif_includes="$ice_cv_motif_includes"
fi
#
#
# Now for the libraries.
#
if test "$motif_libraries" = ""; then
AC_CACHE_VAL(ice_cv_motif_libraries,
[
ice_motif_save_LIBS="$LIBS"
ice_motif_save_CFLAGS="$CFLAGS"
ice_motif_save_CPPFLAGS="$CPPFLAGS"
ice_motif_save_LDFLAGS="$LDFLAGS"
#
LIBS="$X_PRE_LIBS -lXm -lXt -lX11 $X_EXTRA_LIBS $LIBS"
CFLAGS="$X_CFLAGS $CFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"
#
AC_TRY_LINK([#include <Xm/Xm.h>],[XtToolkitInitialize();],
[
# libXm.a is in the standard search path.
ice_cv_motif_libraries=
],
[
# libXm.a is not in the standard search path.
# Locate it and put its directory in `motif_libraries'
#
# /usr/lib/Motif* are used on HP-UX (Motif).
# /usr/lib/X11* are used on HP-UX (X and Athena).
# /usr/dt is used on Solaris (Motif).
# /usr/lesstif is used on Linux (Lesstif).
# /usr/openwin is used on Solaris (X and Athena).
# Other directories are just guesses.
for dir in "$x_libraries" "${prefix}/lib" /usr/lib /usr/local/lib \
           /usr/lib/Motif2.0 /usr/lib/Motif1.2 /usr/lib/Motif1.1 \
           /usr/lib/X11R6 /usr/lib/X11R5 /usr/lib/X11R4 /usr/lib/X11 \
           /usr/dt/lib /usr/openwin/lib \
           /usr/dt/*/lib /opt/*/lib /usr/lib/Motif* \
           /usr/lesstif*/lib /usr/lib/Lesstif* \
           "${prefix}"/*/lib /usr/*/lib /usr/local/*/lib \
           "${prefix}"/lib/* /usr/lib/* /usr/local/lib/*; do
if test -d "$dir" && test "`ls $dir/libXm.* 2> /dev/null`" != ""; then
ice_cv_motif_libraries="$dir"
break
fi
done
])
#
LIBS="$ice_motif_save_LIBS"
CFLAGS="$ice_motif_save_CFLAGS"
CPPFLAGS="$ice_motif_save_CPPFLAGS"
LDFLAGS="$ice_motif_save_LDFLAGS"
])
#
motif_libraries="$ice_cv_motif_libraries"
fi
# Add Motif definitions to X flags
#
if test "$motif_includes" != "" && test "$motif_includes" != "$x_includes" && test "$motif_includes" != "no"
then
X_CFLAGS="-I$motif_includes $X_CFLAGS"
fi
if test "$motif_libraries" != "" && test "$motif_libraries" != "$x_libraries" && test "$motif_libraries" != "no"
then
case "$X_LIBS" in
  *-R\ *) X_LIBS="-L$motif_libraries -R $motif_libraries $X_LIBS";;
  *-R*)   X_LIBS="-L$motif_libraries -R$motif_libraries $X_LIBS";;
  *)      X_LIBS="-L$motif_libraries $X_LIBS";;
esac
fi
#
#
motif_libraries_result="$motif_libraries"
motif_includes_result="$motif_includes"
test "$motif_libraries_result" = "" &&
  motif_libraries_result="in default path"
test "$motif_includes_result" = "" &&
  motif_includes_result="in default path"
test "$motif_libraries_result" = "no" &&
  motif_libraries_result="(none)"
test "$motif_includes_result" = "no" &&
  motif_includes_result="(none)"
AC_MSG_RESULT(
  [libraries $motif_libraries_result, headers $motif_includes_result])
])dnl

