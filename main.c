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
    static AppState state;
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
