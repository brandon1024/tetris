#include <signal.h>
#include <unistd.h>
#include <ncurses.h>

#include "tetris-well.h"

static int drop = 0;
static int game_running = 1;
static struct tetris_well well;

static void draw_board(void);
static void alarm_sig_handler(int);

int main(void)
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	timeout(0);
	curs_set(0);

	start_color();
	init_pair(CELL_TYPE_I, COLOR_CYAN, COLOR_BLACK);
	init_pair(CELL_TYPE_O, COLOR_BLUE, COLOR_BLACK);
	init_pair(CELL_TYPE_T, COLOR_WHITE, COLOR_BLACK);
	init_pair(CELL_TYPE_S, COLOR_YELLOW, COLOR_BLACK);
	init_pair(CELL_TYPE_Z, COLOR_GREEN, COLOR_BLACK);
	init_pair(CELL_TYPE_J, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(CELL_TYPE_L, COLOR_RED, COLOR_BLACK);

	tetris_well_init(&well);
	tetrinimo_new(&well);

	clear();

	signal(SIGALRM, alarm_sig_handler);
	alarm(1);

	while (game_running) {
		if (drop) {
			if (tetrimino_shift(&well, SHIFT_DOWN) < 0) {
				tetris_well_commit_tetrimino(&well);

				if (tetrinimo_new(&well))
					game_running = 0;
			}

			draw_board();
			drop = 0;
		}

		switch (getch()) {
			case 'a':
			case 'A':
			case KEY_LEFT:
				tetrimino_shift(&well, SHIFT_LEFT);
				draw_board();
				break;
			case 's':
			case 'S':
			case KEY_DOWN:
				tetrimino_shift(&well, SHIFT_DOWN);
				draw_board();
				break;
			case 'd':
			case 'D':
			case KEY_RIGHT:
				tetrimino_shift(&well, SHIFT_RIGHT);
				draw_board();
				break;
			case ' ':
				tetrimino_rotate(&well);
				draw_board();
				break;
		}
	}

	wclear(stdscr);
	endwin();

	return 0;
}

static void alarm_sig_handler(int sig)
{
	(void)sig;

	drop = 1;

	alarm(1);
	draw_board();
}

#define ADD_BLOCK(x) do { \
	addch(' '|A_REVERSE|COLOR_PAIR(x)); \
	addch(' '|A_REVERSE|COLOR_PAIR(x)); \
} while(0)
#define ADD_EMPTY() do { \
	addch(' '); \
	addch(' '); \
} while(0)

static void draw_board(void)
{
	for (size_t i = 0; i < BOARD_HEIGHT; i++) {
		move(1 + i, 1);

		for (size_t j = 0; j < BOARD_WIDTH; j++) {
			switch (well.matrix[i][j]) {
				case CELL_TYPE_I:
				case CELL_TYPE_O:
				case CELL_TYPE_T:
				case CELL_TYPE_S:
				case CELL_TYPE_Z:
				case CELL_TYPE_J:
				case CELL_TYPE_L:
					ADD_BLOCK(well.matrix[i][j]);
					break;
				default:
					if (((well.tetrimino_coords[0][0] == j && well.tetrimino_coords[0][1] == i) ||
						 (well.tetrimino_coords[1][0] == j && well.tetrimino_coords[1][1] == i) ||
						 (well.tetrimino_coords[2][0] == j && well.tetrimino_coords[2][1] == i) ||
						 (well.tetrimino_coords[3][0] == j && well.tetrimino_coords[3][1] == i)))
						ADD_BLOCK(well.tetrimino_type);
					else
						ADD_EMPTY();
					break;
			}
		}
	}

	refresh();
}
