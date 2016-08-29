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
    lineLength = (maxNumber * 2 + 1) + 1; /* line length plus LF */
    numberOfLines = (maxNumber * 2 + 1) + 1; /* line length plus LF */
    numberOfChars = lineLength * numberOfLines; /* times number of lines */
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

    for (i = 0; i < numberOfChars; i++) {
        if (i % lineLength) {
            *c = '\n';
        } else {
            *c = ' ';
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
    for (i = 0; i < lineLength - 1; i++) {
        *line = '-';
        line++;
    }

    // last line
    line = output + (maxNumber * lineLength);
    for (i = 0; i < lineLength - 1; i++) {
        *line = '-';
        line++;
    }

    // first column
    line = output;
    for (i = 0; i < numberOfLines; i++) {
        *line = '|';
        line += lineLength;
    }

    // last column
    line = output + lineLength - 1;
    for (i = 0; i < numberOfLines; i++) {
        *line = '|';
        line += lineLength;
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
            *(output + field->y * (lineLength * 2) + field->x * 2) = (char) (value + '0');
        }
    }

}

/**
 * prints the assembled output on stdout
 */
void print() {
    puts(output);
}
