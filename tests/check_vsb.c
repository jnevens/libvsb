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
		ck_assert_ptr_ne(client, NULL);
		vsb_client_close(client);
		vsb_server_close(server);
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
