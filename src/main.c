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

	/* Initialize name-address table */
	table_t table;
	if (init_table(&table) != OK) {
		return ERR;
	}

	/* Run the bridge server */
	pthread_t bridge_thr;
	if (run_server_thread(port, &bridge_thr, &table) != OK) {
		return ERR;
	}

	/* Run as UI thread and block */
	if (run_tui() != OK) {
		return ERR;
	}

	/* Clean up */
	// cleanup(bridge_thr);

	return OK;
}

int run_server_thread(unsigned short port, pthread_t * thrd, table_t * table)
{
	if (port == 0 || table == NULL) {
		return ERR;
	}

	bridge_server_args_t args = {
		.port = port,
		.thrd = thrd,
		.table = table
	};

	/* Create and run the thread */
	if (pthread_create(args.thrd, NULL, run_bridge_server, &args) ||
		pthread_detach(args.thrd))
	{
		perror("pthread_create|detach()");
		return ERR;
	}

	return OK;
}
