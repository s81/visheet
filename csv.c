#include "csv.h"
#include "grid.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

CsvError csv_load(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return CSV_ERR_FILE;
    grid_init();

    int row = 0, col = 0;
    char field[MAX_CELL];
    int fi = 0;
    int c;
    bool in_quotes = false;

    while ((c = fgetc(f)) != EOF) {
        if (in_quotes) {
            if (c == '"') {
                int next = fgetc(f);
                if (next == '"') {
                    /* escaped quote "" → " */
                    if (fi < MAX_CELL - 1) field[fi++] = '"';
                } else {
                    /* end of quoted field */
                    in_quotes = false;
                    /* put back so delimiter/newline is handled below */
                    ungetc(next, f);
                }
            } else {
                if (fi < MAX_CELL - 1) field[fi++] = c;
            }
        } else {
            if (c == '"' && fi == 0) {
                in_quotes = true;
            } else if (c == ',') {
                field[fi] = '\0';
                if (fi > 0 && row < MAX_ROWS && col < MAX_COLS)
                    grid_set_cell(row, col, field);
                fi = 0;
                col++;
            } else if (c == '\n') {
                /* end of record */
                field[fi] = '\0';
                if (fi > 0 && row < MAX_ROWS && col < MAX_COLS)
                    grid_set_cell(row, col, field);
                fi = 0;
                col = 0;
                row++;
            } else if (c == '\r') {
                /* skip \r in \r\n sequences */
                continue;
            } else {
                if (fi < MAX_CELL - 1) field[fi++] = c;
            }
        }
    }
    /* handle final field if file doesn't end with newline */
    if (fi > 0 && row < MAX_ROWS && col < MAX_COLS)
        grid_set_cell(row, col, field);

    fclose(f);
    return CSV_OK;
}

CsvError csv_save(const char *path) {
    FILE *f = fopen(path, "wb");
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

    if (ferror(f)) {
        fclose(f);
        return CSV_ERR_FILE;
    }
    fclose(f);
    return CSV_OK;
}
