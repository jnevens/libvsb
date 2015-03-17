/*
 * libvsb_server.c
 *
 *  Created on: Feb 17, 2015
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
#include "vsb.h"
#include "connection.h"
#include "connection_list.h"
#include "frame_receiver.h"
#include "server.h"

struct vsb_server_s
{
	int server_fd;
	struct sockaddr_un server;
	vsb_conn_list_t *conn_list;
	vsb_server_new_conn_cb_t new_conn_cb;
	void *new_conn_cb_arg;
	vsb_server_receive_data_cb_t recv_cb;
	void *recv_arg;
};

/* static function declarations */
static int vsb_server_send_frame(vsb_conn_t *conn, vsb_frame_t *frame);

void vsb_server_broadcast_frame(vsb_server_t *server, vsb_frame_t *frame, int from_fd)
{
	vsb_conn_t *conn = NULL;
	vsb_conn_list_iter_t *iter = vsb_conn_list_iter_create(vsb_server_get_conn_list(server));
	while ((conn = vsb_conn_list_iter_next(iter)) != NULL) {
		if (vsb_conn_get_fd(conn) != from_fd) {
			if (vsb_server_send_frame(conn, frame) <= 0)
				perror("writing on stream socket");
		}
	}
	vsb_conn_list_iter_destroy(iter);
}

vsb_server_t *vsb_server_init(const char *path)
{
	int fd;

	vsb_server_t *server = calloc(1, sizeof(vsb_server_t));
	if (!server) {
		exit(-ENOMEM);
	}

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("opening stream socket");
		exit(1);
	}

	server->server.sun_family = AF_UNIX;
	strcpy(server->server.sun_path, path);
	if (bind(fd, (struct sockaddr *) &(server->server), sizeof(struct sockaddr_un))) {
		perror("binding stream socket");
		exit(1);
	}

	server->conn_list = vsb_conn_list_create();

	listen(fd, 5);
	server->server_fd = fd;
	return server;
}

void vsb_server_close(vsb_server_t *server)
{
	close(server->server_fd);
	vsb_conn_list_destroy(server->conn_list);
	free(server);
}

int vsb_server_get_fd(vsb_server_t *server)
{
	return server->server_fd;
}

vsb_conn_list_t *vsb_server_get_conn_list(vsb_server_t *server)
{
	return server->conn_list;
}

void vsb_server_register_new_connection_cb(vsb_server_t *server, vsb_server_new_conn_cb_t new_conn_cb, void *arg)
{
	server->new_conn_cb = new_conn_cb;
	server->new_conn_cb_arg = arg;
}

void vsb_server_handle_server_event(vsb_server_t *server)
{
	int nfd = accept(server->server_fd, 0, 0);
	if (nfd > 0) {
		// set socket non blocking
		int flags = fcntl(nfd, F_GETFL, 0);
		fcntl(nfd, F_SETFL, flags | O_NONBLOCK);

		vsb_conn_t *conn = vsb_conn_init(nfd, server);

		if (server->new_conn_cb)
			server->new_conn_cb(conn, NULL);

		vsb_conn_list_add(vsb_server_get_conn_list(server), conn);
	}
}

static void vsb_server_handle_rq_id_frame(vsb_conn_t *conn, vsb_frame_t *frame)
{
	vsb_conn_set_name(conn, (char *) vsb_frame_get_data(frame));
	int conn_id = vsb_conn_get_fd(conn);
	vsb_frame_t *frame_rp = vsb_frame_create(VSB_CMD_RP_ID, (void *) &conn_id, sizeof(int));
	if (vsb_server_send_frame(conn, frame_rp) <= 0) {
		perror("Writing to client socket");
	}
	vsb_frame_destroy(frame_rp);
}

static void vsb_server_handle_rq_conn_name(vsb_conn_t *conn, vsb_frame_t *frame)
{

}

static void vsb_server_handle_incoming_frame(vsb_conn_t *conn, vsb_frame_t *frame)
{
	vsb_server_t *server = (vsb_server_t *) vsb_conn_get_arg(conn);

	switch (vsb_frame_get_cmd(frame)) {
	case VSB_CMD_DATA:
		if (server->recv_cb)
			server->recv_cb(vsb_frame_get_data(frame), vsb_frame_get_datasize(frame), server->recv_arg);
		vsb_server_broadcast_frame(server, frame, vsb_conn_get_fd(conn));
		break;
	case VSB_CMD_RQ_ID:
		vsb_server_handle_rq_id_frame(conn, frame);
		break;
	case VSB_CMD_RQ_CONN_NAME:
		vsb_server_handle_rq_conn_name(conn, frame);
		break;
	default:
		fprintf(stderr, "Cannot handle frame with this command!\n");
		break;
	}
}

void vsb_server_handle_connection_event(vsb_conn_t *conn)
{
	vsb_server_t *server = (vsb_server_t *) vsb_conn_get_arg(conn);
	vsb_frame_receiver_t *receiver = vsb_conn_get_frame_receiver(conn);
	uint8_t buf[1024];
	ssize_t rval;

	bzero(buf, sizeof(buf));

	while (1) {
		if ((rval = read(vsb_conn_get_fd(conn), buf, 64)) < 0) {
			if (errno == EWOULDBLOCK) {
				break;
			}
			perror("reading stream message");
		} else if (rval == 0) { // close connection
			vsb_conn_disconnect(conn);
			vsb_conn_list_remove(vsb_server_get_conn_list(server), conn);
			vsb_conn_destroy(conn);
			break;
		} else { // get data
			vsb_frame_t *frame = NULL;
			vsb_frame_receiver_add_data(receiver, buf, (size_t) rval);

			while ((frame = vsb_frame_receiver_parse_data(receiver)) != NULL) {
				vsb_server_handle_incoming_frame(conn, frame);
				vsb_frame_destroy(frame);
			}
		}
	}
}

int vsb_server_send(vsb_server_t *server, void *data, size_t len)
{
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, data, len);
	vsb_server_broadcast_frame(server, frame, 0);

	return 0;
}

void vsb_server_register_receive_data_cb(vsb_server_t *server, vsb_server_receive_data_cb_t recv_cb, void *arg)
{
	server->recv_cb = recv_cb;
	server->recv_arg = arg;
}

static int vsb_server_send_frame(vsb_conn_t *conn, vsb_frame_t *frame)
{
	size_t frame_len = vsb_frame_get_framesize(frame);
	if (write(vsb_conn_get_fd(conn), frame, frame_len) != frame_len) {
		perror("writing on stream socket");
		return -1;
	}
	return 0;
}

