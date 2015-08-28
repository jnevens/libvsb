/*
 * libvsb_server.h
 *
 *  Created on: Feb 17, 2015
 *      Author: jnevens
 */

#ifndef INCLUDE_LIBVSB_SERVER_H_
#define INCLUDE_LIBVSB_SERVER_H_

#include <libvsb/connection.h>
#include <stdbool.h>

typedef struct vsb_server_s vsb_server_t;

typedef void (*vsb_server_new_conn_cb_t)(vsb_conn_t *vsb_conn, void *arg);
typedef void (*vsb_server_receive_data_cb_t)(vsb_conn_t *vsb_conn, void *data, size_t len, void *arg);

vsb_server_t *vsb_server_init(const char *path);
void vsb_server_close(vsb_server_t *vsb_server);
int vsb_server_get_fd(vsb_server_t *vsb_server);
int vsb_server_register_new_connection_cb(vsb_server_t *vsb_server, vsb_server_new_conn_cb_t new_conn_cb, void *arg);
int vsb_server_handle_server_event(vsb_server_t *vsb_server);
int vsb_server_handle_connection_event(vsb_conn_t *vsb_conn);

int vsb_server_set_auto_broadcast(vsb_server_t *vsb_server, bool value);
int vsb_server_send(vsb_server_t *vsb_server, void *data, size_t len);
int vsb_server_broadcast(vsb_server_t *vsb_server, void *data, size_t len, vsb_conn_t *from);
int vsb_server_register_receive_data_cb(vsb_server_t *vsb_server, vsb_server_receive_data_cb_t recv_cb, void *arg);

#endif /* INCLUDE_LIBVSB_SERVER_H_ */
