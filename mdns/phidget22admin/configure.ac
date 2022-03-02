#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([Phidget22Admin],[m4_esyscmd(./build/version.sh)],[support@phidgets.com],[phidget22admin],[www.phidgets.com])
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([-Wall])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([phidgetadmin.h])
#m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_SEARCH_LIBS([Phidget_getLibraryVersion], [phidget22], [],
	AC_MSG_ERROR([Missing libphidget22; make sure the Phidget22 library is installed.]))

AC_SEARCH_LIBS([getPhidgetExtraVersion], [phidget22extra], [],
	AC_MSG_ERROR([Missing libphidget22extra; make sure the Phidget22Extra library is installed.]))

AC_ARG_ENABLE([debug],
[  --enable-debug    Turn on debugging],
[case "${enableval}" in
	yes) debug=true ;;
	no)  debug=false ;;
	*) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
