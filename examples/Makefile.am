AM_CPPFLAGS = -I$(top_buildir)/include -I$(top_srcdir)/include
LDADD = $(top_builddir)/src/.libs/libvsb.la

bin_PROGRAMS = client server

COMMON = \
	libevquick.c

client_SOURCES = \
	client.c \
	$(COMMON)
server_SOURCES = \
	server.c \
	$(COMMON) 


