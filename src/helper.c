#include "helper.h"

/**
 * Helper function to create socket, bind, and listen
 */
int h_listen(unsigned short port) {

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		printf("socket()\n");
		return -1;
	}

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret;
	ret = bind(sock, (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		printf("bind()\n");
		close(sock);
		return -1;
	}

	ret = listen(sock, LISTEN_Q_LEN);
	if (ret < 0) {
		printf("listen()\n");
		close(sock);
		return -1;
	}

	return sock;
}

/**
 * Helper function to accept connections
 */
int h_accept(int sock) {
	socklen_t csockaddrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in csockaddr;
	int csock = accept(sock, (struct sockaddr *) &csockaddr, &csockaddrlen);
	return csock;
}
