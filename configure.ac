# Process this file with autoconf to produce a configure script.
AC_INIT(main.c)
AM_INIT_AUTOMAKE(xmonobut, 0.4)
AM_CONFIG_HEADER(config.h)

CFLAGS="-Os -Wall"

# Checks for programs.
AC_PROG_CC

# Checks for header files.
AC_PATH_X
AC_PATH_XTRA
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_VPRINTF

AC_OUTPUT(Makefile ipkg/control ipkg/Makefile doc/Makefile)
