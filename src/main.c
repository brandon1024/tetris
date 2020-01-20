#include <stdio.h>

#include "game-engine.h"
#include "display-engine.h"

int main(void)
{
	int level = 0, lines_cleared = 0;

	initialize_display_engine();
	int score = start_game(&level, &lines_cleared);

	stop_display_engine();

	printf("You reached level %d.\n", level);
	printf("You scored %d points and cleared %d lines.\n", score, lines_cleared);

	return 0;
}
