/*
 * libvsb_server.h
 *
 *  Created on: Feb 17, 2015
 *      Author: jnevens
 */

#ifndef INCLUDE_LIBVSB_SERVER_H_
#define INCLUDE_LIBVSB_SERVER_H_

#include "libvsb_frame.h"

typedef struct vsb_server_s vsb_server_t;
typedef struct vsb_conn_s vsb_conn_t;

typedef void (*vsb_server_new_conn_cb_t)(vsb_conn_t *vsb_conn, void *arg);
typedef void (*vsb_server_conn_disconnection_cb_t)(void *arg);

vsb_conn_t *vsb_conn_init(vsb_server_t *vsb_server, int fd);
void vsb_conn_destroy(vsb_conn_t *vsb_conn);
int vsb_conn_get_fd(vsb_conn_t *vsb_conn);
void vsb_conn_register_disconnect_cb(vsb_conn_t *vsb_conn, vsb_server_conn_disconnection_cb_t disco_cb, void *arg);

void vsb_server_broadcast_frame(vsb_server_t *vsb_server, vsb_frame_t *frame, int from_fd);
vsb_server_t *vsb_server_init(const char *path);
void vsb_server_close(vsb_server_t *vsb_server);
int vsb_server_get_fd(vsb_server_t *vsb_server);
void vsb_server_register_new_connection_cb(vsb_server_t *vsb_server, vsb_server_new_conn_cb_t new_conn_cb, void *arg);
void vsb_server_handle_server_event(vsb_server_t *vsb_server);
void vsb_server_handle_connection_event(vsb_conn_t *vsb_conn);

// TODO
int vsb_server_send(vsb_server_t *vsb_server, void *data, size_t len);
// TODO register callback for reception

#endif /* INCLUDE_LIBVSB_SERVER_H_ */
