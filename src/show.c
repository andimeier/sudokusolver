/* 
 * File:   show.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "grid.h"
#include "show.h"
#include "util.h"
#include "logfile.h"
#include "solve.h"
#include "log.h"

// global variables

char *svgFilename; // filename of SVG file

// function prototypes
static void showField(Field *field, int showContainers, int appendLf);
static void showContainer(Container *container);
static void showFieldsVector(FieldsVector *fields, int indent, size_t limit);


//-------------------------------------------------------------------
// @param showInit {integer} if falsey, only print current grid. If truish,
//   print original (init) grid and current grid

void show(int showInit) {
    // display sudoku
    int x, y;
    int index;
    Field *field;

    logAlways(showInit ? "   1 2 3 4 5 6 7 8 9          1 2 3 4 5 6 7 8 9 " : "   1 2 3 4 5 6 7 8 9 ");
    for (y = 0; y < maxNumber; y++) {

        if (!(y % 3)) {
            // intermediate header row
            logAlways(showInit ? "  +-----+-----+-----+        +-----+-----+-----+" : "  +-----+-----+-----+");

        }
        // normal data row
        index = 0;
        if (showInit) {

            buffer[index++] = (char) (y + (int) 'A');
            buffer[index++] = ' ';

            // show starting grid's data row
            for (x = 0; x < maxNumber; x++) {
                if (x % 3)
                    buffer[index++] = ' ';
                else
                    buffer[index++] = '|';

                field = &(fields[y * maxNumber + x]);

                if (field->initialValue)
                    buffer[index++] = (char) (field->initialValue + 48);
                else
                    // leeres Feld
                    buffer[index++] = ' ';
            }
            buffer[index++] = '|';
            buffer[index++] = '\0';

            strcat(buffer, (y == 4) ? "  ->  " : "      ");
            index = strlen(buffer);
        }

        buffer[index++] = (char) (y + (int) 'A');
        buffer[index++] = ' ';

        // show current grid's data row
        for (x = 0; x < maxNumber; x++) {
            if (x % 3)
                buffer[index++] = ' ';
            else
                buffer[index++] = '|';

            field = &(fields[y * maxNumber + x]);

            if (field->value)
                buffer[index++] = (char) (field->value + 48);
            else
                // leeres Feld
                buffer[index++] = ' ';
        }
        buffer[index++] = '|';
        buffer[index++] = '\0';
        logAlways(buffer);
    }

    // intermediate header row
    logAlways(showInit ? "  +-----+-----+-----+        +-----+-----+-----+" : "  +-----+-----+-----+");




    //		if (showInit) {
    //			// first show original grid
    //			if (!(y % 3)) {
    //				strcpy(buffer, "+-----+-----+-----+      ");
    //				index = strlen(buffer);
    //			}
    //
    //			for (x = 0; x < 9; x++) {
    //				if (x % 3)
    //					buffer[index++] = ' ';
    //				else
    //					buffer[index++] = '|';
    //				if (initfields[y][x])
    //					buffer[index++] = (char)(initfields[y][x] + 48);
    //				else
    //					// leeres Feld
    //					buffer[index++] = ' ';
    //			}
    //			buffer[index++] = '|';
    //			
    //			if (y == 4) {
    //				buffer[index++] = ' ';
    //				buffer[index++] = ' ';
    //				buffer[index++] = '-';
    //				buffer[index++] = '>';
    //				buffer[index++] = ' ';
    //				buffer[index++] = ' ';
    //			} else {
    //				buffer[index++] = ' ';
    //				buffer[index++] = ' ';
    //				buffer[index++] = ' ';
    //				buffer[index++] = ' ';
    //				buffer[index++] = ' ';
    //				buffer[index++] = ' ';
    //			}
    //		} else {
    //		}
    //		
    //		if (!(y % 3)) {
    //			printlog("+-----+-----+-----+");
    //		}
    //		
    //		for (x = 0; x < 9; x++) {
    //			if (x % 3)
    //				buffer[index++] = ' ';
    //			else
    //				buffer[index++] = '|';
    //			if (fields[y][x])
    //				buffer[index++] = (char)(fields[y][x] + 48);
    //			else
    //				// leeres Feld
    //				buffer[index++] = ' ';
    //		}
    //		buffer[index++] = '|';
    //		buffer[index++] = '\0';
    //		printlog(buffer);
    //	
    //	printlog("+-----+-----+-----+");
}

/**
 * print string representing the current state of the Sudoku (list of found
 * numbers).
 * 
 * Format:
 *   5600340701000403000130500020400000304
 * 
 * @param showInit {integer} if falsey, only print current grid. If truish,
 *   print original (init) grid and current grid
 */
void printSudokuString(int showInit) {
    // display sudoku string, e.g. 5600340701000403000130500020400000304
    char *buffer;
    int f;
    int val;

    buffer = (char *) xmalloc(sizeof (char) * numberOfFields);

    for (f = 0; f < numberOfFields; f++) {
        val = showInit ? fields[f].initialValue : fields[f].value;
        buffer[f] = (char) (val + '0');
    }
    buffer[f] = '\0';

    // intermediate header row
    logAlways(buffer);

    free(buffer);
}

/**
 * print SVG version of the current Sudoku game board
 * 
 * FIXME description is wrong/obsolete:
 * if parameter final is truish, the specified svg filename will be used,
 * otherwise an indexed file name will be used to store intermediate
 * versions of the grid. This will only happen if logLevel is verbose.
 * 
 * @param finalVersion 
 * If 1, The parameter index specifies the suffix to be used in the filename
 * so that the log file can reference to a specific SVG intermediate file
 * using this suffix. If index is 0, then no suffix will be appended. This
 * indicates that this SVG depicts the final, solved version - not some 
 * intermediate version
 */
void printSvg(int finalVersion) {
//    // display sudoku in SVG format
//    int x, y;
//    FILE *svgfile;
//    char *filename;
//    char suffix[20];
//    static int index = 1;
//    Container *rows;
//    Field *field;
//
//    if (!svgFilename) return;
//
//    // build filename
//    if (finalVersion) {
//        filename = svgFilename;
//    } else {
//        sprintf(suffix, ".%d", index);
//        filename = (char *) malloc(sizeof (char)*(strlen(svgFilename) + strlen(suffix) + 1));
//        if (filename == NULL) {
//            exit(EXIT_FAILURE);
//        }
//
//        if (filename == NULL) {
//            perror("Not enough memory to allocate memory for SVG filename.");
//            exit(EXIT_FAILURE);
//        }
//        strcpy(filename, svgFilename);
//        strcat(filename, suffix);
//    }
//
//    if (logLevel >= LOGLEVEL_VERBOSE) {
//        sprintf(buffer, "Writing SVG file [%s]", filename);
//        logVerbose(buffer);
//    }
//
//    svgfile = fopen(filename, "w");
//
//    fputs("<?xml version='1.0'?>"
//            "<?xml-stylesheet href='sudoku_style.css' type='text/css'?>"
//            "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN'"
//            "  'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>"
//            ""
//            "<svg version='1.1' xmlns='http://www.w3.org/2000/svg'>"
//            ""
//            "  <g transform='scale(8)'>"
//            "	  <rect class='outer' x='0' y='0' width='81' height='81' />"
//            ""
//            "	  <line class='thin'  x1='0' y1='9' x2='81' y2='9' />"
//            "	  <line class='thin'  x1='0' y1='18' x2='81' y2='18' />"
//            "	  <line class='thick' x1='0' y1='27' x2='81' y2='27' />"
//            "	  <line class='thin'  x1='0' y1='36' x2='81' y2='36' />"
//            "	  <line class='thin'  x1='0' y1='45' x2='81' y2='45' />"
//            "	  <line class='thick' x1='0' y1='54' x2='81' y2='54' />"
//            "	  <line class='thin'  x1='0' y1='63' x2='81' y2='63' />"
//            "	  <line class='thin'  x1='0' y1='72' x2='81' y2='72' />"
//            ""
//            "	  <line class='thin'  x1='09' y1='0' x2='09' y2='81' />"
//            "	  <line class='thin'  x1='18' y1='0' x2='18' y2='81' />"
//            "	  <line class='thick' x1='27' y1='0' x2='27' y2='81' />"
//            "	  <line class='thin'  x1='36' y1='0' x2='36' y2='81' />"
//            "	  <line class='thin'  x1='45' y1='0' x2='45' y2='81' />"
//            "	  <line class='thick' x1='54' y1='0' x2='54' y2='81' />"
//            "	  <line class='thin'  x1='63' y1='0' x2='63' y2='81' />"
//            "	  <line class='thin'  x1='72' y1='0' x2='72' y2='81' />", svgfile);
//
//    rows = containerSets[0].containers;
//    for (y = 0; y < maxNumber; y++) {
//        for (x = 0; x < maxNumber; x++) {
//            //            field = rows[y][x];
//            //            if (field->value) {
//            //                float xPos = x * maxNumber + 4.5;
//            //                float yPos = y * maxNumber + 7.65;
//            //                fprintf(svgfile, "<text class=\"final\" x=\"%f\"  y=\"%f\" text-anchor=\"middle\">%d</text>\n", xPos, yPos, field->value);
//            //            } else {
//            //                // alle noch moeglichen Zahlen ausgeben
//            //                int n1;
//            //                for (n1 = 1; n1 <= maxNumber; n1++) {
//            //                    if (field->candidates[n1 - 1] == n1) {
//            //                        float xPos = x * maxNumber + ((n1 - 1) % 3) * 3 + 1;
//            //                        float yPos = y * maxNumber + ((int) ((n1 - 1) / 3) * 3 + 2.4);
//            //                        fprintf(svgfile, "<text class=\"possibilities\" x=\"%f\"  y=\"%f\" text-anchor=\"middle\">%d</text>\n", xPos, yPos, n1);
//            //                    }
//            //                }
//            //            }
//        }
//    }
//    fputs("  </g>\n</svg>", svgfile);
//
//    fclose(svgfile);
//
//    if (filename != svgFilename) free(filename);
}

/**
 * prints a field on stdout. No \n appended
 * 
 * @param field the field to be printed
 * @param showContainers flag whether the containers holding this field should
 *   be printed
 * @param appendLf flag whether a line feed should be appended or not
 */
void showField(Field *field, int showContainers, int appendLf) {
    char *candidates;
    int i;

    assert(maxNumber <= 9);

    printf("Field %s: ", field->name);
    if (field->initialValue) {
        printf(" (initial: %u) ", field->initialValue);
    } 
    
    if (field->value) {
        // already solved
        printf("= %u", field->value);
    } else {
        // not solved yet

        candidates = strdup("123456789");
        for (i = 0; i < maxNumber; i++) {
            if (!field->candidates[i]) {
                candidates[i] = ' ';
            }
        }
        // terminate string
        candidates[maxNumber + 1] = '\0';

        printf("[%s]", candidates);
        if (field->correctSolution) {
            printf(" (solution: %u)", field->correctSolution);
        }

        free(candidates);
    }

    if (showContainers) {
        printf(" in containers: ");
        int first = 1;
        for (int c = 0; c < numberOfContainerSets; c++) {
            Container **containersPtr;
            for (containersPtr = field->containers[c]; *containersPtr;  containersPtr++) {
                printf("%s%s", first ? "" : ", ", (*containersPtr)->name);
                first = 0;
            }
            
        }
    }

    if (appendLf) {

        printf("\n");
    }
}

/**
 * shows the content of a container (for debugging purposes)
 * @param container
 */
void showContainer(Container *container) {

    printf("Container: %s\n", container->name);
    showFieldsVector(container->fields, 1, maxNumber);
}

/**
 * shows the content of a field list (for debugging purposes)
 * @param fields
 * @param indent flag whether the field list should be indented
 * @parma limit if >0, limits the number of fields analysed, this is useful
 *   for list of fields which are not NULL terminated to provide a manual limit
 */
void showFieldsVector(FieldsVector *fields, int indent, size_t limit) {
    int counter;

    counter = 0;
    while (*fields) {
        if (indent) {
            printf("  ");
        }
        showField(*fields, 0, 1);
        fields++;

        counter++;
        if (limit && counter >= limit) {
            break;
        }
    }
}

/**
 * shows the content of a number list (for debugging purposes)
 * @param fields
 * @param indent flag whether the field list should be indented
 */
void showNumbersVector(unsigned *numbers, int indent) {
    while (*numbers) {
        if (indent) {

            printf("  ");
        }
        printf("%u ", *numbers);
        numbers++;
    }
}

/**
 * Alias to showField()
 * sf ... "show field"
 * 
 * @param field
 */
void sf(Field *field) {

    showField(field, 1, 1);
}

/**
 * Alias to showContainer
 * sc ... "show container"
 * 
 * @param container
 */
void sc(Container *container) {

    showContainer(container);
}

/**
 * Alias to showFieldsVector
 * sfv ... "show fields vector"
 * 
 * @param fields
 * @parma limit if >0, limits the number of fields analysed, this is useful
 *   for list of fields which are not NULL terminated to provide a manual limit
 */
void sfv(FieldsVector *fields, size_t limit) {
    showFieldsVector(fields, 0, limit);
}

/**
 * Alias to showNumbersVector
 * sns ... "Show NumberS"
 * 
 * @param numbers
 */
void sns(unsigned *numbers) {
    showNumbersVector(numbers, 0);
}

/**
 * Alias to showFieldsVector.
 * sfs ... "show fields"
 * 
 * @param fields
 * @parma limit if >0, limits the number of fields analysed, this is useful
 *   for list of fields which are not NULL terminated to provide a manual limit
 */
void sfs(FieldsVector *fields, size_t limit) {
    showFieldsVector(fields, 0, limit);
}

/**
 * print all strategies which have been used
 */
void printInvolvedStrategies() {
    Strategy **strategyPtr;
    size_t count;

    // FIXME this text should be in a place where it is guaranteed that the Sudoku
    // has indeed been SOLVED, not aborted (I refer to the wording of the text):
    logAlways("Utilized strategies:");

    strategyPtr = strategies;
    count = 0;
    while (*strategyPtr) {
        if ((*strategyPtr)->used) {

            // strategy has been used
            sprintf(buffer, "  X %s", (*strategyPtr)->name);
            logAlways(buffer);

            count++;
        } else {
            // strategy has not been used
            sprintf(buffer, "  - %s (not used)", (*strategyPtr)->name);
            logAlways(buffer);
        }

        strategyPtr++;
    }

    if (!count) {
        // no strategy has been used
        logAlways("(no strategy has been used)");
    }
}
