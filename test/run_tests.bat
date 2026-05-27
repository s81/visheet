@echo off
setlocal

echo === Running unit tests ===
make test
if errorlevel 1 goto :fail

echo.
echo === Manual integration checklist ===
echo  1. Launch: visheet.exe test\basic.csv
echo     - Confirm: name/age/city header row visible, Alice/Bob/Carol data rows
echo     - Press :w test\out_basic.csv, then diff with fc test\basic.csv test\out_basic.csv
echo.
echo  2. Launch: visheet.exe test\formulas.csv
echo     - Confirm: SUM cell shows 60, AVG shows 20, MIN shows 10, MAX shows 30, COUNT shows 3
echo.
echo  3. Launch: visheet.exe test\edge.csv
echo     - Confirm: "hello, world" renders without split, embedded quote shows correctly
echo.
echo  4. Launch: visheet.exe test\circular.csv
echo     - Confirm: A1 displays #CIRC!
echo.
echo  5. Vi mode spot checks (in any file):
echo     - 5j moves cursor 5 rows down
echo     - gg jumps to row 1
echo     - G jumps to last used row
echo     - dd clears a row (shows dirty [+] in status)
echo     - :sort B sorts by column B
echo     - :theme solarized changes colors
echo     - :theme monokai changes colors again
echo     - /Alice searches and jumps to the Alice cell
echo     - n finds next match

goto :done
:fail
echo UNIT TESTS FAILED
exit /b 1
:done
echo All unit tests passed. Complete manual checklist above.
