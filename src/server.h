/*
 * server.h
 *
 *  Created on: Mar 17, 2015
 *      Author: jnevens
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

#include "frame.h"
#include "connection_list.h"

vsb_conn_list_t *vsb_server_get_conn_list(vsb_server_t *server);
void vsb_server_broadcast_frame(vsb_server_t *vsb_server, vsb_frame_t *frame, int from_fd); // TODO: connection instead of from_fd

#endif /* SRC_SERVER_H_ */
