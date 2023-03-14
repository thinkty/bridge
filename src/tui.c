#include "tui.h"

ui_t * init_tui()
{
	/* Initialize ncurses mode */
	initscr();

	/* Allow control characters, disable echo, allow function keys */
	if (cbreak() != OK || noecho() != OK || keypad(stdscr, TRUE) != OK) {
		endwin();
		return NULL;
	}

	/* Initialize UI and its fields */
	ui_t * ui = malloc(sizeof(ui_t));
	if (ui == NULL) {
		perror("malloc(ui_t)");
		return NULL;
	}
	ui->index = 0;
	ui->status = START;
	ui->update_sem = malloc(sizeof(sem_t));
	if (ui->update_sem == NULL) {
		perror("malloc(ui_t->update_sem");
		return NULL;
	}
	if (sem_init(ui->update_sem, 0, 1)) {
		perror("sem_init(ui->update_sem)");
		return NULL;
	}
	ui->logs.list = malloc(sizeof(char *) * TUI_LOGGER_HEIGHT);
	if (ui->logs.list == NULL) {
		perror("malloc(ui_t->logs.list)");
		return NULL;
	}
	ui->logs.size = TUI_LOGGER_HEIGHT;
	ui->logs.head = 0;
	for (int i = 0; i < TUI_LOGGER_HEIGHT; i++) {
		ui->logs.list[i] = NULL;
	}

	return ui;
}

void * run_tui(void * args)
{
	table_t * table = ((ui_args_t *) args)->table;
	ui_t * ui = ((ui_args_t *) args)->ui;
	if (table == NULL || ui == NULL) {
		return NULL;
	}

	/* Detach from main thread */
	if (pthread_detach(pthread_self())) {
		return NULL;
	}

	/* Refresh the UI when needed */
	for (;;) {
		/* Check status and quit if FINISH */
		if (ui->status == FINISH) {
			return NULL;
		}

		/* Wait for update */
		sem_wait(ui->update_sem);
		if (ui->status == FINISH) {
			return NULL;
		}

		/* Clear the previous stuff on screen */
		clear();

		/* TODO: make sure there is enough screen to display all */

		/* TODO: the top two lines are reserved for the ???  */

		display_server_info(ui);

		/* TODO: wait for mutex and display table */

		display_keys();		
	}
}

void display_server_info(const ui_t * ui)
{
	/* Display at the top */
	mvprintw(0, 0, "Bridge - %s:%u", ui->ip, ui->port);
}

void display_keys()
{
	/* Show keys on bottom */
	int maxy = getmaxy(stdscr);
	move(maxy-1, 0);
	addstr(" ArrUp ");
	attron(A_STANDOUT);
	addstr(" Move Up ");
	attroff(A_STANDOUT);
	addstr(" ArrDn ");
	attron(A_STANDOUT);
	addstr(" Move Down ");
	attroff(A_STANDOUT);
	addstr(" q ");
	attron(A_STANDOUT);
	addstr(" Quit ");
	attroff(A_STANDOUT);
}

void cleanup_ui(ui_t * ui)
{
	/* End curses mode */
	endwin();

	// TODO: free anything else

	if (ui != NULL) {
		if (ui->update_sem != NULL) {
			sem_destroy(ui->update_sem);
			free(ui->update_sem);
		}
		if (ui->logs.list != NULL) {
			for (int i = 0; i < ui->logs.size; i++) {
				if (ui->logs.list[i] != NULL) {
					free(ui->logs.list[i]);
				}
			}
			free(ui->logs.list);
		}
		free(ui);
	}
}
