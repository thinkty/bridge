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
	ui->logs.head = 0;
	ui->logs.tail = 0;
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
	if (ui->key_scr == NULL || keypad(ui->key_scr, TRUE) != OK) {
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

void log_tui(ui_t * ui, char * message)
{
	if (ui == NULL) {
		return;
	}

	/* If there is a message at tail, free it */
	if (ui->logs.list[ui->logs.tail] != NULL) {
		free(ui->logs.list[ui->logs.tail]);
		ui->logs.list[ui->logs.tail] = NULL;

		if (ui->logs.tail == ui->logs.head) {
			ui->logs.head++;
			if (ui->logs.head >= TUI_LOGGER_HEIGHT) {
				ui->logs.head = 0;
			}
		}
	}

	ui->logs.list[ui->logs.tail] = malloc(strlen(message));
	if (ui->logs.list[ui->logs.tail] == NULL) {
		/* TODO: Should reset the indices for tail and head as well */
		return;
	}


	/* Copy the message to list */
	strcpy(ui->logs.list[ui->logs.tail], message);
	ui->logs.tail++;
	
	/* If it goes beyond, wrap around */
	if (ui->logs.tail >= TUI_LOGGER_HEIGHT) {
		ui->logs.tail = 0;
	}

	/* Signal to update */
	sem_post(ui->update_sem);
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
	/* Clear the previous stuff on screen */
	wclear(ui->title_scr);

	/* Display at the top */
	mvwprintw(ui->title_scr, 0, 1, "Bridge - %s:%u", ui->ip, ui->port);
	wrefresh(ui->title_scr);
}

void display_logs(const ui_t * ui)
{
	/* Clear the previous stuff on screen */
	wclear(ui->log_scr);

	/* Border */
	box(ui->log_scr, 0, 0);

	/* Title */
	mvwprintw(ui->log_scr, 0, 2, "Logs");

	/* Print the actual logs */
	for (int i = 0; i < TUI_LOGGER_HEIGHT; i++) {
		int index = (ui->logs.head + i) % TUI_LOGGER_HEIGHT;
		if (ui->logs.list[index] == NULL) {
			continue;
		}
		wmove(ui->log_scr, i+1, 1);
		wprintw(ui->log_scr, ui->logs.list[index]);
	}

	wrefresh(ui->log_scr);
}

void display_table(const ui_t * ui, const table_t * table)
{
	/* Clear the previous stuff on screen */
	wclear(ui->table_scr);

	/* Border */
	box(ui->table_scr, 0, 0);

	/* Title */
	mvwprintw(ui->table_scr, 0, 2, "Table (%ld)", table->num_topics);

	/* Return if nothing in table */
	if (table->num_topics == 0) {
		wrefresh(ui->table_scr);
		return;
	}

	/* Print the entries in the table */
	int num = 0;
	for (int i = 0; i < table->map_size; i++) {
		if (table->map[i] == NULL) {
			continue;
		}

		/* Highlight the current selection */
		if (num == ui->index) {
			wattron(ui->table_scr, A_STANDOUT);
		}

		/* Adding 1s since border */
		pthread_mutex_lock(table->lock);
		mvwprintw(ui->table_scr, num+1, 1, table->map[i]->str);
		pthread_mutex_unlock(table->lock);
		if (num == ui->index) {
			wattroff(ui->table_scr, A_STANDOUT);
		}
		num++;
	}

	wrefresh(ui->table_scr);
}

void display_subscribers(const ui_t * ui, const table_t * table)
{
	/* Clear the previous stuff on screen */
	wclear(ui->topic_scr);

	/* Border */
	box(ui->topic_scr, 0, 0);

	/* Title */
	mvwprintw(ui->topic_scr, 0, 2, "Topic");

	/* Return if nothing in table */
	if (table->num_topics == 0) {
		wrefresh(ui->topic_scr);
		return;
	}

	/* Iterate to the currently selected topic */
	int num = 0;
	for (int i = 0; i < table->map_size; i++) {
		if (table->map[i] == NULL) {
			continue;
		}

		/* Print all the subscribers to the topic */
		if (num == ui->index) {
			int sub_num = 0;
			pthread_mutex_lock(table->lock);
			subscriber_t * subscriber = table->map[i]->subscriber;
			while (subscriber != NULL) {

				uint32_t ip = subscriber->ip;
				unsigned int f4 = 0xff & ip; ip = ip >> 8;
				unsigned int f3 = 0xff & ip; ip = ip >> 8;
				unsigned int f2 = 0xff & ip; ip = ip >> 8;
				unsigned int f1 = 0xff & ip;

				/* Adding 1s since border */
				mvwprintw(ui->topic_scr, sub_num+1, 1, "%u.%u.%u.%u:%u (%d)", f1, f2, f3, f4, subscriber->port, subscriber->csock);

				subscriber = subscriber->next;
				sub_num++;
			}
			pthread_mutex_unlock(table->lock);
			break;
		}

		/* If not current selected topic, increment */
		num++;
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

		/* Free the messages in the log list */
		while (ui->logs.head != ui->logs.tail) {
			free(ui->logs.list[ui->logs.head]);
			ui->logs.head++;
			if (ui->logs.head >= TUI_LOGGER_HEIGHT) {
				ui->logs.head = 0;
			}
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
