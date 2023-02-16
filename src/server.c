#include "server.h"

void * run_bridge_server(unsigned short port)
{
	/* Setup TCP IPv4 port */
	int sock = tcp_listen(port);
	if (sock < 0) {
		fprintf(stderr, "Failed to setup port\n");
		return NULL;
	}

	/* Accept incoming connections and handle it in new thread */
	for (;;) {
		int csock;
		if (tcp_accept(sock, &csock) != OK) {
			fprintf(stderr, "Failed to accept new connection\n");
			close(sock);
			return NULL;
		}

		/* Handle client in new thread */
		pthread_t thread;
		if (pthread_create(&thread, NULL, handle, &csock) != 0 ||
				pthread_detach(thread) != 0) {
			perror("pthread()");
			close(sock);
			close(csock);
			return NULL;
		}
	}

	/* Clean-up */
	close(sock);
	return NULL;
}

void * handle(void * csock)
{
	int sock = *((int *) csock);

	char buff[SERVER_BUFFER_SIZE] = {0};
	ssize_t ret;

	for (;;) {

		/* Unable to read from client socket */
		if ((ret = read(sock, buff, SERVER_BUFFER_SIZE)) < 0) {
			perror("read()");
			close(sock);
			return NULL;
		}

		/* EOF */
		if (ret == 0) {
			break;
		}

		buff[ret] = '\0';
		printf("< %s", buff);

		/* Echo */
		if (write(sock, buff, ret) < 0) {
			perror("write()");
			close(sock);
			return NULL;
		}
	}

	/* Clean-up */
	fprintf(stdout, "EOF\n");
	close(sock);
	return NULL;
}

