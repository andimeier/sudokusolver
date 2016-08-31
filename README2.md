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