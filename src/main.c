#include "server.h"

int main(int argc, char *argv[])
{
    /* Check arguments */
	if (argc != 2 || argv[1] == NULL) {
		printf("Usage: %s <port>\n", argv[0]);
		return ERR;
	}

    debug("Starting Bridge on port %s", argv[1]);

    /* Start the bridge server */
    run_bridge_server((unsigned short) atoi(argv[1]));
}
