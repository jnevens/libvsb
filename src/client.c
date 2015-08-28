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
#include "frame.h"
#include "frame_receiver.h"

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

vsb_client_t *vsb_client_init(const char *path, const char *name)
{
	int fd;
	vsb_client_t *client = NULL;

	if(!path || !name)
		return NULL;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Cannot create socket!\n");
		return NULL;
	}

	client = calloc(1, sizeof(vsb_client_t));
	if (!client) {
		fprintf(stderr, "Cannot allocate memory!\n");
		exit(-ENOMEM);
	}

	if (name) {
		client->name = strdup(name);
		if (!client->name) {
			fprintf(stderr, "Cannot allocate memory!\n");
			exit(-ENOMEM);
		}
	}

	// set socket non blocking
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	client->fd = fd;
	client->server.sun_family = AF_UNIX;
	strcpy(client->server.sun_path, path);

	if (connect(fd, (struct sockaddr *) &(client->server), sizeof(struct sockaddr_un)) < 0) {
		close(fd);
		free(client);
		fprintf(stderr, "Cannot connect stream socket!\n");
		return NULL;
	}

	return client;
}

void vsb_client_close(vsb_client_t *client)
{
	if(!client)
		return;

	vsb_frame_receiver_reset(&client->receiver);
	close(client->fd);
	free(client->name);
	free(client);
}

int vsb_client_get_fd(vsb_client_t *client)
{
	if(!client)
		return -1;

	return client->fd;
}

int vsb_client_get_id(vsb_client_t *client)
{
	if(!client)
		return -1;

	return client->id;
}

int vsb_client_register_incoming_data_cb(vsb_client_t *client, vsb_client_incoming_data_cb_t data_callback,
		void *arg)
{
	if(!client || !data_callback)
		return -1;

	client->data_callback = data_callback;
	client->data_callback_arg = arg;

	return 0;
}

int vsb_client_register_disconnect_cb(vsb_client_t *client, vsb_client_disconnection_cb_t disco_cb, void *arg)
{
	if(!client || !disco_cb)
		return -1;

	client->disco_callback = disco_cb;
	client->disco_callback_arg = arg;

	return 0;
}

static void vsb_client_handle_incoming_frame(vsb_client_t *client, vsb_frame_t *frame)
{
	switch (vsb_frame_get_cmd(frame)) {
	case VSB_CMD_DATA: {
		if (client->data_callback) {
			client->data_callback(vsb_frame_get_data(frame), vsb_frame_get_datasize(frame),
					client->data_callback_arg);
		}
		break;
	}
	default:
		fprintf(stderr, "Cannot handle frame with this command!\n");
		break;
	}
}

int vsb_client_handle_incoming_event(vsb_client_t *client)
{
	uint8_t buf[1024];
	int rval;

	if(!client)
		return -1;

	while (1) {
		if ((rval = read(client->fd, buf, sizeof(buf))) < 0) {
			if (errno == EWOULDBLOCK) {
				break;
			}
			fprintf(stderr, "Error reading stream message\n");
		} else if (rval == 0) { // close connection
			close(client->fd);
			if (client->disco_callback)
				client->disco_callback(client->disco_callback_arg);
			break;
		} else { // get data
			vsb_frame_t *frame = NULL;
			vsb_frame_receiver_add_data(&client->receiver, buf, (size_t) rval);

			while ((frame = vsb_frame_receiver_parse_data(&client->receiver)) != NULL) {
				vsb_client_handle_incoming_frame(client, frame);
				vsb_frame_destroy(frame);
			}
		}
	}

	return 0;
}

int vsb_client_send_data(vsb_client_t *client, void *data, size_t len)
{
	int rv = -1;

	if(!client || !data || len < 1)
		return rv;

	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, data, len);
	rv = vsb_client_send_frame(client, frame);
	vsb_frame_destroy(frame);

	return rv;
}

static int vsb_client_send_frame(vsb_client_t *client, vsb_frame_t *frame)
{
	vsb_frame_set_src(frame, client->id);
	size_t frame_len = vsb_frame_get_framesize(frame);
	if (write(client->fd, frame, frame_len) != frame_len) {
		fprintf(stderr, "Error writing on stream socket\n");
		return -1;
	}
	return 0;
}

