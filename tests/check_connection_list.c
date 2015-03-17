/*
 * check_connection_list.c
 *
 *  Created on: Mar 17, 2015
 *      Author: jnevens
 */
#include <check.h>
#include <stdlib.h>
#include <unistd.h>

#include "../src/connection_list.h"

START_TEST(test_connection_list_init_destroy)
{
	vsb_conn_list_t *list = vsb_conn_list_create();
	ck_assert_ptr_ne(list, NULL);
	vsb_conn_list_destroy(list);
}
END_TEST

START_TEST(test_connection_list_init_destroy_with_connections)
{
	vsb_conn_t *conn1 = vsb_conn_init(1, NULL);
	vsb_conn_t *conn2 = vsb_conn_init(1, NULL);
	vsb_conn_t *conn3 = vsb_conn_init(1, NULL);
	vsb_conn_list_t *list = vsb_conn_list_create();
	vsb_conn_list_add(list, conn1);
	vsb_conn_list_add(list, conn2);
	vsb_conn_list_add(list, conn3);
	vsb_conn_list_destroy(list);
}
END_TEST


START_TEST(test_connection_list_add_delete_1)
{
	vsb_conn_t *conn1 = vsb_conn_init(1, NULL);
	vsb_conn_list_t *list = vsb_conn_list_create();
	vsb_conn_list_add(list, conn1);
	vsb_conn_list_remove(list, conn1);
	vsb_conn_destroy(conn1); /* not deleted in list_Destroy because it is not in the list anymore */
	vsb_conn_list_destroy(list);
}
END_TEST

START_TEST(test_connection_list_add_delete_multiple)
{
	vsb_conn_t *conn1 = vsb_conn_init(1, NULL);
	vsb_conn_t *conn2 = vsb_conn_init(1, NULL);
	vsb_conn_t *conn3 = vsb_conn_init(1, NULL);

	vsb_conn_list_t *list = vsb_conn_list_create();
	vsb_conn_list_add(list, conn1);
	vsb_conn_list_add(list, conn2);
	vsb_conn_list_add(list, conn3);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 3);

	vsb_conn_list_remove(list, conn1);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 2);
	vsb_conn_list_add(list, conn1);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 3);
	vsb_conn_list_remove(list, conn2);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 2);
	vsb_conn_list_add(list, conn2);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 3);
	vsb_conn_list_remove(list, conn3);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 2);
	vsb_conn_list_add(list, conn3);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 3);
	vsb_conn_list_remove(list, conn3);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 2);
	vsb_conn_list_add(list, conn3);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 3);
	vsb_conn_list_remove(list, conn2);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 2);
	vsb_conn_list_add(list, conn2);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 3);
	vsb_conn_list_remove(list, conn1);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 2);
	vsb_conn_list_add(list, conn1);
	ck_assert_int_eq(vsb_conn_list_get_count(list), 3);

	vsb_conn_list_destroy(list);
}
END_TEST

START_TEST(test_connection_list_list_iter)
{
	vsb_conn_t *conn1 = vsb_conn_init(1, NULL);
	vsb_conn_t *conn2 = vsb_conn_init(1, NULL);
	vsb_conn_t *conn3 = vsb_conn_init(1, NULL);
	vsb_conn_list_t *list = vsb_conn_list_create();
	vsb_conn_list_add(list, conn1);
	vsb_conn_list_add(list, conn2);
	vsb_conn_list_add(list, conn3);

	vsb_conn_list_iter_t *iter = vsb_conn_list_iter_create(list);
	ck_assert_ptr_ne(iter, NULL);
	vsb_conn_t *conn_test1 = vsb_conn_list_iter_next(iter);
	ck_assert_ptr_eq(conn_test1, conn1);
	vsb_conn_t *conn_test2 = vsb_conn_list_iter_next(iter);
	ck_assert_ptr_eq(conn_test2, conn2);
	vsb_conn_t *conn_test3 = vsb_conn_list_iter_next(iter);
	ck_assert_ptr_eq(conn_test3, conn3);
	vsb_conn_t *conn_test4 = vsb_conn_list_iter_next(iter);
	ck_assert_ptr_eq(conn_test4, NULL);
	vsb_conn_list_iter_destroy(iter);

	vsb_conn_list_destroy(list);
}
END_TEST

Suite * vsb_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("vsb connecton list");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_connection_list_init_destroy);
	tcase_add_test(tc_core, test_connection_list_init_destroy_with_connections);
	tcase_add_test(tc_core, test_connection_list_add_delete_1);
	tcase_add_test(tc_core, test_connection_list_add_delete_multiple);
	tcase_add_test(tc_core, test_connection_list_list_iter);
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

