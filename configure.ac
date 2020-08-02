AC_INIT([liblog], [1.0], [jibux@jibux.info])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_PROG_AR
LT_INIT
AC_PROG_CC
AC_CONFIG_MACRO_DIRS([m4])

AC_CHECK_HEADERS([libconfig.h],[mypj_found_libconfig_headers=yes; break;])

AS_IF([test "x$mypj_found_libconfig_headers" != "xyes"],
	[AC_MSG_ERROR([Unable to find libconfig.h])])

AC_CONFIG_FILES([
Makefile
shared/Makefile
src/Makefile
])

CFLAGS=${FLAGS}" "-lconfig

AC_OUTPUT
