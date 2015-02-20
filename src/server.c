#include <stdio.h> 
#include <unistd.h> 
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>

#include "libevquick.h"
#include "libvsb_server.h"

static evquick_event *server_event = NULL;

void incoming_connection_callback(int fd, short revents, void *arg)
{
	printf("%s: %d\n", __func__, __LINE__);
	vsb_conn_t *vsb_conn = (vsb_conn_t *) arg;

	vsb_server_handle_connection_event(vsb_conn);
}

void incoming_vsb_server_socket(int fd, short revents, void *arg)
{
	printf("%s: %d\n", __func__, __LINE__);
	vsb_server_t *vsb_server = (vsb_server_t *) arg;
	printf("incoming connection! \n");
	vsb_server_handle_server_event(vsb_server);
}

void connection_disconnect_callback(void *arg)
{
	printf("%s: %d\n", __func__, __LINE__);
	evquick_event *event = (evquick_event *)arg;
	evquick_delevent(event);
}

void new_connection_callback(vsb_conn_t *vsb_conn, void *arg)
{
	printf("%s: %d\n", __func__, __LINE__);
	evquick_event *event = evquick_addevent(vsb_conn_get_fd(vsb_conn), EVQUICK_EV_READ, incoming_connection_callback, NULL,
			vsb_conn);
	vsb_conn_register_disconnect_cb(vsb_conn, connection_disconnect_callback, (void *)event);
}

int main(int argc, char *argv[])
{
	evquick_init();
	const char *path = "/tmp/vsb.socket";
	unlink(path);
	vsb_server_t *vsb_server = vsb_server_init(path);
	vsb_server_register_new_connection_cb(vsb_server, new_connection_callback, NULL);

	server_event = evquick_addevent(vsb_server_get_fd(vsb_server), EVQUICK_EV_READ, incoming_vsb_server_socket, NULL,
			vsb_server);

	evquick_loop();

	evquick_delevent(server_event);
	vsb_server_close(vsb_server);
	return 0;
}
