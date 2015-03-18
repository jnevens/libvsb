/*
 * libvsb_frame.h
 *
 *  Created on: Feb 18, 2015
 *      Author: jnevens
 */

#ifndef INCLUDE_LIBVSB_FRAME_H_
#define INCLUDE_LIBVSB_FRAME_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct vsb_frame_s vsb_frame_t;

typedef enum vsb_cmd_e
{
	VSB_CMD_DATA = 0,
	VSB_CMD_RQ_ID,
	VSB_CMD_RP_ID,
	VSB_CMD_RQ_CONN_NAME,
	VSB_CMD_RP_CONN_NAME
} vsb_cmd_t;

vsb_frame_t *vsb_frame_create(vsb_cmd_t cmd, void *data, size_t len);
void vsb_frame_destroy(vsb_frame_t *frame);

void *vsb_frame_get_data(vsb_frame_t *vsb_frame);
size_t vsb_frame_get_datasize(vsb_frame_t *vsb_frame);
size_t vsb_frame_get_framesize(vsb_frame_t *vsb_frame);
vsb_cmd_t vsb_frame_get_cmd(vsb_frame_t *vsb_frame);
int vsb_frame_get_src(vsb_frame_t *vsb_frame);
bool vsb_frame_is_valid(uint8_t *data, size_t rlen);

void vsb_frame_set_src(vsb_frame_t *vsb_frame, int src_id);

const char *vsb_frame_cmd_to_string(vsb_cmd_t cmd);

#endif /* INCLUDE_LIBVSB_FRAME_H_ */
