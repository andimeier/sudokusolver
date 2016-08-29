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

// function prototypes
static void clear();
static void junction();
static void printBorders();
static void fillValues(FieldValue whichValue);
static void print();



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
            *c = '.';
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
        *line = '-';
        line++;
    }

    // last line
    line = output + ((numberOfLines -1) * lineLengthWithLf);
    for (i = 1; i < lineLength; i++) {
        *line = '-';
        line++;
    }

    // first column
    line = output;
    for (i = 0; i < numberOfLines; i++) {
        *line = '|';
        line += lineLengthWithLf;
    }

    // last column
    line = output + lineLength - 1;
    for (i = 0; i < numberOfLines; i++) {
        *line = '|';
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
 * determines the junction type on a specific position
 */
void junction() {
    // 
}