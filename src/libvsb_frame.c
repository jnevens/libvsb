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

#include "libvsb_frame.h"

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
	printf("offset len: %lu\n", offsetof(vsb_frame_t, len));
	memcpy(&len, &data[offsetof(vsb_frame_t, len)], sizeof(uint32_t));
	printf("len in data: %u\n", len);

	if(rlen != len + sizeof(vsb_frame_t))
		goto end;

	is_valid = true;
end:
	printf("frame is: %s\n", (is_valid) ? "valid" : "invalid");
	return is_valid;
}

//vsb_frame_state_t vsb_frame_receiver_add_byte(vsb_frame_receiver_t *receiver, uint8_t byte)
//{
//
//}
//
//void vsb_frame_receiver_reset(vsb_frame_receiver_t *receiver)
//{
//	free(receiver->buffer);
//	receiver->size = 0;
//	receiver->state = VSB_FRAME_INCOMPLETE;
//}
//
//vsb_frame_state_t frame_get_state(const vsb_frame_receiver_t *receiver)
//{
//	return receiver->state;
//}
