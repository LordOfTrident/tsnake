#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#define CTRL(key) (key & 31)

#define FPS 60

#define MAP_WIDTH  40
#define MAP_HEIGHT 30

#define SNAKE_MAX_LEN 256
#define SNAKE_SPEED   6
#define SNAKE_HEAD    0

#define APPLES_MAX_AMOUNT (MAP_WIDTH * MAP_HEIGHT)
#define APPLE_SPAWN_SPEED 70

#define LOST_SCREEN_TEXT "You have lost!"

int win_x = 0, win_y = 0;

WINDOW* map_win = NULL;
int map_width = MAP_WIDTH, map_height = MAP_HEIGHT;

enum color_pairs {
	COLOR_PAIR_APPLE = 1,
	COLOR_PAIR_SNAKE,
	COLOR_PAIR_BG
};

typedef enum {
	GAME_STATE_RUNNING = 0,
	GAME_STATE_LOST
} game_state_t;

typedef enum {
	DIR_UP,
	DIR_LEFT,
	DIR_DOWN,
	DIR_RIGHT
} dir_t;

typedef struct {
	int x, y;
} vec2_t;

vec2_t snake[SNAKE_MAX_LEN];
size_t snake_len = 1;
dir_t  snake_dir = DIR_DOWN;

vec2_t apples[APPLES_MAX_AMOUNT];
size_t apples_amount = 0;

size_t tick = 0;
bool   quit = false;
game_state_t game_state = GAME_STATE_RUNNING;

void snake_change_dir(dir_t p_dir) {
	if (p_dir - 2 == snake_dir || p_dir + 2 == snake_dir)
		return;

	snake_dir = p_dir;
}

void init(void) {
	initscr();

	noecho();
	raw();
	curs_set(0);
	keypad(stdscr, true);
	nodelay(stdscr, true);

	getmaxyx(stdscr, win_y, win_x);

	start_color();
	use_default_colors();

	init_pair(COLOR_PAIR_APPLE, COLOR_RED,   COLOR_BLACK);
	init_pair(COLOR_PAIR_SNAKE, COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_PAIR_BG,    COLOR_WHITE, COLOR_BLACK);

	snake[SNAKE_HEAD] = (vec2_t){map_width / 2, map_height / 3};

	if (map_width > win_x)
		map_width = win_x;

	if (map_height > win_y)
		map_height = win_y;

	map_win = newwin(map_height, map_width, win_y / 2 - map_height / 2, win_x / 2 - map_width / 2);
	refresh();
}

void finish(void) {
	delwin(map_win);

	endwin();
}

void render(void) {
	wattron(map_win, COLOR_PAIR(COLOR_PAIR_BG));

	wbkgd(map_win, COLOR_PAIR(COLOR_PAIR_BG));
	werase(map_win);
	wborder(map_win, 0, 0, 0, 0, 0, 0, 0, 0);

	char score[23];
	sprintf(score, "SCORE: %ld", snake_len - 1);

	mvwaddstr(map_win, 0, 1, score);

	wattron(map_win, COLOR_PAIR(COLOR_PAIR_APPLE));
	wattron(map_win, A_ALTCHARSET);
	for (size_t i = 0; i < apples_amount; ++ i)
		mvwaddch(map_win, apples[i].y, apples[i].x, ACS_BULLET);
	wattroff(map_win, A_ALTCHARSET);
	wattroff(map_win, COLOR_PAIR(COLOR_PAIR_APPLE));

	char head_ch = game_state == GAME_STATE_LOST? 'X' : 'O';

	wattron(map_win, COLOR_PAIR(COLOR_PAIR_SNAKE));
	for (size_t i = 0; i < snake_len; ++ i)
		mvwaddch(map_win, snake[i].y, snake[i].x, i == 0? head_ch : 'o');

	if (game_state == GAME_STATE_LOST)
		mvwaddch(map_win, snake[SNAKE_HEAD].y, snake[SNAKE_HEAD].x, head_ch);

	wattroff(map_win, COLOR_PAIR(COLOR_PAIR_SNAKE));

	if (game_state == GAME_STATE_LOST) {
		mvwaddstr(map_win, map_height / 2, map_width / 2 -
		          strlen(LOST_SCREEN_TEXT) / 2, LOST_SCREEN_TEXT);

		mvwaddstr(map_win, map_height / 2 + 1, map_width / 2 - strlen(score) / 2, score);
	}

	wrefresh(map_win);

	wattron(map_win, COLOR_PAIR(COLOR_PAIR_BG));
}

void input(void) {
	int input = getch();

	switch (input) {
	case CTRL('q'): quit = true; break;

	case KEY_UP:    snake_change_dir(DIR_UP);    break;
	case KEY_DOWN:  snake_change_dir(DIR_DOWN);  break;
	case KEY_LEFT:  snake_change_dir(DIR_LEFT);  break;
	case KEY_RIGHT: snake_change_dir(DIR_RIGHT); break;

	default: break;
	}
}

void update(void) {
	if (game_state == GAME_STATE_RUNNING) {
		if (tick % SNAKE_SPEED == 0) {
			for (size_t i = 0; i < apples_amount; ++ i) {
				if (snake[SNAKE_HEAD].x == apples[i].x && snake[SNAKE_HEAD].y == apples[i].y) {
					++ snake_len;
					-- apples_amount;

					for (size_t j = i; j < apples_amount; ++ j)
						apples[j] = apples[j + 1];
				}
			}

			for (size_t i = snake_len; i > 0; -- i)
				snake[i] = snake[i - 1];

			switch (snake_dir) {
			case DIR_UP:    -- snake[SNAKE_HEAD].y; break;
			case DIR_DOWN:  ++ snake[SNAKE_HEAD].y; break;
			case DIR_LEFT:  -- snake[SNAKE_HEAD].x; break;
			case DIR_RIGHT: ++ snake[SNAKE_HEAD].x; break;
			}

			for (size_t i = 1; i < snake_len; ++ i) {
				if (snake[SNAKE_HEAD].x == snake[i].x && snake[SNAKE_HEAD].y == snake[i].y) {
					game_state = GAME_STATE_LOST;

					return;
				}
			}

			if (
				snake[SNAKE_HEAD].x <= 0 || snake[SNAKE_HEAD].x >= map_width  - 1 ||
				snake[SNAKE_HEAD].y <= 0 || snake[SNAKE_HEAD].y >= map_height - 1
			) {
				game_state = GAME_STATE_LOST;

				return;
			}
		}

		if (tick % APPLE_SPAWN_SPEED == 0) {
			++ apples_amount;
			apples[apples_amount - 1].x = rand() % (map_width  - 2) + 1;
			apples[apples_amount - 1].y = rand() % (map_height - 2) + 1;
		}
	}
}

void game_loop(void) {
	while (!quit) {
		render();
		input();
		update();

		napms(1000 / FPS);
		++ tick;
	}
}

int main(void) {
	init();
	game_loop();
	finish();

	return 0;
}
