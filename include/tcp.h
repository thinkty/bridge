#ifndef BRIDGE_TCP_H
#define BRIDGE_TCP_H

#include <arpa/inet.h>  /* sockaddr_in, htons(), htonl() */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /* socket(), bind(), listen(), accept() */
#include <unistd.h>     /* read(), write() */

#include "util.h"

#define SOCK_LISTEN_Q_LEN (5) /* Number of connections to buffer on socket */

/**
 * @brief Create socket, bind to available port, and listen.
 *
 * @return Server socket on success. ERR on failure.
 */
int tcp_listen();

/**
 * @brief Wait (block), accept new connections, and save connection client info
 * ip and port to the given address.
 *
 * @param sock TCP server socket
 * @param csock TCP client socket
 * @param ip IP address of the requester
 * @param port Port number of the requester
 *
 * @return OK on success. ERR on failure.
 */
int tcp_accept(int sock, int * csock, uint32_t * ip, uint16_t * port);

/**
 * @brief Write to the given client socket the message of specified length.
 * 
 * @param csock Client socket
 * @param msg Buffer containing the message to send
 * @param len Length of the message
 * 
 * @return OK on success. ERR on failure.
 */
int tcp_write(int csock, const char * msg, size_t len);

#endif
