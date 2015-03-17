/*
 * frame_receiver.c
 *
 *  Created on: Feb 18, 2015
 *      Author: jnevens
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "libvsb/frame.h"
#include "frame_receiver.h"

void vsb_frame_receiver_add_data(vsb_frame_receiver_t *receiver, uint8_t *data, size_t len)
{
	receiver->data = realloc(receiver->data, receiver->data_size + len);
	memcpy(&receiver->data[receiver->data_size], data, len);
	receiver->data_size += len;
}

vsb_frame_t *vsb_frame_receiver_parse_data(vsb_frame_receiver_t *receiver)
{
	vsb_frame_t *frame = NULL;

	if (vsb_frame_is_valid(receiver->data, receiver->data_size)) {
		// retrieve frame
		vsb_frame_t *tmp_frame = (vsb_frame_t *) receiver->data;
		size_t frame_size = vsb_frame_get_framesize(tmp_frame);
		frame = malloc(frame_size);
		memcpy(frame, receiver->data, frame_size);

		// adjust buffer
		if ((receiver->data_size - frame_size) > 0) {
			uint8_t *new_data = malloc(receiver->data_size - frame_size);
			memcpy(new_data, &receiver->data[frame_size], receiver->data_size - frame_size);
			free(receiver->data);
			receiver->data = new_data;
			receiver->data_size -= frame_size;
		} else {
			vsb_frame_receiver_reset(receiver);
		}
	}

	return frame;
}

void vsb_frame_receiver_reset(vsb_frame_receiver_t *receiver)
{
	free(receiver->data);
	receiver->data = NULL;
	receiver->data_size = 0;
}
