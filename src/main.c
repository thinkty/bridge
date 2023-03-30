#include "main.h"

int main(int argc, char *argv[])
{
	/* Initialize UI and topic-subscriber table */
	thr_args_t args = {
		.table = init_table(),
		.ui = init_tui(),
	};
	if (args.table == NULL || args.ui == NULL) {
		fprintf(stderr, "Error : failed to initialize\n");
		return ERR;
	}
	log_tui(args.ui, "UI and table initialized...");

	/* Run the server thread and detach */
	pthread_t server_thr;
	if (pthread_create(&server_thr, NULL, run_server, &args)) {
		cleanup_table(args.table);
		cleanup_ui(args.ui);
		fprintf(stderr, "Error : failed to run server thread\n");
		return ERR;
	}
	log_tui(args.ui, "Server thread detached and running...");

	/* Run the UI thread */
	pthread_t ui_thr;
	if (pthread_create(&ui_thr, NULL, run_tui, &args)) {
		cleanup_table(args.table);
		cleanup_ui(args.ui);
		fprintf(stderr, "Error : failed to run UI thread\n");
		return ERR;
	}
	log_tui(args.ui, "UI thread detached and running...");

	/* Block and handle user input */
	handle_input(args.table, args.ui);

	/* Clean up */
	pthread_cancel(server_thr);
	pthread_cancel(ui_thr);
	cleanup_table(args.table);
	cleanup_ui(args.ui);

	return OK;
}

void handle_input(table_t * table, ui_t * ui)
{
	if (ui == NULL || table == NULL) {
		return;
	}

	for (;;) {
		int input = wgetch(ui->key_scr);

		switch (input) {

			/* Increment current index */
			case KEY_NPAGE:
			case KEY_DOWN:
				pthread_mutex_lock(table->lock);
				if (ui->index < table->num_topics) {
					pthread_mutex_unlock(table->lock);
					ui->index++;
					sem_post(ui->update_sem);
					break;
				}
				pthread_mutex_unlock(table->lock);
				break;

			/* Decrement current index */
			case KEY_PPAGE:
			case KEY_UP:
				if (ui->index > 0) {
					ui->index--;
					sem_post(ui->update_sem);
				}
				break;

			/* Quit the program */ 
			case 'q':
				ui->status = FINISH;
				sem_post(ui->update_sem);
				return;

			/* Reactive display */
			case KEY_RESIZE:
				sem_post(ui->update_sem);
				break;

			default:
				/* TODO: not handling reactive display at the moment */
				break;
		}
	}
}
