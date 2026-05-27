#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>

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
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) return;
    free(grid[row][col]);
    grid[row][col] = NULL;
}

void grid_clear_row(int row) {
    for (int c = 0; c <= last_col; c++)
        grid_clear_cell(row, c);
}

void grid_set_cell(int row, int col, const char *raw) {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) return;
    if (!grid[row][col]) {
        grid[row][col] = calloc(1, sizeof(Cell));
        if (!grid[row][col]) return;
    }
    Cell *cell = grid[row][col];
    strncpy(cell->raw, raw, MAX_CELL - 1);
    cell->raw[MAX_CELL - 1] = '\0';

    if (raw[0] == '=') {
        cell->type = CELL_FORMULA;
        cell->display[0] = '\0';
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
    if (!s || !s[0]) return -1;
    int len = (int)strlen(s);
    if (len > 2) return -1;
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
    long rlong = strtol(s + i, NULL, 10);
    if (rlong < 1 || rlong > MAX_ROWS) return false;
    int r = (int)rlong - 1;
    int c = str_to_col(cbuf);
    if (c < 0 || c >= MAX_COLS) return false;
    *row = r; *col = c;
    return true;
}
