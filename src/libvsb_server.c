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

#define VSB_MAX_CONNECTIONS	20

// typedef void (*vsb_server_conn_new_cb_t)(vsb_conn_t *vsb_conn, void *arg);

struct vsb_conn_s
{
	int fd;
	char *client_name;
	vsb_server_t *vsb_server;
	vsb_server_conn_disconnection_cb_t disco_cb;
	void *disco_arg;
	vsb_frame_receiver_t receiver;
};

struct vsb_server_s
{
	int server_fd;
	struct sockaddr_un server;
	vsb_conn_t *connections[VSB_MAX_CONNECTIONS];
	vsb_server_new_conn_cb_t new_conn_cb;
	void *new_conn_cb_arg;
	vsb_server_receive_data_cb_t recv_cb;
	void *recv_arg;
};

/* static function declarations */
static int vsb_server_send_frame(vsb_conn_t *conn, vsb_frame_t *frame);

vsb_conn_t *vsb_conn_init(vsb_server_t *vsb_server, int fd)
{
	vsb_conn_t *vsb_conn = calloc(1, sizeof(vsb_conn_t));
	vsb_conn->vsb_server = vsb_server;
	vsb_conn->fd = fd;
	return vsb_conn;
}

void vsb_conn_destroy(vsb_conn_t *vsb_conn)
{
	vsb_frame_receiver_reset(&vsb_conn->receiver);
	free(vsb_conn);
}

int vsb_conn_get_fd(vsb_conn_t *vsb_conn)
{
	return vsb_conn->fd;
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

static void vsb_server_conn_list_add(vsb_server_t *vsb_server, vsb_conn_t *vsb_conn)
{
	int i;

	for (i = 0; i < VSB_MAX_CONNECTIONS; i++) {
		if (vsb_server->connections[i] == NULL) {
			vsb_server->connections[i] = vsb_conn;
			break;
		}
	}
}

static void vsb_server_conn_list_remove(vsb_conn_t *vsb_conn)
{
	vsb_server_t *vsb_server = vsb_conn->vsb_server;
	int i;
	for (i = 0; i < VSB_MAX_CONNECTIONS; i++) {
		if (vsb_server->connections[i] == vsb_conn) {
			vsb_server->connections[i] = NULL;
			break;
		}
	}
}

void vsb_server_broadcast_frame(vsb_server_t *vsb_server, vsb_frame_t *frame, int from_fd)
{
	int i;
	for (i = 0; i < VSB_MAX_CONNECTIONS; i++) {
		if (vsb_server->connections[i] != NULL) {
			if (vsb_server->connections[i]->fd != from_fd) {
				if (vsb_server_send_frame(vsb_server->connections[i], frame) <= 0)
					perror("writing on stream socket");
			}
		}
	}
}

vsb_server_t *vsb_server_init(const char *path)
{
	int fd;

	vsb_server_t *vsb_server = calloc(1, sizeof(vsb_server_t));
	if (!vsb_server) {
		exit(-ENOMEM);
	}

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("opening stream socket");
		exit(1);
	}

	vsb_server->server.sun_family = AF_UNIX;
	strcpy(vsb_server->server.sun_path, path);
	if (bind(fd, (struct sockaddr *) &(vsb_server->server), sizeof(struct sockaddr_un))) {
		perror("binding stream socket");
		exit(1);
	}

	listen(fd, 5);
	vsb_server->server_fd = fd;
	return vsb_server;
}

void vsb_server_close(vsb_server_t *vsb_server)
{
	close(vsb_server->server_fd);
	free(vsb_server);
}

int vsb_server_get_fd(vsb_server_t *vsb_server)
{
	return vsb_server->server_fd;
}

void vsb_server_register_new_connection_cb(vsb_server_t *vsb_server, vsb_server_new_conn_cb_t new_conn_cb, void *arg)
{
	vsb_server->new_conn_cb = new_conn_cb;
	vsb_server->new_conn_cb_arg = arg;
}

void vsb_server_handle_server_event(vsb_server_t *vsb_server)
{
	int nfd = accept(vsb_server->server_fd, 0, 0);
	if (nfd > 0) {
		// set socket non blocking
		int flags = fcntl(nfd, F_GETFL, 0);
		fcntl(nfd, F_SETFL, flags | O_NONBLOCK);

		vsb_conn_t *vsb_conn = vsb_conn_init(vsb_server, nfd);

		if (vsb_server->new_conn_cb)
			vsb_server->new_conn_cb(vsb_conn, NULL);

		vsb_server_conn_list_add(vsb_server, vsb_conn);
	}
}

static void vsb_server_handle_rq_id_frame(vsb_conn_t *conn, vsb_frame_t *frame)
{
	vsb_conn_set_name(conn, (char *)vsb_frame_get_data(frame));
	vsb_frame_t *frame_rp = vsb_frame_create(VSB_CMD_RP_ID, (void *)&conn->fd, sizeof(int));
	if(vsb_server_send_frame(conn, frame_rp) <= 0){
		perror("Writing to client socket");
	}
	vsb_frame_destroy(frame_rp);
}

static void vsb_server_handle_rq_conn_name(vsb_conn_t *conn, vsb_frame_t *frame)
{

}

static void vsb_server_handle_incoming_frame(vsb_conn_t *conn, vsb_frame_t *frame)
{
	vsb_server_t *vsb_server = conn->vsb_server;

	switch (vsb_frame_get_cmd(frame)) {
	case VSB_CMD_DATA:
		if (vsb_server->recv_cb)
			vsb_server->recv_cb(vsb_frame_get_data(frame), vsb_frame_get_datasize(frame), vsb_server->recv_arg);
		vsb_server_broadcast_frame(vsb_server, frame, vsb_conn_get_fd(conn));
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

void vsb_server_handle_connection_event(vsb_conn_t *vsb_conn)
{
	vsb_server_t *vsb_server = vsb_conn->vsb_server;
	vsb_frame_receiver_t *receiver = &vsb_conn->receiver;
	uint8_t buf[1024];
	ssize_t rval;

	bzero(buf, sizeof(buf));

	while (1) {
		if ((rval = read(vsb_conn->fd, buf, 64)) < 0) {
			if (errno == EWOULDBLOCK) {
				break;
			}
			perror("reading stream message");
		} else if (rval == 0) { // close connection
			close(vsb_conn->fd);
			if (vsb_conn->disco_cb)
				vsb_conn->disco_cb(vsb_conn->disco_arg);
			vsb_server_conn_list_remove(vsb_conn);
			vsb_conn_destroy(vsb_conn);
			break;
		} else { // get data
			vsb_frame_t *frame = NULL;
			vsb_frame_receiver_add_data(receiver, buf, (size_t) rval);

			while ((frame = vsb_frame_receiver_parse_data(receiver)) != NULL) {
				vsb_server_handle_incoming_frame(vsb_conn, frame);
				vsb_frame_destroy(frame);
			}
		}
	}
}

int vsb_server_send(vsb_server_t *vsb_server, void *data, size_t len)
{
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, data, len);
	vsb_server_broadcast_frame(vsb_server, frame, 0);

	return 0;
}

void vsb_server_register_receive_data_cb(vsb_server_t *vsb_server, vsb_server_receive_data_cb_t recv_cb, void *arg)
{
	vsb_server->recv_cb = recv_cb;
	vsb_server->recv_arg = arg;
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

