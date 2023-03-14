#ifndef BRIDGE_TUI_H
#define BRIDGE_TUI_H

#include <arpa/inet.h>  /* INET_ADDRSTRLEN */
#include <pthread.h>
#include <ncurses.h>
#include <semaphore.h>
#include <stdlib.h>

#include "table.h"
#include "util.h"

#define TUI_HEADER_HEIGHT (1)
#define TUI_LOGGER_HEIGHT (5)

/* Status enum */
enum Status {
    START,
    FINISH,
};

/**
 * @brief Data structure to store the logs to be displayed in the UI.
 * 
 * @param list The circular buffer of strings that holds the actual logs
 * @param size The size of the list
 * @param head The next index to store the log (for the circular buffer)
 */
typedef struct logs {
    char ** list;
    int size;
    int head;
} logs_t;

/**
 * @brief This will be used throughout the program to display logs, handle the
 * index in the UI, etc.
 * 
 * @param logs List of logs
 * @param index Currently selected entry (topic) in the table
 * @param update_sem Semaphore to indicate number of updates pending
 * @param status If finish is set to 1, stop all threads and clean up
 * @param ip IP address of the server
 * @param port Port number assigned to the server
 */
typedef struct ui {
    logs_t logs;
    int index;
    sem_t * update_sem;
    enum Status status;
    char ip[INET_ADDRSTRLEN];
    uint16_t port;
} ui_t;

/**
 * @brief Store information to be passed on to the UI thread.
 * 
 * @param table Table containing all topic entries and subscription information
 * @param ui Initialized UI data structure
 */
typedef struct ui_args {
	table_t * table;
	ui_t * ui;
} ui_args_t;

/**
 * @brief Initialize and enter ncurses mode.
 *
 * @returns The newly allocated UI data structure or NULL on error.
 */
ui_t * init_tui();

/**
 * @brief Display the table and logs.
 *
 * @param args Contains table and UI data structure
 */
void * run_tui(void * args);

/**
 * @brief Display the name and server info (IP and port) to the top.
 * 
 * @param ui UI data structure to get the stored IP and port
*/
void display_server_info(const ui_t * ui);

/**
 * @brief Display the key options to the bottom.
 */
void display_keys();

/**
 * @brief End Ncurses mode and free UI.
 * 
 * @param ui UI to free
 */
void cleanup_ui(ui_t * ui);

#endif
