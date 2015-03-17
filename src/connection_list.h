/*
 * connection_list.h
 *
 *  Created on: Mar 17, 2015
 *      Author: jnevens
 */

#ifndef SRC_CONNECTION_LIST_H_
#define SRC_CONNECTION_LIST_H_

#include <libvsb/connection.h>

typedef struct vsb_conn_list_s vsb_conn_list_t;
typedef struct vsb_conn_list_iter_s vsb_conn_list_iter_t;

vsb_conn_list_t *vsb_conn_list_create(void);
void vsb_conn_list_destroy(vsb_conn_list_t *conn_list);
void vsb_conn_list_add(vsb_conn_list_t *conn_list, vsb_conn_t *vsb_conn);
void vsb_conn_list_remove(vsb_conn_list_t *conn_list, vsb_conn_t *conn);
int vsb_conn_list_get_count(vsb_conn_list_t *conn_list);

vsb_conn_list_iter_t *vsb_conn_list_iter_create(vsb_conn_list_t *conn_list);
void vsb_conn_list_iter_destroy(vsb_conn_list_iter_t *iter);
vsb_conn_t *vsb_conn_list_iter_next(vsb_conn_list_iter_t *iter);

#endif /* SRC_CONNECTION_LIST_H_ */
