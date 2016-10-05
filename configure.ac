# Versioning.
m4_define([dc_version_major],[0])
m4_define([dc_version_minor],[1])
m4_define([dc_version_micro],[0])
m4_define([dc_version_suffix],[devel])
m4_define([dc_version],dc_version_major.dc_version_minor.dc_version_micro[]m4_ifset([dc_version_suffix],-[dc_version_suffix])) 
# Libtool versioning.
m4_define([dc_version_lt_current],[0])
m4_define([dc_version_lt_revision],[0])
m4_define([dc_version_lt_age],[0])

# Initialize autoconf
AC_PREREQ([2.60])
AC_INIT([libvsb],[dc_version])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT([/usr])
# AC_PROG_RANLIB

# Initialize automake.
AM_INIT_AUTOMAKE([-Wall -Werror -Wno-portability foreign])

# Fix for automake >= 1.12
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Initialize libtool.
LT_PREREQ([2.2.0])
LT_INIT()
LT_PROG_RC

# Tool applications.
AC_ARG_ENABLE([vsbtools],
	[AS_HELP_STRING([--enable-vsbtools=@<:@yes/no@:>@],
		[Build vsb tools @<:@default=yes@:>@])],
	[], [enable_vsbtools=yes])
AM_CONDITIONAL([ENABLE_TOOLS], [test "x$enable_vsbtools" = "xyes"])

# Example applications.
AC_ARG_ENABLE([examples],
	[AS_HELP_STRING([--enable-examples=@<:@yes/no@:>@],
		[Build vsb example applications@<:@default=yes@:>@])],
	[], [enable_examples=yes])
AM_CONDITIONAL([ENABLE_EXAMPLES], [test "x$enable_examples" = "xyes"])

AC_MSG_CHECKING([Don't build libevquick])
AC_ARG_WITH(libevquick,
    [  --with-libevquick       Do/Don't build libevquick @<:@default=yes@:>@],
    with_libevquick=$withval,
    with_libevquick=yes)
AC_MSG_RESULT($with_libevquick)

# Tests
AC_ARG_ENABLE([tests],
	[AS_HELP_STRING([--enable-tests=@<:@yes/no@:>@],
		[Build self tests @<:@default=no@:>@])],
	[], [enable_tests=no])
AM_CONDITIONAL([ENABLE_TESTS], [test "x$enable_tests" = "xyes"])

AX_VALGRIND_CHECK

AM_CONDITIONAL([ENABLE_LIBEVQUICK], [test "x$with_libevquick" = "xyes"])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99

# gcov coverage reporting

m4_include([m4/gcov.m4])
AC_TDD_GCOV
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_CXXFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

# Versioning.
AC_SUBST([DC_VERSION],[dc_version])
AC_SUBST([DC_VERSION_MAJOR],[dc_version_major])
AC_SUBST([DC_VERSION_MINOR],[dc_version_minor])
AC_SUBST([DC_VERSION_MICRO],[dc_version_micro])
AC_SUBST([DC_VERSION_LIBTOOL],[dc_version_lt_current:dc_version_lt_revision:dc_version_lt_age])

# Version suffix.
m4_ifset([dc_version_suffix],[
        AC_DEFINE(HAVE_VERSION_SUFFIX, [1], [Define if a version suffix is p    resent.])
])

AC_CONFIG_FILES([
	libvsb.pc
	Makefile
	include/Makefile
	include/libvsb/Makefile
	src/Makefile
	tools/Makefile
	examples/Makefile
	libevquick/Makefile
	tests/Makefile
])

AC_OUTPUT
