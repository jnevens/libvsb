/*
 * connection.h
 *
 *  Created on: Mar 16, 2015
 *      Author: jnevens
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

typedef struct vsb_conn_s vsb_conn_t;

typedef void (*vsb_server_conn_disconnection_cb_t)(vsb_conn_t *conn, void *arg);

vsb_conn_t *vsb_conn_init(int fd, void *arg);
//void vsb_conn_disconnect(vsb_conn_t *conn);
void vsb_conn_destroy(vsb_conn_t *conn);

int vsb_conn_get_fd(vsb_conn_t *conn);
void *vsb_conn_get_arg(vsb_conn_t *conn);
char *vsb_conn_get_name(vsb_conn_t *conn);

int vsb_conn_set_name(vsb_conn_t *conn, char *name);
int vsb_conn_register_disconnect_cb(vsb_conn_t *conn, vsb_server_conn_disconnection_cb_t disco_cb, void *arg);

#endif /* CONNECTION_H_ */
