#ifndef BRIDGE_TUI_H
#define BRIDGE_TUI_H

/*  */
#include <ncurses.h>

#include "util.h"

/* TODO: create a ui struct to handle current index, etc */
typedef struct ui {
    int index;
} ui_t;

/**
 * @brief Initialize and enter ncurses mode.
 *
 * @return OK on success. ERR on failure.
 */
int run_tui();

/**
 * @brief Print the table and menu on screen.
 * 
 * @param args
 */
void display_screen(ui_t * args);

/**
 * @brief Display the menu to the bottom.
 */
void display_menu();

/**
 * @brief Block for user input.
 * 
 * @param args
 */
void handle_input(ui_t * args);

#endif
