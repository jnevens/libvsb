/*
 * connection_list.c
 *
 *  Created on: Mar 17, 2015
 *      Author: jnevens
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <libvsb/connection.h>
#include "connection_list.h"
#include "vsb.h"

typedef struct vsb_conn_list_node_s
{
	vsb_conn_t *connection;
	struct vsb_conn_list_node_s *next;
} vsb_conn_list_node_t;

struct vsb_conn_list_s
{
	int conn_count;
	vsb_conn_list_node_t *connections;
};

struct vsb_conn_list_iter_s
{
	vsb_conn_list_t *conn_list;
	vsb_conn_list_node_t *node;
};

/* static function declarations */
static vsb_conn_list_node_t *vsb_conn_list_node_create(vsb_conn_t *conn);
static void vsb_conn_list_node_destroy(vsb_conn_list_node_t *conn_node);
static void vsb_conn_list_node_destroy_with_conn(vsb_conn_list_node_t *conn_node);

vsb_conn_list_t *vsb_conn_list_create(void)
{
	vsb_conn_list_t *conn_list = calloc(1, sizeof(vsb_conn_list_t));
	if (!conn_list) {
		exit(-ENOMEM);
	}
	return conn_list;
}

void vsb_conn_list_destroy(vsb_conn_list_t *conn_list)
{
	vsb_conn_list_node_t *conn_node_ptr = conn_list->connections;
	vsb_conn_list_node_t *conn_node_del = NULL;
	while (conn_node_ptr) {
		conn_node_del = conn_node_ptr;
		conn_node_ptr = conn_node_ptr->next;
		vsb_conn_list_node_destroy_with_conn(conn_node_del);
	}

	free(conn_list);
}

void vsb_conn_list_add(vsb_conn_list_t *conn_list, vsb_conn_t *vsb_conn)
{
	vsb_conn_list_node_t *conn_node = vsb_conn_list_node_create(vsb_conn);
	if (conn_list->connections == NULL) {
		conn_list->connections = conn_node;
		conn_list->conn_count++;
	} else {
		vsb_conn_list_node_t *conn_node_ptr = conn_list->connections;
		while (conn_node_ptr) {
			if (conn_node_ptr->next == NULL) {
				conn_node_ptr->next = conn_node;
				conn_list->conn_count++;
				break;
			}
			conn_node_ptr = conn_node_ptr->next;
		}
	}
}

int vsb_conn_list_get_count(vsb_conn_list_t *conn_list)
{
	return conn_list->conn_count;
}

void vsb_conn_list_remove(vsb_conn_list_t *conn_list, vsb_conn_t *conn)
{
	vsb_conn_list_node_t *conn_node_ptr = conn_list->connections;
	vsb_conn_list_node_t *conn_node_last_ptr = conn_list->connections;
	while (conn_node_ptr) {
		if (conn_node_ptr->connection == conn) {
			if (conn_node_ptr == conn_list->connections) {
				conn_list->connections = conn_node_ptr->next;
			} else {
				conn_node_last_ptr->next = conn_node_ptr->next;
			}
			vsb_conn_list_node_destroy(conn_node_ptr);
			conn_list->conn_count--;
			break;
		}
		conn_node_last_ptr = conn_node_ptr;
		conn_node_ptr = conn_node_ptr->next;
	}
}

vsb_conn_list_iter_t *vsb_conn_list_iter_create(vsb_conn_list_t *conn_list)
{
	vsb_conn_list_iter_t *iter = calloc(1, sizeof(vsb_conn_list_iter_t));
	iter->node = conn_list->connections;
	return iter;
}

void vsb_conn_list_iter_destroy(vsb_conn_list_iter_t *iter)
{
	free(iter);
}

vsb_conn_t *vsb_conn_list_iter_next(vsb_conn_list_iter_t *iter)
{
	vsb_conn_list_node_t *node = iter->node;
	if (node != NULL) {
		iter->node = iter->node->next;
		return node->connection;
	} else {
		return NULL;
	}
}

static vsb_conn_list_node_t *vsb_conn_list_node_create(vsb_conn_t *conn)
{
	vsb_conn_list_node_t *conn_node = calloc(1, sizeof(vsb_conn_list_node_t));
	if (!conn_node) {
		exit(-ENOMEM);
	}
	conn_node->connection = conn;
	return conn_node;
}

static void vsb_conn_list_node_destroy(vsb_conn_list_node_t *conn_node)
{
	free(conn_node);
}

static void vsb_conn_list_node_destroy_with_conn(vsb_conn_list_node_t *conn_node)
{
	vsb_conn_destroy(conn_node->connection);
	free(conn_node);
}

