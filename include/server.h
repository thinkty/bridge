#ifndef BRIDGE_SERVER_H
#define BRIDGE_SERVER_H

#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

#include "table.h"
#include "tcp.h"
#include "tui.h"

/* Protocol related constants */
#define P_CMD_LEN          (1)
#define P_TOPIC_LEN        (TABLE_TOPIC_LEN)
#define P_TOPIC_SUBSCRIBE   "S"
#define P_TOPIC_UNSUBSCRIBE "U"
#define P_TOPIC_PUBLISH     "P"

/* Server response constants */
#define SERVER_MSG_OK   "OK"
#define SERVER_MSG_FAIL "FAIL"

/**
 * @brief Store information to be passed on to the server thread.
 * 
 * @param table Table containing all topic entries and subscription information
 * @param ui Initialized UI data structure
 */
typedef struct server_args {
	table_t * table;
	ui_t * ui;
} server_args_t;

/**
 * @brief Store client info to be passed on to the handler thread.
 * 
 * @param csock Client socket descriptor
 * @param ip IP address of the requester
 * @param port Port number of the requester
 * @param table Table containing all topic entries
 */
typedef struct handler_args {
	int csock;
    uint32_t ip;
    uint16_t port;
	table_t * table;
	ui_t * ui;
} handler_args_t;

/**
 * @brief Initialize the bridge server and start accepting new connections.
 *
 * @param args Contains table, UI, socket file descriptor.
 */
void * run_server(void * args);

/**
 * @brief Get server's IP (interface) and port number to display.
 * 
 * @param ui Initialized UI data structure
 * @param sock Server socket descriptor
*/
void fetch_server_info(ui_t * ui, int sock);

/**
 * @brief Handle the connection between newly accepted client. Parse the input
 * following the Bridge protocol.
 *
 * @param args Contains table, UI, client host information.
 */
void * handle(void * args);

/**
 * @brief Handle subscribing to a new topic.
 * 
 * @param table Table containing all topic entries
 * @param topic Topic to subscribe to
 * @param csock Client socket descriptor
 * @param ip IP address of the requester
 * @param port Port number of the requester
 */
void subscribe(table_t * table, char * topic, int csock, uint32_t ip, uint16_t port);

/**
 * @brief Handle unsubscribing given subscriber from the list.
 * 
 * @param table Table containing all topic entries
 * @param topic Topic to subscribe to
 * @param csock Client socket descriptor
 * @param ip IP address of the requester
 * @param port Port number of the requester
 */
void unsubscribe(table_t * table, char * topic, int csock, uint32_t ip, uint16_t port);

/**
 * @brief Handle publishing to the subscribers of the given topic.
 * 
 * @param table Table containing all topic entries
 * @param topic Topic to subscribe to
 * @param csock Client socket descriptor
 */
void publish(table_t * table, char * topic, int csock);

#endif
