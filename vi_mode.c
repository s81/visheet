#include "vi_mode.h"
#include <string.h>
void vi_init(AppState *s) { memset(s, 0, sizeof(*s)); s->mode = MODE_NORMAL; s->needs_redraw = true; }
void vi_handle_key(AppState *s, int key) { (void)key; s->needs_redraw = true; }
