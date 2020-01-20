#include <ncurses.h>

#include "display-engine.h"
#include "tetris-well.h"

static WINDOW *well_window;
static WINDOW *score_window;

#define ADD_BLOCK(w,x) do { \
	waddch((w), ' '|A_REVERSE|COLOR_PAIR(x)); \
	waddch((w), ' '|A_REVERSE|COLOR_PAIR(x)); \
} while(0)
#define ADD_EMPTY(w) do { \
	waddch((w), ' '); \
	waddch((w), ' '); \
} while(0)

void initialize_display_engine(void)
{
	initscr();

	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	timeout(0);
	curs_set(0);

	if (has_colors()) {
		start_color();
		init_pair(CELL_TYPE_I, COLOR_CYAN, COLOR_BLACK);
		init_pair(CELL_TYPE_O, COLOR_BLUE, COLOR_BLACK);
		init_pair(CELL_TYPE_T, COLOR_WHITE, COLOR_BLACK);
		init_pair(CELL_TYPE_S, COLOR_YELLOW, COLOR_BLACK);
		init_pair(CELL_TYPE_Z, COLOR_GREEN, COLOR_BLACK);
		init_pair(CELL_TYPE_J, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(CELL_TYPE_L, COLOR_RED, COLOR_BLACK);
	}

	well_window = newwin(BOARD_HEIGHT + 2, BOARD_WIDTH * 2 + 2, 1, 1);
	box(well_window, 0 , 0);
	score_window = newwin(5, BOARD_WIDTH * 2 + 2, BOARD_HEIGHT + 3, 1);
	box(score_window, 0 , 0);

	wrefresh(well_window);
	wrefresh(score_window);
}

int user_input(void)
{
	switch (getch()) {
		case 'a':
		case 'A':
		case KEY_LEFT:
			return INPUT_LEFT;
		case 's':
		case 'S':
		case KEY_DOWN:
			return INPUT_DOWN;
		case 'd':
		case 'D':
		case KEY_RIGHT:
			return INPUT_RIGHT;
		case ' ':
			return INPUT_ROTATE;
		case 'p':
		case 'P':
			return INPUT_PAUSE;
		case 'q':
		case 'Q':
			return INPUT_STOP;
	}

	return 0;
}

void stop_display_engine(void)
{
	delwin(well_window);
	delwin(score_window);

	endwin();
}

void draw_board(struct tetris_well *well, int level, int score, int lines)
{
	for (size_t i = 0; i < BOARD_HEIGHT; i++) {
		wmove(well_window, i + 1, 1);

		for (size_t j = 0; j < BOARD_WIDTH; j++) {
			switch (well->matrix[i][j]) {
				case CELL_TYPE_I:
				case CELL_TYPE_O:
				case CELL_TYPE_T:
				case CELL_TYPE_S:
				case CELL_TYPE_Z:
				case CELL_TYPE_J:
				case CELL_TYPE_L:
					ADD_BLOCK(well_window, well->matrix[i][j]);
					break;
				default:
					if (((well->tetrimino_coords[0][0] == j && well->tetrimino_coords[0][1] == i) ||
						 (well->tetrimino_coords[1][0] == j && well->tetrimino_coords[1][1] == i) ||
						 (well->tetrimino_coords[2][0] == j && well->tetrimino_coords[2][1] == i) ||
						 (well->tetrimino_coords[3][0] == j && well->tetrimino_coords[3][1] == i)))
						ADD_BLOCK(well_window, well->tetrimino_type);
					else
						ADD_EMPTY(well_window);
			}
		}
	}

	box(well_window, 0 , 0);
	wrefresh(well_window);

	mvwprintw(score_window, 1, 1, "Level: %d", level);
	mvwprintw(score_window, 2, 1, "Score: %d", score);
	mvwprintw(score_window, 3, 1, "Lines: %d", lines);

	box(score_window, 0 , 0);
	wrefresh(score_window);
}
