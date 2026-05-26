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
const char *col_to_str(int col, char *buf, size_t buflen) { (void)col; (void)buflen; buf[0]='\0'; return buf; }
int  str_to_col(const char *s) { (void)s; return 0; }
void cell_addr(int row, int col, char *buf, size_t buflen) { (void)row; (void)col; (void)buflen; buf[0]='\0'; }
bool parse_cell_addr(const char *s, int *row, int *col) { (void)s; (void)row; (void)col; return false; }
