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
