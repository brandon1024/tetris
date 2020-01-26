#include "test-lib.h"
#include "tetris-well.h"

TEST_DEFINE(tetris_well_init_test)
{
	struct tetris_well well;

	TEST_START() {
		tetris_well_init(&well);

		for (size_t i = 0; i < BOARD_HEIGHT; i++) {
			for (size_t j = 0; j < BOARD_WIDTH; j++) {
				assert_zero_msg(well.matrix[i][j], "well matrix at pos %d:%d was non-zero", i, j);
			}
		}

		for (size_t i = 0; i < 4; i++) {
			assert_zero_msg(well.tetrimino_coords[i][0],
					"current tetrimino coord %d had a non-zero x value %zu",
					i, well.tetrimino_coords[i][0]);
			assert_zero_msg(well.tetrimino_coords[i][1],
					"current tetrimino coord %d had a non-zero y value %zu",
					i, well.tetrimino_coords[i][1]);
		}

		assert_eq_msg(CELL_TYPE_NONE, well.tetrimino_type,
				"expected the tetrimino type to be initialized to CELL_TYPE_NONE");

		for (size_t i = 0; i < 7; i++) {
			assert_zero_msg(well.tetrimino_bag[i],
					"tetrimino bag was not initialized to zeroes, was %zu",
					well.tetrimino_bag[i]);
		}

		assert_zero_msg(well.tetrimino_bag_index,
				"tetrimino bag index was not initialized to zeroes, was %zu",
				well.tetrimino_bag_index);
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_new_should_update_tetrimino_bag_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	TEST_START() {
		assert_zero_msg(well.tetrimino_bag_index,
				"tetrimino bag index was not initialized to zeroes, was %zu",
				well.tetrimino_bag_index);

		int ret = tetrimino_new(&well);
		assert_eq_msg(0, ret, "expected a return value of zero from tetrimino_new(), but was %d", ret);

		/*
		 * when the tetrimino bag is filled, the bag index will be 7, but then decremented once
		 * the tetrimino is used, so we should expect a value of 6 here.
		 * */
		assert_eq_msg(6, well.tetrimino_bag_index, "expected tetrimino bag index to be 6, but was %zu",
				well.tetrimino_bag_index);

		for (size_t i = 0; i < 7; i++) {
			assert_true_msg(well.tetrimino_bag[i] < 7 && well.tetrimino_bag[i] >= 0,
					"tetrimino bag at index %zu should have a value between 0 (inclusive) and 7 (exclusive), but was %zu",
					i, well.tetrimino_bag[i]);
		}
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_new_return_if_overlapping_test)
{
	const size_t test_coords_fail[4][2] = {{4, 0}, {5, 0}, {4, 1}, {5, 1}};
	const size_t test_coords_pass[4][2] = {{6, 0}, {0, 0}, {9, 23}, {5, 25}};

	struct tetris_well well;
	tetris_well_init(&well);

	TEST_START() {
		for (size_t i = 0; i < 4; i++) {
			// trick tetrimino_new() to avoid filling tetrimino bag
			well.tetrimino_bag_index = 7;
			well.tetrimino_bag[well.tetrimino_bag_index - 1] = 1; // Type-O tetrimino

			size_t x = test_coords_fail[i][0];
			size_t y = test_coords_fail[i][1];
			well.matrix[y][x] = CELL_TYPE_L;

			int ret = tetrimino_new(&well);
			assert_nonzero_msg(ret, "expected tetrimino_new() return non-zero "
					"since new tetrimino overlaps with cell in matrix, but return value was zero");

			well.matrix[y][x] = CELL_TYPE_NONE;
		}

		for (size_t i = 0; i < 4; i++) {
			// trick tetrimino_new() to avoid filling tetrimino bag
			well.tetrimino_bag_index = 7;
			well.tetrimino_bag[well.tetrimino_bag_index - 1] = 1; // Type-O tetrimino

			size_t x = test_coords_pass[i][0];
			size_t y = test_coords_pass[i][1];
			well.matrix[y][x] = CELL_TYPE_L;

			int ret = tetrimino_new(&well);
			assert_zero_msg(ret, "expected tetrimino_new() return zero since new tetrimino"
					" does not overlap with cell in matrix, but return value was non-zero");

			well.matrix[y][x] = CELL_TYPE_NONE;
		}
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_new_initialize_correct_coords_from_tetrimino_bag_test)
{
	const size_t expected_cell_init_coords[7][4][2] = {
			{{4, 0}, {4, 1}, {4, 2}, {4, 3}}, // type I
			{{4, 0}, {5, 0}, {4, 1}, {5, 1}}, // type O
			{{4, 0}, {5, 0}, {6, 0}, {5, 1}}, // type T
			{{5, 0}, {5, 1}, {6, 0}, {4, 1}}, // type S
			{{4, 0}, {5, 1}, {5, 0}, {6, 1}}, // type Z
			{{4, 0}, {5, 1}, {4, 1}, {6, 1}}, // type J
			{{4, 1}, {5, 1}, {6, 1}, {6, 0}}, // type L
	};

	int ret;
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	for (size_t i = 7; i > 0; i--)
		well.tetrimino_bag[i - 1] = 7 - i;

	TEST_START() {
		ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected tetrimino_new() return zero, but was %d", ret);
		assert_eq_msg(CELL_TYPE_I, well.tetrimino_type, "expected tetrimino type I");
		assert_true_msg(!memcmp(expected_cell_init_coords[0], well.tetrimino_coords, sizeof(size_t) * 4 * 2),
				"tetrimino coords were not initialized with expected default values for cell type I");

		ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected tetrimino_new() return zero, but was %d", ret);
		assert_eq_msg(CELL_TYPE_O, well.tetrimino_type, "expected tetrimino type O");
		assert_true_msg(!memcmp(expected_cell_init_coords[1], well.tetrimino_coords, sizeof(size_t) * 4 * 2),
				"tetrimino coords were not initialized with expected default values for cell type O");

		ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected tetrimino_new() return zero, but was %d", ret);
		assert_eq_msg(CELL_TYPE_T, well.tetrimino_type, "expected tetrimino type T");
		assert_true_msg(!memcmp(expected_cell_init_coords[2], well.tetrimino_coords, sizeof(size_t) * 4 * 2),
				"tetrimino coords were not initialized with expected default values for cell type T");

		ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected tetrimino_new() return zero, but was %d", ret);
		assert_eq_msg(CELL_TYPE_S, well.tetrimino_type, "expected tetrimino type S");
		assert_true_msg(!memcmp(expected_cell_init_coords[3], well.tetrimino_coords, sizeof(size_t) * 4 * 2),
				"tetrimino coords were not initialized with expected default values for cell type S");

		ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected tetrimino_new() return zero, but was %d", ret);
		assert_eq_msg(CELL_TYPE_Z, well.tetrimino_type, "expected tetrimino type Z");
		assert_true_msg(!memcmp(expected_cell_init_coords[4], well.tetrimino_coords, sizeof(size_t) * 4 * 2),
				"tetrimino coords were not initialized with expected default values for cell type Z");

		ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected tetrimino_new() return zero, but was %d", ret);
		assert_eq_msg(CELL_TYPE_J, well.tetrimino_type, "expected tetrimino type J");
		assert_true_msg(!memcmp(expected_cell_init_coords[5], well.tetrimino_coords, sizeof(size_t) * 4 * 2),
				"tetrimino coords were not initialized with expected default values for cell type J");

		ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected tetrimino_new() return zero, but was %d", ret);
		assert_eq_msg(CELL_TYPE_L, well.tetrimino_type, "expected tetrimino type L");
		assert_true_msg(!memcmp(expected_cell_init_coords[6], well.tetrimino_coords, sizeof(size_t) * 4 * 2),
				"tetrimino coords were not initialized with expected default values for cell type L");
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_shift_down_update_coords_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	well.tetrimino_bag[6] = 6;

	TEST_START() {
		int ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected return value of zero from tetrimino_new() but was %d", ret);

		for (size_t i = 0; i < 4; i++) {
			size_t initial_coords[4][2];
			memcpy(initial_coords, well.tetrimino_coords, sizeof(size_t) * 4 * 2);

			ret = tetrimino_shift(&well, SHIFT_DOWN);
			assert_zero_msg(ret, "expected tetrimino_shift(DOWN) return zero, but was %d", ret);

			for (size_t j = 0; j < 4; j++) {
				assert_eq_msg(initial_coords[j][0], well.tetrimino_coords[j][0],
						"expected x coordinate to remain but was permuted (expected %zu, actual %zu)",
						initial_coords[j][0], well.tetrimino_coords[j][0]);

				assert_eq_msg(initial_coords[j][1] + 1, well.tetrimino_coords[j][1],
						"expected y coordinate to shift down by one cell (expected %zu, actual %zu)",
						initial_coords[j][1] + 1, well.tetrimino_coords[j][1]);
			}
		}
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_shift_down_return_nonzero_not_legal_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	well.tetrimino_bag[6] = 6;

	TEST_START() {
		int ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected return value of zero from tetrimino_new() but was %d", ret);

		for (size_t i = 0; i < 4; i++) {
			size_t x = well.tetrimino_coords[i][0];
			size_t y = well.tetrimino_coords[i][1];

			well.matrix[y + 1][x] = CELL_TYPE_I;

			ret = tetrimino_shift(&well, SHIFT_DOWN);
			assert_eq_msg(-1, ret, "expected tetrimino_shift(DOWN) to fail "
					"since the current tetrimino overlaps with another cell in the matrix");
			assert_eq_msg(x, well.tetrimino_coords[i][0], "tetrimino coords "
					"changed unexpectedly; expected %zu but was %zu", x, well.tetrimino_coords[i][0]);
			assert_eq_msg(y, well.tetrimino_coords[i][1], "tetrimino coords "
					"changed unexpectedly; expected %zu but was %zu", y, well.tetrimino_coords[i][1]);

			well.matrix[y + 1][x] = CELL_TYPE_NONE;
		}

		// test boundary
		size_t boundary_coords[4][2] = {{5, 23}, {5, 22}, {5, 21}, {5, 20}};
		memcpy(well.tetrimino_coords, boundary_coords, sizeof(size_t) * 4 * 2);
		well.tetrimino_type = CELL_TYPE_I;

		ret = tetrimino_shift(&well, SHIFT_DOWN);
		assert_eq_msg(-1, ret, "expected tetrimino_shift(DOWN) to fail "
				"since the current tetrimino is at the bottom matrix boundary");
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_shift_left_update_coords_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	well.tetrimino_bag[6] = 6;

	TEST_START() {
		int ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected return value of zero from tetrimino_new() but was %d", ret);

		for (size_t i = 0; i < 4; i++) {
			size_t initial_coords[4][2];
			memcpy(initial_coords, well.tetrimino_coords, sizeof(size_t) * 4 * 2);

			ret = tetrimino_shift(&well, SHIFT_LEFT);
			assert_zero_msg(ret, "expected tetrimino_shift(LEFT) return zero, but was %d", ret);

			for (size_t j = 0; j < 4; j++) {
				assert_eq_msg(initial_coords[j][0] - 1, well.tetrimino_coords[j][0],
						"expected x coordinate to shift left by one cell (expected %zu, actual %zu)",
						initial_coords[j][0] - 1, well.tetrimino_coords[j][0]);

				assert_eq_msg(initial_coords[j][1], well.tetrimino_coords[j][1],
						"expected y coordinate to remain the same (expected %zu, actual %zu)",
						initial_coords[j][1], well.tetrimino_coords[j][1]);
			}
		}
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_shift_left_return_nonzero_not_legal_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	well.tetrimino_bag[6] = 6;

	TEST_START() {
		int ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected return value of zero from tetrimino_new() but was %d", ret);

		for (size_t i = 0; i < 4; i++) {
			size_t x = well.tetrimino_coords[i][0];
			size_t y = well.tetrimino_coords[i][1];

			well.matrix[y][x - 1] = CELL_TYPE_I;

			ret = tetrimino_shift(&well, SHIFT_LEFT);
			assert_eq_msg(1, ret, "expected tetrimino_shift(LEFT) to fail "
					"since the current tetrimino overlaps with another cell in the matrix");
			assert_eq_msg(x, well.tetrimino_coords[i][0], "tetrimino coords "
					"changed unexpectedly; expected %zu but was %zu", x, well.tetrimino_coords[i][0]);
			assert_eq_msg(y, well.tetrimino_coords[i][1], "tetrimino coords "
					"changed unexpectedly; expected %zu but was %zu", y, well.tetrimino_coords[i][1]);

			well.matrix[y][x - 1] = CELL_TYPE_NONE;
		}

		// test boundary
		size_t boundary_coords[4][2] = {{0, 22}, {1, 22}, {2, 22}, {3, 22}};
		memcpy(well.tetrimino_coords, boundary_coords, sizeof(size_t) * 4 * 2);
		well.tetrimino_type = CELL_TYPE_I;

		ret = tetrimino_shift(&well, SHIFT_LEFT);
		assert_eq_msg(1, ret, "expected tetrimino_shift(LEFT) to fail "
				"since the current tetrimino is at the left matrix boundary");
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_shift_right_update_coords_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	well.tetrimino_bag[6] = 6;

	TEST_START() {
		int ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected return value of zero from tetrimino_new() but was %d", ret);

		for (size_t i = 0; i < 3; i++) {
			size_t initial_coords[4][2];
			memcpy(initial_coords, well.tetrimino_coords, sizeof(size_t) * 4 * 2);

			ret = tetrimino_shift(&well, SHIFT_RIGHT);
			assert_zero_msg(ret, "expected tetrimino_shift(RIGHT) return zero, but was %d", ret);

			for (size_t j = 0; j < 4; j++) {
				assert_eq_msg(initial_coords[j][0] + 1, well.tetrimino_coords[j][0],
						"expected x coordinate to shift right by one cell (expected %zu, actual %zu)",
						initial_coords[j][0] + 1, well.tetrimino_coords[j][0]);

				assert_eq_msg(initial_coords[j][1], well.tetrimino_coords[j][1],
						"expected y coordinate to remain the same (expected %zu, actual %zu)",
						initial_coords[j][1], well.tetrimino_coords[j][1]);
			}
		}
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_shift_right_return_nonzero_not_legal_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	well.tetrimino_bag[6] = 6;

	TEST_START() {
		int ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected return value of zero from tetrimino_new() but was %d", ret);

		for (size_t i = 0; i < 4; i++) {
			size_t x = well.tetrimino_coords[i][0];
			size_t y = well.tetrimino_coords[i][1];

			well.matrix[y][x + 1] = CELL_TYPE_I;

			ret = tetrimino_shift(&well, SHIFT_RIGHT);
			assert_eq_msg(1, ret, "expected tetrimino_shift(RIGHT) to fail "
					"since the current tetrimino overlaps with another cell in the matrix");
			assert_eq_msg(x, well.tetrimino_coords[i][0], "tetrimino coords "
					"changed unexpectedly; expected %zu but was %zu", x, well.tetrimino_coords[i][0]);
			assert_eq_msg(y, well.tetrimino_coords[i][1], "tetrimino coords "
					"changed unexpectedly; expected %zu but was %zu", y, well.tetrimino_coords[i][1]);

			well.matrix[y][x + 1] = CELL_TYPE_NONE;
		}

		// test boundary
		size_t boundary_coords[4][2] = {{9, 22}, {8, 22}, {7, 22}, {6, 22}};
		memcpy(well.tetrimino_coords, boundary_coords, sizeof(size_t) * 4 * 2);
		well.tetrimino_type = CELL_TYPE_I;

		ret = tetrimino_shift(&well, SHIFT_RIGHT);
		assert_eq_msg(1, ret, "expected tetrimino_shift(RIGHT) to fail "
				"since the current tetrimino is at the left matrix boundary");
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_rotate_not_rotate_O_type_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	well.tetrimino_bag[6] = 1; // type O

	TEST_START() {
		int ret = tetrimino_new(&well);
		assert_zero_msg(ret, "expected return value of zero from tetrimino_new() but was %d", ret);

		for (size_t i = 0; i < 4; i++) {
			size_t x = well.tetrimino_coords[i][0];
			size_t y = well.tetrimino_coords[i][1];

			ret = tetrimino_rotate(&well);
			assert_eq_msg(0, ret, "tetrimino_rotate() with tetrimino type O should always succeed but returned %d", ret);
			assert_eq_msg(x, well.tetrimino_coords[i][0], "expected x coordinate to remain the same (expected %zu, actual %zu)", x, well.tetrimino_coords[i][0]);
			assert_eq_msg(y, well.tetrimino_coords[i][1], "expected x coordinate to remain the same (expected %zu, actual %zu)", y, well.tetrimino_coords[i][1]);
		}
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_rotate_correctly_rotate_tetrimino_test)
{
	const size_t tetrimino_coords_rotation_values[7][5][4][2] = {
			{
				/* type I */
				{{9, 2}, {9, 3}, {9, 4}, {9, 5}},
				{{9, 3}, {8, 3}, {7, 3}, {6, 3}},
				{{8, 4}, {8, 3}, {8, 2}, {8, 1}},
				{{6, 3}, {7, 3}, {8, 3}, {9, 3}},
				{{7, 2}, {7, 3}, {7, 4}, {7, 5}},
			}, {
				/* type O */
				{{4, 0}, {5, 0}, {4, 1}, {5, 1}},
				{{4, 0}, {5, 0}, {4, 1}, {5, 1}},
				{{4, 0}, {5, 0}, {4, 1}, {5, 1}},
				{{4, 0}, {5, 0}, {4, 1}, {5, 1}},
				{{4, 0}, {5, 0}, {4, 1}, {5, 1}},
			}, {
				/* type T */
				{{0, 2}, {0, 3}, {0, 4}, {1, 3}},
				{{2, 3}, {1, 3}, {0, 3}, {1, 4}},
				{{1, 4}, {1, 3}, {1, 2}, {0, 3}},
				{{0, 3}, {1, 3}, {2, 3}, {1, 2}},
				{{1, 2}, {1, 3}, {1, 4}, {2, 3}},
			}, {
				/* type S */
				{{5, 10}, {5, 11}, {6, 10}, {4, 11}},
				{{6, 11}, {5, 11}, {6, 12}, {5, 10}},
				{{5, 12}, {5, 11}, {4, 12}, {6, 11}},
				{{4, 11}, {5, 11}, {4, 10}, {5, 12}},
				{{5, 10}, {5, 11}, {6, 10}, {4, 11}},
			}, {
				/* type Z */
				{{4, 10}, {5, 11}, {5, 10}, {6, 11}},
				{{6, 10}, {5, 11}, {6, 11}, {5, 12}},
				{{6, 12}, {5, 11}, {5, 12}, {4, 11}},
				{{4, 12}, {5, 11}, {4, 11}, {5, 10}},
				{{4, 10}, {5, 11}, {5, 10}, {6, 11}},
			}, {
				/* type J */
				{{4, 22}, {5, 23}, {4, 23}, {6, 23}},
				{{6, 21}, {5, 22}, {5, 21}, {5, 23}},
				{{6, 23}, {5, 22}, {6, 22}, {4, 22}},
				{{4, 23}, {5, 22}, {5, 23}, {5, 21}},
				{{4, 21}, {5, 22}, {4, 22}, {6, 22}},
			}, {
				/* type L */
				{{4, 11}, {5, 11}, {6, 11}, {6, 10}},
				{{5, 10}, {5, 11}, {5, 12}, {6, 12}},
				{{6, 11}, {5, 11}, {4, 11}, {4, 12}},
				{{5, 12}, {5, 11}, {5, 10}, {4, 10}},
				{{4, 11}, {5, 11}, {6, 11}, {6, 10}},
			}
	};

	struct tetris_well well;
	tetris_well_init(&well);

	TEST_START() {
		for (size_t i = 0; i < 7; i++) {
			well.tetrimino_type = (unsigned)1 << i;

			memcpy(well.tetrimino_coords, tetrimino_coords_rotation_values[i][0],
					sizeof(size_t) * 4 * 2);

			for (size_t j = 1; j < 5; j++) {
				int ret = tetrimino_rotate(&well);
				assert_zero_msg(ret, "expected rotation to succeed, but tetrimino_rotate() returned non-zero %d", ret);
				assert_true_msg(!memcmp(well.tetrimino_coords, tetrimino_coords_rotation_values[i][j], sizeof(size_t) * 4 * 2),
						"rotation of coordinates did not match expected (i = %zu, j = %zu)", i, j);
			}
		}
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_rotate_return_nonzero_if_not_legal_test)
{
	const size_t rotated_coords[5][4][2] = {
			{{5, 10}, {5, 11}, {6, 10}, {4, 11}},
			{{6, 11}, {5, 11}, {6, 12}, {5, 10}},
			{{5, 12}, {5, 11}, {4, 12}, {6, 11}},
			{{4, 11}, {5, 11}, {4, 10}, {5, 12}},
			{{5, 10}, {5, 11}, {6, 10}, {4, 11}},
	};

	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_type = CELL_TYPE_S;

	TEST_START() {
		for (size_t i = 0; i < 4; i++) {
			memcpy(well.tetrimino_coords, rotated_coords[i],sizeof(size_t) * 4 * 2);

			size_t x = rotated_coords[i + 1][3][0];
			size_t y = rotated_coords[i + 1][3][1];
			well.matrix[y][x] = CELL_TYPE_O;

			int ret = tetrimino_rotate(&well);
			assert_nonzero_msg(ret, "expected tetrimino_rotate() to fail, given that the "
					"rotated tetrimino overlaps with another cell in the matrix, but returned zero");
			assert_true_msg(!memcmp(well.tetrimino_coords, rotated_coords[i],sizeof(size_t) * 4 * 2),
					"tetrimino_rotate() returned nonzero, but coordinates were updated");
		}
	}

	TEST_END();
}

TEST_DEFINE(tetrimino_commit_collapse_rows_test)
{
	struct tetris_well well;
	tetris_well_init(&well);

	well.tetrimino_bag_index = 7;
	well.tetrimino_bag[6] = 2; // type T

	TEST_START() {
		int ret = tetrimino_new(&well);
		assert_zero_msg(ret, "tetrimino_new should have succeeded with a zero return value, but returned %d", ret);

		ret = tetrimino_shift(&well, SHIFT_DOWN);
		assert_zero_msg(ret, "tetrimino_shift should have succeeded with a return value of 0, but returned %d", ret);
		ret = tetrimino_shift(&well, SHIFT_DOWN);
		assert_zero_msg(ret, "tetrimino_shift should have succeeded with a return value of 0, but returned %d", ret);
		ret = tetrimino_shift(&well, SHIFT_DOWN);
		assert_zero_msg(ret, "tetrimino_shift should have succeeded with a return value of 0, but returned %d", ret);

		size_t tetrimino_coords[4][2];
		memcpy(tetrimino_coords, well.tetrimino_coords, sizeof(size_t) * 4 * 2);

		size_t coord1[2] = {6, 15};
		well.matrix[coord1[1]][coord1[0]] = CELL_TYPE_S;

		size_t coord2[2] = {4, 16};
		well.matrix[coord2[1]][coord2[0]] = CELL_TYPE_L;

		for (size_t i = 0; i < BOARD_WIDTH; i++)
			well.matrix[20][i] = CELL_TYPE_O;

		ret = tetris_well_commit_tetrimino(&well);
		assert_eq_msg(1, ret, "expected tetris_well_commit_tetrimino() return 1, "
				"since a single line should have been collapsed, but returned %d", ret);

		for (size_t i = 0; i < BOARD_WIDTH; i++)
			assert_eq_msg(CELL_TYPE_NONE, well.matrix[20][i], "expected cell [%zu, %zu] to be empty", 20, i);

		assert_eq_msg(CELL_TYPE_NONE, well.matrix[coord1[1]][coord1[0]],
				"expected cell [%zu, %zu] to be empty", coord1[0], coord1[1]);
		assert_eq_msg(CELL_TYPE_NONE, well.matrix[coord2[1]][coord2[0]],
				"expected cell [%zu, %zu] to be empty", coord2[0], coord2[1]);

		assert_eq_msg(CELL_TYPE_S, well.matrix[coord1[1] + 1][coord1[0]],
				"expected cell [%zu, %zu] to be filled", coord1[0], coord1[1] - 1);
		assert_eq_msg(CELL_TYPE_L, well.matrix[coord2[1] + 1][coord2[0]],
				"expected cell [%zu, %zu] to be filled", coord2[0], coord2[1] - 1);

		for (size_t i =  0; i < 4; i++) {
			assert_eq_msg(CELL_TYPE_T, well.matrix[tetrimino_coords[i][1] + 1][tetrimino_coords[i][0]],
					"expected cell [%zu, %zu] to have cell type T",
					tetrimino_coords[i][1] + 1, tetrimino_coords[i][0]);
		}
	}

	TEST_END();
}

int tetris_well_test(struct test_runner_instance *instance)
{
	struct unit_test tests[] = {
			{ "tetris_well_init should correctly initialize playing field", tetris_well_init_test },
			{ "tetrimino_new should update the tetrimino_bag", tetrimino_new_should_update_tetrimino_bag_test },
			{ "tetrimino_new should return non-zero if the new tetrimino overlaps with another in the matrix", tetrimino_new_return_if_overlapping_test },
			{ "tetrimino_new should correctly initialize tetrimino coordinates from the tetrimino bag", tetrimino_new_initialize_correct_coords_from_tetrimino_bag_test },
			{ "tetrimino_shift with direction down should update coords if move is legal", tetrimino_shift_down_update_coords_test },
			{ "tetrimino_shift with direction down should return -1 and not update coords if not legal", tetrimino_shift_down_return_nonzero_not_legal_test },
			{ "tetrimino_shift with direction left should update coords if move is legal", tetrimino_shift_left_update_coords_test },
			{ "tetrimino_shift with direction left should return 1 and not update coords if not legal", tetrimino_shift_left_return_nonzero_not_legal_test },
			{ "tetrimino_shift with direction right should update coords if move is legal", tetrimino_shift_right_update_coords_test },
			{ "tetrimino_shift with direction right should return 1 and not update coords if not legal", tetrimino_shift_right_return_nonzero_not_legal_test },
			{ "tetrimino_rotate should not rotate tetrimino if type O", tetrimino_rotate_not_rotate_O_type_test },
			{ "tetrimino_rotate should correctly rotate all tetrimino types", tetrimino_rotate_correctly_rotate_tetrimino_test },
			{ "tetrimino_rotate should return nonzero and not update coords if rotation not possible", tetrimino_rotate_return_nonzero_if_not_legal_test },
			{ "tetrimino_commit should collapse and shift rows that have been filled", tetrimino_commit_collapse_rows_test },
			{ NULL, NULL }
	};

	return execute_tests(instance, tests);
}
