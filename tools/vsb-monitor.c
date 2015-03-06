#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <argp.h>
#include <error.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#include <libevquick.h>
#include <libvsb/client.h>

vsb_client_t *vsb_client = NULL;

typedef enum monitor_mode
{
	MODE_TEXT, MODE_HEX,
} monitor_mode_t;

/* Used by main to communicate with parse_opt. */
struct arguments
{
	char *vsb_socket; /* vsb_socket */
	bool print_time;
	monitor_mode_t mode;
};

/* Default arguments */
struct arguments arguments =
{ .mode = MODE_HEX, .print_time = false };

/* Program documentation. */
static char doc[] = "vsb-monitor, a tool to monitor a virtual serial bus.";

/* A description of the arguments we accept. */
static char args_doc[] = "VSB-SOCKET";

/* The options we understand. */
static struct argp_option options[] =
{
{ "mode-text", 't', 0, 0, "Enable text mode" },
{ "mode-hex", 'h', 0, 0, "Enable hex mode" },
{ "time", 's', 0, 0, "Show timing info" },
{ 0 } };

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
	 know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key)
	{
	case 't':
		arguments->mode = MODE_TEXT;
		break;
	case 'h':
		arguments->mode = MODE_HEX;
		break;
	case 's':
		arguments->print_time = true;
		break;
	case ARGP_KEY_ARG:
		arguments->vsb_socket = arg;
		break;
	case ARGP_KEY_END:
		if (state->arg_num < 1)
			/* Not enough arguments. */
			argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}
/* print bus data */
static void print_line_prefix(size_t len)
{
	if (arguments.print_time) {
		char s[32];
		struct timeval time;
		gettimeofday(&time, NULL);
		time_t t = (time_t)time.tv_sec;
		struct tm * p = localtime(&t);
		strftime(s, 1000, "%y/%m/%d %k:%M:%S", p);
		printf("%s.%03d ", s, (int)(time.tv_usec/1000));
	}

	printf("[%3u] ", (unsigned) len);
}

static void print_line_end(void)
{
	printf("\n");
}

static void print_data_text(char *data, size_t len)
{
	int i;
	print_line_prefix(len);
	for (i = 0; i < len; i++) {
		printf("%c", data[i]);
	}
	print_line_end();
}

static void print_data_hex(uint8_t *data, size_t len)
{
	int i;
	print_line_prefix(len);

	for (i = 0; i < len; i++) {
		if (i && (i % 4 == 0))
			printf(" ");
		printf("%02X", (int) data[i]);
	}

	print_line_end();
}

void incoming_data(void *data, size_t len, void *arg)
{
	switch (arguments.mode)
	{
	case MODE_TEXT:
		print_data_text((char *) data, len);
		break;
	case MODE_HEX:
		print_data_hex((uint8_t *) data, len);
		break;
	}
}

/* redirect incoming event on vsb fd, to libvsb */
void handle_incoming_event(int fd, short revents, void *arg)
{
	vsb_client_t *vsb_client = (vsb_client_t *) arg;
	vsb_client_handle_incoming_event(vsb_client);
}

/* connection closed by remote, callback */
void handle_connection_disconnect(void *arg)
{
	fprintf(stderr, "Connection lost with server!\n");
	exit(0);
}

/* termination handler */
static void termination_handler(int signum)
{
	evquick_fini();
}

static struct argp argp =
{ options, parse_opt, args_doc, doc };

int main(int argc, char *argv[])
{
	// handle signals
	signal(SIGTERM, termination_handler);
	signal(SIGINT, termination_handler);

	/* parse arguments */
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	/* init vsb */
	vsb_client = vsb_client_init(arguments.vsb_socket);
	if (!vsb_client) {
		printf("error: problem initializing bus!\n");
		exit(-1);
	} else {
		printf("Connected to bus: %s\n", arguments.vsb_socket);
	}

	/* initialize scheduler */
	evquick_init();

	/* add vsb in scheduler */
	int vsb_client_fd = vsb_client_get_fd(vsb_client);
	vsb_client_register_incoming_data_cb(vsb_client, incoming_data, NULL);
	evquick_addevent(vsb_client_fd, EVQUICK_EV_READ, handle_incoming_event, NULL, vsb_client);

	/* register disconnection cb */
	vsb_client_register_disconnect_cb(vsb_client, handle_connection_disconnect, NULL);

	/* loop */
	evquick_loop();
	vsb_client_close(vsb_client);
	printf("%s terminated!\n", argv[0]);

	return 0;
}

