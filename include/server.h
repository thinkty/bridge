#ifndef BRIDGE_SERVER_H
#define BRIDGE_SERVER_H

#include <ifaddrs.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>  /* getifaddrs() */
#include <unistd.h>

#include "table.h"
#include "tcp.h"
#include "tui.h"

#define SERVER_BUF_SIZE   (100)

/* Protocol related constants */
#define P_CMD_LEN         (1)
#define P_CMD_SUBSCRIBE   'S'
#define P_CMD_UNSUBSCRIBE 'U'
#define P_CMD_PUBLISH     'P'
#define P_TOPIC_LEN       (TABLE_TOPIC_LEN)

/* Server response constants */
#define SERVER_MSG_OK   "OK"
#define SERVER_MSG_FAIL "FAIL"

/**
 * Commands for the protocol
 */
enum CMD {
	CMD_UNDEFINED,
	CMD_SUBSCRIBE,
	CMD_UNSUBSCRIBE,
	CMD_PUBLISH,
};

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
	table_t * table;
	ui_t * ui;
    uint32_t ip;
    uint16_t port;
	int csock;
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
 * @brief Log the newly accepted connection and its specifics.
 * 
 * @param ui Initialized UI data structure
 * @param ip IP address of the requester
 * @param port Port number of the requester
 * @param cmd Parsed command
 * @param topic Parsed topic
 */
void log_connection(ui_t * ui, uint32_t ip, uint16_t port, enum CMD cmd, char * topic);

/**
 * @brief Read and parse the command from connection.
 * 
 * @param csock Client socket file descriptor
 *
 * @returns The appropriate command enum for the input. On error or a unknown
 * input given, P_CMD_UNRECOGNIZED is returned.
 */
enum CMD parse_cmd(int csock);

/**
 * @brief Read and parse the topic with length of P_TOPIC_LEN.
 * 
 * @param csock Client socket file descriptor
 * @param topic 7 bytes long character array to store topic
 * 
 * @returns OK on successfully topic parsed.
 */
int parse_topic(int csock, char * topic);

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
