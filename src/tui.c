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

	/* Initialize the semaphore for indicating refresh */
	ui->update_sem = malloc(sizeof(sem_t));
	if (ui->update_sem == NULL || sem_init(ui->update_sem, 0, 1)) {
		cleanup_ui(ui);
		perror("malloc|sem_init(ui_t->update_sem");
		return NULL;
	}

	/* Initialize the logs */
	ui->log_head = 0;
	ui->log_tail = 0;
	for (int i = 0; i < TUI_LOGGER_HEIGHT; i++) {
		for (int j = 0; j < TUI_LOGGER_LENGTH; j++) {
			memset(ui->logs[i], 0, TUI_LOGGER_LENGTH+1);
		}
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

	/* Table height will be the left over of all other fixed height windows */
	int table_height = height
		-(getmaxy(ui->title_scr)
		+getmaxy(ui->log_scr)
		+getmaxy(ui->key_scr));

	ui->table_scr = newwin(
		table_height,
		TUI_TABLE_WIDTH,
		getmaxy(ui->title_scr) + getmaxy(ui->log_scr),
		0);

	if (ui->table_scr == NULL) {
		cleanup_ui(ui);
		return NULL;
	}

	ui->topic_scr = newwin(
		table_height,
		width - TUI_TABLE_WIDTH - 1,
		getmaxy(ui->title_scr) + getmaxy(ui->log_scr),
		TUI_TABLE_WIDTH + 1);

	if (ui->topic_scr == NULL) {
		cleanup_ui(ui);
		return NULL;
	}

	return ui;
}

void log_tui(ui_t * ui, char * message)
{
	if (ui == NULL || message == NULL) {
		return;
	}

	/* If there is something at tail, clear it before overwriting */
	if (ui->logs[ui->log_head][0] != 0) {
		memset(ui->logs[ui->log_head], 0, TUI_LOGGER_LENGTH+1);
	}

	/* If the oldest message needs to be cleared, increment the tail */
	if (ui->log_head == ui->log_tail) {
		ui->log_tail++;
		if (ui->log_tail >= TUI_LOGGER_HEIGHT) {
			ui->log_tail = 0;
		}
	}

	/* Copy in length of whatever is shorter */
	strncpy(ui->logs[ui->log_head], message, MIN(TUI_LOGGER_LENGTH, strlen(message)));

	/* If it goes beyond, wrap around */
	ui->log_head++;
	if (ui->log_head >= TUI_LOGGER_HEIGHT) {
		ui->log_head = 0;
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

		/* There should be enough space to display topics and subscribers */
		int height = getmaxy(stdscr);
		int table_height = height-(getmaxy(ui->title_scr)+getmaxy(ui->log_scr)+getmaxy(ui->key_scr));
		if (table_height < TUI_MIN_TABLE_HEIGHT) {
			clear_screens(ui, true);
			mvwprintw(ui->title_scr, 0, 1, "Screen too small to display topics and subscribers...");
			wrefresh(ui->title_scr);
			continue;
		}

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
		int index = (ui->log_tail + i) % TUI_LOGGER_HEIGHT;

		/* Empty, skip */
		if (ui->logs[index][0] == 0) {
			continue;
		}

		wmove(ui->log_scr, i+1, 1);
		wprintw(ui->log_scr, ui->logs[index]);
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

	/* Return if nothing in table */
	if (table->num_topics == 0) {
		wrefresh(ui->topic_scr);
		return;
	}

	/* Iterate to the currently selected topic */
	int topic_num = 0, count = 0;
	for (int i = 0; i < table->map_size; i++) {
		if (table->map[i] == NULL) {
			continue;
		}

		/* Print all the subscribers to the topic */
		if (topic_num == ui->index) {
			pthread_mutex_lock(table->lock);
			subscriber_t * subscriber = table->map[i]->subscriber;
			while (subscriber != NULL) {

				uint32_t ip = subscriber->ip;
				unsigned int f4 = 0xff & ip; ip = ip >> 8;
				unsigned int f3 = 0xff & ip; ip = ip >> 8;
				unsigned int f2 = 0xff & ip; ip = ip >> 8;
				unsigned int f1 = 0xff & ip;

				/* Adding 1s since border */
				mvwprintw(ui->topic_scr, count+1, 1, "[%d] %u.%u.%u.%u : %u", subscriber->csock, f1, f2, f3, f4, subscriber->port);

				subscriber = subscriber->next;
				count++;
			}
			pthread_mutex_unlock(table->lock);
			break;
		}

		/* If not current selected topic, increment */
		topic_num++;
	}

	/* Title */
	mvwprintw(ui->topic_scr, 0, 2, "Subscribers (%d)", count);

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

void clear_screens(const ui_t * ui, bool refresh)
{
	wclear(ui->title_scr);
	wclear(ui->log_scr);
	wclear(ui->table_scr);
	wclear(ui->topic_scr);
	wclear(ui->key_scr);

	if (refresh) {
		wrefresh(ui->title_scr);
		wrefresh(ui->log_scr);
		wrefresh(ui->table_scr);
		wrefresh(ui->topic_scr);
		wrefresh(ui->key_scr);
	}
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
