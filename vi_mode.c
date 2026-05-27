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
        if (cell && (strstr(cell->display, s->search_pat) || strstr(cell->raw, s->search_pat))) {
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
    if (last_row <= 0) { snprintf(s->status_msg, 256, "E: nothing to sort"); return; }
    bool desc = (arg[0] == '!');
    if (desc) arg++;
    while (*arg == ' ') arg++;
    int sort_col = *arg ? str_to_col(arg) : 0;
    if (sort_col < 0 || sort_col >= MAX_COLS) { snprintf(s->status_msg, 256, "E: bad column"); return; }

    for (int i = 0; i < last_row; i++) {
        for (int j = 0; j < last_row - i; j++) {
            const char *va = grid[j][sort_col]   ? grid[j][sort_col]->display   : "";
            const char *vb = grid[j+1][sort_col] ? grid[j+1][sort_col]->display : "";
            char *ea, *eb;
            double da = strtod(va, &ea);
            double db = strtod(vb, &eb);
            int cmp;
            if (ea != va && eb != vb) {
                /* both numeric */
                cmp = (da > db) - (da < db);  /* -1, 0, or 1 */
            } else {
                cmp = strcmp(va, vb);
            }
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
            int r2 = MAX(s->visual_start_row, s->cursor_row);
            for (int c = 0; c < MAX_COLS; c++) {
                Cell *cell = grid[r1][c];
                s->yank_row_data[c][0] = '\0';
                if (cell) strncpy(s->yank_row_data[c], cell->raw, MAX_CELL - 1);
            }
            s->yank_valid = true;
            s->mode = MODE_NORMAL;
            if (r2 > r1)
                snprintf(s->status_msg, 256, "1 row yanked (multi-row yank not supported, yanked row %d)", r1 + 1);
            else
                snprintf(s->status_msg, 256, "1 row yanked");
            break;
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

    /* Digit accumulation — must happen before consuming count */
    switch (key) {
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
            s->count = s->count * 10 + (key - '0');
            return;
        case '0':
            if (s->count > 0) {
                s->count = s->count * 10; /* append 0 to multi-digit count */
                return;
            }
            /* s->count == 0: '0' means go to col A */
            s->cursor_col = 0;
            adjust_viewport(s);
            return;
        default:
            break;
    }

    /* Consume count for movement/action */
    int cnt = s->count > 0 ? s->count : 1;
    s->count = 0;

    switch (key) {
        case 'h': s->cursor_col = MAX(0,          s->cursor_col - cnt); break;
        case 'j': s->cursor_row = MIN(MAX_ROWS-1, s->cursor_row + cnt); break;
        case 'k': s->cursor_row = MAX(0,          s->cursor_row - cnt); break;
        case 'l': s->cursor_col = MIN(MAX_COLS-1, s->cursor_col + cnt); break;
        case 'w': s->cursor_col = MIN(MAX_COLS-1, s->cursor_col + cnt); break;
        case 'b': s->cursor_col = MAX(0,          s->cursor_col - cnt); break;
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
