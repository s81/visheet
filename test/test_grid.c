#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../grid.h"

static int tests_run = 0, tests_passed = 0;
#define RUN(name) do { tests_run++; test_##name(); tests_passed++; \
                       printf("PASS: " #name "\n"); } while(0)

static void test_grid_init(void) {
    grid_init();
    assert(last_row == 0 && last_col == 0);
    assert(grid[0][0] == NULL);
    assert(grid[MAX_ROWS-1][MAX_COLS-1] == NULL);
}

static void test_set_text(void) {
    grid_init();
    grid_set_cell(0, 0, "hello");
    assert(grid[0][0] != NULL);
    assert(strcmp(grid[0][0]->raw, "hello") == 0);
    assert(grid[0][0]->type == CELL_TEXT);
    assert(grid[0][0]->dirty == true);
}

static void test_set_number(void) {
    grid_init();
    grid_set_cell(0, 0, "42");
    assert(grid[0][0]->type == CELL_NUMBER);
    grid_set_cell(0, 1, "3.14");
    assert(grid[0][1]->type == CELL_NUMBER);
    grid_set_cell(0, 2, "0");
    assert(grid[0][2]->type == CELL_NUMBER);
}

static void test_set_formula(void) {
    grid_init();
    grid_set_cell(0, 0, "=SUM(A1:A3)");
    assert(grid[0][0]->type == CELL_FORMULA);
}

static void test_clear_cell(void) {
    grid_init();
    grid_set_cell(0, 0, "hello");
    grid_clear_cell(0, 0);
    assert(grid[0][0] == NULL);
}

static void test_clear_row(void) {
    grid_init();
    grid_set_cell(2, 0, "a"); grid_set_cell(2, 1, "b"); grid_set_cell(2, 2, "c");
    grid_clear_row(2);
    assert(grid[2][0] == NULL && grid[2][1] == NULL && grid[2][2] == NULL);
}

static void test_last_tracking(void) {
    grid_init();
    grid_set_cell(5, 10, "x");
    assert(last_row == 5 && last_col == 10);
    grid_set_cell(2, 3, "y");
    assert(last_row == 5 && last_col == 10);
}

static void test_col_to_str_single(void) {
    char buf[4];
    assert(strcmp(col_to_str(0,  buf, sizeof(buf)), "A") == 0);
    assert(strcmp(col_to_str(25, buf, sizeof(buf)), "Z") == 0);
}

static void test_col_to_str_double(void) {
    char buf[4];
    assert(strcmp(col_to_str(26,  buf, sizeof(buf)), "AA") == 0);
    assert(strcmp(col_to_str(701, buf, sizeof(buf)), "ZZ") == 0);
}

static void test_str_to_col(void) {
    assert(str_to_col("A")  == 0);
    assert(str_to_col("Z")  == 25);
    assert(str_to_col("AA") == 26);
    assert(str_to_col("ZZ") == 701);
}

static void test_cell_addr(void) {
    char buf[10];
    cell_addr(0, 0, buf, sizeof(buf));
    assert(strcmp(buf, "A1") == 0);
    cell_addr(9998, 701, buf, sizeof(buf));
    assert(strcmp(buf, "ZZ9999") == 0);
}

static void test_parse_cell_addr(void) {
    int row, col;
    assert(parse_cell_addr("A1",   &row, &col) && row == 0    && col == 0);
    assert(parse_cell_addr("B2",   &row, &col) && row == 1    && col == 1);
    assert(parse_cell_addr("ZZ9999", &row, &col) && row == 9998 && col == 701);
    assert(!parse_cell_addr("1A",  &row, &col));
    assert(!parse_cell_addr("",    &row, &col));
}

int main(void) {
    RUN(grid_init); RUN(set_text); RUN(set_number); RUN(set_formula);
    RUN(clear_cell); RUN(clear_row); RUN(last_tracking);
    RUN(col_to_str_single); RUN(col_to_str_double); RUN(str_to_col);
    RUN(cell_addr); RUN(parse_cell_addr);
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
