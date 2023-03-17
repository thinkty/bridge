#ifndef BRIDGE_TABLE_H
#define BRIDGE_TABLE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "util.h"

#define MIN(X,Y) (X < Y ? X : Y)

#define TABLE_TOPIC_LEN       (7)
#define TABLE_INITIAL_SIZE    (10)
#define TABLE_HASH_FNV_PRIME  (0x100000001b3)
#define TABLE_HASH_FNS_OFFSET (0xcbf29ce484222325)

/**
 * @brief Subscriber doubly-linked list.
 * 
 * @param next The next subscriber subscribed to the same topic
 * @param prev The previous subscriber
 * @param ip IP address of the requester
 * @param port Port number of the requester
 * @param csock Socket file descriptor for the subscribed client
 */
typedef struct subscriber {
    struct subscriber * next;
    struct subscriber * prev;
    uint32_t ip;
    uint16_t port;
    int csock;
} subscriber_t;

/**
 * @brief Intermediate data structure to store the subscriber list at given topic.
 * 
 * @param topic The corresponding topic
 * @param subscriber Linked list of subscriber(s)
 */
typedef struct topic {
    char str[TABLE_TOPIC_LEN];
    subscriber_t * subscriber;
} topic_t;

/**
 * @brief Table keeping track of subscriber entries.
 * 
 * @param lock Mutex lock for the pub/sub table
 * @param map The array of entries that correspond to the given topic
 * @param map_size The size of the table (map)
 * @param num_topics Number of entries in the table (map)
 */
typedef struct {
	pthread_mutex_t * lock;
    topic_t ** map;
    uint64_t map_size;
    uint64_t num_topics;
} table_t;

/**
 * @brief Allocate and initialize the pub/sub table
 * 
 * @returns pointer to the allocated table.
 */
table_t * init_table(void);

/**
 * @brief Calculate the hash given the topic string using the FNV-1a hash
 * algorithm.
 * 
 * @see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
 * 
 * @param topic_str The topic string with length less than or equal to set topic size
 * @param map_size The size of the map
 * 
 * @returns An unsigned integer to be used to get the correct topic in the table.
 */
uint64_t hash(char * topic_str, uint64_t map_size);

/**
 * @brief Query the table for the given topic.
 * 
 * @param table Table to query on
 * @param topic_str Topic string
 * 
 * @returns The topic or NULL if the topic does not exist.
 */
topic_t * get_topic(table_t * table, char * topic_str);

/**
 * @brief Insert the new topic into the map. If the topic already exists, return
 * the topic. In collision, use linear probing. If the map is full, double the
 * size and re-insert.
 * 
 * @param table Table to insert to
 * @param topic_str Topic string
 * 
 * @returns The topic or NULL if an error has occurred.
*/
topic_t * set_topic(table_t * table, char * topic_str);

/**
 * @brief A helper function to insert a given topic object into the hash map.
 * The function assumes that the parameters are valid and the map has enough
 * space to insert the topic using linear probing.
 * 
 * @param table Table to insert to
 * @param topic Topic object
 */
void insert_topic(table_t * table, topic_t * topic);

/**
 * @brief Add the subscriber to the topic. If a topic does not exist, insert the
 * new topic and then add the new subscriber.
 * 
 * @param table The table to insert to
 * @param topic_str The topic to insert the new subscriber to
 * @param new_sub The new subscriber to insert
 * 
 * @returns Positive value if it already exists in the table. OK if successfully
 * inserted. ERR if error while inserting to the table.
 */
int insert_sub(table_t * table, char * topic_str, subscriber_t * new_sub);

/**
 * @brief Remove the subscriber from the topic. If a topic does not exist or the
 * subscriber does not exist, ignore.
 * 
 * @param table The table to remove from
 * @param topic_str The topic to remove the subscriber from
 * @param sub Temporary subscriber info to match when searching
*/
void remove_sub(table_t * table, char * topic_str, subscriber_t sub);

/**
 * @brief Clean up the table and free the topics, subscribers, etc.
 * 
 * @param table Table to clean
 */
void cleanup_table(table_t * table);

#endif
