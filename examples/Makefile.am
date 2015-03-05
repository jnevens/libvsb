AM_CPPFLAGS = -I$(top_buildir)/include -I$(top_srcdir)/include -I$(top_srcdir)/libevquick/
LDADD = $(top_builddir)/src/.libs/libvsb.la

if ENABLE_LIBEVQUICK
LDADD += $(top_builddir)/libevquick/.libs/libevquick.la
else
LDADD += -levquick
endif


sbin_PROGRAMS = client server

client_SOURCES = \
	client.c

server_SOURCES = \
	server.c
