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
#include "libvsb/connection.h"
#include "frame.h"
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
	if(fd < 1)
		return NULL;

	vsb_conn_t *vsb_conn = calloc(1, sizeof(vsb_conn_t));
	if(!vsb_conn) {
		exit(-ENOMEM);
	}

	vsb_conn->arg = arg;
	vsb_conn->fd = fd;
	return vsb_conn;
}

static void vsb_conn_disconnect(vsb_conn_t *conn)
{
	if (conn->disco_cb)
		conn->disco_cb(conn, conn->disco_arg);

	close(conn->fd);
}

void vsb_conn_destroy(vsb_conn_t *conn)
{
	if(!conn)
		return;

	vsb_conn_disconnect(conn);
	vsb_frame_receiver_reset(&conn->receiver);
	free(conn->client_name);
	free(conn);
}

int vsb_conn_get_fd(vsb_conn_t *vsb_conn)
{
	if(!vsb_conn)
		return -1;

	return vsb_conn->fd;
}

void *vsb_conn_get_arg(vsb_conn_t *conn)
{
	if(!conn)
		return NULL;

	return conn->arg;
}

char *vsb_conn_get_name(vsb_conn_t *conn)
{
	if(!conn)
		return NULL;

	return conn->client_name;
}

vsb_frame_receiver_t *vsb_conn_get_frame_receiver(vsb_conn_t *conn)
{
	if(!conn)
		return NULL;

	return &conn->receiver;
}

int vsb_conn_register_disconnect_cb(vsb_conn_t *vsb_conn, vsb_server_conn_disconnection_cb_t disco_cb, void *arg)
{
	if(!vsb_conn || !disco_cb)
		return -1;

	vsb_conn->disco_cb = disco_cb;
	vsb_conn->disco_arg = arg;

	return 0;
}

int vsb_conn_set_name(vsb_conn_t *conn, char *name)
{
	if(!conn || !name)
		return -1;

	conn->client_name = strdup(name);
	if(!conn->client_name) {
		exit(-ENOMEM);
	}

	return 0;
}
