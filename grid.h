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
