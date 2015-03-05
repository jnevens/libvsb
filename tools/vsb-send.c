#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <argp.h>

#include <libvsb/libvsb_client.h>

vsb_client_t *vsb_client = NULL;

typedef enum monitor_mode
{
	MODE_TEXT, MODE_HEX,
} data_mode_t;

/* Used by main to communicate with parse_opt. */
struct arguments
{
	char *vsb_socket; /* vsb_socket */
	char *data;
	data_mode_t mode;
};

/* Default arguments */
struct arguments arguments = { .mode = MODE_HEX };

/* Program documentation. */
static char doc[] = "vsb-monitor, a tool to monitor a virtual serial bus.";

/* A description of the arguments we accept. */
static char args_doc[] = "VSB-SOCKET";

/* The options we understand. */
static struct argp_option options[] = { { "data-text", 't', 0, 0, "Data as text" }, { "data-hex", 'h', 0, 0,
		"Data as hex" }, { 0 } };

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
	 know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key) {
	case 't':
		arguments->mode = MODE_TEXT;
		break;
	case 'h':
		arguments->mode = MODE_HEX;
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num == 0)
			arguments->vsb_socket = arg;
		else
			arguments->data = arg;
		break;
	case ARGP_KEY_END:
		if (state->arg_num < 2)
			/* Not enough arguments. */
			argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

ssize_t hexstr2byte(uint8_t* byte, size_t maxbytes, const char* hex)
{
	size_t i = 0;
	if (NULL == byte)
		return -1;
	if (NULL == hex)
		return -1;
	for (i = 0; i < maxbytes; i++) {
		int value = 0;
		if ('\0' == hex[2 * i] || '\0' == hex[2 * i + 1])
			break;
		if (1 != sscanf(hex + 2 * i, "%2x", &value)) {
			return -1;
		}
		byte[i] = (uint8_t) (value & 0xff);
	}
	return (ssize_t) i;
}

int send_text_data(char *data)
{
	if (vsb_client_send_data(vsb_client, (void *) data, strlen(data) + 1)) {
		perror("failed sending data!");
		return errno;
	}
	return 0;
}

int send_hex_data(char *text_data)
{
	size_t text_data_len = strlen(text_data);
	uint8_t *hex_data = calloc(1, text_data_len);
	ssize_t hex_data_len = hexstr2byte(hex_data, text_data_len, text_data);
	if (hex_data_len >= 0) {

		if (vsb_client_send_data(vsb_client, (void *) hex_data, hex_data_len)) {
			perror("failed sending data!");
			return errno;
		}
	} else {
		fprintf(stderr, "Invalid HEX data!");
		return -1;
	}
	return 0;
}

int send_data(char *data, data_mode_t mode)
{
	if (mode == MODE_TEXT) {
		return send_text_data(data);
	} else if (mode == MODE_HEX) {
		return send_hex_data(data);
	}
	return -1;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char *argv[])
{
	int rv = -1;

	/* parse arguments */
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	vsb_client = vsb_client_init(arguments.vsb_socket);

	rv = send_data(arguments.data, arguments.mode);

	vsb_client_close(vsb_client);
	return rv;
}
