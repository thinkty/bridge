#include "table.h"

int init_table(table_t * table)
{
	/* Initialize the table mutex lock */
	if (pthread_mutex_init(table->lock, NULL)) {
		perror("pthread_mutex_init(table->lock)");
        return ERR;
	}

    return OK;
}
