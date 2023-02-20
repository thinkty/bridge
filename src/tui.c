#include "tui.h"

int run_tui()
{
	/* Initialize ncurses mode */
	initscr();

	/* Allow control characters, disable echo, allow function keys */
	if (cbreak() != OK ||
			noecho() != OK ||
			keypad(stdscr, TRUE) != OK)
	{
		fprintf(stderr, "Failed to set ncurses options\n");
		return ERR;
	}

	ui_t args;
	args.index = 0;

	/* TODO: maybe display screen in a separate thread so that */
	/* input handling and table update does not collide? */
	display_screen(&args);

	/* Handle user input */
	handle_input(&args);

	/* Finish UI mode */
	endwin();

	return OK;
}

void display_screen(ui_t * args)
{
	clear();

	/* TODO: take and display volatile table */

	/* Border around window */
	int maxy = getmaxy(stdscr);

	/* Show menu on bottom */
	move(maxy-1, 0);
	display_menu();

	/* TODO:  */
	mvprintw(0, 0, "index: %d", args->index);
}

void display_menu()
{
	addstr(" PgUp ");
	attron(A_STANDOUT);
	addstr(" Move Up ");
	attroff(A_STANDOUT);
	addstr(" PgDn ");
	attron(A_STANDOUT);
	addstr(" Move Down ");
	attroff(A_STANDOUT);
	addstr(" q ");
	attron(A_STANDOUT);
	addstr(" Quit ");
	attroff(A_STANDOUT);
}

void handle_input(ui_t * args)
{
	for (;;) {
		int input = getch();

		switch (input) {

			/* Increment current index */
			case KEY_PPAGE:
				// TODO: set upper limit
				args->index++;
				break;

			/* Decrement current index */
			case KEY_NPAGE:
				if (args->index > 0) {
					args->index--;
				}
				break;

			/* Quit the program */ 
			case 'q':
				return;
			
			default:
				break;
		}

		display_screen(args);

	}
}
