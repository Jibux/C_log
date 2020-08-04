AC_INIT([testlog], [1.0], [jibux@jibux.info])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_PROG_CC

AC_CHECK_HEADERS([libconfig.h],[mypj_found_libconfig_headers=yes; break;])

AS_IF([test "x$mypj_found_libconfig_headers" != "xyes"],
	[AC_MSG_ERROR([Unable to find libconfig.h])])

AC_CONFIG_FILES([
Makefile
example/Makefile
])

CFLAGS=${FLAGS}" "-lconfig

AC_OUTPUT
