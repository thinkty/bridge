#ifndef BRIDGE_SERVER_H
#define BRIDGE_SERVER_H

#include <pthread.h>

#include "tcp.h"

#define SERVER_BUFFER_SIZE (100)

/**
 * @brief Store information to be passed on to the server thread.
 * 
 * @param port Server port number
 * @param bridge_thr The thread running the server
 * @param table_lock Mutex lock for the name-address table
 * @param stats_lock Mutex lock for the server status
 * @param stats Enum for the status of the server
 */
typedef struct bridge_server_args {
	unsigned short port;
	pthread_t bridge_thr;
	pthread_mutex_t table_lock;
} bridge_server_args_t;
int run_server_thread(bridge_server_args_t * args);

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
void * run_bridge_server(void * args);

#endif
