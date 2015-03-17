AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -pthread -lcheck_pic -lrt -lm
LDADD = ../src/libvsb.la

TESTS = check_vsb check_frame check_frame_receiver check_connection check_connection_list
@VALGRIND_CHECK_RULES@

check_PROGRAMS = $(TESTS)

check_vsb_SOURCES = check_vsb.c
check_frame_SOURCES = check_frame.c
check_frame_receiver_SOURCES = check_frame_receiver.c
check_connection_SOURCES = check_connection.c
check_connection_list_SOURCES = check_connection_list.c
