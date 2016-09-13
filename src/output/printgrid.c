/* 
 * File:   printgrid.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "printgrid.h"
#include "grid.h"
#include "util.h"
#include "logfile.h"

// global variables
static char *output; // output in memory
static unsigned gridLineLength;
static unsigned lineLength;
static unsigned lineLengthWithLf;
static unsigned numberOfLines;
static unsigned numberOfChars;
static unsigned containerSetIndexForPrintingBoxes;

/* 2 characters are reserved for row header */
static const int rowHeaderChars = 2;

/*
 * line characters, e.g. t ... "line top", i.e. between quadrant I and II
 */
typedef enum {
    nil, vert, horiz, tl, lb, br, rt, tlb, lbr, brt, rtl, tlbr
} LineChar;

/* box separators on a junction point */
#define RIGHT 0x01
#define TOP 0x02
#define LEFT 0x04
#define BOTTOM 0x08

/* junction types */
#define JUNCTION_NIL   0
#define JUNCTION_VERT  (TOP | BOTTOM)
#define JUNCTION_HORIZ (LEFT | RIGHT)
#define JUNCTION_TL    (TOP | LEFT)
#define JUNCTION_LB    (LEFT | BOTTOM)
#define JUNCTION_BR    (BOTTOM | RIGHT)
#define JUNCTION_RT    (RIGHT | TOP)
#define JUNCTION_TLB   (TOP | LEFT | BOTTOM)
#define JUNCTION_LBR   (LEFT | BOTTOM | RIGHT)
#define JUNCTION_BRT   (BOTTOM | RIGHT | TOP)
#define JUNCTION_RTL   (RIGHT | TOP | LEFT)
#define JUNCTION_TLBR  (TOP | LEFT | BOTTOM | RIGHT)

// function prototypes
static void clear();
static LineChar junction(unsigned junctionX, unsigned junctionY);
static void printBorders();
static void printBoxBoundaries();
static void printJunctions();
static void printCoordinates();
static void fillValues(FieldValue whichValue);
static void printOutput();
static int getBoxAt(unsigned x, unsigned y);
static unsigned getContainerSetIndexForPrintingBoxes();
static void printCharAtRawPosition(unsigned x, unsigned y, char c);
static void printCharAtGridPosition(unsigned x, unsigned y, int deltaX, int deltaY, char c);
static void printCharAtJunction(unsigned junctionX, unsigned junctionY, char c);
static char *gridRow(unsigned y, int deltaY);

static Bool boxDifferentThanAbove(unsigned x, unsigned y);
static Bool boxDifferentThanLeft(unsigned x, unsigned y);
static void drawHorizontalBorderAbove(unsigned x, unsigned y);
static void drawVerticalBorderLeft(unsigned x, unsigned y);

// line character set in the order of the enum LineChars
static char asciiLines[13] = " |-+++++++++";
static char drawnLines[13] = {0x20, 0x78, 0x71, 0x6a, 0x6b, 0x6c, 0x6d, 0x75, 0x77, 0x74, 0x76, 0x6e};
static char *charset;

/**
 * prints the Sudoku grid
 * 
 * @param whichValue INITIAL ... print initial values, SOLVED ... print solved
 *   fields
 */
void printGrid(FieldValue whichValue) {

    // find out which container holds the boxes info
    containerSetIndexForPrintingBoxes = getContainerSetIndexForPrintingBoxes();

    /* 
     * allocate memory for building the output in memory first
     * reserve 2 chars for row header and 1 line for the column header
     */
    gridLineLength = maxNumber * 2 + 1; /* grid line length excl. LF */
    lineLength = gridLineLength + rowHeaderChars; /* line length excl. LF */
    lineLengthWithLf = lineLength + 1; /* line length incl. LF */
    numberOfLines = 1 + maxNumber * 2 + 1; /* column header + data lines + terminating grid border */
    numberOfChars = lineLengthWithLf * numberOfLines; /* times number of lines */
    output = (char *) xmalloc(sizeof (char) * (numberOfChars + 1));

    // clear output
    clear();

    charset = asciiLines;
//        charset = drawnLines;

    printBoxBoundaries();

    printBorders();

    printJunctions();

    printCoordinates();

    // fill out numbers
    fillValues(whichValue);

    printOutput();

    free(output);
}

/**
 * fills all with spaces
 */
void clear() {
    char *c;
    unsigned i;

    c = output;

    // start with 1 to not make the modulo operator fire on the very first char
    for (i = 1; i <= numberOfChars; i++) {
        if (i % lineLengthWithLf) {
            *c = ' ';
        } else {
            *c = '\n';
        }
        c++;
    }

    // terminate entire string
    *c = '\0';
}

/**
 * print the borders, including box boundaries, if applicable
 * 
 * @param whichValue
 */
void printBorders() {
    unsigned i;
    char *line;

    // first line
    line = gridRow(0, -1);
    for (i = 1; i < gridLineLength; i++) {
        *line = charset[horiz];
        line++;
    }

    // last line
    line = gridRow(maxNumber, -1);
    for (i = 1; i < gridLineLength; i++) {
        *line = charset[horiz];
        line++;
    }

    // first column
    line = gridRow(0, -1);
    for (i = 0; i < numberOfLines; i++) {
        *line = charset[vert];
        line += lineLengthWithLf;
    }

    // last column
    line = gridRow(0, -1) + gridLineLength - 1;
    for (i = 0; i < numberOfLines; i++) {
        *line = charset[vert];
        line += lineLengthWithLf;
    }
}

/**
 * fills out the values in the Sudoku grid.
 * 
 * @param whichValue INITIAL ... print initial values, SOLVED ... print solved
 *   fields
 */
void fillValues(FieldValue whichValue) {
    Field *field;
    unsigned f;
    unsigned value;

    for (f = 0; f < numberOfFields; f++) {
        field = &(fields[f]);

        if (whichValue == INITIAL) {
            value = field->initialValue;
        } else {
            value = field->value;
        }
        if (value) {
            printCharAtGridPosition(field->x, field->y, 0, 0, (char) (value + '0'));
        }
    }

}

/**
 * prints the assembled output on stdout
 */
void printOutput() {
    puts(output);
}

/**
 * prints the boundary lines between boxes (or shapes)
 */
void printBoxBoundaries() {
    unsigned x;
    unsigned y;

    // horizontal border lines
    for (y = 1; y < maxNumber; y++) {
        for (x = 0; x < maxNumber; x++) {
            // horizontal border line?
            if (boxDifferentThanAbove(x, y)) {
                drawHorizontalBorderAbove(x, y);
            }
        }
    }

    // vertical border lines
    for (y = 0; y < maxNumber; y++) {
        for (x = 1; x < maxNumber; x++) {
            // vertical border line?
            if (boxDifferentThanLeft(x, y)) {
                drawVerticalBorderLeft(x, y);
            }
        }
    }
}

/**
 * prints the junction points in the middle of 4 adjacent fields
 */
void printJunctions() {
    unsigned junctionX;
    unsigned junctionY;

    for (junctionY = 0; junctionY <= maxNumber; junctionY++) {
        for (junctionX = 0; junctionX <= maxNumber; junctionX++) {
            printCharAtJunction(junctionX, junctionY, charset[junction(junctionX, junctionY)]);
        }
    }
}

/**
 * print the Sudoku coordinates (row and column headers)
 */
void printCoordinates() {
    unsigned i;
    char *line;

    /* column headers */
    line = gridRow(0, -2) + 1;
    for (i = 0; i < maxNumber; i++) {
        *line = (char)(i + 1 + '0');
        line += 2;
    }

    /* row headers */
    line = output + 2 * lineLengthWithLf;
    for (i = 0; i < maxNumber; i++) {
        *line = (char)(i + 'A');
        line += lineLengthWithLf * 2;
    }
}

/**
 * determines if a number on the given position is in a different box than
 * the number above it (one row above)
 * 
 * @param x logical X coordinate of (below) field in Sudoku, ranging from 0...maxNumber
 * @param y logical Y coordinate of (below) field in Sudoku, ranging from 0...maxNumber
 * @return 
 */
Bool boxDifferentThanAbove(unsigned x, unsigned y) {
    int boxAbove;

    assert(y > 0);
    assert(y < maxNumber);

    boxAbove = getBoxAt(x, y - 1);
    return boxAbove != getBoxAt(x, y);
}

/**
 * determines if a number on the given position is in a different box than
 * the number above it (one row above)
 * 
 * @param x logical X coordinate of (below) field in Sudoku, ranging from 0...maxNumber
 * @param y logical Y coordinate of (below) field in Sudoku, ranging from 0...maxNumber
 * @return 
 */
Bool boxDifferentThanLeft(unsigned x, unsigned y) {
    int boxLeft;

    assert(x > 0);
    assert(x < maxNumber);

    boxLeft = getBoxAt(x - 1, y);
    return boxLeft != getBoxAt(x, y);
}

/**
 * draws a horizontal border between two boxes
 * 
 * @param x
 * @param y
 */
void drawHorizontalBorderAbove(unsigned x, unsigned y) {
    printCharAtGridPosition(x, y, 0, -1, charset[horiz]);
}

/**
 * draws a vertical border between two boxes
 * 
 * @param x
 * @param y
 */
void drawVerticalBorderLeft(unsigned x, unsigned y) {
    printCharAtGridPosition(x, y, -1, 0, charset[vert]);
}

/**
 * determines the junction type on a specific position. The junction position
 * is a coordinate in a special coordinate system which is "between" the 
 * normal coordinate system of the Sudoku fields.
 * 
 * 0 +----+----+
 *   |    |    | 
 * 1 +----+----+
 *   |    |    |
 * 2 +----+----+
 *   |    |    |
 * 3 +----+----+
 * 
 * @param junctionX the X position of the junction. 0 means the very first
 *   column, i.e. left to the first field. maxNumber means the column right to
 *   the very last column.
 * @param junctionY the Y position of the junction. 0 means the very first
 *   row, i.e. top of the first field. maxNumber means the row below the
 *   very last field.
 * @return the junction type
 */
LineChar junction(unsigned junctionX, unsigned junctionY) {
    int junctionType;
    int rightTop;
    int topLeft;
    int leftBottom;
    int bottomRight;
    LineChar lineChar;

    rightTop = getBoxAt(junctionX, junctionY - 1);
    topLeft = getBoxAt(junctionX - 1, junctionY - 1);
    leftBottom = getBoxAt(junctionX - 1, junctionY);
    bottomRight = getBoxAt(junctionX, junctionY);

    // determine box borders within the quadrant
    junctionType = 0;
    if (topLeft != rightTop) {
        junctionType |= TOP;
    }
    if (leftBottom != bottomRight) {
        junctionType |= BOTTOM;
    }
    if (topLeft != leftBottom) {
        junctionType |= LEFT;
    }
    if (rightTop != bottomRight) {
        junctionType |= RIGHT;
    }

    /* 
     * depending on the borders in the quadrant, a junction type is determined
     */
    switch (junctionType) {
        case JUNCTION_NIL:
            lineChar = nil;
            break;
        case JUNCTION_VERT:
            lineChar = vert;
            break;
        case JUNCTION_HORIZ:
            lineChar = horiz;
            break;
        case JUNCTION_TL:
            lineChar = tl;
            break;
        case JUNCTION_LB:
            lineChar = lb;
            break;
        case JUNCTION_BR:
            lineChar = br;
            break;
        case JUNCTION_RT:
            lineChar = rt;
            break;
        case JUNCTION_TLB:
            lineChar = tlb;
            break;
        case JUNCTION_LBR:
            lineChar = lbr;
            break;
        case JUNCTION_BRT:
            lineChar = brt;
            break;
        case JUNCTION_RTL:
            lineChar = rtl;
            break;
        case JUNCTION_TLBR:
            lineChar = tlbr;
            break;
        default:
            lineChar = nil;
            sprintf(buffer, "unknown junction type: %d", junctionType);
            logError(buffer);
    }

    return lineChar;
}

/**
 * determines the box number in which the field at (x/y) is located.
 * 
 * It is assumed (and only makes sense) that the container type used for drawing
 * the boxes is non-overlapping, so the *first* container index is used as
 * the result.
 * 
 * @param x
 * @param y
 * @return the box number
 */
int getBoxAt(unsigned x, unsigned y) {
    Field *field;
    int box;

    // on out of bounds, return -2
    if (x < 0 || x >= maxNumber) {
        box = -2;
    } else if (y < 0 || y >= maxNumber) {
        box = -2;
    } else {
        field = getFieldAt(x, y);
        box = field->containerIndexes[containerSetIndexForPrintingBoxes][0];
    }

    return box;
}

/**
 * determines which of the container sets should be used as an input for
 * drawing boxes.
 * 
 * For a standard Sudoku, this will be the container BOXES, for a jigsaw
 * Sudoku, this will be the container SHAPES.
 * 
 * @return the index of container set containing the "boxes" info
 */
unsigned getContainerSetIndexForPrintingBoxes() {
    ContainerType containerTypeContainingBoxes;
    int i;
    int containerSetIndex;

    switch (sudokuType) {
        case STANDARD_SUDOKU:
        case X_SUDOKU:
        case COLOR_SUDOKU:
            containerTypeContainingBoxes = BOXES;
            break;
        case JIGSAW_SUDOKU:
            containerTypeContainingBoxes = SHAPES;
            break;
        default:
            // we should never get here
            assert(0);
    }
    // find out the container set index of this container type
    containerSetIndex = -1;
    for (i = 0; i < numberOfContainerSets; i++) {
        if (containerSets[i].type == containerTypeContainingBoxes) {
            containerSetIndex = i;
            break;
        }
    }

    assert(containerSetIndex > -1);

    return containerSetIndex;
}

/**
 * print a single character on the specified raw position
 * 
 * @param x x position (raw)
 * @param y y position (raw)
 * @param c character to be printed
 */
void printCharAtRawPosition(unsigned x, unsigned y, char c) {
    *(output + (y * lineLengthWithLf) + x) = c;
}

/**
 * print a single character on the specified grid position
 * 
 * @param x x position of field in the grid (logical position)
 * @param y y position of field in the grid (logical position)
 * @param deltaX horizontal offset in screen character positions. A positive
 *   offset means a shift to the right, a negative to the left
 * @param deltaY verticall offset in screen character positions. A positive
 *   offset means a shift to the bottom, a negative to the top
 * @param c character to be printed
 */
void printCharAtGridPosition(unsigned x, unsigned y, int deltaX, int deltaY, char c) {
    *(output + ((2 + y * 2 + deltaY) * lineLengthWithLf)
            + (rowHeaderChars + 1 + x * 2 + deltaX)) = c;
}

/**
 * print a single character on a junction point
 * 
 * @param junctionX x position of the junction
 * @param junctionY y position of the junction
 * @param c junction character
 */
void printCharAtJunction(unsigned junctionX, unsigned junctionY, char c) {
    *(output + ((1 + junctionY * 2) * lineLengthWithLf)
            + (rowHeaderChars + junctionX * 2)) = c;
}

/**
 * returns a pointer to the start of the specified row of the grid 
 * 
 * @param y logical Y coordinate
 * @param deltaY
 * @return 
 */
char *gridRow(unsigned y, int deltaY) {
    return output + ((1 + 1 + y * 2 + deltaY) * lineLengthWithLf)
            + rowHeaderChars;
}