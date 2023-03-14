#ifndef BRIDGE_MAIN_H
#define BRIDGE_MAIN_H

#include <pthread.h>

#include "server.h"
#include "table.h"
#include "tui.h"

/**
 * @brief Arguments to pass to threads
 * 
 * @param table Table containing all topic entries and subscription information
 * @param ui Initialized UI data structure
 */
typedef struct thr_args {
    table_t * table;
    ui_t * ui;
} thr_args_t;

/**
 * @brief Block for user input.
 * 
 * @param table Table containing all topic entries and subscription information
 * @param ui Initialized UI data structure
 */
void handle_input(table_t * table, ui_t * ui);

#endif
