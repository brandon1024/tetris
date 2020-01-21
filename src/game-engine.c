#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "game-engine.h"
#include "display-engine.h"

static const int score_chart[] = {0, 40, 100, 300, 1200};
static const int level_gravity_speeds[] = {
		48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1
};

#define update_score(score, level, lines_cleared) (score_chart[lines_cleared] * (level + 1) + score)
#define update_level(level, lines_cleared, total_lines_cleared) (level + ((total_lines_cleared) > ((level + 1) * 10) ? 1 : 0))

static struct tetris_well well;
static int game_running = 1, paused = 0;
static int drop = 0;
static int frames = 0;
static int *level_ptr = NULL;

struct itimerval timer;

static void alarm_sig_handler(int sig);

int start_game(int *level, int *lines_cleared)
{
	int score = 0;

	level_ptr = level;
	*level = 0;
	*lines_cleared = 0;

	tetris_well_init(&well);
	tetrinimo_new(&well);

	signal(SIGALRM, alarm_sig_handler);
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 20000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 20000;
	setitimer(ITIMER_REAL, &timer, NULL);

	while (game_running) {
		switch (user_input()) {
			case INPUT_RIGHT:
				if (!paused)
					tetrimino_shift(&well, SHIFT_RIGHT);
				break;
			case INPUT_LEFT:
				if (!paused)
					tetrimino_shift(&well, SHIFT_LEFT);
				break;
			case INPUT_DOWN:
				if (!paused)
					drop = 1;
				break;
			case INPUT_ROTATE:
				if (!paused)
					tetrimino_rotate(&well);
				break;
			case INPUT_PAUSE:
				paused = !paused;
				break;
			case INPUT_STOP:
				game_running = 0;
		}

		if (drop && !paused) {
			if (tetrimino_shift(&well, SHIFT_DOWN) < 0) {
				int lines = tetris_well_commit_tetrimino(&well);
				*lines_cleared = *lines_cleared + lines;
				score = update_score(score, *level, *lines_cleared);
				*level = update_level(*level, lines, *lines_cleared);

				if (tetrinimo_new(&well))
					game_running = 0;
			}

			drop = 0;
		}

		draw_board(&well, *level, score, *lines_cleared);
	}

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;

	return score;
}


static void alarm_sig_handler(int sig)
{
	(void)sig;

	int apparent_level = *level_ptr > 29 ? 29 : *level_ptr;
	int gravity = level_gravity_speeds[apparent_level];
	if (!paused)
		frames++;

	if (frames > gravity) {
		frames = 0;
		drop = 1;
	}
}
