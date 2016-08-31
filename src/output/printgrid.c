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

// global variables
static char *output; // output in memory
static unsigned lineLength;
static unsigned lineLengthWithLf;
static unsigned numberOfLines;
static unsigned numberOfChars;
static unsigned containerSetIndexForPrintingBoxes;

// line characters, e.g. t ... "line top", i.e. between quadrant I and II

typedef enum {
    nil, vert, horiz, tl, lb, br, rt, tlb, lbr, brt, rtl, tlbr
} LineChars;

// function prototypes
static void clear();
static void junction();
static void printBorders();
static void printBoxBoundaries();
static void printJunctions();
static void fillValues(FieldValue whichValue);
static void print();
static int getBoxAt(unsigned x, unsigned y);
static unsigned getContainerSetIndexForPrintingBoxes();
static void printChar(unsigned x, unsigned y, int deltaX, int deltaY, char c);

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

    // allocate memory for building the output in memory first
    lineLength = maxNumber * 2 + 1; /* line length excl. LF */
    lineLengthWithLf = lineLength + 1; /* line length incl. LF */
    numberOfLines = maxNumber * 2 + 1;
    numberOfChars = lineLengthWithLf * numberOfLines; /* times number of lines */
    output = (char *) xmalloc(sizeof (char) * (numberOfChars + 1));

    // clear output
    clear();

    charset = asciiLines;
    //    charset = drawnLines;


    printBoxBoundaries();

    printBorders();

    printJunctions();

    // fill out numbers
    fillValues(whichValue);

    print();

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
    line = output;
    for (i = 1; i < lineLength; i++) {
        *line = charset[horiz];
        line++;
    }

    // last line
    line = output + ((numberOfLines - 1) * lineLengthWithLf);
    for (i = 1; i < lineLength; i++) {
        *line = charset[horiz];
        line++;
    }

    // first column
    line = output;
    for (i = 0; i < numberOfLines; i++) {
        *line = charset[vert];
        line += lineLengthWithLf;
    }

    // last column
    line = output + lineLength - 1;
    for (i = 0; i < numberOfLines; i++) {
        *line = charset[vert];
        line += lineLengthWithLf;
    }

    // print corners
    *(output) = charset[tl]; // left top
    *(output + lineLength - 1) = charset[lb]; // left bottom
    *(output + (numberOfLines - 1) * lineLengthWithLf) = charset[rt]; // right top
    *(output + (numberOfLines - 1) * lineLengthWithLf + lineLength - 1) = charset[br]; // right bottom
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
            *(output + ((1 + field->y * 2) * lineLengthWithLf)
                    + (1 + field->x * 2)) = (char) (value + '0');
        }
    }

}

/**
 * prints the assembled output on stdout
 */
void print() {
    puts(output);
}

/**
 * prints the boundary lines between boxes (or squiggles)
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
    printChar(x, y, 0, 1, charset[vert]);
    printChar(x, y, 1, 1, charset[vert]);
}

/**
 * draws a vertical border between two boxes
 * 
 * @param x
 * @param y
 */
void drawVerticalBorderLeft(unsigned x, unsigned y) {
    printChar(x, y, 1, 0, charset[vert]);
    printChar(x, y, 1, 1, charset[vert]);
}

/**
 * determines the junction type on a specific position
 * 
 * @param x the X position of the left top number of the quadrant
 * @param y the Y position of the left top number of the quadrant
 */
void junction(unsigned x, unsigned y) {
    // 
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

    field = getFieldAt(x, y);
    return field->containerIndexes[containerSetIndexForPrintingBoxes][0];
}

/**
 * determines which of the container sets should be used as an input for
 * drawing boxes.
 * 
 * For a standard Sudoku, this will be the container BOXES, for a squiggle
 * Sudoku, this will be the container SQUIGGLES.
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
 * print a single character on the specified grid position
 * 
 * @param x x position of field in the grid (logical position)
 * @param x y position of field in the grid (logical position)
 * @param deltaX horizontal offset in screen character positions. A positive
 *   offset means a shift to the right, a negative to the left
 * @param deltaY verticall offset in screen character positions. A positive
 *   offset means a shift to the bottom, a negative to the top
 */
void printChar(unsigned x, unsigned y, int deltaX, int deltaY, char c) {
    *(output + ((1 + (y - 1) + deltaY) * lineLengthWithLf)
            + (1 + (x - 1) * 2 + deltaX)) = c;
}