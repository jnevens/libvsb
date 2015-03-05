AM_CPPFLAGS = -I$(top_buildir)/include -I$(top_srcdir)/include -I$(top_srcdir)/libevquick
LDADD = $(top_builddir)/src/.libs/libvsb.la

if ENABLE_LIBEVQUICK
LDADD += $(top_builddir)/libevquick/.libs/libevquick.la
else
LDADD += -levquick
endif

sbin_PROGRAMS = vsb-monitor vsb-send

vsb_monitor_SOURCES = \
	vsb-monitor.c

vsb_send_SOURCES = \
	vsb-send.c

