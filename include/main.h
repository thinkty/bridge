#ifndef BRIDGE_MAIN_H
#define BRIDGE_MAIN_H

#include "server.h"
#include "table.h"
#include "tui.h"

/**
 * @brief Initialize the bridge server thread and run it
 *
 * @param port Port to run the bridge server instance
 * @param thrd Thread to run the bridge server
 * @param table The table to store subscription information
 *
 * @returns OK on success, ERR on failure.
 */
int run_server_thread(unsigned short port, pthread_t * thrd, table_t * table);

#endif
