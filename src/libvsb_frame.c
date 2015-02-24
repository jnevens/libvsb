/*
 * libvsb_frame.c
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

#include "libvsb/libvsb_frame.h"

struct vsb_frame_s
{
	uint32_t len;
	vsb_cmd_t cmd;
	uint8_t data[];
};

vsb_frame_t *vsb_frame_create(vsb_cmd_t cmd,void *data, size_t len)
{
	vsb_frame_t *frame = calloc(1,sizeof(vsb_frame_t) + len);
	frame->cmd = cmd;
	frame->len = len;
	memcpy(frame->data, data, len);

	return frame;
}

void *vsb_frame_get_data(vsb_frame_t *vsb_frame)
{
	return (void *)&vsb_frame->data;
}

size_t vsb_frame_get_datasize(vsb_frame_t *vsb_frame)
{
	return vsb_frame->len;
}

size_t vsb_frame_get_framesize(vsb_frame_t *vsb_frame)
{
	return sizeof(vsb_frame_t ) + vsb_frame->len;
}

void vsb_frame_destroy(vsb_frame_t *vsb_frame)
{
	free(vsb_frame);
}

bool vsb_frame_is_valid(uint8_t *data, size_t rlen)
{
	bool is_valid = false;

	if(rlen < sizeof(vsb_frame_t))
		goto end;

	uint32_t len = 0;
	memcpy(&len, &data[offsetof(vsb_frame_t, len)], sizeof(uint32_t));

	if(rlen < len + sizeof(vsb_frame_t))
		goto end;

	is_valid = true;
end:
	return is_valid;
}

void vsb_frame_receiver_add_data(vsb_frame_receiver_t *receiver, uint8_t *data, size_t len)
{
	receiver->data = realloc(receiver->data, receiver->data_size + len);
	memcpy(&receiver->data[receiver->data_size], data, len);
	receiver->data_size += len;
}

vsb_frame_t *vsb_frame_receiver_parse_data(vsb_frame_receiver_t *receiver)
{
	vsb_frame_t *frame = NULL;

	if(vsb_frame_is_valid(receiver->data, receiver->data_size)){
		// retrieve frame
		vsb_frame_t *tmp_frame = (vsb_frame_t *)receiver->data;
		size_t frame_size = vsb_frame_get_framesize(tmp_frame);
		frame = malloc(frame_size);
		memcpy(frame, receiver->data, frame_size);

		// adjust buffer
		if((receiver->data_size - frame_size) > 0) {
			uint8_t *new_data = malloc(receiver->data_size - frame_size);
			memcpy(new_data, &receiver->data[frame_size], receiver->data_size - frame_size);
			free(receiver->data);
			receiver->data = new_data;
			receiver->data_size -= frame_size;
		}else{
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
