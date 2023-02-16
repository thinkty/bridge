#ifndef __TCP_H
#define __TCP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
	/* For */
	#include <winsock2.h>
#else
	/* For sockaddr_in, htons(), htonl() */
	#include <arpa/inet.h>
	/* For socket(), setsockopt(), bind(), listen(), accept() */
	#include <sys/socket.h>
	/* For  read(), write() */
	#include <unistd.h>
#endif

#define OK  (0)
#define ERR (-1)

#define SOCK_LISTEN_Q_LEN (5) /* Number of connections to buffer */
#define SOCK_BUFFER_SIZE  (100)

/**
 * @brief Create socket, bind to given port, and listen.
 *
 * @param port Port number
 *
 * @return Server socket on success. ERR on failure.
 */
int tcp_listen(unsigned short port);

/**
 * @brief Wait (block) and accept new connections.
 *
 * @param sock TCP server socket
 * @param csock TCP client socket
 *
 * @return OK on success. ERR on failure.
 */
int tcp_accept(int sock, int * csock);

#endif

