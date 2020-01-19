#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include "tetris-well.h"

#include <stdio.h>
#include "display-engine.h"

static size_t cell_init_coords[7][4][2] = {
		{{4, 0}, /* pivot */ {4, 1}, {4, 2}, {4, 3}}, // type I
		{{4, 0}, {5, 0}, {4, 1}, {5, 1}}, // type O
		{{4, 0}, /* pivot */ {5, 0}, {6, 0}, {5, 1}}, // type T
		{{5, 0}, /* pivot */ {5, 1}, {6, 0}, {4, 1}}, // type S
		{{4, 0}, /* pivot */ {5, 1}, {5, 0}, {6, 1}}, // type Z
		{{4, 0}, /* pivot */ {5, 1}, {4, 1}, {6, 1}}, // type J
		{{4, 1}, /* pivot */ {5, 1}, {6, 1}, {6, 0}}, // type L
};

static int tetrimino_overlapping_on_board(struct tetris_well *, size_t [4][2]);

void tetris_well_init(struct tetris_well *well)
{
	struct timeval time;
	int ret = gettimeofday(&time, NULL);
	assert(!ret /* gettimeofday() failed; cannot seed RNG */);

	srandom((unsigned)time.tv_sec ^ (unsigned)time.tv_usec);

	memset(well->matrix, 0, sizeof(uint8_t) * BOARD_HEIGHT * BOARD_WIDTH);
	memset(well->tetrimino_coords, 0, sizeof(size_t) * 4 * 2);
	well->tetrimino_type = CELL_TYPE_NONE;
}

int tetrinimo_new(struct tetris_well *well)
{
	size_t index = random() % 7;
	well->tetrimino_type = (uint8_t)((unsigned)1 << index);
	memcpy(well->tetrimino_coords, cell_init_coords[index], sizeof(size_t) * 4 * 2);

	return tetrimino_overlapping_on_board(well, well->tetrimino_coords);
}

int tetrimino_shift(struct tetris_well *well, int direction)
{
	size_t shifted_coordinates[4][2];

	for (size_t i = 0; i < 4; i++) {
		switch (direction) {
			case SHIFT_LEFT:
				// have we reached left board boundary
				if (well->tetrimino_coords[i][0] == 0)
					return 1;

				shifted_coordinates[i][0] = well->tetrimino_coords[i][0] - 1;
				shifted_coordinates[i][1] = well->tetrimino_coords[i][1];
				break;
			case SHIFT_RIGHT:
				// have we reached right board boundary
				if (well->tetrimino_coords[i][0] == (BOARD_WIDTH - 1))
					return 1;

				shifted_coordinates[i][0] = well->tetrimino_coords[i][0] + 1;
				shifted_coordinates[i][1] = well->tetrimino_coords[i][1];
				break;
			case SHIFT_DOWN:
				// have we reached bottom board boundary
				if (well->tetrimino_coords[i][1] == (BOARD_HEIGHT - 1))
					return -1;

				shifted_coordinates[i][0] = well->tetrimino_coords[i][0];
				shifted_coordinates[i][1] = well->tetrimino_coords[i][1] + 1;
				break;
			default:
				assert(0 /* unrecognized shift direction */);
		}
	}

	// determine if any coordinates overlap with other pieces on the board
	if (tetrimino_overlapping_on_board(well, shifted_coordinates))
		return direction == SHIFT_DOWN ? -1 : 1;

	// commit rotation
	for (size_t i = 0; i < 4; i++) {
		well->tetrimino_coords[i][0] = shifted_coordinates[i][0];
		well->tetrimino_coords[i][1] = shifted_coordinates[i][1];
	}

	return 0;
}

int tetrimino_rotate(struct tetris_well *well)
{
	size_t rotated_coordinates[4][2];

	/* game piece O does not rotate */
	if (well->tetrimino_type == CELL_TYPE_O)
		return 0;

	ssize_t off_x = 0, off_y = 0;
	size_t i = 0;

	/* all rotate about tetrimino_coords[1] */
	while (i < 4) {
		ssize_t x = - ((ssize_t)well->tetrimino_coords[i][1] - (ssize_t)well->tetrimino_coords[1][1]) + well->tetrimino_coords[1][0] + off_x;
		ssize_t y = ((ssize_t)well->tetrimino_coords[i][0] - (ssize_t)well->tetrimino_coords[1][0]) + well->tetrimino_coords[1][1] + off_y;

		/*
		 * Check if the cell is off the game board; we need to shift all
		 * coordinates by some offset.
		 * */
		if (x < 0) {
			off_x = -x;
			i = 0;
			continue;
		} else if (x >= BOARD_WIDTH) {
			off_x = -(x - BOARD_WIDTH + 1);
			i = 0;
			continue;
		}

		if (y < 0) {
			off_y = -y;
			i = 0;
			continue;
		} else if (y >= BOARD_HEIGHT) {
			off_y = -(y - BOARD_HEIGHT + 1);
			i = 0;
			continue;
		}

		assert(x >= 0 && x < BOARD_WIDTH);
		assert(y >= 0 && y < BOARD_HEIGHT);
		rotated_coordinates[i][0] = x;
		rotated_coordinates[i][1] = y;
		i++;
	}

	// determine if any coordinates overlap with other pieces on the board
	if (tetrimino_overlapping_on_board(well, rotated_coordinates))
		return 1;

	// commit rotation
	for (i = 0; i < 4; i++) {
		well->tetrimino_coords[i][0] = rotated_coordinates[i][0];
		well->tetrimino_coords[i][1] = rotated_coordinates[i][1];
	}

	return 0;
}

int tetris_well_commit_tetrimino(struct tetris_well *well)
{
	int rows_collapsed = 0;

	for (size_t i = 0; i < 4; i++) {
		size_t x_coord = well->tetrimino_coords[i][0];
		size_t y_coord = well->tetrimino_coords[i][1];

		assert(well->matrix[y_coord][x_coord] == CELL_TYPE_NONE
			   /* cannot commit game piece; another piece is directly below */);
		well->matrix[y_coord][x_coord] = well->tetrimino_type;
	}

	// from bottom to top, shift any rows that are full
	for (size_t i = 0; i < BOARD_HEIGHT; i++) {
		if (!memchr(well->matrix[i], CELL_TYPE_NONE, sizeof(uint8_t) * BOARD_WIDTH)) {
			for (size_t j = i; j > 0; j--)
				memcpy(well->matrix[j], well->matrix[j - 1], sizeof(uint8_t) * BOARD_WIDTH);

			memset(well->matrix[0], CELL_TYPE_NONE, sizeof(uint8_t) * BOARD_WIDTH);
			rows_collapsed++;
		}
	}

	return rows_collapsed;
}

static int tetrimino_overlapping_on_board(struct tetris_well *well, size_t coords[4][2])
{
	for (size_t i = 0; i < 4; i++) {
		size_t x_coord = coords[i][0];
		size_t y_coord = coords[i][1];

		if (well->matrix[y_coord][x_coord] != CELL_TYPE_NONE)
			return 1;
	}

	return 0;
}
