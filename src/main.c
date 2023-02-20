#include "main.h"

int main(int argc, char *argv[])
{
	/* Check arguments */
	unsigned short port;
	if (argc != 2 ||
			argv[1] == NULL ||
			(port = (unsigned short) atoi(argv[1])) == 0)
	{
		printf("Usage: %s <port>\n", argv[0]);
		return ERR;
	}
	debug("Starting Bridge on port %s", argv[1]);

	/* TODO: Initialize name-address table */

	/* Run the bridge server */
	bridge_server_args_t args;
	args.port = port;
	if (run_server_thread(&args) != OK) {
		return ERR;
	}

	/* Run as UI thread and block */
	if (run_tui() != OK) {
		return ERR;
	}

	return OK;
}

int run_server_thread(bridge_server_args_t * args)
{
	if (args == NULL) {
		return ERR;
	}

	/* Initialize the mutex lock */
	if (pthread_mutex_init(&args->table_lock, NULL)) {
		perror("pthread_mutex_init()");
	}

	/* Create and run the thread */
	if (pthread_create(&args->bridge_thr, NULL, run_bridge_server, &args) ||
			pthread_detach(args->bridge_thr))
	{
		perror("pthread_create|detach()");
		return ERR;
	}

	return OK;
}

int cleanup(bridge_server_args_t * args)
{
	if (pthread_mutex_destroy(&args->table_lock)) {
		perror("pthread_mutex_destroy()");
		return ERR;
	}

	/* TODO: cleanup any datastructures */

	return OK;
}

