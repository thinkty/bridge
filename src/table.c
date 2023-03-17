#include "table.h"

table_t * init_table()
{
	/* Allocate table */
	table_t * table = malloc(sizeof(table_t));
	if (table == NULL) {
		return NULL;
	}

	/* Initialize the table mutex lock */
	table->lock = malloc(sizeof(pthread_mutex_t));
	if (table->lock == NULL) {
		return NULL;
	}
	if (pthread_mutex_init(table->lock, NULL)) {
		perror("pthread_mutex_init(table->lock)");
        return NULL;
	}

	/* Initialize the map */
	table->num_topics = 0;
	table->map_size = TABLE_INITIAL_SIZE;
	table->map = malloc(sizeof(topic_t *) * table->map_size);
	if (table->map == NULL) {
		return NULL;
	}

	/* Initialize each topic in the map to NULL */
	for (int i = 0; i < table->map_size; i++) {
		table->map[i] = NULL;
	}

    return table;
}

uint64_t hash(char * topic_str, uint64_t map_size)
{
	uint64_t hash = TABLE_HASH_FNS_OFFSET;

	for (int i = 0; i < MIN(strlen(topic_str), TABLE_TOPIC_LEN); i++) {
		hash ^= (uint64_t)topic_str[i];
		hash *= TABLE_HASH_FNV_PRIME;
	}

	return hash % map_size;
}

topic_t * get_topic(table_t * table, char * topic_str)
{
	uint64_t index = hash(topic_str, table->map_size);

	/* If it breaks from the while loop, topic does not exist */
	pthread_mutex_lock(table->lock);
	while (table->map[index] != NULL) {

		if (strcmp(table->map[index]->str, topic_str) == 0) {
			pthread_mutex_unlock(table->lock);
			return table->map[index];
		}

		/* Using linear probing */
		index++;
		if (index >= table->map_size) {
			index = 0;
		}
	}
	pthread_mutex_unlock(table->lock);

	return NULL;
}

topic_t * set_topic(table_t * table, char * topic_str)
{
	/* If the topic already exists, return it instead */
	topic_t * topic;
	if ((topic = get_topic(table, topic_str)) != NULL) {
		return topic;
	}

	/* Allocate new topic to insert */
	topic = malloc(sizeof(topic_t));
	if (topic == NULL) {
		return NULL;
	}
	topic->subscriber = NULL;
	strncpy(topic->str, topic_str, TABLE_TOPIC_LEN);

	/* If table is full, double the table size and re-insert */
	pthread_mutex_lock(table->lock);
	if (table->num_topics + 1 >= table->map_size) {
		topic_t ** new_map = malloc(sizeof(topic_t *) * (table->map_size * 2));
		if (new_map == NULL) {
			pthread_mutex_unlock(table->lock);
			return NULL;
		}

		topic_t ** old_map = table->map;
		uint64_t old_map_size = table->map_size;

		table->map = new_map;
		table->map_size = table->map_size * 2;
		table->num_topics = 0;
		
		/* Re-insert previous topics */
		for (int i = 0; i < old_map_size; i++) {
			if (old_map[i] != NULL) {
				insert_topic(table, old_map[i]);
			}
		}
		
		/* Free the old map */
		free(old_map);
	}

	insert_topic(table, topic);
	pthread_mutex_unlock(table->lock);
	return topic;
}

void insert_topic(table_t * table, topic_t * topic)
{
	/* Assumes the table mutex is locked before calling this function */

	uint64_t index = hash(topic->str, table->map_size);

	for(;;) {

		/* Empty slot found */
		if (table->map[index] == NULL) {
			table->map[index] = topic;
			table->num_topics++;
			return;
		}

		/* If not empty, collision. Use linear probing */
		index++;
		if (index >= table->map_size) {
			index = 0;
		}
	}	
}

int insert_sub(table_t * table, char * topic_str, subscriber_t * new_sub)
{
	/* Get the topic or create if it does not exist */
	topic_t * topic = set_topic(table, topic_str);
	if (topic == NULL) {
		return ERR;
	}

	/* First subscriber to add */
	if (topic->subscriber == NULL) {
		topic->subscriber = new_sub;
		new_sub->prev = NULL;
		new_sub->next = NULL;
		return OK;
	}

	/* Iterate to the end and check if it already exists in the list */
	subscriber_t * iter = topic->subscriber;
	for (;;) {
		/* Already subscribed */
		if (iter->csock == new_sub->csock && iter->ip == new_sub->ip && iter->port == new_sub->port) {
			return 1;
		}
		/* Last subscriber */
		if (iter->next == NULL) {
			break;
		}
		iter = iter->next;
	}

	/* Append to the end of the subscriber list */
	iter->next = new_sub;
	new_sub->prev = iter;
	new_sub->next = NULL;
	return OK;
}

void remove_sub(table_t * table, char * topic_str, subscriber_t sub)
{
	topic_t * topic = get_topic(table, topic_str);
	if (topic == NULL || topic->subscriber == NULL) {
		return;
	}

	/* Iterate to the given subscriber */
	subscriber_t * iter = topic->subscriber;
	for (;;) {
		if (iter == NULL) {
			break;
		}
		if (iter->csock == sub.csock && iter->ip == sub.ip && iter->port == sub.port) {
			break;
		}
		iter = iter->next;
	}

	/* Subscriber not found in given topic */
	if (iter == NULL) {
		return;
	}

	/* Unlink the subscriber and free it */
	if (iter->prev == NULL && iter->next == NULL) {
		topic->subscriber = NULL;
	} else if (iter->prev == NULL) {
		topic->subscriber = iter->next;
	} else if (iter->next == NULL) {
		iter->prev->next = NULL;
	} else {
		iter->prev->next = iter->next;
		iter->next->prev = iter->prev;
	}

	free(iter);
}

void cleanup_table(table_t * table)
{
	if (table == NULL) {
		return;
	}

	if (table->lock != NULL) {
		pthread_mutex_destroy(table->lock);
		free(table->lock);
		table->lock = NULL;
	}

	if (table->map != NULL) {

		/* Free the sbuscribers in each topic and the topic itself */
		for (int i = 0; i < table->map_size; i++) {

			if (table->map[i] == NULL) {
				continue;
			}

			subscriber_t * list = table->map[i]->subscriber;
			while (list != NULL) {
				/* If last item, free itself */
				if (list->next == NULL) {
					free(list);
					list = NULL;
				} else {
					list = list->next;
					free(list->prev);
					list->prev = NULL;
				}

				/* Close the client socket */
				// TODO: for some reason, this is causing segmentation fault
				// close(list->csock);
			}

			free(table->map[i]);
			table->map[i] = NULL;
		}
		free(table->map);
		table->map = NULL;
	}

	free(table);
}
