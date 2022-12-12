#include "helper.h"

/**
 * Helper function to connect to given host and port
 */
int h_connect(const char * host, const char * port) {

	struct addrinfo hints, * server;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Lookup host name and get server address
	int ret = getaddrinfo(host, port, &hints, &server);
	if (ret != 0) {
		printf("getaddrinfo()\n");
		return -1;
	}

	int sock = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (sock == -1) {
		printf("socket()\n");
		return -1;
	}

	ret = connect(sock, server->ai_addr, server->ai_addrlen);
	if (ret == -1) {
		printf("connect()\n");
		close(sock);
		return -1;
	}

	freeaddrinfo(server);
	return sock;
}

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

/**
 * Helper function to recv and read until the given delimiter. If the delimeter
 * was found, return the position of the found delimeter. If not found, return 0.
 * On error, return -1.
 */
int h_find(int sock, char delim, char * buff, ssize_t size) {

	int ret = recv(sock, buff, size, 0);

	if (ret == -1) {
		printf("h_readuntil:recv()\n");
		return -1;
	}

	for (int i = 0; i < ret; i++) {
		if (buff[i] == delim) {
			return i; 
		}
	}

	return 0;
}

