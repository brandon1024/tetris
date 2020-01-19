#include "game-engine.h"
#include "display-engine.h"

int main(void)
{
	initialize_display_engine();
	start_game();

	stop_display_engine();

	return 0;
}
