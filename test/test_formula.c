#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "../grid.h"
#include "../formula.h"

static int tests_run    = 0;
static int tests_passed = 0;

/* helper: set a NUMBER cell directly (bypasses formula eval) */
static void set_num(int row, int col, const char *val) {
    grid_set_cell(row, col, val);
}

static void test_arithmetic(void) {
    grid_init();
    grid_set_cell(0, 1, "=1+2*3");
    formula_eval_cell(0, 1);
    /* operator precedence: 2*3=6, then 1+6=7 */
    assert(atof(grid[0][1]->display) == 7.0);
    tests_passed++;
}

static void test_cell_ref(void) {
    grid_init();
    set_num(0, 0, "5");          /* A1 = 5 */
    grid_set_cell(0, 1, "=A1+3");
    formula_eval_cell(0, 1);
    assert(atof(grid[0][1]->display) == 8.0);
    tests_passed++;
}

static void test_sum(void) {
    grid_init();
    set_num(0, 0, "10");
    set_num(1, 0, "20");
    set_num(2, 0, "30");
    grid_set_cell(0, 1, "=SUM(A1:A3)");
    formula_eval_cell(0, 1);
    assert(atof(grid[0][1]->display) == 60.0);
    tests_passed++;
}

static void test_avg(void) {
    grid_init();
    set_num(0, 0, "10");
    set_num(1, 0, "20");
    set_num(2, 0, "30");
    grid_set_cell(0, 1, "=AVG(A1:A3)");
    formula_eval_cell(0, 1);
    assert(atof(grid[0][1]->display) == 20.0);
    tests_passed++;
}

static void test_min(void) {
    grid_init();
    set_num(0, 0, "10");
    set_num(1, 0, "20");
    set_num(2, 0, "30");
    grid_set_cell(0, 1, "=MIN(A1:A3)");
    formula_eval_cell(0, 1);
    assert(atof(grid[0][1]->display) == 10.0);
    tests_passed++;
}

static void test_max(void) {
    grid_init();
    set_num(0, 0, "10");
    set_num(1, 0, "20");
    set_num(2, 0, "30");
    grid_set_cell(0, 1, "=MAX(A1:A3)");
    formula_eval_cell(0, 1);
    assert(atof(grid[0][1]->display) == 30.0);
    tests_passed++;
}

static void test_count(void) {
    grid_init();
    set_num(0, 0, "10");
    set_num(1, 0, "20");
    set_num(2, 0, "30");
    grid_set_cell(0, 1, "=COUNT(A1:A3)");
    formula_eval_cell(0, 1);
    assert(atof(grid[0][1]->display) == 3.0);
    tests_passed++;
}

static void test_div_by_zero(void) {
    grid_init();
    grid_set_cell(0, 0, "=1/0");
    formula_eval_cell(0, 0);
    assert(strstr(grid[0][0]->display, "#DIV0!") != NULL);
    tests_passed++;
}

static void test_circular_ref(void) {
    grid_init();
    grid_set_cell(0, 0, "=A1");   /* A1 references itself */
    formula_eval_cell(0, 0);
    assert(strstr(grid[0][0]->display, "#CIRC!") != NULL);
    tests_passed++;
}

static void test_parse_error(void) {
    grid_init();
    grid_set_cell(0, 0, "=+++");
    formula_eval_cell(0, 0);
    assert(strstr(grid[0][0]->display, "#ERR!") != NULL);
    tests_passed++;
}

static void test_unary_minus(void) {
    grid_init();
    grid_set_cell(0, 0, "=-5");
    formula_eval_cell(0, 0);
    assert(atof(grid[0][0]->display) == -5.0);
    tests_passed++;
}

#define RUN(name) do { tests_run++; test_##name(); \
                       printf("PASS: " #name "\n"); } while(0)

int main(void) {
    RUN(arithmetic);
    RUN(cell_ref);
    RUN(sum);
    RUN(avg);
    RUN(min);
    RUN(max);
    RUN(count);
    RUN(div_by_zero);
    RUN(circular_ref);
    RUN(parse_error);
    RUN(unary_minus);
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
