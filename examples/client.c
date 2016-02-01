#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <argp.h>

#include <libevquick.h>
#include <libvsb/client.h>

/* Used by main to communicate with parse_opt. */
struct arguments
{
	bool use_tcp;
	char *unix_sock;
	char *inet_ip;
	char *data;
	uint16_t inet_port;
};

vsb_client_t *vsb_client = NULL;

/* Default arguments */
struct arguments arguments = { .use_tcp = false, .inet_port = 12221 };

/* Program documentation. */
static char doc[] = "vsb client side example.";

/* A description of the arguments we accept. */
static char args_doc[] = "client [options] UNIX_SOCKET DATA\n" \
			"client -t [options] DESTINATION_IP PORT DATA";

/* The options we understand. */
static struct argp_option options[] = {
		{ "tcp", 't', 0, 0, "Use tcp/ip socket instead of unix socket" },
		{ 0 }
	};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
	 know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key) {
	case 't':
		arguments->use_tcp = true;
		break;
	case ARGP_KEY_ARG:
		if(arguments->use_tcp) {
			if (state->arg_num == 0)
				arguments->inet_ip = arg;
			else if (state->arg_num == 1)
				arguments->inet_port = strtoll(arg, NULL, 16);
			else if (state->arg_num == 2)
				arguments->data= arg;

		} else {
			if (state->arg_num == 0)
				arguments->unix_sock = arg;
			else if (state->arg_num == 1)
				arguments->data= arg;
		}
		break;
	case ARGP_KEY_END:
		if ((arguments->use_tcp && state->arg_num < 3)
				|| (!arguments->use_tcp && state->arg_num < 2)) {
			argp_usage(state);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

void incoming_data(void *data, size_t len, void *arg)
{
	printf("recv data: %s [%d]\n", (char *)data, (int)len);
}

void handle_incoming_event(int fd, short revents, void *arg)
{
	vsb_client_t *vsb_client = (vsb_client_t *)arg;
	vsb_client_handle_incoming_event(vsb_client);
}

void handle_connection_disconnect(void *arg)
{
	fprintf(stderr, "Connection lost with server!\n");
	exit(0);
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

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if(arguments.use_tcp)
		vsb_client = vsb_client_init_tcp(arguments.inet_ip, arguments.inet_port, "example-client");
	else
		vsb_client = vsb_client_init(arguments.unix_sock, "example-client");

	if(vsb_client == NULL) {
		fprintf(stderr, "Failed to connect!\n");
		return -1;
	}

	int vsb_client_fd = vsb_client_get_fd(vsb_client);
	vsb_client_register_incoming_data_cb(vsb_client, incoming_data, NULL);
	evquick_addevent(vsb_client_fd, EVQUICK_EV_READ, handle_incoming_event, NULL, vsb_client);
	vsb_client_register_disconnect_cb(vsb_client, handle_connection_disconnect, NULL);

	// simulation of data to send
	evquick_addtimer(1000, EVQUICK_EV_RETRIGGER, generate_data, arguments.data);

	evquick_loop();
	vsb_client_close(vsb_client);

	return 0;
}

