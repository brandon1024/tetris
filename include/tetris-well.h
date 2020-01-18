#ifndef TETRIS_TETRIS_WELL_H
#define TETRIS_TETRIS_WELL_H

#include <stdint.h>

/**
 * tetris-well:
 * Manipulate the tetris game by creating and manipulating "tetriminos".
 *
 * definitions:
 * - well: the playfield, a 10x24 grid into which the tetriminos fall.
 * - tetrimino: a "polymino" made up of four square blocks. There are seven types.
 *
 * standard tetrimino types:
 *        ■ ■ ■ ■
 *        (I type)
 *
 *     ■ ■        ■
 *     ■ ■      ■ ■ ■
 *   (O type)  (T type)
 *
 *    ■ ■          ■ ■
 *      ■ ■      ■ ■
 *   (Z type)  (S type)
 *
 *       ■       ■
 *       ■       ■
 *     ■ ■       ■ ■
 *   (J type)  (L type)
 *
 * data structures:
 *   struct tetris_well
 *     - matrix:
 *       A 10x24 matrix that represents the current state of the well. Each cell
 *       in the matrix describes what type of block lives there.
 *     - tetrimino_coords:
 *       The coordinates for the current tetrimino.
 *     - tetrimino_type:
 *       The type of the current tetrimino.
 *
 * basic usage example:
 * int main(void)
 * {
 *     struct tetris_well well;
 *     tetris_well_init(&well);
 *
 *     int game_running = 1;
 *     int score = 0;
 *     while (game_running) {
 *         if (tetrinimo_new(&well)) {
 *             game_running = 0;
 *         } else {
 *             while (1) {
 *                 int ret = tetrimino_shift(&well, SHIFT_DOWN);
 *                 if (ret < 0)
 *                     break;
 *             }
 *
 *             score += tetris_well_commit_tetrimino(&well);
 *         }
 *     }
 *
 *     return 0;
 * }
 * */

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 24

#define SHIFT_LEFT 0
#define SHIFT_RIGHT 1
#define SHIFT_DOWN 2

#define CELL_TYPE_NONE (0)
#define CELL_TYPE_I ((unsigned)1 << (unsigned)0)
#define CELL_TYPE_O ((unsigned)1 << (unsigned)1)
#define CELL_TYPE_T ((unsigned)1 << (unsigned)2)
#define CELL_TYPE_S ((unsigned)1 << (unsigned)3)
#define CELL_TYPE_Z ((unsigned)1 << (unsigned)4)
#define CELL_TYPE_J ((unsigned)1 << (unsigned)5)
#define CELL_TYPE_L ((unsigned)1 << (unsigned)6)

struct tetris_well {
	uint8_t matrix[BOARD_HEIGHT][BOARD_WIDTH];
	size_t tetrimino_coords[4][2];
	uint8_t tetrimino_type;
};

/**
 * Initialize the tetris well with empty cells. By default, the current tetrimino
 * is initialized with type CELL_TYPE_NONE and coordinates at the origin of the
 * well (top-left corner, [0,0]).
 *
 * To add a tetrimino to the well, see tetrinimo_new()
 * */
void tetris_well_init(struct tetris_well *well);

/**
 * Add a new random tetrimino to the top of the well. If the new tetrimino
 * overlaps with another on the well, the game cannot continue and this function
 * returns non-zero.
 * */
int tetrinimo_new(struct tetris_well *well);

/**
 * Shift the current tetrimino in the direction given. If the tetrimino cannot be
 * shifted because it reached a wall or boundary, a positive integer is returned.
 * If the tetrimino collided with another (only possible with SHIFT_DOWN), a
 * negative integer is returned. Otherwise, zero is returned to indicate that
 * the shift was carried out.
 * */
int tetrimino_shift(struct tetris_well *well, int direction);

/**
 * Rotate the current tetrimino 90 degrees in a clockwise direction. If the
 * tetrimino could not rotate because it was obstructed by another tetrimino in
 * the well, returns non-zero. Otherwise, returns zero to indicate that the
 * rotation was carried out successfully.
 * */
int tetrimino_rotate(struct tetris_well *well);

/**
 * Commit the current tetrimino to the well matrix, clearing any rows that have
 * been filled and shifting all rows downward. Returns the number of rows
 * that were cleared.
 * */
int tetris_well_commit_tetrimino(struct tetris_well *well);

#endif //TETRIS_TETRIS_WELL_H
