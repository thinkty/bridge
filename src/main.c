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

	/* Run the server thread and detach */
	pthread_t server_thr;
	if (pthread_create(&server_thr, NULL, run_server, &args)) {
		cleanup_table(args.table);
		cleanup_ui(args.ui);
		fprintf(stderr, "Error : failed to run server thread\n");
		return ERR;
	}

	/* Run the UI thread */
	pthread_t ui_thr;
	if (pthread_create(&ui_thr, NULL, run_tui, &args)) {
		cleanup_table(args.table);
		cleanup_ui(args.ui);
		fprintf(stderr, "Error : failed to run UI thread\n");
		return ERR;
	}

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
		int input = wgetch(ui->title_scr);

		switch (input) {

			/* Increment current index */
			case KEY_PPAGE:
			case KEY_UP:
				if (ui->index < table->num_entries) {
					ui->index++;
					sem_post(ui->update_sem);
				}
				break;

			/* Decrement current index */
			case KEY_NPAGE:
			case KEY_DOWN:
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

			default:
				break;
		}
	}
}
