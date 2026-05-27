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
