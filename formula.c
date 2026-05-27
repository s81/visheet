#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "formula.h"
#include "grid.h"

#define MAX_DEPTH 32

typedef struct {
    const char *src;
    int         pos;
    bool        error;
    char        errtok[16];
} Parser;

static int eval_depth = 0;

static double parse_expr(Parser *p);

static void skip_ws(Parser *p) {
    while (p->src[p->pos] == ' ') p->pos++;
}

static double set_err(Parser *p, const char *tok) {
    p->error = true;
    snprintf(p->errtok, sizeof(p->errtok), "%s", tok);
    return 0.0;
}

static bool try_cell_ref(Parser *p, int *rout, int *cout) {
    int i = p->pos;
    if (!isupper((unsigned char)p->src[i])) return false;
    int cs = i;
    while (isupper((unsigned char)p->src[i])) i++;
    if (!isdigit((unsigned char)p->src[i])) return false;
    int rs = i;
    while (isdigit((unsigned char)p->src[i])) i++;
    int clen = rs - cs;
    if (clen < 1 || clen > 2) return false;
    char col_s[4] = {0};
    memcpy(col_s, p->src + cs, clen);
    int col = str_to_col(col_s);
    int row = (int)strtol(p->src + rs, NULL, 10) - 1;
    if (col < 0 || col >= MAX_COLS || row < 0 || row >= MAX_ROWS) return false;
    *cout = col;
    *rout = row;
    p->pos = i;
    return true;
}

static double cell_val(Parser *p, int row, int col) {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS)
        return set_err(p, "#REF!");
    if (!grid[row][col]) return 0.0;
    Cell *c = grid[row][col];
    if (c->type == CELL_FORMULA) {
        if (eval_depth >= MAX_DEPTH) return set_err(p, "#CIRC!");
        formula_eval_cell(row, col);
        /* If the display starts with '#', propagate the error */
        if (c->display[0] == '#') return set_err(p, c->display);
    }
    const char *disp = c->display[0] ? c->display : "0";
    char *end;
    double v = strtod(disp, &end);
    if (end == disp) return set_err(p, "#VAL!");
    return v;
}

static double eval_range(Parser *p, int func) {
    /* func: 0=SUM, 1=AVG, 2=MIN, 3=MAX, 4=COUNT */
    skip_ws(p);
    if (p->src[p->pos] != '(') return set_err(p, "#ERR!");
    p->pos++;
    int r1, c1, r2, c2;
    if (!try_cell_ref(p, &r1, &c1)) return set_err(p, "#ERR!");
    skip_ws(p);
    if (p->src[p->pos] != ':') return set_err(p, "#ERR!");
    p->pos++;
    skip_ws(p);
    if (!try_cell_ref(p, &r2, &c2)) return set_err(p, "#ERR!");
    skip_ws(p);
    if (p->src[p->pos] != ')') return set_err(p, "#ERR!");
    p->pos++;
    /* normalize range */
    int rlo = r1 < r2 ? r1 : r2, rhi = r1 < r2 ? r2 : r1;
    int clo = c1 < c2 ? c1 : c2, chi = c1 < c2 ? c2 : c1;
    double sum = 0.0, mn = 0.0, mx = 0.0;
    int count = 0;
    bool first = true;
    for (int r = rlo; r <= rhi && !p->error; r++) {
        for (int c = clo; c <= chi && !p->error; c++) {
            double v = cell_val(p, r, c);
            if (p->error) return 0.0;
            sum += v;
            count++;
            if (first || v < mn) mn = v;
            if (first || v > mx) mx = v;
            first = false;
        }
    }
    if (func == 1 && count == 0) return set_err(p, "#DIV0!");
    switch (func) {
        case 0: return sum;
        case 1: return sum / count;
        case 2: return mn;
        case 3: return mx;
        case 4: return (double)count;
        default: return 0.0;
    }
}

static double parse_factor(Parser *p) {
    if (p->error) return 0.0;
    skip_ws(p);

    /* unary minus / plus */
    if (p->src[p->pos] == '-') {
        p->pos++;
        return -parse_factor(p);
    }
    if (p->src[p->pos] == '+') {
        p->pos++;
        return parse_factor(p);
    }

    /* range functions: check keyword followed by '(' */
    static const char *funcs[] = {"SUM", "AVG", "MIN", "MAX", "COUNT"};
    for (int fi = 0; fi < 5; fi++) {
        int len = (int)strlen(funcs[fi]);
        if (strncmp(p->src + p->pos, funcs[fi], len) == 0 &&
            p->src[p->pos + len] == '(') {
            p->pos += len;
            return eval_range(p, fi);
        }
    }
    /* try cell reference (uppercase letters + digits) */
    int saved_pos = p->pos;
    int row, col;
    if (try_cell_ref(p, &row, &col)) {
        return cell_val(p, row, col);
    }
    p->pos = saved_pos;
    /* number literal */
    if (isdigit((unsigned char)p->src[p->pos]) || p->src[p->pos] == '.') {
        char *end;
        double v = strtod(p->src + p->pos, &end);
        if (end > p->src + p->pos) {
            p->pos += (int)(end - (p->src + p->pos));
            return v;
        }
    }
    /* parenthesized expression */
    if (p->src[p->pos] == '(') {
        p->pos++;
        double v = parse_expr(p);
        skip_ws(p);
        if (p->src[p->pos] == ')') p->pos++;
        return v;
    }
    return set_err(p, "#ERR!");
}

static double parse_term(Parser *p) {
    double v = parse_factor(p);
    while (!p->error) {
        skip_ws(p);
        char op = p->src[p->pos];
        if (op != '*' && op != '/') break;
        p->pos++;
        double r = parse_factor(p);
        if (p->error) return 0.0;
        if (op == '*') {
            v *= r;
        } else {
            if (r == 0.0) return set_err(p, "#DIV0!");
            v /= r;
        }
    }
    return v;
}

static double parse_expr(Parser *p) {
    double v = parse_term(p);
    while (!p->error) {
        skip_ws(p);
        char op = p->src[p->pos];
        if (op != '+' && op != '-') break;
        p->pos++;
        double r = parse_term(p);
        if (p->error) return 0.0;
        if (op == '+') v += r;
        else           v -= r;
    }
    return v;
}

void formula_eval_cell(int row, int col) {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) return;
    Cell *c = grid[row][col];
    if (!c || c->type != CELL_FORMULA) return;
    /* Skip if already evaluated and clean */
    if (!c->dirty && c->display[0] != '\0') return;

    /* Detect circular reference via global depth */
    if (eval_depth >= MAX_DEPTH) {
        snprintf(c->display, MAX_CELL, "#CIRC!");
        c->dirty = false;
        return;
    }

    const char *expr = c->raw;
    if (*expr == '=') expr++;

    /* Write sentinel so re-entrant cell_val sees '#' immediately */
    snprintf(c->display, MAX_CELL, "#CIRC!");

    eval_depth++;
    Parser p = { .src = expr, .pos = 0, .error = false, .errtok = {0} };
    double result = parse_expr(&p);
    eval_depth--;

    if (p.error) {
        snprintf(c->display, MAX_CELL, "%s", p.errtok);
    } else {
        /* Format: integer if whole number, %g otherwise */
        long long ival = (long long)result;
        if ((double)ival == result) {
            snprintf(c->display, MAX_CELL, "%lld", ival);
        } else {
            snprintf(c->display, MAX_CELL, "%g", result);
        }
    }
    c->dirty = false;
}

void formula_eval_all(void) {
    for (int r = 0; r <= last_row && r < MAX_ROWS; r++) {
        for (int c = 0; c <= last_col && c < MAX_COLS; c++) {
            if (grid[r][c] && grid[r][c]->type == CELL_FORMULA && grid[r][c]->dirty)
                formula_eval_cell(r, c);
        }
    }
}
