#include "pixler.h"

void gotoxy(int x, int y) {
    COORD pos = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void set_color(int fg, int bg) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)((bg << 4) | fg));
}

void hide_cursor(void) {
    CONSOLE_CURSOR_INFO ci = {1, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}

void show_cursor(void) {
    CONSOLE_CURSOR_INFO ci = {1, TRUE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}

void clear_screen(void) {
    system("cls");
}

static void print_block(int color_idx) {
    if (color_idx == 0) {
        set_color(COLOR_LGRAY, COLOR_BLACK);
        printf("  ");
    } else {
        set_color(PIECE_COLORS[color_idx - 1], COLOR_BLACK);
        printf("[]");
    }
}

static void print_ghost_block(void) {
    set_color(COLOR_LGRAY, COLOR_BLACK);
    printf("::");
}

void draw_border(void) {
    int bx = BOARD_X - 1;
    int by = BOARD_Y - 1;
    int bw = BOARD_W * 2 + 2;
    int bh = BOARD_H + 2;

    set_color(COLOR_LWHITE, COLOR_BLACK);

    gotoxy(bx, by);
    printf("+");
    for (int i = 0; i < bw - 2; i++) printf("-");
    printf("+");

    for (int i = 1; i < bh - 1; i++) {
        gotoxy(bx, by + i);           printf("|");
        gotoxy(bx + bw - 1, by + i);  printf("|");
    }

    gotoxy(bx, by + bh - 1);
    printf("+");
    for (int i = 0; i < bw - 2; i++) printf("-");
    printf("+");

    int px = bx + bw + 1;   

    gotoxy(px, by);     printf("+-NEXT------+");
    for (int i = 1; i <= 6; i++) {
        gotoxy(px, by + i);
        printf("|           |");
    }
    gotoxy(px, by + 7); printf("+-----------+");

    gotoxy(px, by + 9);  printf("+-HOLD------+");
    for (int i = 1; i <= 6; i++) {
        gotoxy(px, by + 9 + i);
        printf("|           |");
    }
    gotoxy(px, by + 16); printf("+-----------+");

    gotoxy(px, by + 18); printf("+-STATS-----+");
    for (int i = 1; i <= 10; i++) {
        gotoxy(px, by + 18 + i);
        printf("|           |");
    }
    gotoxy(px, by + 29); printf("+-----------+");

    set_color(COLOR_LGRAY, COLOR_BLACK);
    gotoxy(px, by + 31); printf(" Arrows: Move ");
    gotoxy(px, by + 32); printf(" Up:     Rotate");
    gotoxy(px, by + 33); printf(" Space:  Drop  ");
    gotoxy(px, by + 34); printf(" C:      Hold  ");
    gotoxy(px, by + 35); printf(" P:      Pause ");
    gotoxy(px, by + 36); printf(" S:      Save  ");
    gotoxy(px, by + 37); printf(" Q:      Quit  ");
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

void draw_board(GameState *gs) {
    for (int row = 0; row < BOARD_H; row++) {
        gotoxy(BOARD_X, BOARD_Y + row);
        for (int col = 0; col < BOARD_W; col++)
            print_block(gs->board[row][col]);
    }
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

int get_ghost_y(GameState *gs) {
    int gy = gs->current_y;
    while (!check_collision(gs, gs->current_piece, gs->current_rot, gs->current_x, gy + 1))
        gy++;
    return gy;
}

void draw_piece(GameState *gs, int draw_ghost) {
    int ghost_y = get_ghost_y(gs);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (!PIECES[gs->current_piece][gs->current_rot][row][col]) continue;

            int bx = BOARD_X + (gs->current_x + col) * 2;
            int by = BOARD_Y + gs->current_y + row;
            int gy = BOARD_Y + ghost_y + row;

            if (draw_ghost && ghost_y != gs->current_y) {
                if (gy >= BOARD_Y && gy < BOARD_Y + BOARD_H) {
                    gotoxy(bx, gy);
                    print_ghost_block();
                }
            }

            if (by >= BOARD_Y && by < BOARD_Y + BOARD_H) {
                gotoxy(bx, by);
                set_color(PIECE_COLORS[gs->current_piece], COLOR_BLACK);
                printf("[]");
            }
        }
    }
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

static void draw_piece_preview(int piece, int px_panel, int py_panel) {
    for (int r = 0; r < 4; r++) {
        gotoxy(px_panel + 1, py_panel + 2 + r);
        printf("           ");  
    }

    if (piece < 0) return;

    int min_col = 4, max_col = -1, min_row = 4, max_row = -1;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (PIECES[piece][0][r][c]) {
                if (c < min_col) min_col = c;
                if (c > max_col) max_col = c;
                if (r < min_row) min_row = r;
                if (r > max_row) max_row = r;
            }
        }
    }

    int piece_w = (max_col - min_col + 1) * 2; 
    int piece_h =  max_row - min_row + 1;

    int x_off = px_panel + 1 + (11 - piece_w) / 2;
    int y_off = py_panel + 2 + (4 - piece_h) / 2;

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (PIECES[piece][0][r][c]) {
                gotoxy(x_off + (c - min_col) * 2, y_off + (r - min_row));
                set_color(PIECE_COLORS[piece], COLOR_BLACK);
                printf("[]");
            }
        }
    }
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

void draw_next(GameState *gs) {
    int px = (BOARD_X - 1) + (BOARD_W * 2 + 2) + 1;
    int py = BOARD_Y - 1;
    draw_piece_preview(gs->next_piece, px, py);
}

void draw_hold(GameState *gs) {
    int px = (BOARD_X - 1) + (BOARD_W * 2 + 2) + 1;
    int py = (BOARD_Y - 1) + 9;   

    if (gs->hold_used) set_color(COLOR_LGRAY, COLOR_BLACK);
    draw_piece_preview(gs->hold_piece, px, py);
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

void draw_stats(GameState *gs) {
    int by    = BOARD_Y - 1;
    int px    = (BOARD_X - 1) + (BOARD_W * 2 + 2) + 1;
    int py    = by + 18;

    set_color(COLOR_LWHITE,   COLOR_BLACK); gotoxy(px + 1, py + 1);  printf(" SCORE:");
    set_color(COLOR_LYELLOW,  COLOR_BLACK); gotoxy(px + 1, py + 2);  printf(" %-9d", gs->score);
    set_color(COLOR_LWHITE,   COLOR_BLACK); gotoxy(px + 1, py + 3);  printf(" LEVEL:");
    set_color(COLOR_LCYAN,    COLOR_BLACK); gotoxy(px + 1, py + 4);  printf(" %-9d", gs->level);
    set_color(COLOR_LWHITE,   COLOR_BLACK); gotoxy(px + 1, py + 5);  printf(" LINES:");
    set_color(COLOR_LGREEN,   COLOR_BLACK); gotoxy(px + 1, py + 6);  printf(" %-9d", gs->lines);
    set_color(COLOR_LWHITE,   COLOR_BLACK); gotoxy(px + 1, py + 7);  printf(" PIECES:");
    set_color(COLOR_LMAGENTA, COLOR_BLACK); gotoxy(px + 1, py + 8);  printf(" %-9d", gs->total_pieces);
    set_color(COLOR_LWHITE,   COLOR_BLACK); gotoxy(px + 1, py + 9);  printf("           ");
    set_color(COLOR_LWHITE,   COLOR_BLACK); gotoxy(px + 1, py + 10); printf("           ");
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

void draw_ui(GameState *gs) {
    draw_board(gs);
    draw_piece(gs, 1);
    draw_next(gs);
    draw_hold(gs);
    draw_stats(gs);
}

void draw_pause(void) {
    int cx = BOARD_X + BOARD_W - 5;
    int cy = BOARD_Y + BOARD_H / 2 - 1;
    set_color(COLOR_LYELLOW, COLOR_BLACK);
    gotoxy(cx, cy);     printf("           ");
    gotoxy(cx, cy + 1); printf("  PAUSED   ");
    gotoxy(cx, cy + 2); printf("           ");
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

void draw_game_over(GameState *gs) {
    int cx = BOARD_X + BOARD_W - 6;
    int cy = BOARD_Y + BOARD_H / 2 - 3;
    set_color(COLOR_LRED,    COLOR_BLACK);
    gotoxy(cx, cy);     printf("               ");
    gotoxy(cx, cy + 1); printf("   GAME  OVER  ");
    gotoxy(cx, cy + 2); printf("               ");
    set_color(COLOR_LWHITE,  COLOR_BLACK);
    gotoxy(cx, cy + 3); printf("  Score:%6d  ", gs->score);
    gotoxy(cx, cy + 4); printf("  Level:%6d  ", gs->level);
    gotoxy(cx, cy + 5); printf("  Lines:%6d  ", gs->lines);
    set_color(COLOR_LYELLOW, COLOR_BLACK);
    gotoxy(cx, cy + 7); printf("  [Q]uit [R]etry");
    set_color(COLOR_LWHITE,  COLOR_BLACK);
}

void init_game(GameState *gs, const char *name) {
    memset(gs->board, 0, sizeof(gs->board));
    gs->score = gs->lines = gs->total_pieces = gs->hold_used = gs->game_over = gs->paused = 0;
    gs->level = 1;
    gs->hold_piece = -1;
    strncpy(gs->player_name, name, 29);
    gs->player_name[29] = '\0';
    srand((unsigned int)time(NULL));
    gs->next_piece = rand() % NUM_PIECES;
    new_piece(gs);
}

int new_piece(GameState *gs) {
    gs->current_piece = gs->next_piece;
    gs->next_piece    = rand() % NUM_PIECES;
    gs->current_rot   = 0;
    gs->current_x     = BOARD_W / 2 - 2;
    gs->current_y     = 0;
    gs->hold_used     = 0;
    gs->total_pieces++;
    if (check_collision(gs, gs->current_piece, gs->current_rot, gs->current_x, gs->current_y)) {
        gs->game_over = 1;
        return 0;
    }
    return 1;
}

int check_collision(GameState *gs, int piece, int rot, int x, int y) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (!PIECES[piece][rot][row][col]) continue;
            int nx = x + col, ny = y + row;
            if (nx < 0 || nx >= BOARD_W) return 1;
            if (ny >= BOARD_H) return 1;
            if (ny >= 0 && gs->board[ny][nx]) return 1;
        }
    }
    return 0;
}

void lock_piece(GameState *gs) {
    int c = gs->current_piece + 1;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (!PIECES[gs->current_piece][gs->current_rot][row][col]) continue;
            int ny = gs->current_y + row, nx = gs->current_x + col;
            if (ny >= 0 && ny < BOARD_H && nx >= 0 && nx < BOARD_W)
                gs->board[ny][nx] = c;
        }
    }
}

int clear_lines(GameState *gs) {
    int cleared = 0;
    for (int row = BOARD_H - 1; row >= 0; row--) {
        int full = 1;
        for (int col = 0; col < BOARD_W; col++)
            if (!gs->board[row][col]) { full = 0; break; }
        if (full) {
            for (int r = row; r > 0; r--)
                memcpy(gs->board[r], gs->board[r - 1], sizeof(gs->board[0]));
            memset(gs->board[0], 0, sizeof(gs->board[0]));
            cleared++;
            row++;
        }
    }
    return cleared;
}

void update_score(GameState *gs, int lines_cleared) {
    int base[] = {0, SCORE_SINGLE, SCORE_DOUBLE, SCORE_TRIPLE, SCORE_TETRIS};
    if (lines_cleared >= 1 && lines_cleared <= 4)
        gs->score += base[lines_cleared] * gs->level;
    gs->lines += lines_cleared;
    int new_level = gs->lines / 10 + 1;
    if (new_level > gs->level) gs->level = new_level;
}

void rotate_piece(GameState *gs, int dir) {
    int new_rot = (gs->current_rot + dir + 4) % 4;
    int kicks[] = {0, -1, 1, -2, 2};
    for (int k = 0; k < 5; k++) {
        if (!check_collision(gs, gs->current_piece, new_rot, gs->current_x + kicks[k], gs->current_y)) {
            gs->current_rot = new_rot;
            gs->current_x  += kicks[k];
            return;
        }
    }
}

void move_piece(GameState *gs, int dx, int dy) {
    if (!check_collision(gs, gs->current_piece, gs->current_rot, gs->current_x + dx, gs->current_y + dy)) {
        gs->current_x += dx;
        gs->current_y += dy;
        if (dy > 0) gs->score += SCORE_SOFT_DROP * gs->level;
    }
}

void hard_drop(GameState *gs) {
    int dist = 0;
    while (!check_collision(gs, gs->current_piece, gs->current_rot, gs->current_x, gs->current_y + 1)) {
        gs->current_y++;
        dist++;
    }
    gs->score += dist * SCORE_HARD_DROP * gs->level;
    lock_piece(gs);
    update_score(gs, clear_lines(gs));
    new_piece(gs);
}

void hold_piece_fn(GameState *gs) {
    if (gs->hold_used) return;
    gs->hold_used = 1;
    int tmp = gs->hold_piece;
    gs->hold_piece = gs->current_piece;
    if (tmp == -1) {
        new_piece(gs);
    } else {
        gs->current_piece = tmp;
        gs->current_rot   = 0;
        gs->current_x     = BOARD_W / 2 - 2;
        gs->current_y     = 0;
        if (check_collision(gs, gs->current_piece, gs->current_rot, gs->current_x, gs->current_y))
            gs->game_over = 1;
    }
}

int save_game(GameState *gs) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return 0;
    fwrite(gs, sizeof(GameState), 1, f);
    fclose(f);
    return 1;
}

int load_game(GameState *gs) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return 0;
    int ok = (fread(gs, sizeof(GameState), 1, f) == 1);
    fclose(f);
    return ok;
}

int save_score(GameState *gs) {
    FILE *f = fopen(SCORES_FILE, "a");
    if (!f) return 0;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M", tm_info);
    fprintf(f, "%s,%d,%d,%d,%s\n", gs->player_name, gs->score, gs->level, gs->lines, date);
    fclose(f);
    return 1;
}

void show_scores(void) {
    FILE *f = fopen(SCORES_FILE, "r");
    int sx = 10, sy = 4;

    set_color(COLOR_LWHITE, COLOR_BLACK);
    gotoxy(sx, sy);     printf("+---HIGH  SCORES---+");
    gotoxy(sx, sy + 1); printf("| %-10s %5s |", "NAME", "SCORE");
    gotoxy(sx, sy + 2); printf("+------------------+");

    if (!f) {
        gotoxy(sx, sy + 3); printf("| No scores yet... |");
        gotoxy(sx, sy + 4); printf("+------------------+");
        gotoxy(sx, sy + 6);
        set_color(COLOR_LYELLOW, COLOR_BLACK);
        printf("  Press any key...");
        set_color(COLOR_LWHITE, COLOR_BLACK);
        _getch();
        return;
    }

    char line[128];
    int row = 0;
    while (fgets(line, sizeof(line), f) && row < 10) {
        char name[30]; int score, level, lines; char date[20];
        if (sscanf(line, "%29[^,],%d,%d,%d,%19[^\n]", name, &score, &level, &lines, date) == 5) {
            gotoxy(sx, sy + 3 + row);
            printf("| %-10.10s %5d |", name, score);
            row++;
        }
    }
    fclose(f);

    if (row == 0) { gotoxy(sx, sy + 3); printf("| No scores yet... |"); row = 1; }
    gotoxy(sx, sy + 3 + row); printf("+------------------+");
    gotoxy(sx, sy + 4 + row);
    set_color(COLOR_LYELLOW, COLOR_BLACK);
    printf("  Press any key...");
    set_color(COLOR_LWHITE, COLOR_BLACK);
    _getch();
}

static void draw_title(void) {
    set_color(COLOR_LCYAN, COLOR_BLACK);
    // gotoxy(6, 1); printf("  ####   ##  ##  ##  ##     ##  ##  ");
    // gotoxy(6, 2); printf("  ## ##  ##  ##  ##  ##     ##  ##  ");
    // gotoxy(6, 3); printf("  ####   ##    ##    ##       ##    ");
    // gotoxy(6, 4); printf("  ##     ##  ##  ##  ##     ##  ##  ");
    // gotoxy(6, 5); printf("  ##     ##  ##  ##  #####  ##  ##  ");
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

static void draw_main_menu_box(char *options[], int n, int selected, int has_save, char *player_name) {
    int mx = 14, my = 7, mh = n + 4;
    set_color(COLOR_LWHITE, COLOR_BLACK);
    gotoxy(mx, my); printf("+----[ MAIN MENU ]---+");
    for (int i = 0; i < mh - 2; i++) { gotoxy(mx, my + 1 + i); printf("|                    |"); }
    gotoxy(mx, my + mh - 1); printf("+--------------------+");

    for (int i = 0; i < n; i++) {
        gotoxy(mx + 2, my + 2 + i);
        if (i == selected) {
            set_color(COLOR_BLACK, COLOR_LWHITE);
        } else if (i == 1 && !has_save) {
            set_color(COLOR_LGRAY, COLOR_BLACK);
        } else {
            set_color(COLOR_LWHITE, COLOR_BLACK);
        }
        printf(" %-18s", options[i]);
    }

    gotoxy(mx + 1, my + 1);
    set_color(COLOR_LYELLOW, COLOR_BLACK);
    printf(" Player: %-11.11s", player_name[0] ? player_name : "???");
    set_color(COLOR_LWHITE, COLOR_BLACK);
}

static void input_name(char *name, int max_len) {
    int mx = 10, my = 18;
    set_color(COLOR_LWHITE, COLOR_BLACK);
    gotoxy(mx, my);     printf("+---[ Enter Name ]------+");
    gotoxy(mx, my + 1); printf("|                       |");
    gotoxy(mx, my + 2); printf("+------------------------+");

    int pos = strlen(name);
    show_cursor();
    while (1) {
        gotoxy(mx + 1, my + 1);
        set_color(COLOR_BLACK, COLOR_LWHITE);
        printf(" %-21.21s ", name);
        gotoxy(mx + 2 + pos, my + 1);
        int ch = _getch();
        if (ch == KEY_ENTER) break;
        else if (ch == 8 && pos > 0) { pos--; name[pos] = '\0'; }
        else if (ch >= 32 && ch <= 126 && pos < max_len - 1 && pos < 21) {
            name[pos++] = (char)ch;
            name[pos] = '\0';
        }
    }
    hide_cursor();
    for (int i = 0; i < 3; i++) { gotoxy(mx, my + i); printf("                           "); }
}

int main_menu(char *player_name) {
    char *options[] = { "New Game", "Load Game", "High Scores", "Enter Name", "Exit" };
    int n = 5, selected = 0;

    FILE *sf = fopen(SAVE_FILE, "rb");
    int has_save = (sf != NULL);
    if (sf) fclose(sf);

    if (!player_name[0]) strcpy(player_name, "Player");

    while (1) {
        draw_title();
        draw_main_menu_box(options, n, selected, has_save, player_name);
        set_color(COLOR_LGRAY, COLOR_BLACK);
        gotoxy(10, 23);
        printf("  Use arrows to navigate, ENTER to select  ");
        set_color(COLOR_LWHITE, COLOR_BLACK);

        int ch = _getch();
        if (ch == 0 || ch == 224) {
            ch = _getch();
            if (ch == KEY_UP)   { selected--; if (selected < 0) selected = n - 1; }
            if (ch == KEY_DOWN) { selected++; if (selected >= n) selected = 0; }
        } else if (ch == KEY_ENTER) {
            if (selected == 0) return 0;
            if (selected == 1 && has_save) return 1;
            if (selected == 2) { clear_screen(); show_scores(); clear_screen(); }
            if (selected == 3) input_name(player_name, 30);
            if (selected == 4) return -1;
            sf = fopen(SAVE_FILE, "rb");
            has_save = (sf != NULL);
            if (sf) fclose(sf);
        }
    }
}

void game_loop(GameState *gs) {
    int speed = INITIAL_SPEED - (gs->level - 1) * SPEED_STEP;
    if (speed < MIN_SPEED) speed = MIN_SPEED;

    DWORD last_tick = GetTickCount();
    draw_border();
    draw_ui(gs);

    while (!gs->game_over) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 0 || ch == 224) {
                ch = _getch();
                if (ch == KEY_LEFT)  move_piece(gs, -1, 0);
                if (ch == KEY_RIGHT) move_piece(gs,  1, 0);
                if (ch == KEY_DOWN)  move_piece(gs,  0, 1);
                if (ch == KEY_UP)    rotate_piece(gs, 1);
            } else {
                switch (ch) {
                    case KEY_SPACE:          hard_drop(gs);            break;
                    case 'z': case 'Z':      rotate_piece(gs, -1);     break;
                    case 'x': case 'X':      rotate_piece(gs,  1);     break;
                    case 'c': case 'C':      hold_piece_fn(gs);        break;
                    case 'p': case 'P':
                        gs->paused = !gs->paused;
                        if (gs->paused) draw_pause();
                        break;
                    case 's': case 'S':
                        if (save_game(gs)) {
                            int sx = BOARD_X, sy = BOARD_Y + BOARD_H + 1;
                            set_color(COLOR_LGREEN, COLOR_BLACK);
                            gotoxy(sx, sy); printf("  Game Saved!   ");
                            Sleep(800);
                            gotoxy(sx, sy); printf("                ");
                            set_color(COLOR_LWHITE, COLOR_BLACK);
                        }
                        break;
                    case 'q': case 'Q': case KEY_ESC:
                        save_score(gs);
                        return;
                }
            }
        }

        if (gs->paused) { Sleep(50); continue; }

        DWORD now = GetTickCount();
        if ((int)(now - last_tick) >= speed) {
            last_tick = now;
            if (check_collision(gs, gs->current_piece, gs->current_rot, gs->current_x, gs->current_y + 1)) {
                lock_piece(gs);
                update_score(gs, clear_lines(gs));
                if (!new_piece(gs)) break;
                speed = INITIAL_SPEED - (gs->level - 1) * SPEED_STEP;
                if (speed < MIN_SPEED) speed = MIN_SPEED;
            } else {
                gs->current_y++;
            }
        }

        draw_ui(gs);
        Sleep(10);
    }

    draw_ui(gs);
    draw_game_over(gs);
    save_score(gs);
    remove(SAVE_FILE);

    while (1) {
        int ch = _getch();
        if (ch == 'q' || ch == 'Q' || ch == KEY_ESC) return;
        if (ch == 'r' || ch == 'R') {
            init_game(gs, gs->player_name);
            draw_border();
            game_loop(gs);
            return;
        }
    }
}

int main(void) {
    SetConsoleTitle("Pixler");
    system("mode con: cols=60 lines=40");
    hide_cursor();
    clear_screen();
    set_color(COLOR_LWHITE, COLOR_BLACK);

    char player_name[30] = "";
    GameState gs;

    while (1) {
        clear_screen();
        int choice = main_menu(player_name);
        if (choice == -1) break;

        clear_screen();
        if (choice == 0) {
            init_game(&gs, player_name);
        } else if (choice == 1) {
            if (!load_game(&gs))
                init_game(&gs, player_name);
            else
                strncpy(player_name, gs.player_name, 29);
        }
        game_loop(&gs);
    }

    clear_screen();
    show_cursor();
    set_color(COLOR_LWHITE, COLOR_BLACK);
    gotoxy(0, 0);
    printf("Thanks for playing Pixler! :D* (i hate my life decisions)\n");
    return 0;
}
