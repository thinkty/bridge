#ifndef BRIDGE_TABLE_H
#define BRIDGE_TABLE_H

#include <pthread.h>
#include <stdint.h>

#include "util.h"

#define ENTRY_NAME_LEN (20)

/**
    TODO:
 * @brief Table keeping track of subscriber entries
 * 
 * @param lock Mutex lock for the name-address table
 */
typedef struct {
	pthread_mutex_t * lock;
} table_t;

/**
 * @brief An entry in the table
 * 
 * @param ip IP address of the requester
 * @param port Port number of the requester
 * @param topics Topics currently subscribed by the requester
 */
typedef struct entry {
    uint32_t ip;
    uint16_t port;
    char topics[ENTRY_NAME_LEN]; // TODO:
} entry_t;

/**
 * @brief Initialize the pub/sub table (mutexes, etc.)
 * 
 * @returns OK on success and ERR on failure.
 */
int init_table(table_t * table);




void subscribe(table_t * table, entry_t entry);
void unsubscribe(table_t * table, entry_t entry);
void publish(table_t * table, entry_t entry);

#endif
