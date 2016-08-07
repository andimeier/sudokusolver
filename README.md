Sudoku solver
========

A flexible and extensible Sudoku solver. Tries to solve Sudokus like a human would do. No brute force computing, but the tool applies different solving strategies in order to solve the riddle. 

Some key features of the Sudoku solver:

* the tool has no notion of a "standard Sudoku"
* Sudokus can basically have any number of numbers. A standard Sudoku has 9 numbers, but the solver does not care. Sudokus with a lower or higher number of different candidates are implicitly supported.
* X-Sudokus, "Squigglies" and "Color Sudokus" are supported.
* new strategies can be added easily

The internal structures for rows, columns and boxes are nothing more than specializations of a generic construct named "container". Thus, a container is just a set of fields on the Sudoku board which contains each number exactly once. Common examples of a container are:

* a row
* a column
* a box (3x3 box in a standard Sudoku)

Some common examples of containers in non-standard Sudokus are:

* a diagonal (in a X-Sudoku)
* "colors" in a color Sudoku

The strategies are very much influenced by the superb Sudoku solver web page by Andrew Stuart. Although I knew and used the strategies before, Andrew Stuart's site was the inspiration for the naming of the strategies as well as an input for some user interface details.

The purpose of the Sudoko solver was to have a quick way of assessing a Sudoku with respect to its "difficulty". The "difficulty" of a Sudoku is not easily rated, but for me personally, the difficulty corresponds to the set and level of strategies I have to apply in order to solve it. I regard a Sudoku that can be solved with only hidden singles and naked pairs as more or less equally difficult to another one where the same strategies are used, regardless of how many clues are given initially. So I wanted to have a tool that tells me if a Sudoku requires more than the standard strategies, e.g. if an X-wing technique is required. Such Sudokus are more interesting to solve, so I wanted to identify them.

## Implementing another container type

A new container type must provide the following functions:

* getContainerIndexFunc ... returns the index of the container of the new container type which contains the specified field (specified by the coordinates x and y)
* getContainerNameFunc) ... returns the name of the container for the given container index

The new container type must be linked to a certain "game type". For example, a game type "standard Sudoku" consists of the container types "rows", "columns" and "boxes". For a new container type to be used, it must be assigned to a (new) game type.

Note that a field could be member of more than one container of a spdcific type (for example, the crossing between ascending and falling diagonal is the same field)

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
3. lines starting with a hashbang '#' are considered comments and are silently skipped
4. Optional control lines can be used: these start with a keyword and a colon and then the specific setting.

#### Control lines 
With control lines in the Sudoku file, certain parameters can be set for the Sudoku.

Such lines are made up like this:

    Name: Value

The name of the setting is case-insensitive. There may be spaces between the colon and the value.

The following settings are recognized in control lines:

##### Type
Specifies the type of Sudoku. Possible values are:
* standard
* x
* color

Default is "standard". The values are recognized with a minimum of matching characters, so you can abbreviate the setting, e.g. "st" works also for setting a "standard" Sudoku.

Example:

    Type: col


A valid Sudoku input file may look like this:

```
type: standard
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
