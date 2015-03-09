AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -pthread -lcheck_pic -lrt -lm
LDADD = ../src/libvsb.la

TESTS = check_vsb
@VALGRIND_CHECK_RULES@

check_PROGRAMS = $(TESTS)

check_vsb_SOURCES = check_vsb.c
