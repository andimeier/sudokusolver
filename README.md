Sudoku solver
=======

**Solve Sudokus like a human.**

A flexible and extensible Sudoku solver. Tries to solve Sudokus _like a human would do_. No brute force computing- instead, the tool applies different solving strategies in order to solve the riddle. This implies that this tool is not able to solve 100 % of all Sudokus. Anyway, equipped with its strategies, it is able to solve most if not all of the "newspaper" Sudokus, even the tough ones.

## Why yet another Sudoku solver?

The purpose of this Sudoku solver is to have a quick way of assessing the "difficulty" of a Sudoku. There are no standard ways to determine how difficult a Sudoku is, but for me personally, the difficulty corresponds to the set and extent of strategies I have to apply in order to solve it. I regard a Sudoku that can be solved with only hidden singles and naked pairs as more or less equally difficult to another one where the same strategies are used, regardless of how many clues (starting numbers) are given initially. So I wanted to have a tool that tells me if a Sudoku requires more than the standard strategies, e.g. if an X-wing technique is required. Such Sudokus are more interesting to solve, so I wanted to identify them easily. That's why I wrote this Sudoku solver.

## Features

Some key features of the Sudoku solver:

* generic architecture using "containers" enables the tool to tackle standard Sudokus as well as X-Sudokus, "Jigsaw Sudokus" (with irregular shaped containers) and "Color Sudokus"
* Sudokus can basically have any number of numbers. A standard Sudoku has 9 numbers, but the solver does not care. Sudokus with a lower or higher number of different candidates are implicitly supported.
* new solving strategies can be added easily

The internal structures for e.g. rows, columns and boxes are nothing more than specializations of a generic construct named "container". A container is just a set of fields on the Sudoku board which contains each number exactly once. So, in a standard Suduoku, a container has 9 fields and must contain every number from 1 to 9 once. Common examples of a container are:

* a row
* a column
* a box (3x3 box in a standard Sudoku)

Some common examples of containers in non-standard Sudokus are:

* a diagonal (in a X-Sudoku)
* "colors" in a color Sudoku
* a "shape", i.e. an irregularly shaped container in a jigsaw Sudoku

## Solving strategies

The solving strategies are very much influenced by the superb Sudoku solver web page by Andrew Stuart. Although I knew and used the strategies before, Andrew Stuart's site was the inspiration for the naming of the strategies as well as an input for some user interface details.

## Build

Just run the Makefile:

    make

This will build the C sources into the following executable:

    out/sudoku-solver


## Usage

    sudoku-solver [ -l LOGFILE -s SVGFILE -v -h ] SUDOKU_FILE

### Parameters

    -l LOGFILE  printlog into LOGFILE (filename) instead of stdout
    
    -s SVGFILE  write SVG representation of Sudoku grid into SVG files, each
    iteration will write another SVG file with a numeric suffix appended.
    The SVG file without an additional numeric suffix is the final, solved
    grid.
    For example, when the parameter -s test.svg is specified, you will end up with SVG
    files of test.svg.1, test.svg.2, test.svg.3 etc. plus the final grid, stored in the
    file test.svg (without additional suffix).

    -v          verbose logging
  
    -h          this help screen

### Format of the Sudoku input file

1. Each Sudoku row is written in a separate line
2. Each line contains the initial numbers of a Sudoku row, empty Sudoku cells are represented by a dot '.' or '0' or '\_'.
3. lines starting with a hashbang '#' are considered comments and are silently skipped
4. Optional control lines can be used: these start with a keyword and a colon and then the specific setting.

#### Control lines 
With control lines in the Sudoku file, certain parameters can be set for the Sudoku.
With control lines in the Sudoku file, certain parameters can be set for the Sudoku.
With control lines in the Sudoku file, certain parameters can be set for the Sudoku.

Such lines are made up like this:

    Name: Value

The name of the setting is case-insensitive. There may be spaces between the colon and the value.

The following settings are recognized in control lines:

##### Type
Specifies the type of Sudoku. Possible values are:
* `standard`
* `x`
* `color`

Default is "standard". The values are recognized with a minimum of matching characters, so you can abbreviate the setting, e.g. "st" works also for setting a "standard" Sudoku.

Example:

    Type: col


##### Box
Specifies the box dimensions of the "boxes" (rectangular shaped containers). For a standard Sudoku, the boxes are 3x3.

Format: `WIDTHxHEIGHT`
 
Default is "3x3". 

Example:

    Box: 3x2


##### Shapes

Specifies the shapes in a Jigsaw Sudoku (a Sudoku with irregular shaped boxes).

The following lines specify the "shape ID" each field belongs to. The shape IDs are a single character of either

* a digit
* a lowercase letter
* an uppercase letter

There are no default shapes - if a Jigsaw Sudoku is used, it is mandatory to specify the
shapes using this directive.

Example:

    Shapes:
    111222333
    112222233
    111123333
    ... [etc]


A valid Jigsaw Sudoku input file may look like this:

```
type: standard
box: 3x3
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

Note that you could have omitted each or both of the lines

    type: standard
    box: 3x3

in this case because these are the default values for these parameters anyway.

## For developers

### Implementing another container type

A new container type must provide the following functions:

* getContainerIndexFunc ... returns the index of the container of the new container type which contains the specified field (specified by the coordinates x and y)
* getContainerNameFunc) ... returns the name of the container for the given container index

The new container type must be linked to a certain "game type". For example, a game type "standard Sudoku" consists of the container types "rows", "columns" and "boxes". For a new container type to be used, it must be assigned to a (new) game type.

Note that a field could be member of more than one container of a spdcific type (for example, the crossing between ascending and falling diagonal is the same field)

Compile:

    make clean all

Call:

    out/sudoku-solver < examples/sudoku1.sudoku

### Install Unity (Unit testing)

Download Unity from 

    https://github.com/ThrowTheSwitch/Unity/archive/master.zip

and place it in a folder beside the Sudoku solver project.

Running 

    make test

in the project directory will try to locate Unity at `../unity/`.


