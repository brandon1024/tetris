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
#define update_level(level, lines_cleared) (level + ((lines += lines_cleared) > ((level + 1) * 10) ? 1 : 0))

static struct tetris_well well;
static int game_running = 1, paused = 0;
static int drop = 0;
static int level = 0, score = 0, lines = 0, frames = 0;

struct itimerval timer;

static void alarm_sig_handler(int sig);

void start_game(void)
{
	tetris_well_init(&well);
	tetrinimo_new(&well);

	signal(SIGALRM, alarm_sig_handler);
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 20000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 20000;
	setitimer(ITIMER_REAL, &timer, NULL);

	while (game_running) {
		if (drop && !paused) {
			if (tetrimino_shift(&well, SHIFT_DOWN) < 0) {
				int lines_cleared = tetris_well_commit_tetrimino(&well);
				score = update_score(score, level, lines_cleared);
				level = update_level(level, lines_cleared);

				if (tetrinimo_new(&well))
					game_running = 0;
			}

			drop = 0;
		}

		switch (user_input()) {
			case INPUT_RIGHT:
				tetrimino_shift(&well, SHIFT_RIGHT);
				break;
			case INPUT_LEFT:
				tetrimino_shift(&well, SHIFT_LEFT);
				break;
			case INPUT_DOWN:
				tetrimino_shift(&well, SHIFT_DOWN);
				break;
			case INPUT_ROTATE:
				tetrimino_rotate(&well);
				break;
			case INPUT_PAUSE:
				paused = !paused;
				break;
			case INPUT_STOP:
				return;
		}

		draw_board(&well, level, score);
	}
}


static void alarm_sig_handler(int sig)
{
	(void)sig;

	int apparent_level = level > 29 ? 29 : level;
	int gravity = level_gravity_speeds[apparent_level];
	if (!paused)
		frames++;

	if (frames > gravity) {
		frames = 0;
		drop = 1;
	}
}
