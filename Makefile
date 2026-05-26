CC        = gcc
CFLAGS    = -Wall -Wextra -O2 -std=c11 -Ipdcurses
LIBS      = -Lpdcurses -lpdcurses
SRCS      = main.c grid.c render.c input.c vi_mode.c formula.c csv.c theme.c
OBJS      = $(SRCS:.c=.o)
TARGET    = visheet.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test_grid.exe: test/test_grid.c grid.c
	$(CC) $(CFLAGS) test/test_grid.c grid.c -o test_grid.exe

test_csv.exe: test/test_csv.c csv.c grid.c
	$(CC) $(CFLAGS) test/test_csv.c csv.c grid.c -o test_csv.exe

test_formula.exe: test/test_formula.c formula.c grid.c
	$(CC) $(CFLAGS) test/test_formula.c formula.c grid.c -o test_formula.exe

test: test_grid.exe test_csv.exe test_formula.exe
	.\test_grid.exe && .\test_csv.exe && .\test_formula.exe

clean:
	del /Q *.o *.exe 2>NUL || exit 0
