#include <check.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/libvsb/server.h"
#include "../include/libvsb/client.h"

const char *tmp_vsb_socket = "/tmp/vsb.ut.socket";

START_TEST(test_vsb_server_create_destroy)
{
	unlink(tmp_vsb_socket);
	vsb_server_t *server = vsb_server_init(tmp_vsb_socket);
	ck_assert_ptr_ne(server, NULL);
	vsb_server_close(server);
}END_TEST

START_TEST(test_vsb_client_create_destroy)
{
	unlink(tmp_vsb_socket);
	vsb_server_t *server = vsb_server_init(tmp_vsb_socket);
	vsb_client_t *client = vsb_client_init(tmp_vsb_socket, "name");
	ck_assert_int_gt(vsb_client_get_fd(client),2);
	ck_assert_int_gt(vsb_server_get_fd(server),2);
	ck_assert_ptr_ne(client, NULL);
	vsb_client_close(client);
	vsb_server_close(server);
}END_TEST

static vsb_conn_t *client_connection = NULL;
static void new_conn_cb(vsb_conn_t *vsb_conn, void *arg)
{
	client_connection = vsb_conn;
}

START_TEST(test_vsb_client_get_id)
{
	unlink(tmp_vsb_socket);
	vsb_server_t *server = vsb_server_init(tmp_vsb_socket);
	vsb_server_register_new_connection_cb(server, new_conn_cb, NULL);
	vsb_client_t *client = vsb_client_init(tmp_vsb_socket, "name");
	vsb_server_handle_server_event(server);
	vsb_server_handle_connection_event(client_connection);
	vsb_client_handle_incoming_event(client);
	ck_assert_int_eq(vsb_client_get_id(client), vsb_conn_get_fd(client_connection));
	vsb_client_close(client);
	vsb_server_close(server);
	client_connection = NULL;
}END_TEST

static char *server_incoming_data = NULL;
static void server_incoming_data_cb(void *data, size_t len, void *arg)
{
	server_incoming_data = strdup((char *)data);
}

START_TEST(test_vsb_client_send_data)
{
	unlink(tmp_vsb_socket);
	vsb_server_t *server = vsb_server_init(tmp_vsb_socket);
	vsb_server_register_new_connection_cb(server, new_conn_cb, NULL);
	vsb_server_register_receive_data_cb(server, server_incoming_data_cb, NULL);
	vsb_client_t *client = vsb_client_init(tmp_vsb_socket, "name");
	vsb_server_handle_server_event(server);
	vsb_server_handle_connection_event(client_connection);
	vsb_client_handle_incoming_event(client);
	ck_assert_int_eq(vsb_client_get_id(client), vsb_conn_get_fd(client_connection));

	vsb_client_send_data(client, "foobar", 7);
	vsb_server_handle_connection_event(client_connection);
	ck_assert_str_eq(server_incoming_data, "foobar");

	vsb_client_close(client);
	vsb_server_close(server);
	free(server_incoming_data);
	server_incoming_data = NULL;
	client_connection = NULL;
}END_TEST

static char *client_incoming_data = NULL;
static void client_incoming_data_cb(void *data, size_t len, void *arg)
{
	client_incoming_data = strdup((char *)data);
}


START_TEST(test_vsb_server_send_data)
{
	unlink(tmp_vsb_socket);
	vsb_server_t *server = vsb_server_init(tmp_vsb_socket);
	vsb_server_register_new_connection_cb(server, new_conn_cb, NULL);
	vsb_client_t *client = vsb_client_init(tmp_vsb_socket, "name");
	vsb_client_register_incoming_data_cb(client, client_incoming_data_cb, NULL);
	vsb_server_handle_server_event(server);
	vsb_server_handle_connection_event(client_connection);
	vsb_client_handle_incoming_event(client);
	ck_assert_int_eq(vsb_client_get_id(client), vsb_conn_get_fd(client_connection));

	vsb_server_send(server, "foobar", 7);
	vsb_client_handle_incoming_event(client);
	ck_assert_str_eq(client_incoming_data, "foobar");

	vsb_client_close(client);
	vsb_server_close(server);
	free(client_incoming_data);
	client_incoming_data = NULL;
	client_connection = NULL;
}END_TEST


Suite * vsb_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("vsb");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_vsb_server_create_destroy);
	tcase_add_test(tc_core, test_vsb_client_create_destroy);
	tcase_add_test(tc_core, test_vsb_client_get_id);
	tcase_add_test(tc_core, test_vsb_client_send_data);
	tcase_add_test(tc_core, test_vsb_server_send_data);
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
