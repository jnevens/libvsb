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

#include "libvsb/client.h"
#include "libvsb/frame.h"

struct vsb_client_s
{
	char *name;
	vsb_client_incoming_data_cb_t data_callback;
	void *data_callback_arg;
	vsb_client_disconnection_cb_t disco_callback;
	void *disco_callback_arg;
	vsb_frame_receiver_t receiver;
	struct sockaddr_un server;
	int fd;
};

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

	if(name) {
		vsb_client->name = strdup(name);
		if(!vsb_client->name)
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

	return vsb_client;
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
			if(vsb_client->disco_callback)
				vsb_client->disco_callback(vsb_client->disco_callback_arg);
			break;
		} else { // get data
			vsb_frame_t *frame = NULL;
			vsb_frame_receiver_add_data(&vsb_client->receiver, buf, (size_t) rval);

			while ((frame = vsb_frame_receiver_parse_data(&vsb_client->receiver)) != NULL) {
				if (vsb_client->data_callback) {
					vsb_client->data_callback(vsb_frame_get_data(frame), vsb_frame_get_datasize(frame), vsb_client->data_callback_arg);
				}
			}
		}
	}
}

int vsb_client_send_data(vsb_client_t *vsb_client, void *data, size_t len)
{
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, data, len);

	if (write(vsb_client->fd, frame, vsb_frame_get_framesize(frame)) < 0)
		perror("writing on stream socket");

	vsb_frame_destroy(frame);

	return 0;
}

