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
#include "log.h"

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
		LOG_SYSERROR(errno);
		return NULL;
	}

	client = calloc(1, sizeof(vsb_client_t));
	if (!client) {
		LOG_SYSERROR(errno);
		close(fd);
		return NULL;
	}

	if (name) {
		client->name = strdup(name);
		if (!client->name) {
			LOG_SYSERROR(errno);
			close(fd);
			free(client);
			return NULL;
		}
	}

	// set socket non blocking
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	client->fd = fd;
	client->server.sun_family = AF_UNIX;
	strcpy(client->server.sun_path, path);

	if (connect(fd, (struct sockaddr *) &(client->server), sizeof(struct sockaddr_un)) < 0) {
		LOG_SYSERROR(errno);
		close(fd);
		free(client->name);
		free(client);
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
		LOG_ERROR("Cannot handle frame with this command!");
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
			LOG_SYSERROR(errno);
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
	if(!client || !data || len < 1)
		return -1;

	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, data, len);
	int rv = vsb_client_send_frame(client, frame);
	vsb_frame_destroy(frame);

	return rv;
}

static ssize_t write_blocking(int fd, const void *data, size_t size)
{
	ssize_t n = 0;

	while ((n = write(fd, data, size)) < 0 && errno == EAGAIN) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		LOG_WARNING("Waiting after write failed with EAGAIN!");

		int rc = select(fd + 1, NULL, &fds, NULL, NULL);
		if (rc < 0)
			return -1;
	}

	return n;
}

static int vsb_client_send_frame(vsb_client_t *client, vsb_frame_t *frame)
{
	vsb_frame_set_src(frame, client->id);
	size_t frame_len = vsb_frame_get_framesize(frame);
	ssize_t len = write_blocking(client->fd, frame, frame_len);
	if (len != frame_len) {
		if (len < 0) {
			LOG_SYSERROR(errno);
		} else {
			LOG_ERROR("Partial write on socket (%zd of %zu bytes).", len, frame_len);
		}
		return -1;
	}
	return 0;
}

