#include "../include/helper.h"

int chandle(int);

int main(int argc, char *argv[])
{

	// Check arguments (domain and port)
	if (argc != 3) {
		fprintf(stderr, "main()\n");
		return -1;
	}

	// Connect to server
	int sock = h_connect(argv[1], argv[2]);
	if (sock < 0) {
		fprintf(stderr, "h_connect()\n");
		return -1;
	}

	// Interact with the server
	if (chandle(sock) < 0) {
		fprintf(stderr, "handle()\n");
	}

	close(sock);
	return 0;
}

/**
 * Client-side handler to handle user interaction in a loop-like manner
 * or do anything that needs to be done with the connected server via
 * the socket.
 */
int chandle(int sock)
{
	// Get user input and send to server
	char * input = NULL;
	size_t len = 0;

	for (;;) {
		printf("> ");
		ssize_t nread = getline(&input, &len, stdin);
		if (nread <= 0 || strcmp(input, "exit\n") == 0) {
			break;
		}
		if (write(sock, input, nread) <= 0) {
			fprintf(stderr, "write()\n");
			return -1;
		}
		if (read(sock, input, nread) < 0) {
			fprintf(stderr, "read()\n");
			return -1;
		}
		printf("< %s", input);
	}

	if (input) {
		free(input);
		input = NULL;
	}

	return 0;
}

