dnl --- *@-mdw_CHECK_MANYLIBS-@* ---
dnl
dnl Author:     Mark Wooding
dnl
dnl Synopsis:   mdw_CHECK_MANYLIBS(FUNC, LIBS, [IF-FOUND], [IF-NOT-FOUND])
dnl
dnl Arguments:  FUNC = a function to try to find
dnl             LIBS = a whitespace-separated list of libraries to search
dnl             IF-FOUND = what to do when the function is found
dnl             IF-NOT-FOUND = what to do when the function isn't found
dnl
dnl Use:        Searches for a library which defines FUNC.  It first tries
dnl             without any libraries; then it tries each library specified
dnl             in LIBS in turn.  If it finds a match, it adds the
dnl             appropriate library to `LIBS'.
dnl
dnl             This is particularly handy under DIREIX: if you link with
dnl             `-lnsl' then you get non-NIS-aware versions of getpwnam and
dnl             so on, which is clearly a Bad Thing.
dnl
dnl Modified:   by Evgeny Stambulchik to add (found) libraries to `LIBS'
dnl             *only* if `IF-FOUND' is absent. As well, if no additional
dnl             library is needed for `FUNC', `mdw_cv_lib_$1' sets to "".

AC_DEFUN(mdw_CHECK_MANYLIBS,
[AC_CACHE_CHECK([for library containing $1], [mdw_cv_lib_$1],
[mdw_save_LIBS="$LIBS"
mdw_cv_lib_$1="no"
AC_TRY_LINK(,[$1()], [mdw_cv_lib_$1="none required"])
test "$mdw_cv_lib_$1" = "no" && for i in $2; do
LIBS="-l$i $mdw_save_LIBS"
AC_TRY_LINK(,[$1()],
[mdw_cv_lib_$1="-l$i"
break])
done
LIBS="$mdw_save_LIBS"])
if test "$mdw_cv_lib_$1" != "no"; then
  if test "x$3" != "x"; then
    test "$mdw_cv_lib_$1" = "none required" && mdw_cv_lib_$1=""
    $3
  else
    test "$mdw_cv_lib_$1" = "none required" || LIBS="$mdw_cv_lib_$1 $LIBS"
  fi
else :
  $4
fi])
