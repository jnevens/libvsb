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

#include "frame.h"

static const char *cmd_str[] = {
		"DATA",
		"RQ_ID",
		"RP_ID",
		"RQ_CONN_NAME",
		"RP_CONN_NAME"
};

struct vsb_frame_s
{
	uint32_t len;
	vsb_cmd_t cmd;
	int src;
	uint8_t data[];
};

vsb_frame_t *vsb_frame_create(vsb_cmd_t cmd, void *data, size_t len)
{
	vsb_frame_t *frame = calloc(1, sizeof(vsb_frame_t) + len);
	frame->cmd = cmd;
	frame->len = len;
	memcpy(frame->data, data, len);

	return frame;
}

void *vsb_frame_get_data(vsb_frame_t *vsb_frame)
{
	return (void *) &vsb_frame->data;
}

size_t vsb_frame_get_datasize(vsb_frame_t *vsb_frame)
{
	return vsb_frame->len;
}

vsb_cmd_t vsb_frame_get_cmd(vsb_frame_t *vsb_frame)
{
	return vsb_frame->cmd;
}

int vsb_frame_get_src(vsb_frame_t *vsb_frame)
{
	return vsb_frame->src;
}

size_t vsb_frame_get_framesize(vsb_frame_t *vsb_frame)
{
	return sizeof(vsb_frame_t) + vsb_frame->len;
}

void vsb_frame_set_src(vsb_frame_t *vsb_frame, int src_id)
{
	vsb_frame->src = src_id;
}

void vsb_frame_destroy(vsb_frame_t *vsb_frame)
{
	free(vsb_frame);
}

bool vsb_frame_is_valid(uint8_t *data, size_t rlen)
{
	bool is_valid = false;

	if (rlen < sizeof(vsb_frame_t))
		goto end;

	uint32_t len = 0;
	memcpy(&len, &data[offsetof(vsb_frame_t, len)], sizeof(uint32_t));

	if (rlen < len + sizeof(vsb_frame_t))
		goto end;

	is_valid = true;
	end: return is_valid;
}

const char *vsb_frame_cmd_to_string(vsb_cmd_t cmd)
{
	return cmd_str[cmd];
}
