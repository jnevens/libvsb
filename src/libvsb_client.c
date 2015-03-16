/*
 * libvsb_client.c
 *
 *  Created on: Feb 16, 2015
 *      Author: jnevens
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define VSB_MAX_CONNECTIONS 20

#include "libvsb/client.h"
#include "libvsb/frame.h"

struct vsb_client_id_name
{
	char *name;
	int id;
};

struct vsb_client_s
{
	char *name;
	int id;
	vsb_client_incoming_data_cb_t data_callback;
	void *data_callback_arg;
	vsb_client_disconnection_cb_t disco_callback;
	void *disco_callback_arg;
	vsb_frame_receiver_t receiver;
	struct sockaddr_un server;
	struct vsb_client_id_name client_id_lookup_table[VSB_MAX_CONNECTIONS];
	int fd;
};

/* static function declarations */
static int vsb_client_send_frame(vsb_client_t *client, vsb_frame_t *frame);
static void vsb_client_request_id(vsb_client_t *client);

vsb_client_t *vsb_client_init(const char *path, const char *name)
{
	int fd;
	vsb_client_t *vsb_client = NULL;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("opening stream socket");
		return NULL;
	}
	vsb_client = calloc(1, sizeof(vsb_client_t));
	if (!vsb_client) {
		exit(ENOMEM);
	}

	if (name) {
		vsb_client->name = strdup(name);
		if (!vsb_client->name)
			exit(ENOMEM);
	}

	// set socket non blocking
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	vsb_client->fd = fd;
	vsb_client->server.sun_family = AF_UNIX;
	strcpy(vsb_client->server.sun_path, path);

	if (connect(fd, (struct sockaddr *) &(vsb_client->server), sizeof(struct sockaddr_un)) < 0) {
		close(fd);
		free(vsb_client);
		perror("connecting stream socket");
		return NULL;
	}

	vsb_client_request_id(vsb_client);

	return vsb_client;
}

static void vsb_client_request_id(vsb_client_t *client)
{
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_RQ_ID, (uint8_t *) client->name, strlen(client->name) + 1);

	vsb_client_send_frame(client, frame);

	vsb_frame_destroy(frame);
}

void vsb_client_close(vsb_client_t *vsb_client)
{
	vsb_frame_receiver_reset(&vsb_client->receiver);
	close(vsb_client->fd);
	free(vsb_client->name);
	free(vsb_client);
}

int vsb_client_get_fd(vsb_client_t *vsb_client)
{
	return vsb_client->fd;
}

void vsb_client_register_incoming_data_cb(vsb_client_t *vsb_client, vsb_client_incoming_data_cb_t data_callback,
		void *arg)
{
	vsb_client->data_callback = data_callback;
	vsb_client->data_callback_arg = arg;
}

void vsb_client_register_disconnect_cb(vsb_client_t *vsb_client, vsb_client_disconnection_cb_t disco_cb, void *arg)
{
	vsb_client->disco_callback = disco_cb;
	vsb_client->disco_callback_arg = arg;
}

void vsb_client_handle_incoming_frame(vsb_client_t *vsb_client, vsb_frame_t *frame)
{
	switch (vsb_frame_get_cmd(frame)) {
	case VSB_CMD_DATA: {
		if (vsb_client->data_callback) {
			vsb_client->data_callback(vsb_frame_get_data(frame), vsb_frame_get_datasize(frame),
					vsb_client->data_callback_arg);
		}
		break;
	}
	case VSB_CMD_RP_ID: {
		int id = *(int *)vsb_frame_get_data(frame);
		vsb_client->id = id;
		printf("Client received id: %d\n", id);
		break;
	}
	case VSB_CMD_RP_CONN_NAME: {
		break;
	}
	default:
		fprintf(stderr, "Cannot handle frame with this command!\n");
		break;
	}
}

void vsb_client_handle_incoming_event(vsb_client_t *vsb_client)
{
	uint8_t buf[1024];
	int rval;

	while (1) {
		if ((rval = read(vsb_client->fd, buf, 1024)) < 0) {
			if (errno == EWOULDBLOCK) {
				break;
			}
			perror("reading stream message");
		} else if (rval == 0) { // close connection
			close(vsb_client->fd);
			if (vsb_client->disco_callback)
				vsb_client->disco_callback(vsb_client->disco_callback_arg);
			break;
		} else { // get data
			vsb_frame_t *frame = NULL;
			vsb_frame_receiver_add_data(&vsb_client->receiver, buf, (size_t) rval);

			while ((frame = vsb_frame_receiver_parse_data(&vsb_client->receiver)) != NULL) {
				vsb_client_handle_incoming_frame(vsb_client, frame);
				vsb_frame_destroy(frame);
			}
		}
	}
}

int vsb_client_send_data(vsb_client_t *vsb_client, void *data, size_t len)
{
	int rv = -1;
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, data, len);
	rv = vsb_client_send_frame(vsb_client, frame);
	vsb_frame_destroy(frame);

	return rv;
}

static int vsb_client_send_frame(vsb_client_t *client, vsb_frame_t *frame)
{
	vsb_frame_set_src(frame, client->id);
	size_t frame_len = vsb_frame_get_framesize(frame);
	if (write(client->fd, frame, frame_len) != frame_len) {
		perror("writing on stream socket");
		return -1;
	}
	return 0;
}

