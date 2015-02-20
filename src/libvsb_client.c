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

#include "libvsb_client.h"
#include "libvsb_frame.h"

struct vsb_client_s
{
	vsb_client_incoming_data_cb_t data_callback;
	void *data_callback_arg;
	struct sockaddr_un server;
	int fd;
};

vsb_client_t *vsb_client_init(const char *path)
{
	int fd;
	vsb_client_t *vsb_client = NULL;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("opening stream socket");
		exit(1);
	}
	vsb_client = calloc(1, sizeof(vsb_client_t));
	if (!vsb_client) {
		exit(ENOMEM);
	}

	vsb_client->fd = fd;
	vsb_client->server.sun_family = AF_UNIX;
	strcpy(vsb_client->server.sun_path, path);

	if (connect(fd, (struct sockaddr *) &(vsb_client->server), sizeof(struct sockaddr_un)) < 0) {
		close(fd);
		perror("connecting stream socket");
		exit(1);
	}

	return vsb_client;
}

void vsb_client_close(vsb_client_t *vsb_client)
{
	close(vsb_client->fd);
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

void vsb_client_handle_incoming_event(vsb_client_t *vsb_client)
{
	uint8_t buf[1024];
	int rval;

	bzero(buf, sizeof(buf));
	if ((rval = read(vsb_client->fd, buf, 1024)) < 0) {
		perror("reading stream message");
	} else if (rval == 0) {
		printf("Ending connection\n");
		close(vsb_client->fd);
		exit(0);
	} else {
		if (vsb_frame_is_valid(buf, rval) == true) {
			vsb_frame_t *frame = (vsb_frame_t *)buf;
			if (vsb_client->data_callback) {
				vsb_client->data_callback(vsb_frame_get_data(frame), vsb_client->data_callback_arg);
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

