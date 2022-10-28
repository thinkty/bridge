#include "../include/helper.h"

#define BUFFERSIZE (100)

void * handle(void * csock);

int main(int argc, char *argv[])
{
	// Check arguments
	if (argc != 2) {
		printf("main()\n");
		return 1;
	}

	// Listen to port
	int sock = h_listen((unsigned short) atoi(argv[1]));
	if (sock < 0) {
		printf("h_listen()\n");
		return 1;
	}

	// Accept incoming connections and handle it
	for (;;) {
		int * csock = (int *) malloc(sizeof(int));
		if (csock == NULL) {
			printf("malloc()\n");
			close(sock);
			return 1;
		}

		*csock = h_accept(sock);
		if (*csock < 0) {
			printf("h_accept()\n");
			close(sock);
			return 1;
		}

		// Handle client in new thread
		pthread_t thread;
		if (pthread_create(&thread, NULL, handle, csock) != 0) {
			printf("pthread_create()\n");
			close(*csock);
			close(sock);
			return 1;
		}
		if (pthread_detach(thread) != 0) {
			printf("pthread_detach()\n");
			close(*csock);
			close(sock);
			return 1;
		}
	}

	// Clean up - I don't think this part is reachable
	close(sock);
	return 0;	
}

/**
 * Function to run in a thread to handle commands from client
 */
void * handle(void * csock)
{
	// Parse argument
	int sock = *((int *) csock);
	free(csock);

	// Read for buffer size
	// if bigger than buffersize, too bad :/
	char buff[BUFFERSIZE] = {0};
	int ret = 0;
	for (;;) {
		if ((ret = read(sock, buff, BUFFERSIZE)) < 0) {
			fprintf(stderr, "read()\n");
			close(sock);
			return NULL;
		}

		// EOF
		if (ret == 0) {
			close(sock);
			return NULL;
		}

		buff[ret] = '\0';
		printf("< %s", buff);

		if (write(sock, buff, ret) <= 0) {
			fprintf(stderr, "write()\n");
			close(sock);
			return NULL;
		}
	}

	close(sock);
	return NULL;
}

