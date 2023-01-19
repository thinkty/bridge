#include "helper.hh"

/**
 * Helper function to create, bind, and listen to a socket using the given port
 * number.
 */
int h_listen(unsigned short port)
{
	// Create socket for IPv4 and TCP stream
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Unable to create socket");
		return ERR;
	}

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind socket to given port
	if (bind(sock, (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in)) < 0) {
		perror("Unable to bind socket to port");
		close(sock);
		return ERR;
	}

	// Start listening to the given port
	if (listen(sock, LISTEN_Q_LEN) < 0) {
		perror("Unable to listen to port");
		close(sock);
		return ERR;
	}

	return sock;
}

/**
 * Helper function to accept new connection.
 */
int h_accept(int sock, int * csock)
{
	struct sockaddr_in csockaddr;
	socklen_t csockaddrlen = sizeof(struct sockaddr_in);

	*csock = accept(sock, (struct sockaddr *) &csockaddr, &csockaddrlen);
	if (*csock < 0) {
		perror("Unable to accept new connection");
		return ERR;
	}

	return OK;
}

