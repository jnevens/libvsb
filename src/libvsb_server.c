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

#include "libvsb_server.h"
#include "libvsb_frame.h"

#define VSB_MAX_CONNECTIONS	20

// typedef void (*vsb_server_conn_new_cb_t)(vsb_conn_t *vsb_conn, void *arg);

struct vsb_conn_s
{
	int fd;
	vsb_server_t *vsb_server;
	vsb_server_conn_disconnection_cb_t disco_cb;
	void *disco_arg;
};

struct vsb_server_s
{
	int server_fd;
	struct sockaddr_un server;
	vsb_conn_t *connections[VSB_MAX_CONNECTIONS];
	vsb_server_new_conn_cb_t new_conn_cb;
	void *new_conn_cb_arg;
};

vsb_conn_t *vsb_conn_init(vsb_server_t *vsb_server, int fd)
{
	vsb_conn_t *vsb_conn = calloc(1, sizeof(vsb_conn_t));\
	vsb_conn->vsb_server = vsb_server;
	vsb_conn->fd = fd;
	return vsb_conn;
}

void vsb_conn_destroy(vsb_conn_t *vsb_conn)
{
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

void vsb_server_broadcast_frame(vsb_server_t *vsb_server, vsb_frame_t *frame)
{
	int i;
	for (i = 0; i < VSB_MAX_CONNECTIONS; i++) {
		if (vsb_server->connections[i] != NULL) {
			if (write(vsb_server->connections[i]->fd, frame, vsb_frame_get_framesize(frame)) < 0)
				perror("writing on stream socket");
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
	printf("Socket has name %s\n", vsb_server->server.sun_path);

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
		printf("new connection created on fd: %d\n", nfd);

		vsb_conn_t *vsb_conn = vsb_conn_init(vsb_server, nfd);

		if (vsb_server->new_conn_cb)
			vsb_server->new_conn_cb(vsb_conn, NULL);

		vsb_server_conn_list_add(vsb_server, vsb_conn);
	}
}

void vsb_server_handle_connection_event(vsb_conn_t *vsb_conn)
{
	vsb_server_t *vsb_server = vsb_conn->vsb_server;
	uint8_t buf[1024];
	ssize_t rval;
	printf("incoming message on fd: %d\n", vsb_conn->fd);

	bzero(buf, sizeof(buf));
	if ((rval = read(vsb_conn->fd, buf, 1024)) < 0) {
		perror("reading stream message");
	} else if (rval == 0) {
		close(vsb_conn->fd);
		if(vsb_conn->disco_cb)
			vsb_conn->disco_cb(vsb_conn->disco_arg);
		vsb_server_conn_list_remove(vsb_conn);
		vsb_conn_destroy(vsb_conn);
	} else {
		printf("<--%s (%d)\n", (char *) buf, (int)rval);
		if(vsb_frame_is_valid(buf, rval) == true) {
			vsb_frame_t *frame = (vsb_frame_t *)buf;
			vsb_server_broadcast_frame(vsb_server, frame);
		}
	}
}
