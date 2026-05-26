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
