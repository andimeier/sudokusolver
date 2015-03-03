Sudoku-Solver
============

Description
---------

Solves a Sudoku puzzle using the basic solving  techniques like pointing pairs etc.

No intermediate/advanced techniques like X-Wing, Swordfish etc. are used


Compile:

    gcc solve_sudoku.c

Call:

    solve_sudoku.c < examples/sudoku1.sudoku

Usage
----

    solve_sudoku -l LOGFILE -s SVGFILE -v -h

### Parameters

  -l LOGFILE  printlog into LOGFILE (filename) instead of stdout
  
  -s SVGFILE  write SVG representation of Sudoku grid into SVG files, each iteration will write
            another SVG file with a numeric suffix appended. The SVG file without an additional
              numeric suffix is the final, solved grid.
              For example, when the parameter -s test.svg is specified, you will end up with SVG
              files of test.svg.1, test.svg.2, test.svg.3 etc. plus the final grid, stored in the
              file test.svg (without additional suffix).
			  
  -v          verbose logging
  
  -h          this help screen

  
### Format of Sudoku input file

1. Empty cells are represented by a dot '.'.
2. Each Sudoku row is written in a separate line
3. lines starting with a hashbang '#' are considered comments and are silently skippde

```
7....56.4
6.8...5.7
..12..9..
...5...4.
...391...
.1...2...
..7..94..
9.4...3.8
2.56....1
```
