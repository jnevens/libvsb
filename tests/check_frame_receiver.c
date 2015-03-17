/*
 * check_frame_receiver.c
 *
 *  Created on: Mar 16, 2015
 *      Author: jnevens
 */
#include <check.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/libvsb/frame.h"
#include "../src/frame_receiver.h"

START_TEST(test_framereceiver_msg_in_2_parts)
{
	vsb_frame_receiver_t receiver = {};
	uint8_t buf_part1[] = { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t buf_part2[] = { 0x00, 0xff, 0x00, 0x00, 0x00, 'l', 'o', 'l', 0x00 };
	vsb_frame_receiver_add_data(&receiver, buf_part1, 7);
	ck_assert_ptr_eq(vsb_frame_receiver_parse_data(&receiver), NULL);
	vsb_frame_receiver_add_data(&receiver, buf_part2, 9);
	vsb_frame_t *frame = vsb_frame_receiver_parse_data(&receiver);
	ck_assert_ptr_ne(frame, NULL);
	ck_assert_int_eq(vsb_frame_get_cmd(frame), VSB_CMD_DATA);
	ck_assert_int_eq(vsb_frame_get_datasize(frame), 4);
	ck_assert_int_eq(vsb_frame_get_src(frame), 0xff);
	ck_assert_str_eq((const char *)vsb_frame_get_data(frame), "lol");
	ck_assert_ptr_eq(vsb_frame_receiver_parse_data(&receiver), NULL);
	vsb_frame_receiver_reset(&receiver);
	vsb_frame_destroy(frame);
}
END_TEST

START_TEST(test_framereceiver_2_msgs_in_1_part)
{
	vsb_frame_receiver_t receiver = {};
	uint8_t buf_part1[] = { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0xff, 0x00, 0x00, 0x00, 'f', 'o', 'o', 0x00,
							0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0xfe, 0x00, 0x00, 0x00, 'b', 'a', 'r', 0x00 };
	vsb_frame_receiver_add_data(&receiver, buf_part1, 32);

	vsb_frame_t *frame1 = vsb_frame_receiver_parse_data(&receiver);
	ck_assert_ptr_ne(frame1, NULL);
	ck_assert_int_eq(vsb_frame_get_cmd(frame1), VSB_CMD_DATA);
	ck_assert_int_eq(vsb_frame_get_datasize(frame1), 4);
	ck_assert_int_eq(vsb_frame_get_src(frame1), 0xff);
	ck_assert_str_eq((const char *)vsb_frame_get_data(frame1), "foo");
	vsb_frame_destroy(frame1);

	vsb_frame_t *frame2 = vsb_frame_receiver_parse_data(&receiver);
	ck_assert_ptr_ne(frame2, NULL);
	ck_assert_int_eq(vsb_frame_get_cmd(frame2), VSB_CMD_DATA);
	ck_assert_int_eq(vsb_frame_get_datasize(frame2), 4);
	ck_assert_int_eq(vsb_frame_get_src(frame2), 0xfe);
	ck_assert_str_eq((const char *)vsb_frame_get_data(frame2), "bar");
	vsb_frame_destroy(frame2);

	ck_assert_ptr_eq(vsb_frame_receiver_parse_data(&receiver), NULL);
	vsb_frame_receiver_reset(&receiver);
}
END_TEST

Suite * vsb_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("vsb frame receiver");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_framereceiver_msg_in_2_parts);
	tcase_add_test(tc_core, test_framereceiver_2_msgs_in_1_part);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = vsb_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
