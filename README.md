# Pixler Console Falling Block Game

A console-based falling block puzzle game for Windows, written in C using the Win32 API.
Implements standard falling block gameplay rules, scoring, piece rotation, ghost piece,
hold queue, save/load, and a CSV high-score system.

> **Note on the Pixl Engine**
>
> Pixler was originally intended to be built on top of the
> **[Pixl Engine](https://github.com/croxcut/Pixl-Source-tree)** a custom engine
> developed alongside this project. However, due to the current nature and stage of
> the Pixl engine, it was not feasible to use it as the foundation for this game at
> this time. As a result, Pixler runs directly on the Windows console via the Win32 API
> as a standalone implementation. The intention is to eventually port or integrate Pixler
> into the Pixl engine once it matures enough to support this kind of project.

---

> **Disclaimer**
>
> Pixler is an independent hobby project a falling block puzzle game built for
> educational and personal purposes. This project is not affiliated with, endorsed by,
> or sponsored by The Tetris Company or Tetris Holding, LLC.
> Tetris® is a registered trademark of Tetris Holding, LLC.
> No commercial use of this software is intended or permitted.

---

## Table of Contents

- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Building](#building)
- [Running](#running)
- [Controls](#controls)
- [Gameplay Rules](#gameplay-rules)
- [Save and Load System](#save-and-load-system)
- [High Score CSV](#high-score-csv)
- [Code Architecture](#code-architecture)
- [Win32 API Usage](#win32-api-usage)
- [Known Limitations](#known-limitations)
- [References](#references)

---

## Project Structure

```
pixler/
├── pixler.h            // Constants, structs, and function declarations
├── pixler.c            // Full game implementation and entry point
├── Makefile            // GCC build rules
├── pixler_save.dat     // Binary save file (created at runtime)
└── pixler_scores.csv   // Append-only score log (created at runtime)
```

---

## Requirements

- **OS:** Windows (7, 10, or 11)
- **Compiler:** GCC via [MinGW-w64](https://www.mingw-w64.org/) or [MSYS2](https://www.msys2.org/)
- **Make:** GNU Make (ships with MinGW/MSYS2)
- **Terminal:** Windows Command Prompt or Windows Terminal (not WSL)

Install MinGW-w64 via MSYS2:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
```

---

## Building

```bash
make
```

This produces `pixler.exe`.

```bash
make run          # Build and launch immediately
make clean        # Remove pixler.o and pixler.exe
make clean-saves  # Remove pixler_save.dat and pixler_scores.csv
```

| Flag | Purpose |
|------|---------|
| `-O2` | Speed optimization |
| `-Wall` | Enable all common warnings |
| `-std=c11` | C11 standard compliance |
| `-mconsole` | Windows console subsystem |
| `-lkernel32 -luser32` | Link Win32 libraries |

---

## Running

```bash
pixler.exe
# or:
make run
```

The console auto-resizes to 60×38 on startup. If your terminal clips the display,
increase its window size in Settings → Default Terminal → Layout.

---

## Controls

| Key | Action |
|-----|--------|
| `←` `→` | Move piece left / right |
| `↓` | Soft drop (1 × level per row) |
| `↑` | Rotate clockwise |
| `Z` | Rotate counter-clockwise |
| `X` | Rotate clockwise (alternate) |
| `Space` | Hard drop (2 × level per row) |
| `C` | Hold current piece |
| `P` | Pause / resume |
| `S` | Save game to disk |
| `Q` / `Esc` | Save score and return to menu |
| `R` | Retry (game over screen only) |

---

## Gameplay Rules

Pixler implements the standard falling block ruleset documented by the
**Tetris Wiki** and the **Tetris Guideline** (see [References](#references)).

### Board

10 columns × 20 visible rows. Stored as a 2D int array — `0` = empty, `1–7` = locked piece type.
Each cell renders as `[]` (2 chars wide).

### Pieces

| ID | Name | Color |
|----|------|-------|
| 0 | I | Cyan |
| 1 | O | Yellow |
| 2 | T | Magenta |
| 3 | S | Green |
| 4 | Z | Red |
| 5 | J | Blue |
| 6 | L | Red/Orange |

All 4 rotation states are pre-baked as 4×4 binary grids in `PIECES[piece][rotation][row][col]`.

### Rotation System

Simplified SRS-style wall kick. On collision, five horizontal offsets are tried:

```c
int kicks[] = {0, -1, 1, -2, 2};
```

### Scoring

| Lines Cleared | Base Points |
|---------------|-------------|
| 1 | 100 |
| 2 | 300 |
| 3 | 500 |
| 4 | 800 |

All values multiplied by current level. Soft drop: `1 × level` per row. Hard drop: `2 × level` per row.

### Leveling and Speed

Level advances every 10 lines. Speed formula:

```c
int speed = INITIAL_SPEED - (level - 1) * SPEED_STEP;
// 500ms start, -30ms per level, 50ms minimum
```

### Ghost Piece

Renders `::` in gray at the projected landing position, recalculated every frame.

### Hold Queue

`C` swaps the active piece with the hold slot. Can only be used once per piece placement.

### Lock and Game Over

Piece locks when it can't move down. Game over when a spawned piece immediately collides with locked cells.

---

## Save and Load System

**Save (`S` key):** Writes the full `GameState` struct to `pixler_save.dat` as a raw binary blob via `fwrite`.

**Load (Main Menu → Load Game):** Reads it back with `fread`. Load option only appears when the save file exists.
Save file is deleted automatically on game over.

> The binary format is not portable across compilers or machines due to struct padding. Single-session use only.

---

## High Score CSV

Appended to `pixler_scores.csv` on every game over or quit:

```
PlayerName,Score,Level,Lines,YYYY-MM-DD HH:MM
```

The High Scores screen shows up to 10 records. Records are in append order —
sort column B descending in any spreadsheet app to rank by score.

---

## Code Architecture

### pixler.h

- Key codes, board constants, scoring constants
- `PIECES[7][4][4][4]` — all piece rotation data
- `PIECE_COLORS[7]` — console color per piece
- `GameState` struct — complete game state
- `ScoreRecord` struct — CSV structure
- All function prototypes

### pixler.c

| Section | Functions |
|---------|-----------|
| Console utilities | `gotoxy`, `set_color`, `hide_cursor`, `show_cursor` |
| Drawing | `draw_border`, `draw_board`, `draw_piece`, `draw_next`, `draw_hold`, `draw_stats` |
| Game logic | `init_game`, `new_piece`, `check_collision`, `lock_piece`, `clear_lines`, `update_score` |
| Piece movement | `rotate_piece`, `move_piece`, `hard_drop`, `hold_piece_fn` |
| Ghost | `get_ghost_y` |
| Save / Load | `save_game`, `load_game`, `save_score`, `show_scores` |
| Menu | `draw_title`, `draw_main_menu_box`, `input_name`, `main_menu` |
| Game loop | `game_loop` |
| Entry point | `main` |

---

## Win32 API Usage

| Function | Purpose |
|----------|---------|
| `SetConsoleCursorPosition` | Move cursor to (x, y) |
| `SetConsoleTextAttribute` | Set fg/bg color |
| `GetStdHandle` | Get console output handle |
| `SetConsoleCursorInfo` | Show/hide cursor |
| `SetConsoleTitle` | Set title bar text |
| `GetTickCount` | Millisecond timer for gravity |
| `Sleep` | Yield CPU between frames |
| `_getch()` | Read keypress without echo |
| `_kbhit()` | Non-blocking keypress check |

Extended keys (arrows) send a `0` or `224` prefix — a second `_getch()` gives the actual code.

---

## Known Limitations

- **Windows only** — Win32 and `conio.h` unavailable on Linux/macOS without compatibility layers
- **No lock delay** — pieces lock immediately; no slide/rotate window after floor contact
- **No T-spin detection**
- **No ARE (entry delay)** — pieces spawn instantly
- **Save not portable** — binary struct dump, single-session use
- **Scores unsorted** — append order only

---

## References

### Falling Block Ruleset

**The Tetris Company — Tetris Guideline**
> Official specification for licensed falling block games since 2001.
> - https://tetris.com

**Tetris Wiki — Tetris Guideline**
> Full technical documentation of the Guideline. Primary reference for this project.
> - https://tetris.wiki/Tetris_Guideline

### Scoring

**Tetris Wiki — Scoring**
> Documents all major scoring systems. Source for the 100/300/500/800 × level values.
> - https://tetris.wiki/Scoring

### Rotation and Wall Kicks

**Tetris Wiki — Super Rotation System (SRS)**
> Full SRS documentation with piece-specific wall kick tables.
> - https://tetris.wiki/Super_Rotation_System

### Piece Definitions

**Tetris Wiki — Tetromino**
> Canonical piece names, spawn orientations, and color assignments.
> - https://tetris.wiki/Tetromino

### Speed Curve

**Tetris Wiki — NES Tetris**
> Frames-per-gridcell speed table. Pixler uses a simplified linear approximation.
> - https://tetris.wiki/Tetris_(NES,_Nintendo)

### Ghost and Hold

**Tetris Wiki — Ghost Piece** — https://tetris.wiki/Ghost_piece

**Tetris Wiki — Hold Piece** — https://tetris.wiki/Hold_piece

### Win32 Console API

**Microsoft Learn — Console Reference (Win32)**
> - https://learn.microsoft.com/en-us/windows/console/console-reference

**Microsoft Learn — `_getch`, `_kbhit`**
> - https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/getch-getwch
> - https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/kbhit

### Toolchain

**GCC** — https://gcc.gnu.org/onlinedocs/

**MinGW-w64** — https://www.mingw-w64.org/

**MSYS2** — https://www.msys2.org/

### Original Game

**Pajitnov, A. (1984). Tetris.**
> Created at the Dorodnitsyn Computing Centre, Soviet Academy of Sciences.
> - https://tetris.wiki/Tetris_(Electronika_60)

---

*Pixler is an independent hobby project. Tetris® is a registered trademark of Tetris Holding, LLC.*
