#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "libevquick.h"
#include "libvsb_client.h"

vsb_client_t *vsb_client = NULL;

void incoming_data(void *data, void *arg)
{
	printf("%s-->%s\n", (char *)arg, (char *)data);
}

void handle_incoming_event(int fd, short revents, void *arg)
{
	vsb_client_t *vsb_client = (vsb_client_t *)arg;
	vsb_client_handle_incoming_event(vsb_client);
}

void generate_data(void *arg)
{
	const char *data = (const char *) arg;
	printf("send data: %s [%d]\n", data, (int)(strlen(data) + 1));

	if(vsb_client_send_data(vsb_client, (void *)data, strlen(data) + 1)) {
		perror("failed sending data!");
	}
}

int main(int argc, char *argv[])
{
	evquick_init();

	if (argc < 3) {
		printf("usage:%s <pathname> <data>", argv[0]);
		exit(1);
	}

	vsb_client = vsb_client_init(argv[1]);
	int vsb_client_fd = vsb_client_get_fd(vsb_client);
	vsb_client_register_incoming_data_cb(vsb_client, incoming_data, (void *)"CLIENT");
	evquick_addevent(vsb_client_fd, EVQUICK_EV_READ, handle_incoming_event, NULL, vsb_client);

	// simulation of data to send
	evquick_addtimer(1000, EVQUICK_EV_RETRIGGER, generate_data, argv[2]);

	evquick_loop();
	vsb_client_close(vsb_client);

	return 0;
}

