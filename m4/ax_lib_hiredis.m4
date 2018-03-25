# ===========================================================================
#           http://autoconf-archive.cryp.to/ax_lib_hiredis.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_LIB_HIREDIS([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   This macro provides tests of availability of Hiredis 'libhiredis' library
#   of particular version or newer.
#
#   AX_LIB_HIREDIS macro takes only one argument which is optional. If
#   there is no required version passed, then macro does not run version
#   test.
#
#   The --with-hiredis option takes one of three possible values:
#
#   no - do not check for Hiredis client library
#
#   yes - do check for Hiredis library in standard locations (pg_config
#   should be in the PATH)
#
#   path - complete path to pg_config utility, use this option if pg_config
#   can't be found in the PATH
#
#   This macro calls:
#
#     AC_SUBST(HIREDIS_CFLAGS)
#     AC_SUBST(HIREDIS_LDFLAGS)
#     AC_SUBST(HIREDIS_VERSION)
#
#   And sets:
#
#     HAVE_HIREDIS
#
# LICENSE
#
#   Copyright (c) 2008 Mateusz Loskot <mateusz@loskot.net>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AX_LIB_HIREDIS],
[
    AC_ARG_WITH([hiredis],
        AC_HELP_STRING([--with-hiredis=@<:@ARG@:>@],
            [use Hiredis library @<:@default=yes@:>@, optionally specify path to pg_config]
        ),
        [
        if test "$withval" = "no"; then
            want_hiredis="no"
        elif test "$withval" = "yes"; then
            want_hiredis="yes"
        else
            want_hiredis="yes"
            PG_CONFIG="$withval"
        fi
        ],
        [want_hiredis="yes"]
    )

    HIREDIS_CFLAGS=""
    HIREDIS_LDFLAGS=""
    HIREDIS_VERSION=""

    dnl
    dnl Check Hiredis libraries (libhiredis)
    dnl

    if test "$want_hiredis" = "yes"; then

        if test -z "$PG_CONFIG" -o test; then
            AC_PATH_PROG([PG_CONFIG], [pg_config], [])
        fi

        if test ! -x "$PG_CONFIG"; then
            AC_MSG_ERROR([$PG_CONFIG does not exist or it is not an exectuable file])
            PG_CONFIG="no"
            found_hiredis="no"
        fi

        if test "$PG_CONFIG" != "no"; then
            AC_MSG_CHECKING([for Hiredis libraries])

            HIREDIS_CFLAGS="-I`$PG_CONFIG --includedir`"
            HIREDIS_LDFLAGS="-L`$PG_CONFIG --libdir` -lhiredis"

            HIREDIS_VERSION=`$PG_CONFIG --version | sed -e 's#Hiredis ##'`

            AC_DEFINE([HAVE_HIREDIS], [1],
                [Define to 1 if Hiredis libraries are available])

            found_hiredis="yes"
            AC_MSG_RESULT([yes])
        else
            found_hiredis="no"
            AC_MSG_RESULT([no])
        fi
    fi

    dnl
    dnl Check if required version of Hiredis is available
    dnl


    hiredis_version_req=ifelse([$1], [], [], [$1])

    if test "$found_hiredis" = "yes" -a -n "$hiredis_version_req"; then

        AC_MSG_CHECKING([if Hiredis version is >= $hiredis_version_req])

        dnl Decompose required version string of Hiredis
        dnl and calculate its number representation
        hiredis_version_req_major=`expr $hiredis_version_req : '\([[0-9]]*\)'`
        hiredis_version_req_minor=`expr $hiredis_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        hiredis_version_req_micro=`expr $hiredis_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$hiredis_version_req_micro" = "x"; then
            hiredis_version_req_micro="0"
        fi

        hiredis_version_req_number=`expr $hiredis_version_req_major \* 1000000 \
                                   \+ $hiredis_version_req_minor \* 1000 \
                                   \+ $hiredis_version_req_micro`

        dnl Decompose version string of installed Hiredis
        dnl and calculate its number representation
        hiredis_version_major=`expr $HIREDIS_VERSION : '\([[0-9]]*\)'`
        hiredis_version_minor=`expr $HIREDIS_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
        hiredis_version_micro=`expr $HIREDIS_VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$hiredis_version_micro" = "x"; then
            hiredis_version_micro="0"
        fi

        hiredis_version_number=`expr $hiredis_version_major \* 1000000 \
                                   \+ $hiredis_version_minor \* 1000 \
                                   \+ $hiredis_version_micro`

        hiredis_version_check=`expr $hiredis_version_number \>\= $hiredis_version_req_number`
        if test "$hiredis_version_check" = "1"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
        fi
    fi

    AC_SUBST([HIREDIS_VERSION])
    AC_SUBST([HIREDIS_CFLAGS])
    AC_SUBST([HIREDIS_LDFLAGS])
])
