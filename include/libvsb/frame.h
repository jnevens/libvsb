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
	VSB_CMD_DATA = 0
} vsb_cmd_t;

typedef struct
{
	uint8_t *data;
	size_t data_size;
} vsb_frame_receiver_t;

vsb_frame_t *vsb_frame_create(vsb_cmd_t cmd, void *data, size_t len);
void vsb_frame_destroy(vsb_frame_t *frame);

void *vsb_frame_get_data(vsb_frame_t *vsb_frame);
size_t vsb_frame_get_datasize(vsb_frame_t *vsb_frame);
size_t vsb_frame_get_framesize(vsb_frame_t *vsb_frame);
bool vsb_frame_is_valid(uint8_t *data, size_t rlen);

void vsb_frame_receiver_add_data(vsb_frame_receiver_t *receiver, uint8_t *data, size_t len);
vsb_frame_t *vsb_frame_receiver_parse_data(vsb_frame_receiver_t *receiver);
void vsb_frame_receiver_reset(vsb_frame_receiver_t *receiver);

#endif /* INCLUDE_LIBVSB_FRAME_H_ */
