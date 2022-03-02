AC_INIT([phidget22extra C Library],
  m4_esyscmd_s([cat library_version]),
  [support@phidgets.com],
  [libphidget22extra],
  [www.phidgets.com])

AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([-Wall subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_PROG_AR],[AM_PROG_AR])

AC_CANONICAL_HOST
AS_CASE([$host_cpu],
  [x86_64],
	AC_DEFINE_UNQUOTED(_HOST_CPU, [$host_cpu])
	AC_DEFINE_UNQUOTED(_MACHINE, [$host_cpu])
  [
  ],
  [i?86],
  [
	AC_DEFINE_UNQUOTED(_HOST_CPU, [$host_cpu])
	AC_DEFINE(_MACHINE, [i386])
  ],
  [*],
  [
	AC_DEFINE_UNQUOTED(_HOST_CPU, [$host_cpu])
	AC_DEFINE_UNQUOTED(_MACHINE, [$host_cpu])
  ]
)

LT_INIT

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([no])])

# Install in /usr/local
AC_PREFIX_DEFAULT([/usr/local])

AC_PROG_CC_C99
if test "x$ac_cv_prog_c99" == "xno" ; then
	AC_MSG_ERROR([A c99 compiler is required])
fi

AC_PROG_LIBTOOL

AC_ARG_VAR([LDCONFIG], [ldconfig utility])
AC_PATH_PROG([LDCONFIG], [ldconfig], [true], [/sbin /usr/sbin $PATH])

case "$host_os" in
*linux*)
	# Checks for libraries.
	AC_SEARCH_LIBS([dlopen], [dl], [],
		AC_MSG_ERROR([Missing libdl!]))
	AC_SEARCH_LIBS([sqrt], [m], [],
		AC_MSG_ERROR([Missing libm!]))
	AC_SEARCH_LIBS([pthread_create], [pthread],
		[AC_DEFINE(_MOS_PTHREADS, 1)],
		AC_MSG_ERROR([Missing libpthread!]))
	AC_SEARCH_LIBS([libusb_init], [usb-1.0], [],
		AC_MSG_ERROR([Missing libusb!]))
	AC_DEFINE(UNIX, 1)
	AC_DEFINE(_LINUX, 1)
	AC_DEFINE(Linux, 1)
	;;
freebsd*)
	AC_SEARCH_LIBS([sqrt], [m], [],
		AC_MSG_ERROR([Missing libm!]))
	AC_SEARCH_LIBS([pthread_create], [pthread],
		[AC_DEFINE(_MOS_PTHREADS, 1)],
		AC_MSG_ERROR([Missing libpthread!]))
	AC_SEARCH_LIBS([libusb_init], [usb], [],
		AC_MSG_ERROR([Missing libusb!]))
	AC_DEFINE(UNIX, 1)
	AC_DEFINE(_FREEBSD, 1)
	AC_DEFINE(FreeBSD, 1)
	;;
*)
	AC_MSG_ERROR(Unrecognized OS $host_os)
	;;
esac

# Checks for libraries.
AC_SEARCH_LIBS([Phidget_getLibraryVersion], [phidget22],
	[AC_DEFINE([_PHIDGET22], 1)],
	AC_MSG_ERROR([Missing libphidget22; make sure the Phidget22 library is installed.]))
AC_SEARCH_LIBS([dlopen], [dl], [], 
	AC_MSG_ERROR([Missing libdl!]))

AC_ARG_ENABLE([debug],
[AS_HELP_STRING([--enable-debug], [Define to enable debugging])])
AS_IF([test "x$enable_debug" == "xyes"], [
  AC_DEFINE([DEBUG], 1, [Define to enable debugging])
  AC_DEFINE([_DEBUG], 1, [Define to enable debugging])
])

AS_IF([test "x$enable_debug" == "xno"], [
  AC_DEFINE([NDEBUG], 1, [Define to disable debugging])
])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT([libphidget22extra.pc])
