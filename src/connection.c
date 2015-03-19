/*
 * connection.c
 *
 *  Created on: Mar 16, 2015
 *      Author: jnevens
 */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "libvsb/server.h"
#include "libvsb/frame.h"
#include "libvsb/connection.h"
#include "frame_receiver.h"
#include "connection.h"

struct vsb_conn_s
{
	int fd;
	char *client_name;
	void *arg;
	vsb_server_conn_disconnection_cb_t disco_cb;
	void *disco_arg;
	vsb_frame_receiver_t receiver;
};

vsb_conn_t *vsb_conn_init(int fd, void *arg)
{
	vsb_conn_t *vsb_conn = calloc(1, sizeof(vsb_conn_t));
	vsb_conn->arg = arg;
	vsb_conn->fd = fd;
	return vsb_conn;
}

void vsb_conn_disconnect(vsb_conn_t *conn)
{
	close(conn->fd);
	if (conn->disco_cb)
		conn->disco_cb(conn->disco_arg);
}

void vsb_conn_destroy(vsb_conn_t *conn)
{
	vsb_conn_disconnect(conn);
	vsb_frame_receiver_reset(&conn->receiver);
	free(conn->client_name);
	free(conn);
}

int vsb_conn_get_fd(vsb_conn_t *vsb_conn)
{
	return vsb_conn->fd;
}

void *vsb_conn_get_arg(vsb_conn_t *conn)
{
	return conn->arg;
}

char *vsb_conn_get_name(vsb_conn_t *conn)
{
	return conn->client_name;
}

vsb_frame_receiver_t *vsb_conn_get_frame_receiver(vsb_conn_t *conn)
{
	return &conn->receiver;
}

void vsb_conn_register_disconnect_cb(vsb_conn_t *vsb_conn, vsb_server_conn_disconnection_cb_t disco_cb, void *arg)
{
	vsb_conn->disco_cb = disco_cb;
	vsb_conn->disco_arg = arg;
}

void vsb_conn_set_name(vsb_conn_t *conn, char *name)
{
	conn->client_name = strdup(name);
}
