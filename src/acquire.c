/**
 * Acquire Sudoku from any source.
 * E.g. from file.
 * Or from command line string.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "typedefs.h"
#include "grid.h"
#include "logfile.h"
#include "acquire.h"
#include "gametype.h"
#include "box.h"

static void toLowerStr(char *str);

/**
 * read Sudoku from file.
 * Ignore lines starting with '#'.
 * Space or dot will be interpreted as emtpy fields.
 * 
 * @param inputFilename
 * @return success flag: TRUE if Sudoku could be read successfully, FALSE if not
 */
Bool readSudoku(char *inputFilename) {
    char line[201];
    int linecount;
    char c;
    Bool ok;
    int x, y;
    int f;
    FILE *file;
    char *settingName;
    char *settingValue;
    Bool dimensioned;
    Bool shapes;
    unsigned boxWidth;
    unsigned boxHeight;

    sprintf(buffer, "Reading Sudoku from file %s ...", inputFilename);
    logAlways(buffer);

    // open file
    file = fopen(inputFilename, "r");

    if (!file) {
        sprintf(buffer, "Error opening Sudoku file %s", inputFilename);
        logError(buffer);
        return 0;
    }

    // read Sudoku
    ok = TRUE; // optimistic preassumption

    linecount = 0;
    y = 0;
    dimensioned = 0; // we do not know the Sudoku dimension yet
    shapes = 0; // not in the defiition of jigsaw shapes
    while (ok && !feof(file)) {

        if (!fgets(line, 200, file)) {
            break;
        }
        linecount++;

        // remove trailing CR/LF
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1 ] = '\0';
        }
        if (line[strlen(line) - 1] == '\r') {
            line[strlen(line) - 1 ] = '\0';
        }

        if (line[0] == '#') {
            // a comment line => ignore it

        } else if (strchr(line, ':')) {
            // a control line containing the setting name and the value
            settingName = strtok(line, ":");
            settingValue = strtok(NULL, "\r\n");

            // skip spaces at the beginning of the value
            while (*settingValue == ' ') {
                settingValue++;
            }

            // settingName should be case-insensitive
            toLowerStr(settingName);

            // interpret the setting
            if (!strcmp(settingName, "type")) {
                // specify type of Sudoku
                setSudokuType(parseGametypeString(settingValue));
            } else if (!strcmp(settingName, "box")) {
                // specify box size
                parseBoxDimensionString(settingValue, &boxWidth, &boxHeight);
                setBoxDimensions(boxWidth, boxHeight);
            }

        } else {
            // so this must be a standard data line
            sprintf(buffer, "... is a data line and contains row %d ...", y);
            logVerbose(buffer);

            /*
             * the first data line determines intrinsically the geometry of
             * out Sudoku. By reading the first data line, we know how many
             * fields to expect
             */
            if (!dimensioned) {
                dimensionGrid(strlen(line));

                // initialize Sudoku
                allocateFields(numberOfFields);
                for (f = 0; f < numberOfFields; f++) {
                    fields[f].initialValue = 0;
                }

                dimensioned = 1;
            }

            /*
             * go through all chars of the line, should be only digits and 
             * spaces
             */
            if (y >= maxNumber) {
                logError("Error reading the Sudoku from file: too many data rows.");
                ok = FALSE; // oops
                break;
            }

            /*
             * check line length: all data lines must have the same length
             */
            if (strlen(line) != maxNumber) {
                sprintf(buffer, "Error reading the Sudoku from file: first data row has %zu numbers, but line %u has %zu.", maxNumber, linecount, strlen(line));
                logError(buffer);
                ok = FALSE; // oops
                break;
            }

            sprintf(buffer, "Storing line %d ...", y);
            logVerbose(buffer);
            for (x = 0; x < maxNumber; x++) {
                c = line[x];
                if ((c >= '0') && (c <= (char) (maxNumber + (int) '0'))) {
                    fields[y * maxNumber + x].initialValue = (int) (c - '0');
                } else if ((c == ' ') || (c == '.') || (c == '_')) {
                    fields[y * maxNumber + x].initialValue = 0;
                } else {
                    sprintf(buffer, "Error reading the Sudoku from file: illegal character ('%c') in line %d at position %d.", c, x + 1, linecount);
                    logError(buffer);
                    ok = FALSE; // oops, this was no number
                    break;
                }
            }
            y++;
        }
    }
    logVerbose("Sudoku read");

    fclose(file);

    if (ok && y != maxNumber) {
        logError("Error reading the Sudoku from file: too few data rows.");
        ok = FALSE;
    }

    logVerbose("Copy original grid ...");

    // copy original grid
    for (f = 0; f < numberOfFields; f++) {
        fields[f].value = fields[f].initialValue;
        fields[f].correctSolution = 0; // no solution known
    }

    //#define NAKED_TRIPLE
#ifdef NAKED_TRIPLE
    // FIXME fill out final solution in each field, if given
    char solution[82] = "928547316431986572567312894195673428384251769276894153749168235612435987853729641";
    for (f = 0; f < numberOfFields; f++) {
        fields[f].correctSolution = (unsigned) (solution[f] - '0');
    }
#endif
    //#define POINTING_PAIR
#ifdef POINTING_PAIR
    // FIXME fill out final solution in each field, if given
    char solution[82] = "981724365324615879765983142197836254642571938853249716476398521538162497219457683";
    for (f = 0; f < numberOfFields; f++) {
        fields[f].correctSolution = (unsigned) (solution[f] - '0');
    }
#endif

    logVerbose("Initial values filled.");

    return ok;
}

/**
 * import a sudoku as a sequence of characters representing the initial numbers.
 * Empty fields can be defined by using 0, _ or .
 * 
 * @param sudoku the Sudoku string
 * @result success flag: 1 if the Sudoku could be read successfully, 0 if not
 */
Bool importSudoku(char *sudoku) {
    int f;
    char c;

    for (f = 0; f < numberOfFields; f++) {
        c = sudoku[f];
        if (c == '\0') {
            sprintf(buffer, "Error parsing the Sudoku input: unexpected end of Sudoku data after character #%d", f);
            logError(buffer);
            return FALSE;
        }

        if ((c >= '0') && (c <= (char) (maxNumber + (int) '0'))) {
            fields[f].initialValue = (int) (c - '0');
        } else if ((c == ' ') || (c == '.') || (c == '_')) {
            fields[f].initialValue = 0;
        } else {
            sprintf(buffer, "Error parsing the Sudoku input: illegal character ('%c') at position %d.", c, f);
            logError(buffer);
            return FALSE;
        }
    }

    // copy original grid
    for (f = 0; f < numberOfFields; f++) {
        fields[f].value = fields[f].initialValue;
    }

    return TRUE;
}

/**
 * parse Sudoku string 409176000610389040...
 * 
 * @param sudoku the Sudoku string: all numbers concatenated in one string
 *   or - if maxNumber > 9 - characters beginning with 'A', concatenated in
 *   one string // FIXME characters not implenented
 * @param maxNumber maximum number in the Sudoku = width of Sudoku = height
 *   of Sudoku (will be 9 for a standard Sudoku)
 * @return 
 */
Bool parseSudokuString(char *sudoku, int maxNumber) {
    // FIXME not used yet, should be the common function which readSudoku and importSudoku uses
    return FALSE;
}

/**
 * converts a string to lower case.
 * 
 * @param str the string to be lowercased
 */
void toLowerStr(char *str) {
    int i;

    for (i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

/**
 * parses the game type from the command line and tries to find out which
 * game type has to be chosen. Game types are "standard", "x" (X-Sudoku) or
 * "color" (color Sudoku).
 * 
 * @param gametypeString
 * @return 
 */
GameType parseGametypeString(char *gametypeString) {
    GameType gametype;

    if (!strncmp(gametypeString, "standard", strlen(gametypeString))) {
        gametype = STANDARD_SUDOKU;
        logVerbose("Game type: Standard Sudoku");
    } else if (!strncmp(gametypeString, "x", strlen(gametypeString))) {
        gametype = X_SUDOKU;
        logVerbose("Game type: X-Sudoku");
    } else if (!strncmp(gametypeString, "color", strlen(gametypeString))) {
        gametype = COLOR_SUDOKU;
        logVerbose("Game type: Color Sudoku");
    } else {
        sprintf(buffer, "unnknown game type: %s (must be \"standard\", \"x\" or \"color\")", gametypeString);
        logError(buffer);
        exit(EXIT_FAILURE);
    }

    return gametype;
}

/**
 * parse box dimension string, e.g "3x3"
 * 
 * @param boxDimensionString box dimension string, e.g. "3x3"
 */
void parseBoxDimensionString(char *boxDimensionString, unsigned *width, unsigned *height) {
    sscanf(boxDimensionString, "%ux%u", width, height);
}

