#ifndef BRIDGE_SERVER_H
#define BRIDGE_SERVER_H

#include "tcp.h"

#define SERVER_BUFFER_SIZE (100)

/**
 * @brief Handle the connection between newly accepted client.
 *
 * @param csock Client socket file descriptor to read from and write to
 */
void * handle(void * csock);

/**
 * @brief Initialize the bridge server and start accepting new connections.
 *
 * @param port Port number
 */
void * run_bridge_server(unsigned short port);

#endif

