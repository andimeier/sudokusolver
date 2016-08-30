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

// line characters, e.g. t ... "line top", i.e. between quadrant I and II
typedef enum { nil, vert, horiz, tl, lb, br, rt, tlb, lbr, brt, rtl, tlbr } LineChars;

// function prototypes
static void clear();
static void junction();
static void printBorders();
static void printBoxBoundaries();
static void printJunctions();
static void fillValues(FieldValue whichValue);
static void print();
static int getBoxAt(unsigned x, unsigned y);

static Bool boxDifferentThanAbove(unsigned x, unsigned y);
static void drawHorizontalBorderAbove(unsigned x, unsigned y);

// line character set in the order of the enum LineChars
static char asciiLines[13] = " |-+++++++++";
static char drawnLines[13] = { 0x20, 0x78, 0x71, 0x6a, 0x6b, 0x6c, 0x6d, 0x75, 0x77, 0x74, 0x76, 0x6e };


/**
 * prints the Sudoku grid
 * 
 * @param whichValue INITIAL ... print initial values, SOLVED ... print solved
 *   fields
 */
void printGrid(FieldValue whichValue) {

    // allocate memory for building the output in memory first
    lineLength = maxNumber * 2 + 1; /* line length excl. LF */
    lineLengthWithLf = lineLength + 1; /* line length incl. LF */
    numberOfLines = maxNumber * 2 + 1;
    numberOfChars = lineLengthWithLf * numberOfLines; /* times number of lines */
    output = (char *) xmalloc(sizeof (char) * (numberOfChars + 1));

    // clear output
    clear();

    // first print grid
    printBorders();

    printBoxBoundaries();
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
    char *charset;
    
    charset = asciiLines;
//    charset = drawnLines;
    

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

    for (y = 1; y < maxNumber; y++) {
        for (x = 1; x < maxNumber; x++) {
            // horizontal border line?
            if (boxDifferentThanAbove(x, y)) {
                drawHorizontalBorderAbove(x, y);
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
    
}


void drawHorizontalBorderAbove(unsigned x, unsigned y) {
    
}

/**
 * determines the junction type on a specific position
 */
void junction() {
    // 
}



/**
 * determines the box number in which the field at (x/y) is located
 * 
 * @param x
 * @param y
 * @return the box number
 */
int getBoxAt(unsigned x, unsigned y) {
    
}


