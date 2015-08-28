/*
 * libvsb_client.h
 *
 *  Created on: Feb 16, 2015
 *      Author: jnevens
 */
#ifndef INCLUDE_LIBVSB_CLIENT_H_
#define INCLUDE_LIBVSB_CLIENT_H_

typedef struct vsb_client_s vsb_client_t;

typedef void (*vsb_client_incoming_data_cb_t)(void *data, size_t len, void *arg);
typedef void (*vsb_client_disconnection_cb_t)(void *arg);

vsb_client_t *vsb_client_init(const char *path, const char *name);
void vsb_client_close(vsb_client_t *client);
int vsb_client_get_fd(vsb_client_t *client);
int vsb_client_get_id(vsb_client_t *client);
int vsb_client_register_incoming_data_cb(vsb_client_t *client, vsb_client_incoming_data_cb_t data_callback, void *arg);
int vsb_client_register_disconnect_cb(vsb_client_t *client, vsb_client_disconnection_cb_t disco_cb, void *arg);
int vsb_client_handle_incoming_event(vsb_client_t *client);

int vsb_client_send_data(vsb_client_t *client, void *data, size_t len);

#endif /* INCLUDE_LIBVSB_CLIENT_H_ */
