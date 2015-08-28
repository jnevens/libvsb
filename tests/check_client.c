#include <check.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "../include/libvsb/server.h"
#include "../include/libvsb/client.h"

const char *tmp_vsb_socket = "/tmp/vsb.ut.socket";

START_TEST(test_vsb_client_init_close_with_invalid_input)
{
	ck_assert_ptr_eq(vsb_client_init(NULL, "clientname"), NULL);
	ck_assert_ptr_eq(vsb_client_init("/tmp/path", NULL), NULL);
	vsb_client_close(NULL);
}END_TEST

START_TEST(test_vsb_client_get_fd_with_invalid_input)
{
	ck_assert_int_eq(vsb_client_get_fd(NULL), -1);
}END_TEST

START_TEST(test_vsb_client_get_id_with_invalid_input)
{
	ck_assert_int_eq(vsb_client_get_id(NULL), -1);
}END_TEST

START_TEST(test_vsb_client_register_incoming_data_cb_with_invalid_input)
{
	ck_assert_int_eq(vsb_client_register_incoming_data_cb(NULL, (vsb_client_incoming_data_cb_t)0x1234, NULL), -1);
	ck_assert_int_eq(vsb_client_register_incoming_data_cb((vsb_client_t *)0xdeadbeef, NULL, NULL), -1);
}END_TEST

START_TEST(test_vsb_client_register_disconnect_cb_with_invalid_input)
{
	ck_assert_int_eq(vsb_client_register_disconnect_cb(NULL, (vsb_client_disconnection_cb_t)0x1234, NULL), -1);
	ck_assert_int_eq(vsb_client_register_disconnect_cb((vsb_client_t *)0xdeadbeef, NULL, NULL), -1);
}END_TEST

START_TEST(test_vsb_client_handle_incoming_event_with_invalid_input)
{
	ck_assert_int_eq(vsb_client_handle_incoming_event(NULL), -1);
}END_TEST

START_TEST(test_vsb_client_send_data_with_invalid_input)
{
	ck_assert_int_eq(vsb_client_send_data(NULL, "data", strlen("data")), -1);
	ck_assert_int_eq(vsb_client_send_data((vsb_client_t *)0xdeadbeef, NULL, strlen("data")), -1);
	ck_assert_int_eq(vsb_client_send_data((vsb_client_t *)0xdeadbeef, "data", 0), -1);
}END_TEST


Suite * vsb_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("vsb_client");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_vsb_client_init_close_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_client_get_fd_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_client_get_id_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_client_register_incoming_data_cb_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_client_register_disconnect_cb_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_client_handle_incoming_event_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_client_send_data_with_invalid_input);

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
