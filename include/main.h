#ifndef BRIDGE_MAIN_H
#define BRIDGE_MAIN_H

#include "server.h"
#include "tui.h"

/**
 * @brief Initialize the bridge server thread and run it
 *
 * @param args Server port, mutexes, etc.
 *
 * @returns OK on success and ERR on failure.
 */
int run_server_thread(bridge_server_args_t * args);

/**
 * @brief Signal the server thread to stop
 * 
 * @param args Server port, mutexes, etc.
 */
void stop_server_thread(bridge_server_args_t * args);

/**
 * @brief Cleanup any threads, mutexes.
 *
 * @param arsg Holds server mutexes
 *
 * @returns OK on success and ERR on failure.
 */
int cleanup(bridge_server_args_t * args);

#endif
