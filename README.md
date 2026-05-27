# visheet

A terminal spreadsheet for Windows with Vim-style modal editing.

```
     | A            | B            | C            |
   1 | name         | age          | city         |
   2 | Alice        | 30           | London       |
   3 | Bob          | 25           | Paris        |
 NORMAL | data.csv | A1
```

## Features

- **Vi modal editing** — Normal, Insert, Visual, and Command modes
- **Formulas** — arithmetic, SUM, AVG, MIN, MAX, COUNT with cell references
- **CSV I/O** — RFC 4180 compliant, compatible with Excel and LibreOffice
- **702 columns × 9999 rows** — A through ZZ, viewport-scrollable
- **3 color themes** — default, solarized, monokai
- **Single binary** — no runtime dependencies beyond a Windows console

## Building

**Requirements:** GCC 16.x (MinGW64), PDCurses 3.9, GNU Make

1. Build PDCurses (one-time):
   ```
   cd pdcurses-src\wincon
   make -f Makefile.mingw
   ```
   Copy `pdcurses.a` → `pdcurses\libpdcurses.a` and `curses.h` → `pdcurses\curses.h`

2. Build visheet:
   ```
   make
   ```

3. Run tests:
   ```
   make test
   ```
   Expected: 29/29 tests pass.

## Usage

```
visheet.exe                  open empty spreadsheet
visheet.exe data.csv         open a CSV file
```

See **[docs/visheet-manual.md](docs/visheet-manual.md)** for the full user manual.

## Quick Reference

| Key / Command | Action |
|---------------|--------|
| `h` `j` `k` `l` | Move cursor |
| `i` | Edit cell |
| `Esc` | Return to Normal mode |
| `dd` / `yy` / `p` | Clear / yank / paste row |
| `/pattern` | Search |
| `:w` / `:q!` | Save / force quit |
| `:e file.csv` | Open file |
| `:sort B` | Sort by column B |
| `:theme monokai` | Switch theme |
| `=SUM(A1:A10)` | Formula syntax |

## License

MIT
