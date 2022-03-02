ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS= -Isrc
AM_CFLAGS+= -DPHIDGETEXTRA_SRC=1
AM_CFLAGS+= -Werror
AM_CFLAGS+= -Wreturn-type -Wcast-qual -Wwrite-strings
AM_CFLAGS+= -Wcast-align
AM_CFLAGS+= -Wpointer-arith
AM_CFLAGS+= -Wall -Wno-format-y2k
AM_CFLAGS+= -Wno-switch
AM_CFLAGS+= -Wmissing-prototypes -Wnested-externs -Wreturn-type
AM_CFLAGS+= -Wchar-subscripts
AM_CFLAGS+= -Wredundant-decls -Wsign-compare

#
# PHIDAPI must be defined on each exported symbol
#
AM_CFLAGS+=	-fvisibility=hidden

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libphidget22extra.pc

lib_LTLIBRARIES = libphidget22extra.la

libphidget22extra_la_LDFLAGS = -version-info 0:0:0

libphidget22extra_la_SOURCES = \
	src/pconf.c \
	src/pconf-unix.c \
	src/pconf.flex.c \
	src/projectsupport.c \
	src/phidgethelpers.c \
	src/phidgetdicthelpers.c \
	src/phidgetmatch.c \
	src/plat/unix/modules.c \
	src/plat/unix/daemon.c \
	src/pconf-unix.h

include_HEADERS = \
	src/phidget22extra.h
	
pextraincludedir = $(includedir)/phidget22extra
pextrainclude_HEADERS = \
	src/phidget22extra/phidget22extra_int.h \
	src/phidget22extra/phidgethelpers.h \
	src/phidget22extra/phidgetdicthelpers.h \
	src/phidget22extra/phidgetloghelpers.h \
	src/phidget22extra/projectsupport.h \
	src/phidget22extra/phidgetconfig.h \
	src/phidget22extra/phidgetmatch.h

CLEANFILES = libphidget22extra.pc

EXTRA_DIST = libphidget22extra.pc.in
