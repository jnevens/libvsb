/*
 * check_frame.c
 *
 *  Created on: Mar 16, 2015
 *      Author: jnevens
 */
#include <check.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/libvsb/frame.h"
#include "../src/frame_receiver.h"

START_TEST(test_frame_create_destroy)
{
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, "lol", 4);
	ck_assert_ptr_ne(frame, NULL);
	vsb_frame_destroy(frame);
}
END_TEST

START_TEST(test_frame_check_data_size)
{
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, "lol", 4);
	ck_assert_ptr_ne(frame, NULL);
	ck_assert_int_eq(vsb_frame_get_datasize(frame), 4);
	vsb_frame_destroy(frame);
}
END_TEST

START_TEST(test_frame_check_data)
{
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, "lol", 4);
	ck_assert_ptr_ne(frame, NULL);
	ck_assert_str_eq((const char *)vsb_frame_get_data(frame), "lol");
	vsb_frame_destroy(frame);
}
END_TEST

START_TEST(test_frame_check_frame_size)
{
	vsb_frame_t *frame = vsb_frame_create(VSB_CMD_DATA, "lol", 4);
	ck_assert_ptr_ne(frame, NULL);
	ck_assert_int_eq(vsb_frame_get_framesize(frame), 16);
	vsb_frame_destroy(frame);
}
END_TEST

START_TEST(test_frame_valid)
{
	uint8_t buf[] = { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 'l', 'o', 'l', 0x00 };
	ck_assert_int_eq(vsb_frame_is_valid(buf, 16), 1);
	vsb_frame_t *frame = (vsb_frame_t *)buf;
	ck_assert_int_eq(vsb_frame_get_cmd(frame), VSB_CMD_DATA);
	ck_assert_int_eq(vsb_frame_get_datasize(frame), 4);
	ck_assert_int_eq(vsb_frame_get_src(frame), 0xff);
	ck_assert_str_eq((const char *)vsb_frame_get_data(frame), "lol");
}
END_TEST

START_TEST(test_frame_invalid_incomplete)
{
	uint8_t buf[] = { 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 'l', 'o', 'l', 0x00 };
	ck_assert_int_eq(vsb_frame_is_valid(buf, 12), 0);
}
END_TEST

START_TEST(test_frame_invalid_1_byte_missing)
{
	uint8_t buf[] = { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 'l', 'o', 'l', 0x00 };
	ck_assert_int_eq(vsb_frame_is_valid(buf,16), 0);
}
END_TEST

Suite * vsb_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("vsb frame");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_frame_create_destroy);
	tcase_add_test(tc_core, test_frame_check_data_size);
	tcase_add_test(tc_core, test_frame_check_data);
	tcase_add_test(tc_core, test_frame_check_frame_size);
	tcase_add_test(tc_core, test_frame_valid);
	tcase_add_test(tc_core, test_frame_invalid_incomplete);
	tcase_add_test(tc_core, test_frame_invalid_1_byte_missing);
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
