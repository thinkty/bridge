#include "tcp.h"

int tcp_listen(unsigned short port)
{
	/* Create TCP socket for IPv4 */
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket()");
		return ERR;
	}

	/* Set socket to reuse address */
	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt()");
		return ERR;
	}

	/* Bind socket to given port */
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind()");
		close(sock);
		return ERR;
	}

	/* Set as passive socket */
	if (listen(sock, SOCK_LISTEN_Q_LEN) < 0) {
		perror("listen()");
		close(sock);
		return ERR;
	}

	return sock;
}

int tcp_accept(int sock, int * csock, uint32_t * ip, uint16_t * port)
{
	struct sockaddr_in caddr;
	socklen_t caddrlen = sizeof(caddr);

	/* Block until new connection requested */
	*csock = accept(sock, (struct sockaddr *) &caddr, &caddrlen);
	if (*csock < 0) {
		perror("accept()");
		return ERR;
	}

	*ip = ntohl(caddr.sin_addr.s_addr);
	*port = ntohs(caddr.sin_port);
	return OK;
}

int tcp_write(int csock, const char * msg, size_t len)
{
	if (write(csock, msg, len) < 0) {
		perror("write()");
		return ERR;
	}

	return OK;
}
