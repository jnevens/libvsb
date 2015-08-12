/*
 * frame_receiver.h
 *
 *  Created on: Mar 16, 2015
 *      Author: jnevens
 */

#ifndef SRC_FRAME_RECEIVER_H_
#define SRC_FRAME_RECEIVER_H_

#include "frame.h"

typedef struct
{
	uint8_t *data;
	size_t data_size;
} vsb_frame_receiver_t;

void vsb_frame_receiver_add_data(vsb_frame_receiver_t *receiver, uint8_t *data, size_t len);
vsb_frame_t *vsb_frame_receiver_parse_data(vsb_frame_receiver_t *receiver);
void vsb_frame_receiver_reset(vsb_frame_receiver_t *receiver);

#endif /* SRC_FRAME_RECEIVER_H_ */
