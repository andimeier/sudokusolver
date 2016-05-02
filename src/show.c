/* 
 * File:   show.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"
#include "show.h"
#include "util.h"
#include "log.h"

// globale Variablen
// das Sudoku-Feld selbst
char *svgFilename; // filename of SVG file



//-------------------------------------------------------------------
// @param showInit {integer} if falsey, only print current grid. If truish,
//   print original (init) grid and current grid

void show(int showInit) {
    // display sudoku
    int x, y;
    int index;
    Field *field;

    printlog(showInit ? "   1 2 3 4 5 6 7 8 9          1 2 3 4 5 6 7 8 9 " : "   1 2 3 4 5 6 7 8 9 ");
    for (y = 0; y < MAX_NUMBER; y++) {

        if (!(y % 3)) {
            // intermediate header row
            printlog(showInit ? "  +-----+-----+-----+        +-----+-----+-----+" : "  +-----+-----+-----+");

        }
        // normal data row
        index = 0;
        if (showInit) {

            buffer[index++] = (char) (y + (int) 'A');
            buffer[index++] = ' ';

            // show starting grid's data row
            for (x = 0; x < MAX_NUMBER; x++) {
                if (x % 3)
                    buffer[index++] = ' ';
                else
                    buffer[index++] = '|';

                field = unitDefs.units[ROWS].fields[y][x];

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
        for (x = 0; x < MAX_NUMBER; x++) {
            if (x % 3)
                buffer[index++] = ' ';
            else
                buffer[index++] = '|';

            field = unitDefs.units[ROWS].fields[y][x];

            if (field->value)
                buffer[index++] = (char) (field->value + 48);
            else
                // leeres Feld
                buffer[index++] = ' ';
        }
        buffer[index++] = '|';
        buffer[index++] = '\0';
        printlog(buffer);
    }

    // intermediate header row
    printlog(showInit ? "  +-----+-----+-----+        +-----+-----+-----+" : "  +-----+-----+-----+");




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
    //	}
    //	printlog("+-----+-----+-----+");
}

/**
 * 
 * @param showInit {integer} if falsey, only print current grid. If truish,
 *   print original (init) grid and current grid
 */
void sudokuString(int showInit) {
    // display sudoku string, e.g. 5600340701000403000130500020400000304
    char *buffer;
    int f;
    int val;

    buffer = (char *) xmalloc(sizeof (char) * NUMBER_OF_FIELDS);

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        val = showInit ? fields[f].initialValue : fields[f].value;
        buffer[f] = (char) (val + '0');
    }
    buffer[f] = '\0';

    // intermediate header row
    printlog(buffer);

    free(buffer);
}


//-------------------------------------------------------------------
// FIXME description is wrong/obsolete:
// if parameter final is truish, the specified svg filename will be used,
// otherwise an indexed file name will be used to store intermediate
// versions of the grid. This will only happen if verboseLogging is turned
// on.
// @param final integer ... 
// If 1, The parameter index specifies the suffix to be used in the filename
// so that the log file can reference to a specific SVG intermediate file
// using this suffix. If index is 0, then no suffix will be appended. This
// indicates that this SVG depicts the final, solved version - not some 
// intermediate version

void printSvg(int finalVersion) {
    // display sudoku in SVG format
    int x, y;
    FILE *svgfile;
    char *filename;
    char suffix[20];
    static int index = 1;
    Field ***rows;
    Field *field;

    if (!svgFilename) return;

    // build filename
    if (finalVersion) {
        filename = svgFilename;
    } else {
        sprintf(suffix, ".%d", index);
        filename = (char *) malloc(sizeof (char)*(strlen(svgFilename) + strlen(suffix) + 1));
        if (filename == NULL) {
            exit(EXIT_FAILURE);
        }

        if (filename == NULL) {
            perror("Not enough memory to allocate memory for SVG filename.");
            exit(EXIT_FAILURE);
        }
        strcpy(filename, svgFilename);
        strcat(filename, suffix);
    }

    if (verboseLogging) {
        sprintf(buffer, "Writing SVG file [%s]", filename);
        printlog(buffer);
    }

    svgfile = fopen(filename, "w");

    fputs("<?xml version='1.0'?>"
            "<?xml-stylesheet href='sudoku_style.css' type='text/css'?>"
            "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN'"
            "  'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>"
            ""
            "<svg version='1.1' xmlns='http://www.w3.org/2000/svg'>"
            ""
            "  <g transform='scale(8)'>"
            "	  <rect class='outer' x='0' y='0' width='81' height='81' />"
            ""
            "	  <line class='thin'  x1='0' y1='9' x2='81' y2='9' />"
            "	  <line class='thin'  x1='0' y1='18' x2='81' y2='18' />"
            "	  <line class='thick' x1='0' y1='27' x2='81' y2='27' />"
            "	  <line class='thin'  x1='0' y1='36' x2='81' y2='36' />"
            "	  <line class='thin'  x1='0' y1='45' x2='81' y2='45' />"
            "	  <line class='thick' x1='0' y1='54' x2='81' y2='54' />"
            "	  <line class='thin'  x1='0' y1='63' x2='81' y2='63' />"
            "	  <line class='thin'  x1='0' y1='72' x2='81' y2='72' />"
            ""
            "	  <line class='thin'  x1='09' y1='0' x2='09' y2='81' />"
            "	  <line class='thin'  x1='18' y1='0' x2='18' y2='81' />"
            "	  <line class='thick' x1='27' y1='0' x2='27' y2='81' />"
            "	  <line class='thin'  x1='36' y1='0' x2='36' y2='81' />"
            "	  <line class='thin'  x1='45' y1='0' x2='45' y2='81' />"
            "	  <line class='thick' x1='54' y1='0' x2='54' y2='81' />"
            "	  <line class='thin'  x1='63' y1='0' x2='63' y2='81' />"
            "	  <line class='thin'  x1='72' y1='0' x2='72' y2='81' />", svgfile);

    rows = unitDefs.units[ROWS].fields;
    for (y = 0; y < MAX_NUMBER; y++) {
        for (x = 0; x < MAX_NUMBER; x++) {
            field = rows[y][x];
            if (field->value) {
                float xPos = x * MAX_NUMBER + 4.5;
                float yPos = y * MAX_NUMBER + 7.65;
                fprintf(svgfile, "<text class=\"final\" x=\"%f\"  y=\"%f\" text-anchor=\"middle\">%d</text>\n", xPos, yPos, field->value);
            } else {
                // alle noch moeglichen Zahlen ausgeben
                int n1;
                for (n1 = 1; n1 <= MAX_NUMBER; n1++) {
                    if (field->candidates[n1 - 1] == n1) {
                        float xPos = x * MAX_NUMBER + ((n1 - 1) % 3) * 3 + 1;
                        float yPos = y * MAX_NUMBER + ((int) ((n1 - 1) / 3) * 3 + 2.4);
                        fprintf(svgfile, "<text class=\"possibilities\" x=\"%f\"  y=\"%f\" text-anchor=\"middle\">%d</text>\n", xPos, yPos, n1);
                    }
                }
            }
        }
    }
    fputs("  </g>\n</svg>", svgfile);

    fclose(svgfile);

    if (filename != svgFilename) free(filename);
}

/**
 * print position of field as e.g. "B1" for 2nd row, 1st column
 * @param field
 * @return {char *} the string representing the position of the field in the 
 *   grid, e.g. "B1" (null-terminated C string)
 */
char *position(Field * field) {
    char *position;

    position = (char*) malloc(sizeof (char)*3);
    if (position == NULL) {
        exit(EXIT_FAILURE);
    }
    position[0] = (char) (field->unitPositions[ROWS] + 65);
    position[1] = (char) (field->unitPositions[COLS] + 48);
    position[2] = '\0';
    return position;
}
