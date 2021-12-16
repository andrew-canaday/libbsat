## Run autoconf on this file to produce configure.in
##=============================================================================
##
## libbsat: configure.ac:
##
##=============================================================================


#-----------------------------
#           Init:
#-----------------------------

# NOTE: This is the PACKAGE release version, NOT the ABI version. It is SemVer:
AC_INIT([libbsat],m4_esyscmd([printf "${BSAT_VERSION:-"0.1.1"}"]),
	[andrew.canaday@gmail.com],[libbsat],
	[https://github.com/andrew-canaday/libbsat])

# NOTE: ABI versioning is C.R.A. (libtool) not M.m.p (semver)!!
# C.R.A. = Current, Revision, Age:
# - Current: the interface version
# - Age: how many versions prior to "current" current is backwards compat with
# - Revision: how many times current has been released, publicly
#
# Semver translation:
# - when it would be a "patch level" release: bump revision only
# - when it would be a "minor level" release: bump current, revision, AND age
# - when it would be a "major level" release: bump current; age = revision = 0
#
# For more info, see:
# - https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# - https://autotools.io/libtool/version.html
AC_SUBST([BSAT_LIB_VERSION],[1:2:1]) # ABI Version

# More package metadata:
AC_SUBST([PACKAGE_LICENSE],["MIT"])
AC_SUBST([PACKAGE_VENDOR],["Andrew T. Canaday"])
AC_SUBST([PACKAGE_COPYRIGHT],
    ["© copyright 2021 Andrew T. Canaday"])
AC_COPYRIGHT([© copyright 2021 Andrew T. Canaday])
AC_SUBST([PACKAGE_DESCRIPTION],["Utility library for managing libev timeouts"])
AC_SUBST([PACKAGE_SUMMARY],["Utility library for managing libev timeouts"])

# HACK: get the version components (note: we get awk early here)
AC_PROG_AWK
AC_SUBST([PACKAGE_MAJOR],[`echo $PACKAGE_VERSION | awk -F. '{print $1}'`])
AC_SUBST([PACKAGE_MINOR],[`echo $PACKAGE_VERSION | awk -F. '{print $2}'`])
AC_SUBST([PACKAGE_PATCH],[`echo $PACKAGE_VERSION | awk -F. '{print $3}'`])
AC_SUBST([BSAT_VERSION],[`echo $PACKAGE_VERSION | awk -F. '{printf("0x%02x%02x%02x00", $1, $2, $3)}'`])

# Tell autoconf where to find stuff:
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_MACRO_DIR([m4])

#-----------------------------
#         Programs:
#-----------------------------
AM_INIT_AUTOMAKE([-Wall foreign])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

#-----------------------------
#   Make sure we have libev:
#-----------------------------
AC_CHECK_LIB([ev],[ev_version_major],[],[
    AC_MSG_ERROR([libev is required to build libbsat])
])

AC_CHECK_HEADERS([ev.h],[],[
    AC_MSG_ERROR([libev is required to build libbsat])
])

#-----------------------------
#           Types:
#-----------------------------
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

#-----------------------------
#         Features:
#-----------------------------
AC_ARG_ENABLE([maintainer-debug],
    AS_HELP_STRING(
        [--enable-maintainer-debug],
        [Stricter compile-time rules and increased verbosity for maintainers]))

# If maintainer debug is set, get picky and be verbose:
AS_IF([test "x$enable_maintainer_debug" = "xyes"], [
    AC_MSG_NOTICE([Maintainer debug enabled])
    CFLAGS="${CFLAGS} -O0 -g -Wall -std=c99 -Werror "
    ])
AM_CONDITIONAL([MAINTAINER_DEBUG],[test "x$enable_maintainer_debug" = "xyes"])

#-----------------------------
#          Output:
#-----------------------------
AC_CONFIG_HEADERS([bsat_config.h])
AC_CONFIG_FILES(
  include/bsat.h
  Makefile
  lib/Makefile
  test/Makefile
  util/Makefile
  include/Makefile
  example/Makefile
  libbsat.pc
)
AC_OUTPUT
AS_BOX([libbsat configuration complete])

