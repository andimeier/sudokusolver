Sudoku solver
=======

**Solve Sudokus like a human.**

A flexible and extensible Sudoku solver. Tries to solve Sudokus _like a human would do_. No brute force computing- instead, the tool applies different solving strategies in order to solve the riddle. This implies that this tool is not able to solve 100 % of all Sudokus. Anyway, equipped with its strategies, it is able to solve most if not all of the "newspaper" Sudokus, even the tough ones.

## Why yet another Sudoku solver?

The purpose of this Sudoku solver is to have a quick way of assessing the "difficulty" of a Sudoku. There are no standard ways to determine how difficult a Sudoku is, but for me personally, the difficulty corresponds to the set and extent of strategies I have to apply in order to solve it. I regard a Sudoku that can be solved with only hidden singles and naked pairs as more or less equally difficult to another one where the same strategies are used, regardless of how many clues (starting numbers) are given initially. So I wanted to have a tool that tells me if a Sudoku requires more than the standard strategies, e.g. if an X-wing technique is required. Such Sudokus are more interesting to solve, so I wanted to identify them easily. That's why I wrote this Sudoku solver.

## Output

This Sudoku solver tries to solve the Sudoku. Then, it displays a list of strategies which have been needed to solve the Sudoku.

An example output:

    Utilized strategies:
      X check for solved cells
      X find hidden singles
      - find naked tuples (not used)
      - find pointing tuples (not used)
      - find hidden tuples (not used)

In this example, a quite easy Sudoku has been solved, using only the very basic strategies "check for solved cells" and "find hidden singles".

Of course, the solved Sudoku itself will also be printed. But the list of applied strategies was the main point why this Sudoku solver has been implemented.

## Features

Some key features of the Sudoku solver:

* generic architecture using "containers" enables the tool to tackle standard Sudokus as well as X-Sudokus, "Jigsaw Sudokus" (with irregular shaped containers) and "Color Sudokus"
* Sudokus can basically have any number of numbers. A standard Sudoku has 9 numbers, but the solver does not care. Sudokus with a lower or higher number of different candidates are implicitly supported.
* new solving strategies can be added easily

The internal structures for e.g. rows, columns and boxes are nothing more than specializations of a generic construct named "container". A container is just a set of fields on the Sudoku board which contains each number exactly once. So, in a standard Sudoku, a container has 9 fields and must contain every number from 1 to 9 once. Common examples of a container are:

* a row
* a column
* a box (3x3 box in a standard Sudoku)

Some common examples of containers in non-standard Sudokus are:

* a diagonal (in a X-Sudoku)
* a "color" in a color Sudoku
* a "shape", i.e. an irregularly shaped container in a jigsaw Sudoku

## Solving strategies

The solving strategies are very much influenced by the superb Sudoku solver web page by Andrew Stuart. Although I knew and used the strategies before, Andrew Stuart's site was the inspiration for the naming of the strategies. It also was an input for some user interface details.

### Check for solved cells

This very basic strategy is actually not a strategy but just:

1. go through all fields
2. if only one candidate is left, mark the field as solved

That's it. It marks the fields as solved.

### Find hidden singles

This very basic strategy looks for numbers which can only occur in one place within a container.

For example, the candidate 3 is only allowed in the second field of row 3. 

Result:

* use "hidden single" in the field as solution and mark the field as solved


### Find naked tuples

This important basic strategy is:

1. in any container, look for a set of two fields sharing the same pair of candidates. This is called a "naked pair"
2. eliminate these 2 candidates from all other fields of the same container

The same principle applies to a "naked triple":

1. in any container, look for a set of three fields sharing the same triple of candidates. This is called a "naked triple"
2. eliminate these 3 candidates from all other fields of the same container

The strategy is called "naked tuple" because it searches for pairs, triples, quadruples and further, as you wish. At the moment, it is hard-coded to end after "quadruples".

### Find pointing tuples

If any one number occurs twice or three times in just one container (e.g. a row, column or box) then that number can be removed from the intersection of another container. For example:

A Pair or Triple in a box - if they are aligned on a row, n can be removed from the rest of the row.

### Find hidden tuples

If n numbers can only occur in n fields of the same container, then each of these n fields must contain exactly one of each of these n numbers. Consequently, all other candidates than these n numbers can be removed from these n fields.

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

A Sudoku input file speicifies the starting numbers as well as "metadata" like Sudoku type. It is a plain text file and basically looks like this:

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

The number of values of a Sudoku (i.e., the Sudoku size) is determined automatically from the number of characters per line. In a standard Sudoku, there would be 9 characters per line (initial values and placeholder for empty fields).

Note that you could have omitted each or both of the lines

    type: standard
    box: 3x3

in this case because these are the default values for these parameters anyway.

The rules for a Sudoku file are:

1. Each Sudoku row is written in a separate line
2. Each line contains the initial numbers of a Sudoku row, empty Sudoku cells are represented by a dot '.' or '\_'. If '0' is no valid candidate, then '0' is also interpreted as empty cell.
3. lines starting with a hashbang '#' are considered comments and are silently skipped
4. Optional control lines can be used: these start with a keyword and a colon and then the specific setting.

#### Options in Sudoku files

With options used in the Sudoku file, certain parameters can be set for the Sudoku, e.g.:

* the type of Sudoku (e.g. a X-Sudoku)
* the box dimensions (if different than 3x3 boxes)

Such lines are made up like this:

    NAME: VALUE

The name of the setting is case-insensitive. There may be spaces between the colon and the value.

The following settings are recognized as options:

* type ... set the type of Sudoku (default: standard)
* box ... set the dimension of the "boxes" (default: 3x3)
* shapes ... specifiy the irregular shapes for a Jigsaw Sudoku
* candidates ... characters used for the Sudoku field values 

The following sections describes these options in more detail.

##### Type

Specifies the type of Sudoku. Possible values are:
* `standard`
* `x`
* `color`

Default is "standard". The values are recognized with a minimum of matching characters, so you can abbreviate the setting, e.g. "st" works also for setting a "standard" Sudoku.

Example:

    Type: col

##### Box

Specifies the box dimensions of the "boxes" (rectangular shaped containers). For a standard Sudoku, the boxes are 3x3. However, they could be different. E.g. for a 6x6 Sudoku, the boxes would normally be 3x2, but could also be 2x3.

Format: `WIDTHxHEIGHT`
 
Default is "3x3". 

Example:

    Box: 3x2


##### Shapes

Specifies the shapes in a Jigsaw Sudoku (a Sudoku with irregular shaped boxes).

The following lines specify the "shape identifiers" each field belongs to. A shape identifier is a single character of either

* a digit
* a lowercase letter
* an uppercase letter

There are no default shapes - if a Jigsaw Sudoku is used, it is mandatory to specify the
shapes using this directive.

Each line of the "Shapes" setting correspond to one row of the Sudoku. So, each line must have the same number of characters as the Sudoku rows.

Example:

    Shapes:
    111222333
    112222233
    111123333
    ... [etc]


A valid Jigsaw Sudoku input file may look like this:

```
type: jigsaw
601007208
802403001
080000000
010060002
000000000
200030070
000000080
100208503
903800604
shapes:
111122222
111132322
144333332
444535366
445555566
447575666
87777766u
88787uuuu
88888uuuu
```

Note, that the last jigsaw shape has a "shape identifier" of "u". This is just to demonstrate that the characters used as shape identifiers do not matter as long as they conform to the rules above.
#### Candidates

Specifies the characters to be used for the Sudoku field values. In standard Sudokus, these are the digits 1 through 9. However, in exotic Sudokus or Sudokus more then 9 cells wide, there may be letters used as well. The value of this option specifies all possible "value", i.e. characters. The number of specified characters must exactly match the width of the Sudoku game board.

It is also possible to shorten a range of characters by using the hyphen, e.g.:

    candidates: 1-9A-H

Note that at the moment it is not possible for the character '0' to be a regular candidate. '0' is internally considered a placeholder for "no value set". So, if you try to solve a Sudoku with 0...9 and A...F, you have to map the values to 1...9 and A...G.
	
Format: \<string\>
 
Default is "123456789" (truncated to the width of the Sudoku). 

Example:

    Candidates: "123456789ABCDEFG"


## For developers

### Acquiring from another source

Acquiring a Sudoku means reading the Sudoku's characteristics and initial values from any source. 

At the moment, only acquiring from a Sudoku file is implemented. If you want to implement another source (e.g. command line), it is important to know that the _output_ of your acquisition must be a set up `Parameters` structure containing all information about the Sudoku.

This struct `Parameters` is the interface between acquisition and solver. A complete struct `Parameters` is all there is needed to set up the board and the solver.

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
