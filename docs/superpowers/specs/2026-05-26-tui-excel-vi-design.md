# visheet — TUI Spreadsheet with Vi Mode
**Date:** 2026-05-26
**Status:** Approved

## Overview

`visheet` is a terminal spreadsheet application for Windows, compiled with GCC (MinGW64). It combines a scrollable grid (702 columns × 9999 rows) with a full Vim-style modal editing interface, basic formula support, CSV I/O, and a customizable color theme system. The entire app ships as a single self-contained `visheet.exe` with no runtime dependencies beyond a Windows console.

---

## 1. Architecture & Modules

**Technology stack:** C11, GCC 16.x (MinGW64), PDCurses (win32 console, statically linked).

```
visheet/
├── Makefile
├── main.c          — PDCurses init/teardown, main event loop
├── grid.c/h        — cell storage, cell types, dirty flags
├── render.c/h      — viewport drawing, headers, status/formula bars
├── input.c/h       — raw keypress capture and dispatch
├── vi_mode.c/h     — Normal/Insert/Visual/Command mode state machine
├── formula.c/h     — recursive descent parser and evaluator
├── csv.c/h         — RFC 4180 compliant CSV read/write
└── theme.c/h       — color pair definitions, theme switching
```

**Data flow:**
```
main → input (keypress) → vi_mode (interpret) → grid (mutate)
                                                      ↓
                                               render (redraw viewport)
                                               formula (lazy eval on read)
                                               csv (only on :w / :e)
```

**Grid storage:**
```c
#define MAX_ROWS 9999
#define MAX_COLS 702   // A through ZZ
#define MAX_CELL 1024  // max cell content bytes

typedef enum { CELL_TEXT, CELL_NUMBER, CELL_FORMULA } CellType;

typedef struct {
    char    raw[MAX_CELL];      // what the user typed
    char    display[MAX_CELL];  // evaluated display value
    CellType type;
    bool    dirty;
} Cell;

Cell grid[MAX_ROWS][MAX_COLS];
```

---

## 2. Vi Mode State Machine

### Normal Mode (green status bar — default)

| Key(s) | Action |
|--------|--------|
| `h` `j` `k` `l` | Move left / down / up / right |
| `w` / `b` | Jump columns (word-width steps) |
| `gg` / `G` | Jump to row 1 / last used row |
| `0` / `$` | Jump to column A / last used column |
| `Ctrl+F` / `Ctrl+B` | Page down / page up |
| `[count]motion` | e.g. `5j` moves 5 rows down |
| `i` / `a` | Enter Insert mode (edit / append to cell) |
| `v` | Enter Visual mode |
| `:` | Enter Command mode |
| `dd` | Clear current row |
| `yy` | Yank current row into register |
| `p` | Paste yanked row below cursor |
| `x` | Clear current cell |
| `r<char>` | Replace cell content with typed character |
| `/pattern` | Search cells forward |
| `n` / `N` | Next / previous search match |

### Insert Mode (red status bar)

| Key(s) | Action |
|--------|--------|
| Typing | Edits the current cell buffer |
| `Enter` | Confirm cell, move cursor down |
| `Tab` | Confirm cell, move cursor right |
| `Esc` | Confirm cell, return to Normal mode |
| `=` (first char) | Marks cell as formula cell |

### Visual Mode (blue status bar)

| Key(s) | Action |
|--------|--------|
| `h` `j` `k` `l` | Extend selection |
| `y` | Yank selection into register |
| `d` | Clear all cells in selection |
| `Esc` | Return to Normal mode |

### Command Mode (yellow status bar, `:` prefix)

| Command | Action |
|---------|--------|
| `:w [file]` | Save to CSV |
| `:e [file]` | Open CSV (warns if unsaved) |
| `:e!` | Open CSV, discard unsaved changes |
| `:q` | Quit (warns if unsaved) |
| `:q!` | Force quit |
| `:wq` | Save and quit |
| `:sort [col]` | Sort rows by column (e.g. `:sort B`) |
| `:sum A1:A10` | Display sum in command bar |
| `:goto A42` | Jump to cell address |
| `:theme [name]` | Switch color theme |

---

## 3. Rendering & Color Theme

### Screen Layout

```
┌─────────────────────────────────────────────────────┐
│ [Formula Bar]  A1: =SUM(A1:A10)                     │  row 0
├────┬────────┬────────┬────────┬────────┬────────────┤
│    │   A    │   B    │   C    │   D    │   E    ... │  row 1 (col headers)
├────┼────────┼────────┼────────┼────────┼────────────┤
│  1 │ hello  │  42    │ =SUM.. │        │            │  data rows
│  2 │        │        │        │        │            │
│ .. │        │        │        │        │            │
├────┴────────┴────────┴────────┴────────┴────────────┤
│ [Status Bar]  NORMAL  |  visheet.csv  |  R1C1       │  last row
└─────────────────────────────────────────────────────┘
```

### Color Pairs

| Element | Foreground | Background |
|---------|-----------|-----------|
| Normal cell | White | Black |
| Current cell | Black | Cyan |
| Formula cell | Yellow | Black |
| Number cell | Green | Black |
| Visual selection | Black | Blue |
| Column/row header | Black | White |
| Status bar — NORMAL | Black | Green |
| Status bar — INSERT | Black | Red |
| Status bar — VISUAL | Black | Blue |
| Status bar — COMMAND | Black | Yellow |
| Formula bar | White | Dark gray |

### Themes

Themes are compile-time structs in `theme.c` — no config files. Built-in themes: `default`, `solarized`, `monokai`. `:theme name` swaps the active set and triggers a full redraw.

### Viewport & Column Width

- Fixed column width: **12 characters** (padded or truncated with `…`)
- Renderer tracks `viewport_row` and `viewport_col` offsets
- Only visible cells are drawn each frame
- Column address format: A–Z (cols 0–25), AA–AZ (26–51), … up to ZZ (col 701)

---

## 4. Formula Engine

### Grammar (recursive descent)

```
expr       := term (('+' | '-') term)*
term       := factor (('*' | '/') factor)*
factor     := number | cell_ref | range_func | '(' expr ')'
cell_ref   := [A-Z]{1,2}[0-9]{1,4}
range      := cell_ref ':' cell_ref
range_func := ('SUM'|'AVG'|'MIN'|'MAX'|'COUNT') '(' range ')'
```

### Evaluation

- Results computed as `double`, formatted as string for display
- Cell references resolved by reading `grid[row][col].display`
- Formula cells re-evaluated on every render pass when dirty

### Recalculation

- Dirty-flag approach: editing any cell marks all formula cells dirty
- No dependency graph — full re-evaluation pass on render
- Circular reference detection via recursion depth limit (32)

### Error Tokens

| Token | Cause |
|-------|-------|
| `#CIRC!` | Circular reference (depth > 32) |
| `#REF!` | Cell reference out of bounds |
| `#DIV0!` | Division by zero |
| `#ERR!` | Parse error |
| `#VAL!` | Non-numeric value in arithmetic |

---

## 5. CSV I/O

- **Format:** RFC 4180 — fields with commas, quotes, or newlines are double-quoted; internal quotes escaped as `""`
- **Max cell content:** 1024 characters
- **Cell type auto-detection on load:**
  - Starts with `=` → `CELL_FORMULA`
  - Parses as `double` → `CELL_NUMBER`
  - Otherwise → `CELL_TEXT`
- **Save (`:w`):** writes up to last used row × last used column; no trailing empty rows
- **Load (`:e`):** clears grid, then loads; warns on unsaved changes; `:e!` to force

---

## 6. Build System

```makefile
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
LIBS    = -lpdcurses
SRCS    = main.c grid.c render.c input.c vi_mode.c formula.c csv.c theme.c
OBJS    = $(SRCS:.c=.o)
TARGET  = visheet.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o $(TARGET)
```

**PDCurses setup (one-time):** Download PDCurses win32 console source, build with `make -f Makefile.mingw`, copy `pdcurses.a` and `curses.h` into the project or a system include/lib path. The final `visheet.exe` is fully self-contained.

---

## 7. Error Handling

- PDCurses init failure → print to stderr, exit code 1 (before curses owns the terminal)
- CSV load failure → error message in status bar, grid unchanged
- Formula error → display error token in cell, never crash
- Unsaved changes on `:q` → `E: unsaved changes, use :q! to force` in command bar

---

## 8. Testing

**Test CSV files (`test/` directory):**

| File | Purpose |
|------|---------|
| `test/basic.csv` | Plain text and numbers |
| `test/formulas.csv` | SUM, AVG, MIN, MAX, COUNT cells |
| `test/edge.csv` | Quoted fields, embedded commas, empty cells |
| `test/large.csv` | 9999 rows × 26 columns (scroll/perf) |
| `test/circular.csv` | Circular reference → `#CIRC!` expected |

**`test/run_tests.bat`:** Opens each file via `:e`, saves via `:w test/out_*.csv`, diffs against expected output with `fc`. Pass = no diff output.

**Logic unit tests (`test/test_main.c`):** Tests for `formula.c`, `csv.c`, `grid.c` that bypass PDCurses entirely — compile with `gcc test/test_main.c formula.c csv.c grid.c -o test_runner.exe` and run directly.

---

## Open Questions

None — all design decisions resolved.
