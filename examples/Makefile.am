AM_CPPFLAGS = -I$(top_buildir)/include -I$(top_srcdir)/include -I$(top_srcdir)/libevquick/
LDADD = $(top_builddir)/src/.libs/libvsb.la
LDADD += $(top_builddir)/libevquick/.libs/libevquick.la

sbin_PROGRAMS = client server

COMMON =

client_SOURCES = \
	client.c \
	$(COMMON)
server_SOURCES = \
	server.c \
	$(COMMON)


