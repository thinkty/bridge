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
	if (ui->update_sem == NULL || sem_init(ui->update_sem, 0, 1)) {
		cleanup_ui(ui);
		perror("malloc|sem_init(ui_t->update_sem");
		return NULL;
	}
	ui->logs.list = malloc(sizeof(char *) * TUI_LOGGER_HEIGHT);
	if (ui->logs.list == NULL) {
		cleanup_ui(ui);
		perror("malloc(ui_t->logs.list)");
		return NULL;
	}
	ui->logs.size = TUI_LOGGER_HEIGHT;
	ui->logs.head = 0;
	for (int i = 0; i < TUI_LOGGER_HEIGHT; i++) {
		ui->logs.list[i] = NULL;
	}

	/* Allocate new windows for logging, table, and keys */
	int width = getmaxx(stdscr);
	int height = getmaxy(stdscr);
	ui->title_scr = newwin(TUI_HEADER_HEIGHT, width, 0, 0);
	if (ui->title_scr == NULL) {
		cleanup_ui(ui);
		return NULL;
	}
	ui->log_scr = newwin(TUI_LOGGER_HEIGHT+TUI_BORDERS, width, TUI_HEADER_HEIGHT, 0);
	if (ui->log_scr == NULL) {
		cleanup_ui(ui);
		return NULL;
	}
	ui->key_scr = newwin(TUI_KEY_HEIGHT, width, height-1, 0);
	if (ui->key_scr == NULL) {
		cleanup_ui(ui);
		return NULL;
	}

	/* There should be enough window to display the topics */
	int table_height = height-(getmaxy(ui->title_scr)+getmaxy(ui->log_scr)+getmaxy(ui->key_scr));
	if (table_height < TUI_MIN_TABLE_HEIGHT) {
		cleanup_ui(ui);
		return NULL;
	}
	ui->table_scr = newwin(table_height, width/2, getmaxy(ui->title_scr)+getmaxy(ui->log_scr), 0);
	if (ui->table_scr == NULL) {
		cleanup_ui(ui);
		return NULL;
	}
	ui->topic_scr = newwin(table_height, width/2, getmaxy(ui->title_scr)+getmaxy(ui->log_scr), width/2);
	if (ui->topic_scr == NULL) {
		cleanup_ui(ui);
		return NULL;
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
	
		display_server_info(ui);
		display_logs(ui);
		display_table(ui, table);
		display_subscribers(ui, table);
		display_keys(ui);
	}
}

void display_server_info(const ui_t * ui)
{
	/* Display at the top */
	mvwprintw(ui->title_scr, 0, 1, "Bridge - %s:%u", ui->ip, ui->port);
	wrefresh(ui->title_scr);
}

void display_logs(const ui_t * ui)
{
	/* Border */
	box(ui->log_scr, 0, 0);

	/* Title */
	mvwprintw(ui->log_scr, 0, 2, "Logs");

	/* Print the actual logs */
	wmove(ui->log_scr, 1, 0);
	// TODO:

	wrefresh(ui->log_scr);
}

void display_table(const ui_t * ui, const table_t * table)
{
	/* Border */
	box(ui->table_scr, 0, 0);

	/* Title */
	mvwprintw(ui->table_scr, 0, 2, "Table");

	/* Print the entries in the table */
	wmove(ui->table_scr, 1, 0);
	if (table->num_entries != 0) {
		// TODO:
	}
	// TODO: also highlight the currently selected one

	wrefresh(ui->table_scr);
}

void display_subscribers(const ui_t * ui, const table_t * table)
{
	/* Border */
	box(ui->topic_scr, 0, 0);

	/* Title */
	mvwprintw(ui->topic_scr, 0, 2, "Topic");

	/* Print the subscribers for the selected topic */
	wmove(ui->topic_scr, 1, 0);
	if (table->num_entries != 0) {
		// TODO:
	}

	wrefresh(ui->topic_scr);
}

void display_keys(const ui_t * ui)
{
	/* Show keys on key screen */
	wmove(ui->key_scr, 0,0);
	waddstr(ui->key_scr, " ArrUp ");
	wattron(ui->key_scr, A_STANDOUT);
	waddstr(ui->key_scr, " Move Up ");
	wattroff(ui->key_scr, A_STANDOUT);
	waddstr(ui->key_scr, " ArrDn ");
	wattron(ui->key_scr, A_STANDOUT);
	waddstr(ui->key_scr, " Move Down ");
	wattroff(ui->key_scr, A_STANDOUT);
	waddstr(ui->key_scr, " q ");
	wattron(ui->key_scr, A_STANDOUT);
	waddstr(ui->key_scr, " Quit ");
	wattroff(ui->key_scr, A_STANDOUT);
	wrefresh(ui->key_scr);
}

void cleanup_ui(ui_t * ui)
{
	/* End curses mode */
	endwin();

	if (ui != NULL) {

		/* Free the semaphore */
		if (ui->update_sem != NULL) {
			sem_destroy(ui->update_sem);
			free(ui->update_sem);
		}

		/* Free the log list */
		if (ui->logs.list != NULL) {
			for (int i = 0; i < ui->logs.size; i++) {
				if (ui->logs.list[i] != NULL) {
					free(ui->logs.list[i]);
				}
			}
			free(ui->logs.list);
		}

		/* Free the windows */
		if (ui->title_scr != NULL) {
			delwin(ui->title_scr);
			ui->title_scr = NULL;
		}
		if (ui->log_scr != NULL) {
			delwin(ui->log_scr);
			ui->log_scr = NULL;
		}
		if (ui->table_scr != NULL) {
			delwin(ui->table_scr);
			ui->table_scr = NULL;
		}
		if (ui->key_scr != NULL) {
			delwin(ui->key_scr);
			ui->key_scr = NULL;
		}

		free(ui);
	}
}
