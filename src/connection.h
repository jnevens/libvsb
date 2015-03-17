/*
 * connection.h
 *
 *  Created on: Mar 16, 2015
 *      Author: jnevens
 */

#ifndef SRC_CONNECTION_H_
#define SRC_CONNECTION_H_

#include "frame_receiver.h"

vsb_frame_receiver_t *vsb_conn_get_frame_receiver(vsb_conn_t *conn);

#endif /* SRC_CONNECTION_H_ */
