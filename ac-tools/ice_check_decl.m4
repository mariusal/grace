dnl ICE_CHECK_DECL (FUNCTION, HEADER-FILE...)
dnl -----------------------------------------
dnl
dnl If FUNCTION is available, define `HAVE_FUNCTION'.  If it is declared
dnl in one of the headers named in the whitespace-separated list
dnl HEADER_FILE, define `HAVE_FUNCTION_DECL` (in all capitals).
dnl
AC_DEFUN(ICE_CHECK_DECL,
[
changequote(,)dnl
ice_tr=`echo $1 | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
ice_have_tr=HAVE_$ice_tr
ice_have_decl_tr=${ice_have_tr}_DECL
ice_have_$1=no
AC_CHECK_FUNCS($1, ice_have_$1=yes)
if test "${ice_have_$1}" = yes; then
AC_MSG_CHECKING(for $1 declaration in $2)
AC_CACHE_VAL(ice_cv_have_$1_decl,
[
ice_cv_have_$1_decl=no
changequote(,)dnl
ice_re_params='[a-zA-Z_][a-zA-Z0-9_]*'
ice_re_word='(^|[^a-zA-Z_0-9_])'
changequote([,])dnl
for header in $2; do
# Check for ordinary declaration
AC_EGREP_HEADER([${ice_re_word}$1 *\(], $header,
	ice_cv_have_$1_decl=yes)
if test "$ice_cv_have_$1_decl" = yes; then
	break
fi
# Check for "fixed" declaration like "getpid _PARAMS((int))"
AC_EGREP_HEADER([${ice_re_word}$1 *$ice_re_params\(\(], $header,
	ice_cv_have_$1_decl=yes)
if test "$ice_cv_have_$1_decl" = yes; then
	break
fi
done
])
AC_MSG_RESULT($ice_cv_have_$1_decl)
if test "$ice_cv_have_$1_decl" = yes; then
AC_DEFINE_UNQUOTED(${ice_have_decl_tr})
fi
fi
])dnl

