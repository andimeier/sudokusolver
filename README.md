Sudoku-Solver
============

Description
---------

Solves a Sudoku puzzle using the basic solving  techniques like pointing pairs etc.

No intermediate/advanced techniques like X-Wing, Swordfish etc. are used


Compile:

    make clean all

Call:

    out/sudoku-solver < examples/sudoku1.sudoku


Install Unity (Unit testing)
---------------------

Download Unity from 

    https://github.com/ThrowTheSwitch/Unity/archive/master.zip

and place it in a folder beside the Sudoku solver project.

Running 

    make test

in the project directory will try to locate Unity at `../unity/`.


Usage
----

    sudoku-solver -l LOGFILE -s SVGFILE -v -h

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

1. Empty cells are represented by a dot '.' or '0' or '\_'.
2. Each Sudoku row is written in a separate line
3. lines starting with a hashbang '#' are considered comments and are silently skippde

A valid Sudoku input file may look like this:

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

### Bugs

getUniquePositionInRow unterscheidet nicht zwischen "Unique auf 1. Zelle" (Return-Value 0) und "keine unique Zahl gefunden" (Return-Value 0). Dto. für getUniquePositionInColumn, ev. auch in getUniquePositionInBox?
