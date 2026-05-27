#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../grid.h"
#include "../csv.h"

static int tests_run = 0, tests_passed = 0;
#define RUN(name) do { tests_run++; test_##name(); tests_passed++; \
                       printf("PASS: " #name "\n"); } while(0)

static void set_display(int r, int c, const char *v) {
    Cell *cell = grid[r][c];
    if (cell) { strncpy(cell->display, v, MAX_CELL-1); cell->dirty = false; }
}

static void test_save_load_basic(void) {
    remove("_tmp_basic.csv");
    grid_init();
    grid_set_cell(0, 0, "name"); set_display(0, 0, "name");
    grid_set_cell(0, 1, "age");  set_display(0, 1, "age");
    grid_set_cell(1, 0, "Alice"); set_display(1, 0, "Alice");
    grid_set_cell(1, 1, "30");   set_display(1, 1, "30");

    assert(csv_save("_tmp_basic.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_basic.csv") == CSV_OK);
    assert(grid[0][0] != NULL && strcmp(grid[0][0]->raw, "name") == 0);
    assert(grid[1][1] != NULL && strcmp(grid[1][1]->raw, "30") == 0);
    assert(grid[1][1]->type == CELL_NUMBER);
    remove("_tmp_basic.csv");
}

static void test_quoted_comma(void) {
    remove("_tmp_comma.csv");
    grid_init();
    grid_set_cell(0, 0, "hello, world"); set_display(0, 0, "hello, world");
    assert(csv_save("_tmp_comma.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_comma.csv") == CSV_OK);
    assert(strcmp(grid[0][0]->raw, "hello, world") == 0);
    remove("_tmp_comma.csv");
}

static void test_embedded_quote(void) {
    remove("_tmp_quote.csv");
    grid_init();
    grid_set_cell(0, 0, "say \"hi\""); set_display(0, 0, "say \"hi\"");
    assert(csv_save("_tmp_quote.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_quote.csv") == CSV_OK);
    assert(strcmp(grid[0][0]->raw, "say \"hi\"") == 0);
    remove("_tmp_quote.csv");
}

static void test_formula_preserved(void) {
    remove("_tmp_formula.csv");
    grid_init();
    grid_set_cell(0, 0, "=SUM(A2:A3)"); set_display(0, 0, "5");
    assert(csv_save("_tmp_formula.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_formula.csv") == CSV_OK);
    assert(grid[0][0]->type == CELL_FORMULA);
    assert(strcmp(grid[0][0]->raw, "=SUM(A2:A3)") == 0);
    remove("_tmp_formula.csv");
}

static void test_missing_file(void) {
    grid_init();
    assert(csv_load("_nonexistent_xyz_123.csv") == CSV_ERR_FILE);
}

static void test_empty_cells_skipped(void) {
    remove("_tmp_empty.csv");
    grid_init();
    grid_set_cell(0, 0, "A"); set_display(0, 0, "A");
    /* col 1 intentionally empty */
    grid_set_cell(0, 2, "C"); set_display(0, 2, "C");
    assert(csv_save("_tmp_empty.csv") == CSV_OK);
    grid_init();
    assert(csv_load("_tmp_empty.csv") == CSV_OK);
    assert(strcmp(grid[0][0]->raw, "A") == 0);
    assert(strcmp(grid[0][2]->raw, "C") == 0);
    remove("_tmp_empty.csv");
}

int main(void) {
    RUN(save_load_basic); RUN(quoted_comma); RUN(embedded_quote);
    RUN(formula_preserved); RUN(missing_file); RUN(empty_cells_skipped);
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
