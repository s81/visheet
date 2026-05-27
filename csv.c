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
