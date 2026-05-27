# visheet User Manual

## Launching

```
visheet.exe                    open empty spreadsheet
visheet.exe data.csv           open a CSV file
```

---

## Screen Layout

```
 A1    : =SUM(A1:A10)                        ← Formula bar (row 0)
     | A            | B            | C ...   ← Column headers (row 1)
   1 | hello        | 42           |         ← Data rows
   2 |              |              |
 NORMAL | data.csv | A1                      ← Status bar (last row)
```

---

## Modes

The status bar color shows your current mode:

| Color  | Mode    |
|--------|---------|
| Green  | NORMAL  |
| Red    | INSERT  |
| Blue   | VISUAL  |
| Yellow | COMMAND |

---

## Normal Mode

### Navigation

| Key | Action |
|-----|--------|
| `h` `j` `k` `l` | Left / Down / Up / Right |
| `w` / `b` | Jump columns right / left |
| `0` | Jump to column A |
| `$` | Jump to last used column |
| `gg` | Jump to row 1 |
| `G` | Jump to last used row |
| `Ctrl+F` | Page down |
| `Ctrl+B` | Page up |
| `[count]motion` | Repeat — e.g. `5j` moves 5 rows down |

### Editing

| Key | Action |
|-----|--------|
| `i` | Enter Insert mode (clear cell) |
| `a` | Enter Insert mode (keep existing content) |
| `x` | Clear current cell |
| `r` + key | Replace cell content with a single character |
| `dd` | Clear entire current row |

### Copy / Paste

| Key | Action |
|-----|--------|
| `yy` | Yank (copy) current row |
| `p` | Paste yanked row below cursor |

### Search

| Key | Action |
|-----|--------|
| `/pattern` | Search forward for pattern (matches value or formula text) |
| `n` | Next match |
| `N` | Previous match |

### Other

| Key | Action |
|-----|--------|
| `v` | Enter Visual mode |
| `:` | Enter Command mode |

---

## Insert Mode

Activated with `i` or `a` from Normal mode.

| Key | Action |
|-----|--------|
| Typing | Edits the cell |
| `=` (first character) | Marks cell as a formula |
| `Esc` | Confirm and return to Normal mode |
| `Enter` | Confirm and move cursor down |
| `Tab` | Confirm and move cursor right |
| `Backspace` | Delete last character |

---

## Visual Mode

Activated with `v` from Normal mode. Selection extends from where you pressed `v` to the cursor.

| Key | Action |
|-----|--------|
| `h` `j` `k` `l` | Extend selection |
| `y` | Yank top row of selection |
| `d` | Clear all cells in selection |
| `Esc` | Return to Normal mode |

---

## Command Mode

Activated with `:` from Normal mode. Press `Enter` to execute, `Esc` to cancel.

### Files

| Command | Action |
|---------|--------|
| `:w` | Save to current file |
| `:w filename.csv` | Save to a specific file |
| `:e filename.csv` | Open a file (warns if unsaved changes) |
| `:e!` | Re-open current file, discard changes |
| `:q` | Quit (warns if unsaved changes) |
| `:q!` | Force quit |
| `:wq` | Save and quit |

### Navigation

| Command | Action |
|---------|--------|
| `:goto A42` | Jump to cell address |

### Data

| Command | Action |
|---------|--------|
| `:sort A` | Sort rows by column A (ascending, numeric-aware) |
| `:sort! A` | Sort rows by column A (descending) |
| `:sum A1:A10` | Show sum of range in status bar |

### Appearance

| Command | Action |
|---------|--------|
| `:theme default` | Default color theme |
| `:theme solarized` | Solarized theme |
| `:theme monokai` | Monokai theme |

---

## Formulas

Start a cell with `=` to enter a formula.

### Arithmetic

```
=1+2          → 3
=A1*B1        → product of two cells
=(A1+A2)/2    → average of two cells
=-A1          → negation
```

Operator precedence: `*` `/` before `+` `-`

### Cell References

```
=A1           single cell
=A1+B2        two cells
```

Columns: A–Z (cols 1–26), AA–AZ (27–52), … ZZ (col 702)
Rows: 1–9999

### Range Functions

```
=SUM(A1:A10)      sum of range
=AVG(A1:A10)      average (empty cells skipped)
=MIN(A1:A10)      minimum value
=MAX(A1:A10)      maximum value
=COUNT(A1:A10)    count of non-empty cells
```

### Error Tokens

| Token | Cause |
|-------|-------|
| `#ERR!` | Parse error in formula |
| `#DIV0!` | Division by zero |
| `#CIRC!` | Circular reference |
| `#REF!` | Cell reference out of bounds |
| `#VAL!` | Non-numeric value used in arithmetic |

---

## CSV Files

- Format: RFC 4180 — compatible with Excel, LibreOffice, and most tools
- Fields containing commas or quotes are automatically quoted on save
- Formulas are saved as-is (e.g., `=SUM(A1:A10)`) and re-evaluated on load
- Only rows and columns with content are written — no trailing empty rows

---

## Tips

- **Count prefix:** Type a number before any motion — `10j` moves 10 rows down, `5l` moves 5 columns right
- **Find a formula:** `/SUM` will jump to any cell containing SUM in its formula or value
- **Quick jump:** `:goto B42` is faster than scrolling for distant cells
- **Dirty indicator:** `[+]` appears in the status bar when there are unsaved changes
- **Column A shortcut:** `0` in Normal mode jumps to the first column instantly
