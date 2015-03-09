SUBDIRS = include src

if ENABLE_LIBEVQUICK
SUBDIRS += libevquick
endif

if ENABLE_EXAMPLES
SUBDIRS += examples
endif

if ENABLE_TOOLS
SUBDIRS += tools
endif

if ENABLE_TESTS
SUBDIRS += tests
endif

# AM_MAKEFLAGS = -s
ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libvsb.pc

EXTRA_DIST = \
	libvsb.pc.in
