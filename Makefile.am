SUBDIRS = include src

if ENABLE_EXAMPLES
SUBDIRS += libevquick examples
endif

if ENABLE_TOOLS
SUBDIRS += libevquick tools
endif

# AM_MAKEFLAGS = -s
ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libvsb.pc

EXTRA_DIST = \
	libvsb.pc.in
