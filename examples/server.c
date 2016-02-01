#include <stdio.h> 
#include <unistd.h> 
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <argp.h>

#include <libevquick.h>
#include <libvsb/server.h>

static evquick_event *server_event = NULL;

/* Used by main to communicate with parse_opt. */
struct arguments
{
	bool use_tcp;
	char *unix_sock;
	uint16_t inet_port;
};

/* Default arguments */
struct arguments arguments = { .use_tcp = false, .inet_port = 12221 };

/* Program documentation. */
static char doc[] = "vsb server side example.";

/* A description of the arguments we accept. */
static char args_doc[] = "server [options] UNIX_SOCKET\n" \
			"server -t [options] PORT";

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
				arguments->inet_port = strtoll(arg, NULL, 16);
		} else {
			if (state->arg_num == 0)
				arguments->unix_sock = arg;
		}
		break;
	case ARGP_KEY_END:
		if (state->arg_num < 1) {
			argp_usage(state);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

void incoming_connection_callback(int fd, short revents, void *arg)
{
	vsb_conn_t *vsb_conn = (vsb_conn_t *) arg;

	vsb_server_handle_connection_event(vsb_conn);
}

void incoming_vsb_server_event(int fd, short revents, void *arg)
{
	vsb_server_t *vsb_server = (vsb_server_t *) arg;
	printf("incoming connection! \n");
	vsb_server_handle_server_event(vsb_server);
}

void connection_disconnect_callback(vsb_conn_t *conn, void *arg)
{
	evquick_event *event = (evquick_event *)arg;
	fprintf(stderr, "Disconnecting client! (fd = %d)\n", event->fd);
	evquick_delevent(event);
}

void new_connection_callback(vsb_conn_t *vsb_conn, void *arg)
{
	fprintf(stderr, "new connection callback (fd = %d)\n", vsb_conn_get_fd(vsb_conn));
	evquick_event *event = evquick_addevent(vsb_conn_get_fd(vsb_conn), EVQUICK_EV_READ, incoming_connection_callback, NULL,
			vsb_conn);
	vsb_conn_register_disconnect_cb(vsb_conn, connection_disconnect_callback, (void *)event);
}

int main(int argc, char *argv[])
{
	vsb_server_t *vsb_server = NULL;
	evquick_init();

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if(arguments.use_tcp) {
		vsb_server = vsb_server_init_tcp(arguments.inet_port);
	}else {
		unlink(arguments.unix_sock);
		vsb_server = vsb_server_init(arguments.unix_sock);
	}

	if(vsb_server == NULL) {
		fprintf(stderr, "Failed to open socket!\n");
		return -1;
	}

	vsb_server_register_new_connection_cb(vsb_server, new_connection_callback, NULL);
	fprintf(stderr, "VSB server (fd = %d)\n", vsb_server_get_fd(vsb_server));
	server_event = evquick_addevent(vsb_server_get_fd(vsb_server), EVQUICK_EV_READ, incoming_vsb_server_event, NULL, vsb_server);

	evquick_loop();

	evquick_delevent(server_event);
	vsb_server_close(vsb_server);
	return 0;
}
