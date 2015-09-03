#include <check.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "../include/libvsb/server.h"
#include "../include/libvsb/client.h"

const char *tmp_vsb_socket = "/tmp/vsb.ut.socket";

START_TEST(test_vsb_server_create_with_invalid_input)
{
	unlink(tmp_vsb_socket);
	ck_assert_ptr_eq(vsb_server_init(NULL), NULL);
	ck_assert_ptr_eq(vsb_server_init(""), NULL);
}END_TEST

START_TEST(test_vsb_server_close_with_invalid_input)
{
	vsb_server_close(NULL);
}END_TEST

START_TEST(test_vsb_server_get_fd_with_invalid_input)
{
	ck_assert_int_eq(vsb_server_get_fd(NULL), -1);
}END_TEST

START_TEST(test_vsb_server_register_new_connection_cb_with_invalid_input)
{
	ck_assert_int_eq(vsb_server_register_new_connection_cb((vsb_server_t *)0x01, NULL, NULL), -1);
	ck_assert_int_eq(vsb_server_register_new_connection_cb(NULL, (vsb_server_new_conn_cb_t)0x01, NULL), -1);
	ck_assert_int_eq(vsb_server_register_new_connection_cb(NULL, NULL, NULL), -1);
}END_TEST

START_TEST(test_vsb_server_handle_server_event_with_invalid_input)
{
	ck_assert_int_eq(vsb_server_handle_server_event(NULL), -1);
}END_TEST

START_TEST(test_vsb_server_handle_connection_event_with_invalid_input)
{
	ck_assert_int_eq(vsb_server_handle_connection_event(NULL), -1);
}END_TEST

START_TEST(test_vsb_server_set_auto_broadcast_with_invalid_input)
{
	ck_assert_int_eq(vsb_server_set_auto_broadcast(NULL, true), -1);
	ck_assert_int_eq(vsb_server_set_auto_broadcast(NULL, false), -1);
}END_TEST

START_TEST(test_vsb_server_send_with_invalid_input)
{
	vsb_server_t *server = (vsb_server_t *)0xdeadbeaf;
	char *data = "testdata";
	ck_assert_int_eq(vsb_server_send(server, data, 0), -1);
	ck_assert_int_eq(vsb_server_send(NULL, data, strlen(data)), -1);
	ck_assert_int_eq(vsb_server_send(server, NULL, strlen(data)), -1);
}END_TEST

START_TEST(test_vsb_server_broadcast_with_invalid_input)
{
	vsb_server_t *server = (vsb_server_t *)0xdeadbeaf;
	char *data = "testdata";
	vsb_conn_t *from = (vsb_conn_t *)0xbeafdead;

	ck_assert_int_eq(vsb_server_broadcast(server, data, strlen(data), NULL), -1);
	ck_assert_int_eq(vsb_server_broadcast(server, data, 0, from), -1);
	ck_assert_int_eq(vsb_server_broadcast(server, NULL, strlen(data), from), -1);
	ck_assert_int_eq(vsb_server_broadcast(NULL, data, strlen(data), from), -1);
}END_TEST

START_TEST(test_vsb_server_register_receive_data_cb_with_invalid_input)
{
	ck_assert_int_eq(vsb_server_register_receive_data_cb((vsb_server_t *)0xdeadbeef, NULL, NULL), -1);
	ck_assert_int_eq(vsb_server_register_receive_data_cb(NULL, (vsb_server_receive_data_cb_t)0x123, NULL), -1);
}END_TEST

Suite * vsb_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("vsb_server");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_vsb_server_create_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_close_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_get_fd_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_handle_server_event_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_register_new_connection_cb_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_handle_connection_event_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_set_auto_broadcast_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_send_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_broadcast_with_invalid_input);
	tcase_add_test(tc_core, test_vsb_server_register_receive_data_cb_with_invalid_input);
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
