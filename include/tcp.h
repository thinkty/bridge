#ifndef BRIDGE_TCP_H
#define BRIDGE_TCP_H

#include <arpa/inet.h>  /* sockaddr_in, htons(), htonl() */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /* socket(), bind(), listen(), accept() */
#include <unistd.h>     /* read(), write() */

#include "table.h"
#include "util.h"

#define SOCK_LISTEN_Q_LEN (5) /* Number of connections to buffer on socket */

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
int tcp_accept(int sock, int * csock, entry_t * entry);

#endif

