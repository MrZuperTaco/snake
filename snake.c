#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include "circular.h"

#define WINDOW_MIN_WIDTH 28
#define WINDOW_MIN_HEIGHT 7

#define WIDTH 28
#define HEIGHT 7

#define CHAR_WALL  '#'
#define CHAR_SPACE ' '
#define CHAR_SNAKE 'O'
#define CHAR_DEAD  'X'
#define CHAR_FOOD  'F'

const char *STR_GAME_OVER = " G A M E  O V E R ";
const char *STR_RETRY 	  = " press any key to retry ";
const char *STR_QUIT 	  = " or press \'q\' to quit ";

typedef struct {
	char *buffer;
	uint32_t width;
	uint32_t height;
} Board;

typedef enum {
	NONE,
	UP,
	DOWN,
	LEFT,
	RIGHT
} SnakeDirection;

typedef struct {
	CircularUInt32 *body_indices;
	SnakeDirection direction;
	uint32_t i;
	uint32_t j;
} Snake;

uint32_t board_index_of(Board *board, uint32_t i, uint32_t j) {
	return i * board->width + j;
}

void board_set_char(Board *board, uint32_t i, uint32_t j, char chr) {
	board->buffer[board_index_of(board, i, j)] = chr;
}

int32_t board_is_wall(Board *board, uint32_t i, uint32_t j) {
	return ((i == 0 || i == board->height - 1) && j != board->width - 1) // top or bottom
		|| ((j == 0 || j == board->width - 2)); // left or right
}

void board_init(Board *board) {
	for (uint32_t i = 0; i < board->height; ++i) {
		for (uint32_t j = 0; j < board->width; ++j) {
			if (board_is_wall(board, i, j)) {
				board_set_char(board, i, j, CHAR_WALL);
			}
			else if (j == board->width - 1) {
				board_set_char(board, i, j, '\n');
			}
			else {
				board_set_char(board, i, j, CHAR_SPACE);
			}
		}
	}
	board_set_char(board, board->height - 1, board->width, '\0'); // null term so we can print later
}

uint32_t board_random_index(Board *board) {
	uint32_t i = (rand() % (board->height - 2)) + 1; // two vertical walls
	uint32_t j = (rand() % (board->width - 3)) + 1; // two horizontal walls + wall of newlines
	return board_index_of(board, i, j);
}

uint32_t board_random_food_index(Board *board) {
	uint32_t food_index = board_random_index(board);

	while (board->buffer[food_index] != ' ') {
		food_index = board_random_index(board);
	}

	return food_index;
}

void board_game_over(Board *board) {
	uint32_t i = board->height / 2 - 1;
	uint32_t j = board->width / 2 - 10;
	uint32_t index_game_over = board_index_of(board, i, j);
	i += 1;
	j -= 3; 
	uint32_t index_retry = board_index_of(board, i, j);
	i += 1;
	j += 1;
	uint32_t index_quit = board_index_of(board, i, j);

	memcpy(board->buffer + index_game_over, STR_GAME_OVER, 18);
	memcpy(board->buffer + index_retry, STR_RETRY, 24);
	memcpy(board->buffer + index_quit, STR_QUIT, 22);
}

void render(Board *board, uint64_t score) {
	clear();
	printw("%s\n", board->buffer);
	printw(" Score: %ld", score);
}

int main() {
	WINDOW *ncurses_window = initscr();

	if (ncurses_window == NULL) {
		printf("error initializing curses\n");
		return 1;
	}

	curs_set(0);
	keypad(stdscr, TRUE);	
	timeout(-1);
	srand(time(NULL));
	uint32_t height, width;
	getmaxyx(ncurses_window, height, width);
	
	if (height < WINDOW_MIN_HEIGHT || width < WINDOW_MIN_WIDTH) {
		endwin();
		printf("error initializing snake: window too small\n");
		return 1;
	}

	char buffer[(height - 2) * width + 1];

	Board board = {
		.buffer = buffer,
		.width = width,
		.height = height - 2,
	};

	board_init(&board);
	CircularUInt32 *body_indices = circular_uint32_create(1);

	if (body_indices == NULL) {
		endwin();
		printf("error initializing snake: failed to create snake body\n");
		return 1;
	}

	Snake snake = {
		.body_indices = body_indices,
		.direction = NONE,
		.i = board.height / 2,
		.j = board.width / 2,
	};

	board_set_char(&board, snake.i, snake.j, 'O');
	board.buffer[board_random_food_index(&board)] = CHAR_FOOD;
	uint64_t score = 0;
	render(&board, score);

	while (1) {
		if (snake.direction != NONE) {
			timeout(50);
		}

		int key = getch();

		// interpret user input for snake direction
		if ((key == 'W' || key == 'w' || key == KEY_UP) && snake.direction != DOWN) {
			snake.direction = UP;
		}
		else if ((key == 'S' || key == 's' || key == KEY_DOWN) && snake.direction != UP) {
			snake.direction = DOWN;
		}
		else if ((key == 'A' || key == 'a' || key == KEY_LEFT) && snake.direction != RIGHT) {
			snake.direction = LEFT;
		}
		else if ((key == 'D' || key == 'd' || key == KEY_RIGHT) && snake.direction != LEFT) {
			snake.direction = RIGHT;
		}
		else if (key == 'Q' || key == 'q') { // exit game
			break;
		}

		if (snake.direction == NONE) {
			continue;
		}

		if (circular_uint32_push_head(&snake.body_indices, board_index_of(&board, snake.i, snake.j))) {
			circular_uint32_destroy(snake.body_indices);
			endwin();
			printf("error updating snake body\n");
			return 1;
		}

		// update snake position
		if (snake.direction == UP) {
			snake.i--;
		}
		else if (snake.direction == DOWN) {
			snake.i++;
		}
		else if (snake.direction == LEFT) {
			snake.j--;
		}
		else if (snake.direction == RIGHT) {
			snake.j++;
		}

		uint8_t dead = 0;
		char chr_board = board.buffer[board_index_of(&board, snake.i, snake.j)];

		if (chr_board == CHAR_SPACE) {
			uint32_t last_index = circular_uint32_pop_tail(snake.body_indices);
			board.buffer[last_index] = ' ';
		}
		if (chr_board == CHAR_FOOD) {
			score += 5 * circular_uint32_length(snake.body_indices);
			board.buffer[board_random_food_index(&board)] = CHAR_FOOD;
		}
		if (chr_board == CHAR_SPACE || chr_board == CHAR_FOOD) {
			board_set_char(&board, snake.i, snake.j, CHAR_SNAKE);
		}
		else {
			board_set_char(&board, snake.i, snake.j, CHAR_DEAD);
			dead = 1;
		}	

		render(&board, score);

		if (!dead) {
			continue;
		}

		// snake is dead, ask player if they want to play again or quit
		refresh();
		sleep(1);
		board_game_over(&board);
		render(&board, score);

		timeout(-1);
		key = getch();

		if (key != 'Q' && key != 'q') {
			circular_uint32_clear(snake.body_indices);
			snake.direction = NONE;
			snake.i = board.height / 2;
			snake.j = board.width / 2;
			board_init(&board);
			board_set_char(&board, snake.i, snake.j, 'O');
			board.buffer[board_random_food_index(&board)] = CHAR_FOOD;
			score = 0;
			render(&board, score);
		}
		else {
			break;
		}
	}

	circular_uint32_destroy(snake.body_indices);
	endwin();

	return 0;
}
