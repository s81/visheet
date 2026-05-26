# visheet — TUI Spreadsheet with Vi Mode Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build `visheet.exe`, a terminal spreadsheet with Vim-style modal editing, basic formulas (SUM/AVG/MIN/MAX/COUNT), CSV I/O, and a full color theme system, compiled with GCC 16.x (MinGW64) on Windows using PDCurses.

**Architecture:** Eight focused C11 source files — `grid`, `csv`, `formula`, `theme`, `render`, `vi_mode`, `input`, `main` — each with a single responsibility. Logic modules (`grid`, `csv`, `formula`) are testable without PDCurses; UI modules use PDCurses directly. A single Makefile builds everything into one self-contained `visheet.exe`.

**Tech Stack:** C11, GCC 16.x (MinGW64 x86_64-w64-mingw32), PDCurses 3.x (win32 console backend), GNU Make, Windows 11.

---

## File Map

| File | Responsibility |
|------|---------------|
| `grid.h/c` | Cell struct, pointer grid `Cell*[9999][702]`, address helpers |
| `csv.h/c` | RFC 4180 compliant CSV read/write |
| `formula.h/c` | Recursive descent parser, evaluator, error tokens |
| `theme.h/c` | PDCurses color pairs, named theme structs |
| `render.h/c` | Formula bar, grid viewport, status/command bar |
| `vi_mode.h/c` | Normal/Insert/Visual/Command mode state machine |
| `input.h/c` | `getch()` wrapper, key-name utility |
| `main.c` | PDCurses init/teardown, main event loop |
| `test/test_grid.c` | Unit tests for grid module (no PDCurses) |
| `test/test_csv.c` | Unit tests for CSV module (no PDCurses) |
| `test/test_formula.c` | Unit tests for formula module (no PDCurses) |
| `test/run_tests.bat` | Batch test runner |

---

### Task 1: PDCurses Setup + Project Scaffold

**Files:**
- Create: `Makefile`, `main.c`, `grid.h`, `grid.c`, `csv.h`, `csv.c`
- Create: `formula.h`, `formula.c`, `theme.h`, `theme.c`
- Create: `render.h`, `render.c`, `vi_mode.h`, `vi_mode.c`
- Create: `input.h`, `input.c`, `test/` directory

- [ ] **Step 1: Download and build PDCurses**

```bat
REM In PowerShell or cmd — download PDCurses source from GitHub
REM https://github.com/wmcbrine/PDCurses/archive/refs/tags/3.9.tar.gz
REM Extract to D:\Projects\other\pdcurses-src\
REM Then build the win32 console backend:

cd D:\Projects\other\pdcurses-src\wincon
make -f Makefile.mingw
REM Produces: ..\pdcurses.a  (and uses ..\curses.h as the header)
```

Copy results into the project:
```bat
mkdir D:\Projects\other\pdcurses
copy D:\Projects\other\pdcurses-src\pdcurses.a D:\Projects\other\pdcurses\
copy D:\Projects\other\pdcurses-src\curses.h   D:\Projects\other\pdcurses\
```

Expected: `pdcurses/pdcurses.a` and `pdcurses/curses.h` exist.

- [ ] **Step 2: Create directory structure**

```bat
cd D:\Projects\other
mkdir test
```

- [ ] **Step 3: Write `grid.h` — complete header (all types and declarations)**

```c
#ifndef GRID_H
#define GRID_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_ROWS  9999
#define MAX_COLS  702
#define MAX_CELL  1024
#define COL_WIDTH 12

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

typedef enum { CELL_TEXT, CELL_NUMBER, CELL_FORMULA } CellType;

typedef struct {
    char     raw[MAX_CELL];
    char     display[MAX_CELL];
    CellType type;
    bool     dirty;
} Cell;

/* Grid is a sparse pointer array — NULL means empty cell.
   ~54 MB BSS, lazily paged by the OS. */
extern Cell *grid[MAX_ROWS][MAX_COLS];
extern int   last_row;
extern int   last_col;

void        grid_init(void);
void        grid_clear_cell(int row, int col);
void        grid_clear_row(int row);
void        grid_set_cell(int row, int col, const char *raw);
void        grid_mark_formulas_dirty(void);
const char *col_to_str(int col, char *buf, size_t buflen);
int         str_to_col(const char *s);
void        cell_addr(int row, int col, char *buf, size_t buflen);
bool        parse_cell_addr(const char *s, int *row, int *col);

#endif
```

- [ ] **Step 4: Write stub `grid.c`**

```c
#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

Cell *grid[MAX_ROWS][MAX_COLS];
int   last_row = 0;
int   last_col = 0;

void grid_init(void) {}
void grid_clear_cell(int row, int col) { (void)row; (void)col; }
void grid_clear_row(int row) { (void)row; }
void grid_set_cell(int row, int col, const char *raw) { (void)row; (void)col; (void)raw; }
void grid_mark_formulas_dirty(void) {}
const char *col_to_str(int col, char *buf, size_t buflen) { (void)col; buf[0]='\0'; return buf; (void)buflen; }
int  str_to_col(const char *s) { (void)s; return 0; }
void cell_addr(int row, int col, char *buf, size_t buflen) { (void)row; (void)col; buf[0]='\0'; (void)buflen; }
bool parse_cell_addr(const char *s, int *row, int *col) { (void)s; (void)row; (void)col; return false; }
```

- [ ] **Step 5: Write `csv.h` and stub `csv.c`**

`csv.h`:
```c
#ifndef CSV_H
#define CSV_H

typedef enum { CSV_OK, CSV_ERR_FILE } CsvError;

CsvError csv_load(const char *path);
CsvError csv_save(const char *path);

#endif
```

`csv.c`:
```c
#include "csv.h"
CsvError csv_load(const char *path) { (void)path; return CSV_ERR_FILE; }
CsvError csv_save(const char *path) { (void)path; return CSV_ERR_FILE; }
```

- [ ] **Step 6: Write `formula.h` and stub `formula.c`**

`formula.h`:
```c
#ifndef FORMULA_H
#define FORMULA_H

void formula_eval_cell(int row, int col);
void formula_eval_all(void);

#endif
```

`formula.c`:
```c
#include "formula.h"
void formula_eval_cell(int row, int col) { (void)row; (void)col; }
void formula_eval_all(void) {}
```

- [ ] **Step 7: Write `theme.h` and stub `theme.c`**

`theme.h`:
```c
#ifndef THEME_H
#define THEME_H

#define CP_NORMAL     1
#define CP_CURRENT    2
#define CP_FORMULA    3
#define CP_NUMBER     4
#define CP_VISUAL     5
#define CP_HEADER     6
#define CP_STATUS_N   7
#define CP_STATUS_I   8
#define CP_STATUS_V   9
#define CP_STATUS_CMD 10
#define CP_FORMULABAR 11

void theme_init(void);
int  theme_set(const char *name);

#endif
```

`theme.c`:
```c
#include "theme.h"
void theme_init(void) {}
int  theme_set(const char *name) { (void)name; return 0; }
```

- [ ] **Step 8: Write `vi_mode.h` and stub `vi_mode.c`**

`vi_mode.h`:
```c
#ifndef VI_MODE_H
#define VI_MODE_H

#include "grid.h"
#include <stdbool.h>

typedef enum { MODE_NORMAL, MODE_INSERT, MODE_VISUAL, MODE_COMMAND } VimMode;

typedef struct {
    VimMode mode;
    int     cursor_row, cursor_col;
    int     viewport_row, viewport_col;
    int     visual_start_row, visual_start_col;
    char    cmd_buf[256];
    int     cmd_len;
    char    cell_buf[MAX_CELL];
    int     cell_buf_len;
    char    search_pat[MAX_CELL];
    char    yank_row_data[MAX_COLS][MAX_CELL];
    bool    yank_valid;
    char    status_msg[256];
    bool    dirty;
    char    filename[512];
    int     count;
    int     pending_key;
    bool    needs_redraw;
} AppState;

void vi_init(AppState *s);
void vi_handle_key(AppState *s, int key);

#endif
```

`vi_mode.c`:
```c
#include "vi_mode.h"
#include <string.h>
void vi_init(AppState *s) { memset(s, 0, sizeof(*s)); s->mode = MODE_NORMAL; s->needs_redraw = true; }
void vi_handle_key(AppState *s, int key) { (void)key; s->needs_redraw = true; }
```

- [ ] **Step 9: Write `render.h` and stub `render.c`**

`render.h`:
```c
#ifndef RENDER_H
#define RENDER_H
#include "vi_mode.h"
void render_all(const AppState *s);
#endif
```

`render.c`:
```c
#include "render.h"
void render_all(const AppState *s) { (void)s; }
```

- [ ] **Step 10: Write `input.h` and stub `input.c`**

`input.h`:
```c
#ifndef INPUT_H
#define INPUT_H
int input_get_key(void);
#endif
```

`input.c`:
```c
#include "input.h"
#include <curses.h>
int input_get_key(void) { return getch(); }
```

- [ ] **Step 11: Write stub `main.c`**

```c
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"
#include "render.h"
#include "vi_mode.h"
#include "theme.h"
#include "formula.h"
#include "csv.h"
#include "input.h"

int main(int argc, char *argv[]) {
    grid_init();
    AppState state;
    vi_init(&state);

    if (argc > 1) {
        strncpy(state.filename, argv[1], sizeof(state.filename) - 1);
        if (csv_load(argv[1]) == CSV_OK) {
            formula_eval_all();
            state.dirty = false;
        }
    }

    initscr();
    if (!has_colors()) { endwin(); return 1; }
    start_color();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    curs_set(1);
    theme_init();

    while (1) {
        render_all(&state);
        int key = input_get_key();
        vi_handle_key(&state, key);
    }

    endwin();
    return 0;
}
```

- [ ] **Step 12: Write `Makefile`**

```makefile
CC        = gcc
CFLAGS    = -Wall -Wextra -O2 -std=c11 -Ipdcurses
LIBS      = -Lpdcurses -lpdcurses
SRCS      = main.c grid.c render.c input.c vi_mode.c formula.c csv.c theme.c
OBJS      = $(SRCS:.c=.o)
TARGET    = visheet.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test_grid.exe: test/test_grid.c grid.c
	$(CC) $(CFLAGS) test/test_grid.c grid.c -o test_grid.exe

test_csv.exe: test/test_csv.c csv.c grid.c
	$(CC) $(CFLAGS) test/test_csv.c csv.c grid.c -o test_csv.exe

test_formula.exe: test/test_formula.c formula.c grid.c
	$(CC) $(CFLAGS) test/test_formula.c formula.c grid.c -o test_formula.exe

test: test_grid.exe test_csv.exe test_formula.exe
	.\test_grid.exe && .\test_csv.exe && .\test_formula.exe

clean:
	del /Q *.o *.exe 2>NUL || exit 0
```

- [ ] **Step 13: Verify scaffold compiles**

```bat
cd D:\Projects\other
make
```

Expected: `visheet.exe` created, zero errors. (Stubs do nothing, but it links.)

- [ ] **Step 14: Commit scaffold**

```bat
git add Makefile main.c grid.h grid.c csv.h csv.c formula.h formula.c
git add theme.h theme.c render.h render.c vi_mode.h vi_mode.c input.h input.c
git commit -m "feat: project scaffold with stub implementations"
```

---

### Task 2: Grid Module (TDD)

**Files:**
- Create: `test/test_grid.c`
- Modify: `grid.c` (full implementation replacing stubs)

- [ ] **Step 1: Write failing tests — `test/test_grid.c`**

```c
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../grid.h"

static int tests_run = 0, tests_passed = 0;
#define RUN(name) do { tests_run++; test_##name(); tests_passed++; \
                       printf("PASS: " #name "\n"); } while(0)

static void test_grid_init(void) {
    grid_init();
    assert(last_row == 0 && last_col == 0);
    assert(grid[0][0] == NULL);
    assert(grid[MAX_ROWS-1][MAX_COLS-1] == NULL);
}

static void test_set_text(void) {
    grid_init();
    grid_set_cell(0, 0, "hello");
    assert(grid[0][0] != NULL);
    assert(strcmp(grid[0][0]->raw, "hello") == 0);
    assert(grid[0][0]->type == CELL_TEXT);
    assert(grid[0][0]->dirty == true);
}

static void test_set_number(void) {
    grid_init();
    grid_set_cell(0, 0, "42");
    assert(grid[0][0]->type == CELL_NUMBER);
    grid_set_cell(0, 1, "3.14");
    assert(grid[0][1]->type == CELL_NUMBER);
    grid_set_cell(0, 2, "0");
    assert(grid[0][2]->type == CELL_NUMBER);
}

static void test_set_formula(void) {
    grid_init();
    grid_set_cell(0, 0, "=SUM(A1:A3)");
    assert(grid[0][0]->type == CELL_FORMULA);
}

static void test_clear_cell(void) {
    grid_init();
    grid_set_cell(0, 0, "hello");
    grid_clear_cell(0, 0);
    assert(grid[0][0] == NULL);
}

static void test_clear_row(void) {
    grid_init();
    grid_set_cell(2, 0, "a"); grid_set_cell(2, 1, "b"); grid_set_cell(2, 2, "c");
    grid_clear_row(2);
    assert(grid[2][0] == NULL && grid[2][1] == NULL && grid[2][2] == NULL);
}

static void test_last_tracking(void) {
    grid_init();
    grid_set_cell(5, 10, "x");
    assert(last_row == 5 && last_col == 10);
    grid_set_cell(2, 3, "y");
    assert(last_row == 5 && last_col == 10);
}

static void test_col_to_str_single(void) {
    char buf[4];
    assert(strcmp(col_to_str(0,  buf, sizeof(buf)), "A") == 0);
    assert(strcmp(col_to_str(25, buf, sizeof(buf)), "Z") == 0);
}

static void test_col_to_str_double(void) {
    char buf[4];
    assert(strcmp(col_to_str(26,  buf, sizeof(buf)), "AA") == 0);
    assert(strcmp(col_to_str(701, buf, sizeof(buf)), "ZZ") == 0);
}

static void test_str_to_col(void) {
    assert(str_to_col("A")  == 0);
    assert(str_to_col("Z")  == 25);
    assert(str_to_col("AA") == 26);
    assert(str_to_col("ZZ") == 701);
}

static void test_cell_addr(void) {
    char buf[10];
    cell_addr(0, 0, buf, sizeof(buf));
    assert(strcmp(buf, "A1") == 0);
    cell_addr(9998, 701, buf, sizeof(buf));
    assert(strcmp(buf, "ZZ9999") == 0);
}

static void test_parse_cell_addr(void) {
    int row, col;
    assert(parse_cell_addr("A1",   &row, &col) && row == 0    && col == 0);
    assert(parse_cell_addr("B2",   &row, &col) && row == 1    && col == 1);
    assert(parse_cell_addr("ZZ9999", &row, &col) && row == 9998 && col == 701);
    assert(!parse_cell_addr("1A",  &row, &col));
    assert(!parse_cell_addr("",    &row, &col));
}

int main(void) {
    RUN(grid_init); RUN(set_text); RUN(set_number); RUN(set_formula);
    RUN(clear_cell); RUN(clear_row); RUN(last_tracking);
    RUN(col_to_str_single); RUN(col_to_str_double); RUN(str_to_col);
    RUN(cell_addr); RUN(parse_cell_addr);
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
```

- [ ] **Step 2: Build and run to confirm tests fail**

```bat
make test_grid.exe
.\test_grid.exe
```

Expected: tests fail (stubs return wrong values).

- [ ] **Step 3: Implement `grid.c` fully**

```c
#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

Cell *grid[MAX_ROWS][MAX_COLS];
int   last_row = 0;
int   last_col = 0;

void grid_init(void) {
    for (int r = 0; r <= last_row; r++)
        for (int c = 0; c <= last_col; c++) {
            free(grid[r][c]);
            grid[r][c] = NULL;
        }
    last_row = 0;
    last_col = 0;
}

void grid_clear_cell(int row, int col) {
    free(grid[row][col]);
    grid[row][col] = NULL;
}

void grid_clear_row(int row) {
    for (int c = 0; c <= last_col; c++)
        grid_clear_cell(row, c);
}

void grid_set_cell(int row, int col, const char *raw) {
    if (!grid[row][col]) {
        grid[row][col] = calloc(1, sizeof(Cell));
        if (!grid[row][col]) return;
    }
    Cell *cell = grid[row][col];
    strncpy(cell->raw, raw, MAX_CELL - 1);
    cell->raw[MAX_CELL - 1] = '\0';

    if (raw[0] == '=') {
        cell->type = CELL_FORMULA;
    } else {
        char *end;
        strtod(raw, &end);
        cell->type = (*end == '\0' && raw[0] != '\0') ? CELL_NUMBER : CELL_TEXT;
        strncpy(cell->display, raw, MAX_CELL - 1);
    }
    cell->dirty = true;
    if (row > last_row) last_row = row;
    if (col > last_col) last_col = col;
}

void grid_mark_formulas_dirty(void) {
    for (int r = 0; r <= last_row; r++)
        for (int c = 0; c <= last_col; c++)
            if (grid[r][c] && grid[r][c]->type == CELL_FORMULA)
                grid[r][c]->dirty = true;
}

const char *col_to_str(int col, char *buf, size_t buflen) {
    if (col < 26) {
        snprintf(buf, buflen, "%c", 'A' + col);
    } else {
        int idx = col - 26;
        snprintf(buf, buflen, "%c%c", 'A' + idx / 26, 'A' + idx % 26);
    }
    return buf;
}

int str_to_col(const char *s) {
    int len = (int)strlen(s);
    if (len == 1) return s[0] - 'A';
    return 26 + (s[0] - 'A') * 26 + (s[1] - 'A');
}

void cell_addr(int row, int col, char *buf, size_t buflen) {
    char cbuf[4];
    col_to_str(col, cbuf, sizeof(cbuf));
    snprintf(buf, buflen, "%s%d", cbuf, row + 1);
}

bool parse_cell_addr(const char *s, int *row, int *col) {
    if (!s || !*s) return false;
    int i = 0;
    char cbuf[3] = {0};
    while (s[i] && isupper((unsigned char)s[i]) && i < 2) { cbuf[i] = s[i]; i++; }
    if (i == 0 || !s[i] || !isdigit((unsigned char)s[i])) return false;
    int r = atoi(s + i) - 1;
    if (r < 0 || r >= MAX_ROWS) return false;
    int c = str_to_col(cbuf);
    if (c < 0 || c >= MAX_COLS) return false;
    *row = r; *col = c;
    return true;
}
```

- [ ] **Step 4: Run tests — expect all pass**

```bat
make test_grid.exe
.\test_grid.exe
```

Expected output:
```
PASS: grid_init
PASS: set_text
...
12/12 tests passed
```

- [ ] **Step 5: Commit**

```bat
git add grid.c test/test_grid.c
git commit -m "feat: implement grid module with full unit tests"
```

---

### Task 3: CSV Module (TDD)

**Files:**
- Create: `test/test_csv.c`
- Modify: `csv.c` (full implementation)

- [ ] **Step 1: Write failing tests — `test/test_csv.c`**

```c
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../grid.h"
#include "../csv.h"

static int tests_run = 0, tests_passed = 0;
#define RUN(name) do { tests_run++; test_##name(); tests_passed++; \
                       printf("PASS: " #name "\n"); } while(0)

static void set_display(int r, int c, const char *v) {
    Cell *cell = grid[r][c];
    if (cell) { strncpy(cell->display, v, MAX_CELL-1); cell->dirty = false; }
}

static void test_save_load_basic(void) {
    grid_init();
    grid_set_cell(0, 0, "name"); set_display(0, 0, "name");
    grid_set_cell(0, 1, "age");  set_display(0, 1, "age");
    grid_set_cell(1, 0, "Alice"); set_display(1, 0, "Alice");
    grid_set_cell(1, 1, "30");   set_display(1, 1, "30");

    assert(csv_save("_tmp_basic.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_basic.csv") == CSV_OK);
    assert(grid[0][0] != NULL && strcmp(grid[0][0]->raw, "name") == 0);
    assert(grid[1][1] != NULL && strcmp(grid[1][1]->raw, "30") == 0);
    assert(grid[1][1]->type == CELL_NUMBER);
    remove("_tmp_basic.csv");
}

static void test_quoted_comma(void) {
    grid_init();
    grid_set_cell(0, 0, "hello, world"); set_display(0, 0, "hello, world");
    assert(csv_save("_tmp_comma.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_comma.csv") == CSV_OK);
    assert(strcmp(grid[0][0]->raw, "hello, world") == 0);
    remove("_tmp_comma.csv");
}

static void test_embedded_quote(void) {
    grid_init();
    grid_set_cell(0, 0, "say \"hi\""); set_display(0, 0, "say \"hi\"");
    assert(csv_save("_tmp_quote.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_quote.csv") == CSV_OK);
    assert(strcmp(grid[0][0]->raw, "say \"hi\"") == 0);
    remove("_tmp_quote.csv");
}

static void test_formula_preserved(void) {
    grid_init();
    grid_set_cell(0, 0, "=SUM(A2:A3)"); set_display(0, 0, "5");
    assert(csv_save("_tmp_formula.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_formula.csv") == CSV_OK);
    assert(grid[0][0]->type == CELL_FORMULA);
    assert(strcmp(grid[0][0]->raw, "=SUM(A2:A3)") == 0);
    remove("_tmp_formula.csv");
}

static void test_missing_file(void) {
    grid_init();
    assert(csv_load("_nonexistent_xyz_123.csv") == CSV_ERR_FILE);
}

static void test_empty_cells_skipped(void) {
    grid_init();
    grid_set_cell(0, 0, "A"); set_display(0, 0, "A");
    /* col 1 intentionally empty */
    grid_set_cell(0, 2, "C"); set_display(0, 2, "C");
    assert(csv_save("_tmp_empty.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_empty.csv") == CSV_OK);
    assert(strcmp(grid[0][0]->raw, "A") == 0);
    assert(strcmp(grid[0][2]->raw, "C") == 0);
    remove("_tmp_empty.csv");
}

int main(void) {
    RUN(save_load_basic); RUN(quoted_comma); RUN(embedded_quote);
    RUN(formula_preserved); RUN(missing_file); RUN(empty_cells_skipped);
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
```

- [ ] **Step 2: Build and run to confirm tests fail**

```bat
make test_csv.exe
.\test_csv.exe
```

Expected: `CSV_ERR_FILE` returned by stubs — tests fail.

- [ ] **Step 3: Implement `csv.c` fully**

```c
#include "csv.h"
#include "grid.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

CsvError csv_load(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return CSV_ERR_FILE;

    grid_init();
    char line[MAX_COLS * 16];  /* conservative line buffer */
    int row = 0;

    while (fgets(line, sizeof(line), f) && row < MAX_ROWS) {
        int col = 0;
        char *p = line;

        while (*p && *p != '\n' && *p != '\r' && col < MAX_COLS) {
            char field[MAX_CELL];
            int  fi = 0;

            if (*p == '"') {
                p++;
                while (*p) {
                    if (*p == '"' && *(p+1) == '"') { if (fi < MAX_CELL-1) field[fi++] = '"'; p += 2; }
                    else if (*p == '"') { p++; break; }
                    else { if (fi < MAX_CELL-1) field[fi++] = *p; p++; }
                }
                if (*p == ',') p++;
            } else {
                while (*p && *p != ',' && *p != '\n' && *p != '\r')
                    { if (fi < MAX_CELL-1) field[fi++] = *p; p++; }
                if (*p == ',') p++;
            }
            field[fi] = '\0';

            if (fi > 0) {
                grid_set_cell(row, col, field);
                /* text/number display is already set by grid_set_cell */
            }
            col++;
        }
        row++;
    }

    fclose(f);
    return CSV_OK;
}

CsvError csv_save(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return CSV_ERR_FILE;

    for (int r = 0; r <= last_row; r++) {
        for (int c = 0; c <= last_col; c++) {
            Cell       *cell = grid[r][c];
            const char *val  = cell ? cell->raw : "";

            bool need_quote = false;
            for (const char *v = val; *v; v++)
                if (*v == ',' || *v == '"' || *v == '\n' || *v == '\r')
                    { need_quote = true; break; }

            if (need_quote) {
                fputc('"', f);
                for (const char *v = val; *v; v++) { if (*v == '"') fputc('"', f); fputc(*v, f); }
                fputc('"', f);
            } else {
                fputs(val, f);
            }
            if (c < last_col) fputc(',', f);
        }
        fputs("\r\n", f);
    }

    fclose(f);
    return CSV_OK;
}
```

- [ ] **Step 4: Run tests — expect all pass**

```bat
make test_csv.exe
.\test_csv.exe
```

Expected: `6/6 tests passed`

- [ ] **Step 5: Commit**

```bat
git add csv.c test/test_csv.c
git commit -m "feat: implement CSV module with RFC 4180 support and unit tests"
```

---

### Task 4: Formula Engine (TDD)

**Files:**
- Create: `test/test_formula.c`
- Modify: `formula.c` (full implementation)

- [ ] **Step 1: Write failing tests — `test/test_formula.c`**

```c
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../grid.h"
#include "../formula.h"

static int tests_run = 0, tests_passed = 0;
#define RUN(name) do { tests_run++; test_##name(); tests_passed++; \
                       printf("PASS: " #name "\n"); } while(0)

static void set_num(int r, int c, const char *v) {
    grid_set_cell(r, c, v);
    Cell *cell = grid[r][c];
    if (cell) { strncpy(cell->display, v, MAX_CELL-1); cell->dirty = false; }
}

static void test_arithmetic(void) {
    grid_init();
    grid_set_cell(0, 0, "=1+2*3");
    formula_eval_all();
    assert(strcmp(grid[0][0]->display, "7") == 0);
}

static void test_cell_ref(void) {
    grid_init();
    set_num(0, 1, "10");
    grid_set_cell(0, 0, "=B1*2");
    formula_eval_all();
    assert(strcmp(grid[0][0]->display, "20") == 0);
}

static void test_sum(void) {
    grid_init();
    set_num(0, 0, "1"); set_num(1, 0, "2"); set_num(2, 0, "3");
    grid_set_cell(3, 0, "=SUM(A1:A3)");
    formula_eval_all();
    assert(strcmp(grid[3][0]->display, "6") == 0);
}

static void test_avg(void) {
    grid_init();
    set_num(0, 0, "4"); set_num(1, 0, "8");
    grid_set_cell(2, 0, "=AVG(A1:A2)");
    formula_eval_all();
    assert(strcmp(grid[2][0]->display, "6") == 0);
}

static void test_min_max(void) {
    grid_init();
    set_num(0, 0, "5"); set_num(1, 0, "2"); set_num(2, 0, "8");
    grid_set_cell(3, 0, "=MIN(A1:A3)");
    grid_set_cell(4, 0, "=MAX(A1:A3)");
    formula_eval_all();
    assert(strcmp(grid[3][0]->display, "2") == 0);
    assert(strcmp(grid[4][0]->display, "8") == 0);
}

static void test_count(void) {
    grid_init();
    set_num(0, 0, "1"); set_num(1, 0, "2");
    grid_set_cell(0, 1, "text");
    Cell *t = grid[0][1]; if (t) { strncpy(t->display, "text", MAX_CELL-1); t->dirty = false; }
    grid_set_cell(2, 0, "=COUNT(A1:A2)");
    formula_eval_all();
    assert(strcmp(grid[2][0]->display, "2") == 0);
}

static void test_div_zero(void) {
    grid_init();
    grid_set_cell(0, 0, "=1/0");
    formula_eval_all();
    assert(strcmp(grid[0][0]->display, "#DIV0!") == 0);
}

static void test_circular(void) {
    grid_init();
    grid_set_cell(0, 0, "=A1+1");  /* self-reference */
    formula_eval_all();
    assert(grid[0][0]->display[0] == '#');
}

static void test_parse_error(void) {
    grid_init();
    grid_set_cell(0, 0, "=@@@");
    formula_eval_all();
    assert(strcmp(grid[0][0]->display, "#ERR!") == 0);
}

static void test_parentheses(void) {
    grid_init();
    grid_set_cell(0, 0, "=(2+3)*4");
    formula_eval_all();
    assert(strcmp(grid[0][0]->display, "20") == 0);
}

int main(void) {
    RUN(arithmetic); RUN(cell_ref); RUN(sum); RUN(avg);
    RUN(min_max); RUN(count); RUN(div_zero); RUN(circular);
    RUN(parse_error); RUN(parentheses);
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
```

- [ ] **Step 2: Verify tests fail**

```bat
make test_formula.exe
.\test_formula.exe
```

Expected: assertion failures (stubs do nothing).

- [ ] **Step 3: Implement `formula.c` fully**

```c
#include "formula.h"
#include "grid.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static bool in_eval[MAX_ROWS][MAX_COLS];

/* Forward declaration needed because eval_range_func calls formula_eval_cell */
void formula_eval_cell(int row, int col);

typedef struct {
    const char *src;
    int         pos;
    bool        has_err;
    char        err[8];
} Parser;

static double parse_expr(Parser *p);
static double parse_term(Parser *p);
static double parse_factor(Parser *p);

static void skip_ws(Parser *p) { while (p->src[p->pos] == ' ') p->pos++; }

static void set_err(Parser *p, const char *e) { p->has_err = true; strncpy(p->err, e, 7); }

static double eval_range_func(Parser *p, const char *fn) {
    skip_ws(p);
    if (p->src[p->pos++] != '(') { set_err(p, "#ERR!"); return 0; }

    skip_ws(p);
    int s1 = p->pos;
    while (isupper((unsigned char)p->src[p->pos])) p->pos++;
    while (isdigit((unsigned char)p->src[p->pos])) p->pos++;
    char start_addr[8] = {0}; strncpy(start_addr, p->src + s1, MIN(p->pos - s1, 7));

    skip_ws(p);
    if (p->src[p->pos++] != ':') { set_err(p, "#ERR!"); return 0; }

    skip_ws(p);
    int s2 = p->pos;
    while (isupper((unsigned char)p->src[p->pos])) p->pos++;
    while (isdigit((unsigned char)p->src[p->pos])) p->pos++;
    char end_addr[8] = {0}; strncpy(end_addr, p->src + s2, MIN(p->pos - s2, 7));

    skip_ws(p);
    if (p->src[p->pos++] != ')') { set_err(p, "#ERR!"); return 0; }

    int r1, c1, r2, c2;
    if (!parse_cell_addr(start_addr, &r1, &c1) || !parse_cell_addr(end_addr, &r2, &c2))
        { set_err(p, "#REF!"); return 0; }

    double sum = 0, mn = 1e300, mx = -1e300;
    int cnt = 0;
    for (int r = r1; r <= r2 && r < MAX_ROWS; r++) {
        for (int c = c1; c <= c2 && c < MAX_COLS; c++) {
            Cell *cell = grid[r][c];
            if (!cell) continue;
            if (cell->type == CELL_FORMULA && cell->dirty) formula_eval_cell(r, c);
            char *end;
            double v = strtod(cell->display, &end);
            if (end != cell->display && *end == '\0') {
                sum += v; cnt++;
                if (v < mn) mn = v;
                if (v > mx) mx = v;
            }
        }
    }

    if (strcmp(fn, "SUM")   == 0) return sum;
    if (strcmp(fn, "AVG")   == 0) return cnt ? sum / cnt : 0;
    if (strcmp(fn, "MIN")   == 0) return cnt ? mn : 0;
    if (strcmp(fn, "MAX")   == 0) return cnt ? mx : 0;
    if (strcmp(fn, "COUNT") == 0) return cnt;
    set_err(p, "#ERR!"); return 0;
}

static double parse_factor(Parser *p) {
    if (p->has_err) return 0;
    skip_ws(p);
    char c = p->src[p->pos];

    if (c == '-') { p->pos++; return -parse_factor(p); }
    if (c == '(') {
        p->pos++;
        double v = parse_expr(p);
        skip_ws(p);
        if (p->src[p->pos] == ')') p->pos++;
        return v;
    }
    if (isupper((unsigned char)c)) {
        int start = p->pos;
        while (isupper((unsigned char)p->src[p->pos])) p->pos++;

        if (isdigit((unsigned char)p->src[p->pos])) {
            /* cell reference */
            while (isdigit((unsigned char)p->src[p->pos])) p->pos++;
            char addr[8] = {0}; strncpy(addr, p->src + start, MIN(p->pos - start, 7));
            int row, col;
            if (!parse_cell_addr(addr, &row, &col)) { set_err(p, "#REF!"); return 0; }
            if (in_eval[row][col]) { set_err(p, "#CIRC!"); return 0; }
            Cell *cell = grid[row][col];
            if (!cell) return 0;
            if (cell->type == CELL_FORMULA && cell->dirty) formula_eval_cell(row, col);
            if (cell->display[0] == '#') { set_err(p, cell->display[1]=='C'?"#CIRC!":"#ERR!"); return 0; }
            char *end;
            double v = strtod(cell->display, &end);
            if (end == cell->display || *end != '\0') { set_err(p, "#VAL!"); return 0; }
            return v;
        }
        /* function name */
        char fname[8] = {0}; strncpy(fname, p->src + start, MIN(p->pos - start, 7));
        skip_ws(p);
        if (p->src[p->pos] == '(') return eval_range_func(p, fname);
        set_err(p, "#ERR!"); return 0;
    }
    if (isdigit((unsigned char)c) || c == '.') {
        char *end;
        double v = strtod(p->src + p->pos, &end);
        p->pos = (int)(end - p->src);
        return v;
    }
    set_err(p, "#ERR!"); return 0;
}

static double parse_term(Parser *p) {
    double v = parse_factor(p);
    skip_ws(p);
    while (!p->has_err && (p->src[p->pos] == '*' || p->src[p->pos] == '/')) {
        char op = p->src[p->pos++];
        double r = parse_factor(p); skip_ws(p);
        if (op == '*') v *= r;
        else { if (r == 0.0) { set_err(p, "#DIV0!"); return 0; } v /= r; }
    }
    return v;
}

static double parse_expr(Parser *p) {
    double v = parse_term(p);
    skip_ws(p);
    while (!p->has_err && (p->src[p->pos] == '+' || p->src[p->pos] == '-')) {
        char op = p->src[p->pos++];
        double r = parse_term(p); skip_ws(p);
        v = (op == '+') ? v + r : v - r;
    }
    return v;
}

void formula_eval_cell(int row, int col) {
    Cell *cell = grid[row][col];
    if (!cell || cell->type != CELL_FORMULA || !cell->dirty) return;

    in_eval[row][col] = true;
    Parser p = { .src = cell->raw + 1, .pos = 0, .has_err = false };
    double result = parse_expr(&p);
    if (p.has_err) {
        strncpy(cell->display, p.err, MAX_CELL - 1);
    } else {
        if (result == (long long)result)
            snprintf(cell->display, MAX_CELL, "%lld", (long long)result);
        else
            snprintf(cell->display, MAX_CELL, "%.6g", result);
    }
    cell->dirty = false;
    in_eval[row][col] = false;
}

void formula_eval_all(void) {
    for (int r = 0; r <= last_row; r++)
        for (int c = 0; c <= last_col; c++)
            formula_eval_cell(r, c);
}
```

- [ ] **Step 4: Run tests — expect all pass**

```bat
make test_formula.exe
.\test_formula.exe
```

Expected: `10/10 tests passed`

- [ ] **Step 5: Run all tests to confirm no regressions**

```bat
make test
```

Expected: all three test binaries exit 0.

- [ ] **Step 6: Commit**

```bat
git add formula.c test/test_formula.c
git commit -m "feat: implement formula engine with recursive descent parser and unit tests"
```

---

### Task 5: Theme Module

**Files:**
- Modify: `theme.c` (full implementation)

- [ ] **Step 1: Implement `theme.c`**

```c
#include "theme.h"
#include <curses.h>
#include <string.h>

typedef struct {
    const char *name;
    short normal_fg,  normal_bg;
    short current_fg, current_bg;
    short formula_fg, formula_bg;
    short number_fg,  number_bg;
    short visual_fg,  visual_bg;
    short header_fg,  header_bg;
    short stn_fg,     stn_bg;   /* status NORMAL */
    short sti_fg,     sti_bg;   /* status INSERT */
    short stv_fg,     stv_bg;   /* status VISUAL */
    short stc_fg,     stc_bg;   /* status COMMAND */
    short fb_fg,      fb_bg;    /* formula bar */
} Theme;

static const Theme themes[] = {
    { "default",
      COLOR_WHITE,  COLOR_BLACK,   COLOR_BLACK,   COLOR_CYAN,
      COLOR_YELLOW, COLOR_BLACK,   COLOR_GREEN,   COLOR_BLACK,
      COLOR_BLACK,  COLOR_BLUE,    COLOR_BLACK,   COLOR_WHITE,
      COLOR_BLACK,  COLOR_GREEN,   COLOR_BLACK,   COLOR_RED,
      COLOR_BLACK,  COLOR_BLUE,    COLOR_BLACK,   COLOR_YELLOW,
      COLOR_WHITE,  COLOR_BLACK },
    { "solarized",
      COLOR_WHITE,  COLOR_BLACK,   COLOR_BLACK,   COLOR_BLUE,
      COLOR_YELLOW, COLOR_BLACK,   COLOR_CYAN,    COLOR_BLACK,
      COLOR_BLACK,  COLOR_MAGENTA, COLOR_BLACK,   COLOR_WHITE,
      COLOR_BLACK,  COLOR_CYAN,    COLOR_BLACK,   COLOR_MAGENTA,
      COLOR_BLACK,  COLOR_BLUE,    COLOR_BLACK,   COLOR_YELLOW,
      COLOR_WHITE,  COLOR_BLACK },
    { "monokai",
      COLOR_WHITE,  COLOR_BLACK,   COLOR_BLACK,   COLOR_MAGENTA,
      COLOR_YELLOW, COLOR_BLACK,   COLOR_GREEN,   COLOR_BLACK,
      COLOR_WHITE,  COLOR_BLUE,    COLOR_BLACK,   COLOR_WHITE,
      COLOR_BLACK,  COLOR_GREEN,   COLOR_WHITE,   COLOR_RED,
      COLOR_WHITE,  COLOR_BLUE,    COLOR_BLACK,   COLOR_YELLOW,
      COLOR_YELLOW, COLOR_BLACK },
};

static int current_theme = 0;

static void apply_theme(int idx) {
    const Theme *t = &themes[idx];
    init_pair(CP_NORMAL,     t->normal_fg,  t->normal_bg);
    init_pair(CP_CURRENT,    t->current_fg, t->current_bg);
    init_pair(CP_FORMULA,    t->formula_fg, t->formula_bg);
    init_pair(CP_NUMBER,     t->number_fg,  t->number_bg);
    init_pair(CP_VISUAL,     t->visual_fg,  t->visual_bg);
    init_pair(CP_HEADER,     t->header_fg,  t->header_bg);
    init_pair(CP_STATUS_N,   t->stn_fg,     t->stn_bg);
    init_pair(CP_STATUS_I,   t->sti_fg,     t->sti_bg);
    init_pair(CP_STATUS_V,   t->stv_fg,     t->stv_bg);
    init_pair(CP_STATUS_CMD, t->stc_fg,     t->stc_bg);
    init_pair(CP_FORMULABAR, t->fb_fg,      t->fb_bg);
}

void theme_init(void) { apply_theme(0); }

int theme_set(const char *name) {
    for (int i = 0; i < (int)(sizeof(themes)/sizeof(themes[0])); i++) {
        if (strcmp(themes[i].name, name) == 0) {
            current_theme = i;
            apply_theme(i);
            return 1;
        }
    }
    return 0;
}
```

- [ ] **Step 2: Verify it compiles**

```bat
make
```

Expected: `visheet.exe` builds cleanly.

- [ ] **Step 3: Commit**

```bat
git add theme.c
git commit -m "feat: implement theme module with default/solarized/monokai themes"
```

---

### Task 6: Render Module

**Files:**
- Modify: `render.c` (full implementation)

- [ ] **Step 1: Implement `render.c`**

```c
#include "render.h"
#include "grid.h"
#include "formula.h"
#include "theme.h"
#include <curses.h>
#include <string.h>
#include <stdio.h>

/* Row 0: formula bar | Row 1: column headers | Rows 2..LINES-2: data | Row LINES-1: status */

static void render_formula_bar(const AppState *s) {
    char addr[10];
    cell_addr(s->cursor_row, s->cursor_col, addr, sizeof(addr));
    Cell *cell = grid[s->cursor_row][s->cursor_col];
    const char *content = "";
    if (s->mode == MODE_INSERT) content = s->cell_buf;
    else if (cell) content = cell->raw;

    attron(COLOR_PAIR(CP_FORMULABAR) | A_BOLD);
    mvprintw(0, 0, " %-6s: %-*.*s", addr, COLS - 10, COLS - 10, content);
    attroff(COLOR_PAIR(CP_FORMULABAR) | A_BOLD);
}

static void render_col_headers(const AppState *s) {
    int vis = (COLS - 5) / (COL_WIDTH + 1);
    attron(COLOR_PAIR(CP_HEADER) | A_BOLD);
    mvprintw(1, 0, "    |");
    for (int vc = 0; vc < vis; vc++) {
        int col = s->viewport_col + vc;
        if (col >= MAX_COLS) break;
        char cbuf[4];
        col_to_str(col, cbuf, sizeof(cbuf));
        mvprintw(1, 5 + vc * (COL_WIDTH + 1), "%-*s|", COL_WIDTH, cbuf);
    }
    attroff(COLOR_PAIR(CP_HEADER) | A_BOLD);
}

static void render_grid(const AppState *s) {
    int data_rows = LINES - 3;
    int vis_cols  = (COLS - 5) / (COL_WIDTH + 1);

    for (int vr = 0; vr < data_rows; vr++) {
        int row = s->viewport_row + vr;
        if (row >= MAX_ROWS) break;
        int y = 2 + vr;

        attron(COLOR_PAIR(CP_HEADER));
        mvprintw(y, 0, "%4d|", row + 1);
        attroff(COLOR_PAIR(CP_HEADER));

        for (int vc = 0; vc < vis_cols; vc++) {
            int col = s->viewport_col + vc;
            if (col >= MAX_COLS) break;
            int x = 5 + vc * (COL_WIDTH + 1);

            Cell *cell = grid[row][col];
            const char *text = (cell && cell->display[0]) ? cell->display : "";

            int pair;
            bool is_cursor = (row == s->cursor_row && col == s->cursor_col);
            bool is_visual = false;
            if (s->mode == MODE_VISUAL) {
                int r1 = MIN(s->visual_start_row, s->cursor_row);
                int r2 = MAX(s->visual_start_row, s->cursor_row);
                int c1 = MIN(s->visual_start_col, s->cursor_col);
                int c2 = MAX(s->visual_start_col, s->cursor_col);
                is_visual = (row >= r1 && row <= r2 && col >= c1 && col <= c2);
            }

            if      (is_cursor)                            pair = CP_CURRENT;
            else if (is_visual)                            pair = CP_VISUAL;
            else if (cell && cell->type == CELL_FORMULA)   pair = CP_FORMULA;
            else if (cell && cell->type == CELL_NUMBER)    pair = CP_NUMBER;
            else                                           pair = CP_NORMAL;

            attron(COLOR_PAIR(pair));
            char disp[COL_WIDTH + 2];
            int tlen = (int)strlen(text);
            if (tlen > COL_WIDTH) {
                strncpy(disp, text, COL_WIDTH - 1);
                disp[COL_WIDTH - 1] = '~';
                disp[COL_WIDTH]     = '\0';
            } else {
                snprintf(disp, sizeof(disp), "%-*s", COL_WIDTH, text);
            }
            mvaddstr(y, x, disp);
            mvaddch(y, x + COL_WIDTH, '|');
            attroff(COLOR_PAIR(pair));
        }
    }
}

static void render_status_bar(const AppState *s) {
    int pair; const char *mstr;
    switch (s->mode) {
        case MODE_NORMAL:  pair = CP_STATUS_N;   mstr = "NORMAL";  break;
        case MODE_INSERT:  pair = CP_STATUS_I;   mstr = "INSERT";  break;
        case MODE_VISUAL:  pair = CP_STATUS_V;   mstr = "VISUAL";  break;
        case MODE_COMMAND: pair = CP_STATUS_CMD; mstr = "COMMAND"; break;
        default:           pair = CP_STATUS_N;   mstr = "NORMAL";  break;
    }
    char addr[10]; cell_addr(s->cursor_row, s->cursor_col, addr, sizeof(addr));
    const char *fn = s->filename[0] ? s->filename : "[No File]";
    const char *dm = s->dirty ? " [+]" : "";

    attron(COLOR_PAIR(pair));
    mvprintw(LINES - 1, 0, " %-7s | %s%s | %s%*s", mstr, fn, dm, addr, COLS, "");
    attroff(COLOR_PAIR(pair));
}

static void render_command_bar(const AppState *s) {
    attron(COLOR_PAIR(CP_STATUS_CMD));
    mvprintw(LINES - 1, 0, ":%-*s", COLS - 2, s->cmd_buf);
    attroff(COLOR_PAIR(CP_STATUS_CMD));
    move(LINES - 1, 1 + s->cmd_len);
}

static void position_cursor(const AppState *s) {
    int vr = s->cursor_row - s->viewport_row;
    int vc = s->cursor_col - s->viewport_col;
    if (vr >= 0 && vc >= 0 && vr < LINES - 3)
        move(2 + vr, 5 + vc * (COL_WIDTH + 1));
}

void render_all(const AppState *s) {
    formula_eval_all();
    erase();
    render_formula_bar(s);
    render_col_headers(s);
    render_grid(s);

    if (s->mode == MODE_COMMAND) {
        render_command_bar(s);
    } else {
        render_status_bar(s);
        if (s->status_msg[0]) {
            attron(A_BOLD | A_REVERSE);
            mvprintw(LINES - 1, 0, " %s ", s->status_msg);
            attroff(A_BOLD | A_REVERSE);
        }
        position_cursor(s);
    }
    refresh();
}
```

- [ ] **Step 2: Verify it compiles**

```bat
make
```

Expected: builds cleanly.

- [ ] **Step 3: Commit**

```bat
git add render.c
git commit -m "feat: implement render module with formula bar, viewport, and status bar"
```

---

### Task 7: Vi Mode State Machine

**Files:**
- Modify: `vi_mode.c` (full implementation)

- [ ] **Step 1: Implement `vi_mode.c`**

```c
#include "vi_mode.h"
#include "grid.h"
#include "csv.h"
#include "formula.h"
#include "theme.h"
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void vi_init(AppState *s) {
    memset(s, 0, sizeof(*s));
    s->mode         = MODE_NORMAL;
    s->needs_redraw = true;
}

static void adjust_viewport(AppState *s) {
    int data_rows = LINES - 3;
    int vis_cols  = (COLS - 5) / (COL_WIDTH + 1);

    if (s->cursor_row < s->viewport_row) s->viewport_row = s->cursor_row;
    if (s->cursor_row >= s->viewport_row + data_rows) s->viewport_row = s->cursor_row - data_rows + 1;
    if (s->cursor_col < s->viewport_col) s->viewport_col = s->cursor_col;
    if (s->cursor_col >= s->viewport_col + vis_cols) s->viewport_col = s->cursor_col - vis_cols + 1;
    if (s->viewport_row < 0) s->viewport_row = 0;
    if (s->viewport_col < 0) s->viewport_col = 0;
}

static void enter_insert(AppState *s, bool append) {
    s->mode = MODE_INSERT;
    Cell *cell = grid[s->cursor_row][s->cursor_col];
    if (append && cell) {
        strncpy(s->cell_buf, cell->raw, MAX_CELL - 1);
        s->cell_buf_len = (int)strlen(s->cell_buf);
    } else {
        s->cell_buf[0]  = '\0';
        s->cell_buf_len = 0;
    }
}

static void commit_cell(AppState *s) {
    if (s->cell_buf_len == 0) { grid_clear_cell(s->cursor_row, s->cursor_col); return; }
    grid_set_cell(s->cursor_row, s->cursor_col, s->cell_buf);
    grid_mark_formulas_dirty();
    s->dirty = true;
}

static void paste_row(AppState *s) {
    if (!s->yank_valid) { snprintf(s->status_msg, 256, "Nothing to paste"); return; }
    int trow = MIN(s->cursor_row + 1, MAX_ROWS - 1);
    for (int c = 0; c < MAX_COLS; c++) {
        if (s->yank_row_data[c][0]) {
            grid_set_cell(trow, c, s->yank_row_data[c]);
        } else {
            grid_clear_cell(trow, c);
        }
    }
    grid_mark_formulas_dirty();
    s->dirty = true;
    s->cursor_row = trow;
    adjust_viewport(s);
}

static void search_next(AppState *s, bool reverse) {
    if (!s->search_pat[0]) return;
    int dr = reverse ? -1 : 1;
    int r  = s->cursor_row, c = s->cursor_col + dr;
    if (c < 0) { c = last_col; r--; }
    if (c > last_col) { c = 0; r++; }
    if (r < 0) r = last_row;
    if (r > last_row) r = 0;

    int sr = s->cursor_row, sc = s->cursor_col;
    do {
        Cell *cell = grid[r][c];
        if (cell && strstr(cell->display, s->search_pat)) {
            s->cursor_row = r; s->cursor_col = c; adjust_viewport(s); return;
        }
        c += dr;
        if (c < 0) { c = last_col; r--; }
        if (c > last_col) { c = 0; r++; }
        if (r < 0) r = last_row;
        if (r > last_row) r = 0;
    } while (r != sr || c != sc);
    snprintf(s->status_msg, 256, "Pattern not found: %s", s->search_pat);
}

static void sort_rows(AppState *s, const char *arg) {
    bool desc = (arg[0] == '!');
    if (desc) arg++;
    while (*arg == ' ') arg++;
    int sort_col = *arg ? str_to_col(arg) : 0;
    if (sort_col < 0 || sort_col >= MAX_COLS) { snprintf(s->status_msg, 256, "E: bad column"); return; }

    for (int i = 0; i < last_row; i++) {
        for (int j = 0; j < last_row - i; j++) {
            const char *va = grid[j][sort_col]   ? grid[j][sort_col]->display   : "";
            const char *vb = grid[j+1][sort_col] ? grid[j+1][sort_col]->display : "";
            int cmp = strcmp(va, vb);
            if ((desc && cmp < 0) || (!desc && cmp > 0)) {
                for (int c = 0; c < MAX_COLS; c++) {
                    Cell *tmp       = grid[j][c];
                    grid[j][c]      = grid[j+1][c];
                    grid[j+1][c]    = tmp;
                }
            }
        }
    }
    grid_mark_formulas_dirty();
    s->dirty = true;
    snprintf(s->status_msg, 256, "Sorted%s", desc ? " (desc)" : "");
}

static void execute_command(AppState *s) {
    const char *cmd = s->cmd_buf;
    s->mode = MODE_NORMAL;

    if (cmd[0] == '/') {
        strncpy(s->search_pat, cmd + 1, MAX_CELL - 1);
        search_next(s, false);
        return;
    }
    if (strcmp(cmd, "q") == 0) {
        if (s->dirty) { snprintf(s->status_msg, 256, "E: unsaved changes, use :q!"); return; }
        endwin(); exit(0);
    }
    if (strcmp(cmd, "q!") == 0) { endwin(); exit(0); }

    /* :w [file] */
    if (strncmp(cmd, "w", 1) == 0 && (cmd[1] == '\0' || cmd[1] == ' ') && cmd[0] != 'q') {
        const char *path = (cmd[1] == ' ') ? cmd + 2 : s->filename;
        if (!path[0]) { snprintf(s->status_msg, 256, "E: no filename"); return; }
        if (csv_save(path) == CSV_OK) {
            strncpy(s->filename, path, 511);
            s->dirty = false;
            snprintf(s->status_msg, 256, "Written: %s", path);
        } else snprintf(s->status_msg, 256, "E: cannot write %s", path);
        return;
    }
    /* :wq */
    if (strncmp(cmd, "wq", 2) == 0) {
        const char *path = (cmd[2] == ' ') ? cmd + 3 : s->filename;
        if (!path[0]) { snprintf(s->status_msg, 256, "E: no filename"); return; }
        if (csv_save(path) == CSV_OK) { endwin(); exit(0); }
        snprintf(s->status_msg, 256, "E: cannot write %s", path);
        return;
    }
    /* :e! [file] */
    if (strncmp(cmd, "e!", 2) == 0) {
        const char *path = (cmd[2] == ' ') ? cmd + 3 : s->filename;
        if (!path[0]) { snprintf(s->status_msg, 256, "E: no filename"); return; }
        if (csv_load(path) == CSV_OK) { strncpy(s->filename, path, 511); s->dirty = false; formula_eval_all(); }
        else snprintf(s->status_msg, 256, "E: cannot open %s", path);
        return;
    }
    /* :e [file] */
    if (strncmp(cmd, "e", 1) == 0 && (cmd[1] == '\0' || cmd[1] == ' ')) {
        if (s->dirty) { snprintf(s->status_msg, 256, "E: unsaved changes, use :e!"); return; }
        const char *path = (cmd[1] == ' ') ? cmd + 2 : s->filename;
        if (!path[0]) { snprintf(s->status_msg, 256, "E: no filename"); return; }
        if (csv_load(path) == CSV_OK) { strncpy(s->filename, path, 511); s->dirty = false; formula_eval_all(); }
        else snprintf(s->status_msg, 256, "E: cannot open %s", path);
        return;
    }
    /* :sort[!] [col] */
    if (strncmp(cmd, "sort", 4) == 0) { sort_rows(s, cmd + 4); return; }
    /* :goto ADDR */
    if (strncmp(cmd, "goto ", 5) == 0) {
        int row, col;
        if (parse_cell_addr(cmd + 5, &row, &col)) { s->cursor_row = row; s->cursor_col = col; adjust_viewport(s); }
        else snprintf(s->status_msg, 256, "E: bad address");
        return;
    }
    /* :theme NAME */
    if (strncmp(cmd, "theme ", 6) == 0) {
        if (!theme_set(cmd + 6)) snprintf(s->status_msg, 256, "E: unknown theme '%s'", cmd + 6);
        return;
    }
    /* :sum RANGE */
    if (strncmp(cmd, "sum ", 4) == 0) {
        char range[32]; strncpy(range, cmd + 4, 31);
        char *colon = strchr(range, ':');
        if (colon) {
            *colon = '\0';
            int r1, c1, r2, c2;
            if (parse_cell_addr(range, &r1, &c1) && parse_cell_addr(colon + 1, &r2, &c2)) {
                double sum = 0;
                for (int r = r1; r <= r2; r++)
                    for (int c = c1; c <= c2; c++) {
                        Cell *cell = grid[r][c];
                        if (cell) { char *e; double v = strtod(cell->display, &e); if (e != cell->display) sum += v; }
                    }
                snprintf(s->status_msg, 256, "Sum = %.6g", sum);
            }
        }
        return;
    }
    snprintf(s->status_msg, 256, "E: unknown command: %s", cmd);
}

static void handle_insert(AppState *s, int key) {
    switch (key) {
        case 27:                                       /* ESC */
            commit_cell(s); s->mode = MODE_NORMAL; break;
        case '\r': case '\n': case KEY_ENTER:
            commit_cell(s); s->mode = MODE_NORMAL;
            s->cursor_row = MIN(MAX_ROWS - 1, s->cursor_row + 1); adjust_viewport(s); break;
        case '\t':
            commit_cell(s); s->mode = MODE_NORMAL;
            s->cursor_col = MIN(MAX_COLS - 1, s->cursor_col + 1); adjust_viewport(s); break;
        case KEY_BACKSPACE: case 127: case 8:
            if (s->cell_buf_len > 0) s->cell_buf[--s->cell_buf_len] = '\0'; break;
        default:
            if (key >= 32 && key < 127 && s->cell_buf_len < MAX_CELL - 1) {
                s->cell_buf[s->cell_buf_len++] = (char)key;
                s->cell_buf[s->cell_buf_len]   = '\0';
            }
    }
}

static void handle_visual(AppState *s, int key) {
    switch (key) {
        case 27: s->mode = MODE_NORMAL; break;
        case 'h': s->cursor_col = MAX(0, s->cursor_col - 1); break;
        case 'j': s->cursor_row = MIN(MAX_ROWS-1, s->cursor_row + 1); break;
        case 'k': s->cursor_row = MAX(0, s->cursor_row - 1); break;
        case 'l': s->cursor_col = MIN(MAX_COLS-1, s->cursor_col + 1); break;
        case 'y': {
            int r1 = MIN(s->visual_start_row, s->cursor_row);
            for (int c = 0; c < MAX_COLS; c++) {
                Cell *cell = grid[r1][c];
                s->yank_row_data[c][0] = '\0';
                if (cell) strncpy(s->yank_row_data[c], cell->raw, MAX_CELL - 1);
            }
            s->yank_valid = true; s->mode = MODE_NORMAL; break;
        }
        case 'd': {
            int r1 = MIN(s->visual_start_row, s->cursor_row);
            int r2 = MAX(s->visual_start_row, s->cursor_row);
            int c1 = MIN(s->visual_start_col, s->cursor_col);
            int c2 = MAX(s->visual_start_col, s->cursor_col);
            for (int r = r1; r <= r2; r++)
                for (int c = c1; c <= c2; c++)
                    grid_clear_cell(r, c);
            grid_mark_formulas_dirty(); s->dirty = true; s->mode = MODE_NORMAL; break;
        }
    }
    adjust_viewport(s);
}

static void handle_command(AppState *s, int key) {
    switch (key) {
        case 27: s->mode = MODE_NORMAL; s->cmd_buf[0] = '\0'; s->cmd_len = 0; break;
        case '\r': case '\n': case KEY_ENTER:
            s->cmd_buf[s->cmd_len] = '\0'; execute_command(s); break;
        case KEY_BACKSPACE: case 127: case 8:
            if (s->cmd_len > 0) s->cmd_buf[--s->cmd_len] = '\0';
            else { s->mode = MODE_NORMAL; } break;
        default:
            if (key >= 32 && key < 127 && s->cmd_len < 255) {
                s->cmd_buf[s->cmd_len++] = (char)key;
                s->cmd_buf[s->cmd_len]   = '\0';
            }
    }
}

static void handle_normal(AppState *s, int key) {
    /* Handle pending two-key sequences */
    if (s->pending_key) {
        int pk = s->pending_key; s->pending_key = 0;
        if (pk == 'g' && key == 'g') { s->cursor_row = 0; adjust_viewport(s); return; }
        if (pk == 'd' && key == 'd') {
            grid_clear_row(s->cursor_row); grid_mark_formulas_dirty(); s->dirty = true;
            snprintf(s->status_msg, 256, "Row cleared"); return;
        }
        if (pk == 'y' && key == 'y') {
            for (int c = 0; c < MAX_COLS; c++) {
                Cell *cell = grid[s->cursor_row][c];
                s->yank_row_data[c][0] = '\0';
                if (cell) strncpy(s->yank_row_data[c], cell->raw, MAX_CELL - 1);
            }
            s->yank_valid = true; snprintf(s->status_msg, 256, "1 row yanked"); return;
        }
        if (pk == 'r' && key >= 32 && key < 127) {
            char buf[2] = {(char)key, '\0'};
            grid_set_cell(s->cursor_row, s->cursor_col, buf);
            grid_mark_formulas_dirty(); s->dirty = true; return;
        }
        return; /* unrecognised sequence — ignore */
    }

    int cnt = s->count > 0 ? s->count : 1;
    s->count = 0;

    switch (key) {
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
            s->count = s->count * 10 + (key - '0'); return;

        case 'h': s->cursor_col = MAX(0,          s->cursor_col - cnt); break;
        case 'j': s->cursor_row = MIN(MAX_ROWS-1, s->cursor_row + cnt); break;
        case 'k': s->cursor_row = MAX(0,          s->cursor_row - cnt); break;
        case 'l': s->cursor_col = MIN(MAX_COLS-1, s->cursor_col + cnt); break;
        case 'w': s->cursor_col = MIN(MAX_COLS-1, s->cursor_col + cnt); break;
        case 'b': s->cursor_col = MAX(0,          s->cursor_col - cnt); break;
        case '0':
            if (s->count == 0) { s->cursor_col = 0; }
            else { s->count *= 10; return; }
            break;
        case '$': s->cursor_col = last_col; break;
        case 'G': s->cursor_row = last_row; break;
        case 'g': s->pending_key = 'g'; return;
        case 6:   s->cursor_row = MIN(MAX_ROWS-1, s->cursor_row + (LINES - 3)); break; /* Ctrl+F */
        case 2:   s->cursor_row = MAX(0,          s->cursor_row - (LINES - 3)); break; /* Ctrl+B */
        case 'i': enter_insert(s, false); return;
        case 'a': enter_insert(s, true);  return;
        case 'v':
            s->mode = MODE_VISUAL;
            s->visual_start_row = s->cursor_row;
            s->visual_start_col = s->cursor_col; return;
        case ':':
            s->mode = MODE_COMMAND;
            s->cmd_buf[0] = '\0'; s->cmd_len = 0; return;
        case '/':
            s->mode = MODE_COMMAND;
            s->cmd_buf[0] = '/'; s->cmd_buf[1] = '\0'; s->cmd_len = 1; return;
        case 'd': s->pending_key = 'd'; return;
        case 'y': s->pending_key = 'y'; return;
        case 'p': paste_row(s); return;
        case 'x': grid_clear_cell(s->cursor_row, s->cursor_col); grid_mark_formulas_dirty(); s->dirty = true; break;
        case 'r': s->pending_key = 'r'; return;
        case 'n': search_next(s, false); return;
        case 'N': search_next(s, true);  return;
        default: return;
    }
    adjust_viewport(s);
}

void vi_handle_key(AppState *s, int key) {
    s->needs_redraw  = true;
    s->status_msg[0] = '\0';
    switch (s->mode) {
        case MODE_NORMAL:  handle_normal(s,  key); break;
        case MODE_INSERT:  handle_insert(s,  key); break;
        case MODE_VISUAL:  handle_visual(s,  key); break;
        case MODE_COMMAND: handle_command(s, key); break;
    }
}
```

- [ ] **Step 2: Build to confirm compilation**

```bat
make
```

Expected: `visheet.exe` builds with zero errors.

- [ ] **Step 3: Commit**

```bat
git add vi_mode.c
git commit -m "feat: implement vi mode state machine with Normal/Insert/Visual/Command modes"
```

---

### Task 8: Input Module + Final Main

**Files:**
- Modify: `input.c` (complete)
- Modify: `main.c` (complete — already done in Task 1, verify it's correct)

- [ ] **Step 1: Verify `input.c` is complete**

```c
#include "input.h"
#include <curses.h>

int input_get_key(void) {
    return getch();
}
```

This is already done — no changes needed.

- [ ] **Step 2: Verify `main.c` is complete**

The `main.c` from Task 1, Step 11 is already the final version. Re-read it and confirm it matches:
- Calls `grid_init()`, `vi_init()`, loads argv[1] if given
- Calls `initscr()`, `has_colors()`, `start_color()`, `keypad()`, `noecho()`, `cbreak()`
- Calls `theme_init()`
- Enters `while(1)` loop: `render_all()` → `input_get_key()` → `vi_handle_key()`

- [ ] **Step 3: Build the complete binary**

```bat
make clean
make
```

Expected: `visheet.exe` produced with zero errors or warnings.

- [ ] **Step 4: Smoke test — launch with no arguments**

```bat
.\visheet.exe
```

Expected:
- Terminal switches to full-screen mode
- Formula bar at top, column headers A–whatever fits, row numbers on left
- Status bar at bottom shows `NORMAL | [No File] | A1`
- Press `j` — cursor moves to row 2
- Press `i`, type `hello`, press `Esc` — cell A2 shows `hello`
- Press `:q!` Enter — app exits cleanly

- [ ] **Step 5: Commit**

```bat
git add input.c main.c
git commit -m "feat: wire up input and main event loop — visheet.exe functional"
```

---

### Task 9: Test Data + Integration Testing

**Files:**
- Create: `test/basic.csv`, `test/formulas.csv`, `test/edge.csv`, `test/circular.csv`
- Create: `test/run_tests.bat`

- [ ] **Step 1: Create `test/basic.csv`**

```
name,age,city
Alice,30,London
Bob,25,Paris
Carol,35,Berlin
```

- [ ] **Step 2: Create `test/formulas.csv`**

```
value,result
10,
20,
30,
=SUM(A1:A3),
=AVG(A1:A3),
=MIN(A1:A3),
=MAX(A1:A3),
=COUNT(A1:A3),
```

- [ ] **Step 3: Create `test/edge.csv`**

```
"hello, world","say ""hi""","line1
line2",normal
1,2,3,4
```

*(The third field contains an embedded newline — valid RFC 4180)*

- [ ] **Step 4: Create `test/circular.csv`**

```
=A1+1
```

*(A1 references itself — should display `#CIRC!`)*

- [ ] **Step 5: Create `test/run_tests.bat`**

```bat
@echo off
setlocal

echo === Running unit tests ===
make test
if errorlevel 1 goto :fail

echo.
echo === Manual integration checklist ===
echo  1. Launch: visheet.exe test\basic.csv
echo     - Confirm: name/age/city header row visible, Alice/Bob/Carol data rows
echo     - Press :w test\out_basic.csv, then diff with fc test\basic.csv test\out_basic.csv
echo.
echo  2. Launch: visheet.exe test\formulas.csv
echo     - Confirm: SUM(A1:A3) cell shows 60, AVG shows 20, MIN shows 10, MAX shows 30, COUNT shows 3
echo.
echo  3. Launch: visheet.exe test\edge.csv
echo     - Confirm: "hello, world" renders without split, embedded quote shows correctly
echo.
echo  4. Launch: visheet.exe test\circular.csv
echo     - Confirm: A1 displays #CIRC!
echo.
echo  5. Vi mode spot checks (in any file):
echo     - 5j moves cursor 5 rows down
echo     - gg jumps to row 1
echo     - G jumps to last used row
echo     - dd clears a row (shows dirty [+] in status)
echo     - :sort B sorts by column B
echo     - :theme solarized changes colors
echo     - :theme monokai changes colors again
echo     - /Alice searches and jumps to the Alice cell
echo     - n finds next match

goto :done
:fail
echo UNIT TESTS FAILED
exit /b 1
:done
echo All unit tests passed. Complete manual checklist above.
```

- [ ] **Step 6: Run all unit tests**

```bat
test\run_tests.bat
```

Expected: `All unit tests passed. Complete manual checklist above.`

- [ ] **Step 7: Run manual checklist items 1–5** (open each file, verify behavior as described in the bat file above)

- [ ] **Step 8: Final commit**

```bat
git add test\basic.csv test\formulas.csv test\edge.csv test\circular.csv test\run_tests.bat
git commit -m "feat: add test data files and integration test runner"
```

---

## Done

After Task 9 completes, `visheet.exe` is a fully functional terminal spreadsheet with:
- Vim-style Normal / Insert / Visual / Command modes
- Scrollable 702×9999 grid with viewport tracking
- Basic formula engine: arithmetic, SUM, AVG, MIN, MAX, COUNT
- RFC 4180 CSV read/write
- Full color themes: default, solarized, monokai
- All unit tests passing
