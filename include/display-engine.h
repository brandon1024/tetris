#ifndef TETRIS_DISPLAY_ENGINE_H
#define TETRIS_DISPLAY_ENGINE_H

#include "tetris-well.h"

#define INPUT_LEFT 1
#define INPUT_RIGHT 2
#define INPUT_DOWN 3
#define INPUT_ROTATE 4
#define INPUT_PAUSE 5
#define INPUT_STOP 6

void initialize_display_engine(void);

int user_input(void);

void draw_board(struct tetris_well *well, int level, int score);

void stop_display_engine(void);

#endif //TETRIS_DISPLAY_ENGINE_H
