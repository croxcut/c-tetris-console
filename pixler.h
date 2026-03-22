#ifndef PIXLER_H
#define PIXLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define KEY_UP      72
#define KEY_DOWN    80
#define KEY_LEFT    75
#define KEY_RIGHT   77
#define KEY_ENTER   13
#define KEY_ESC     27
#define KEY_SPACE   32

#define BOARD_W     10
#define BOARD_H     20
#define BOARD_X     2
#define BOARD_Y     2

#define SCORE_SINGLE    100
#define SCORE_DOUBLE    300
#define SCORE_TRIPLE    500
#define SCORE_TETRIS    800
#define SCORE_SOFT_DROP 1
#define SCORE_HARD_DROP 2

#define INITIAL_SPEED   500
#define MIN_SPEED       50
#define SPEED_STEP      30

#define SAVE_FILE       "pixler_save.dat"
#define SCORES_FILE     "pixler_scores.csv"

#define NUM_PIECES      7

#define COLOR_BLACK     0
#define COLOR_BLUE      1
#define COLOR_GREEN     2
#define COLOR_CYAN      3
#define COLOR_RED       4
#define COLOR_MAGENTA   5
#define COLOR_YELLOW    6
#define COLOR_WHITE     7
#define COLOR_LGRAY     8
#define COLOR_LBLUE     9
#define COLOR_LGREEN    10
#define COLOR_LCYAN     11
#define COLOR_LRED      12
#define COLOR_LMAGENTA  13
#define COLOR_LYELLOW   14
#define COLOR_LWHITE    15

static const int PIECE_COLORS[NUM_PIECES] = {
    COLOR_LCYAN,
    COLOR_LYELLOW,
    COLOR_LMAGENTA,
    COLOR_LGREEN,
    COLOR_LRED,
    COLOR_LBLUE,
    COLOR_LRED
};

// All 7 pieces x 4 rotations x 4x4 grid
static const int PIECES[NUM_PIECES][4][4][4] = {
    // I
    {
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}
    },
    // O
    {
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}
    },
    // T
    {
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // S
    {
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
        {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // Z
    {
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}
    },
    // J
    {
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}
    },
    // L
    {
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}
    }
};

typedef struct {
    int board[BOARD_H][BOARD_W];
    int score;
    int level;
    int lines;
    int total_pieces;
    int current_piece;
    int current_rot;
    int current_x;
    int current_y;
    int next_piece;
    int hold_piece;
    int hold_used;
    char player_name[30];
    int game_over;
    int paused;
} GameState;

typedef struct {
    char name[30];
    int score;
    int level;
    int lines;
    char date[20];
} ScoreRecord;

void gotoxy(int x, int y);
void set_color(int fg, int bg);
void hide_cursor(void);
void show_cursor(void);
void clear_screen(void);

void draw_border(void);
void draw_board(GameState *gs);
void draw_piece(GameState *gs, int ghost);
void draw_next(GameState *gs);
void draw_hold(GameState *gs);
void draw_stats(GameState *gs);
void draw_ui(GameState *gs);
void draw_game_over(GameState *gs);
void draw_pause(void);

void init_game(GameState *gs, const char *name);
int  new_piece(GameState *gs);
int  check_collision(GameState *gs, int piece, int rot, int x, int y);
void lock_piece(GameState *gs);
int  clear_lines(GameState *gs);
void update_score(GameState *gs, int lines_cleared);
int  get_ghost_y(GameState *gs);
void hard_drop(GameState *gs);
void hold_piece_fn(GameState *gs);
void rotate_piece(GameState *gs, int dir);
void move_piece(GameState *gs, int dx, int dy);

int  main_menu(char *player_name);
void game_loop(GameState *gs);

int  save_game(GameState *gs);
int  load_game(GameState *gs);
int  save_score(GameState *gs);
void show_scores(void);

#endif // PIXLER_H
