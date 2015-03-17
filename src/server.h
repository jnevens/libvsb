/*
 * server.h
 *
 *  Created on: Mar 17, 2015
 *      Author: jnevens
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

#include "connection_list.h"

vsb_conn_list_t *vsb_server_get_conn_list(vsb_server_t *server);

#endif /* SRC_SERVER_H_ */
