## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = std-options
AUTOMAKE_OPTIONS += subdir-objects

AM_CFLAGS = -DUNIX -D_LINUX -DLinux -D_MACHINE=i386 -DMOS_BYTE_ORDER=MOS_LITTLE_ENDIAN -D_MOS_PTHREADS

AM_CFLAGS+= -Werror
AM_CFLAGS+= -Wreturn-type -Wcast-qual -Wwrite-strings
AM_CFLAGS+= -Wpointer-arith
AM_CFLAGS+= -Wall -Wno-format-y2k
AM_CFLAGS+= -Wno-switch
AM_CFLAGS+= -Wmissing-prototypes -Wnested-externs -Wreturn-type
AM_CFLAGS+= -Wshadow -Wchar-subscripts
AM_CFLAGS+= -Wsign-compare

AM_CFLAGS+= -DAM_FIRMWAREDATADIR='"$(pkgdatadir)/firmware"'

bin_PROGRAMS = phidget22admin

phidget22admin_SOURCES = phidgetadmin.h \
	main.c \
	fwupgrade.c \
	fwupgrade.h

if DEBUG
AM_CFLAGS += -D_DEBUG -DDEBUG
endif

EXTRA_DIST = firmware

install-data-local:
	test -z $(DESTDIR)$(pkgdatadir)/firmware || $(MKDIR_P) $(DESTDIR)$(pkgdatadir)/firmware
	find $(srcdir)/firmware -type f -exec $(INSTALL_DATA) {} $(DESTDIR)$(pkgdatadir)/firmware \;

uninstall-local:
	rm -rf $(DESTDIR)$(pkgdatadir)/firmware
	-rmdir $(DESTDIR)${pkgdatadir}
