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

static void toLowerStr(char *str);

/**
 * read Sudoku from file.
 * Ignore lines starting with '#'.
 * Space or dot will be interpreted as emtpy fields.
 * 
 * @param inputFilename
 * @return success flag: 1 if Sudoku could be read successfully, 0 if not
 */
int readSudoku(char *inputFilename) {
    char line[201];
    int linecount;
    char c;
    int ok;
    int x, y;
    int f;
    FILE *file;
    char *settingName;
    char *settingValue;
    unsigned dimensioned;

    sprintf(buffer, "Reading Sudoku from file %s ...", inputFilename);
    logAlways(buffer);

    // open file
    file = fopen(inputFilename, "r");

    if (!file) {
        sprintf(buffer, "Error opening Sudoku file %s", inputFilename);
        logError(buffer);
        return 0;
    }

    // initialize Sudoku
    for (f = 0; f < numberOfFields; f++) {
        fields[f].initialValue = 0;
    }

    // read Sudoku
    ok = 1; // optimistic preassumption

    linecount = 0;
    y = 0;
    dimensioned = 0; // we do not know the Sudoku dimension yet
    while (ok && !feof(file)) {

        if (!fgets(line, 200, file)) {
            break;
        }
        linecount++;

        sprintf(buffer, "Next line read: %s ...", line);
        logVerbose(buffer);

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
                dimensioned = 1;
            }

            /*
             * go through all chars of the line, should be only digits and 
             * spaces
             */
            if (y >= maxNumber) {
                logError("Error reading the Sudoku from file: too many data rows.");
                ok = 0; // oops
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
                    ok = 0; // oops, this was no number
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
        ok = 0;
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
int importSudoku(char *sudoku) {
    int f;
    char c;

    for (f = 0; f < numberOfFields; f++) {
        c = sudoku[f];
        if (c == '\0') {
            sprintf(buffer, "Error parsing the Sudoku input: unexpected end of Sudoku data after character #%d", f);
            logError(buffer);
            return 0;
        }

        if ((c >= '0') && (c <= (char) (maxNumber + (int) '0'))) {
            fields[f].initialValue = (int) (c - '0');
        } else if ((c == ' ') || (c == '.') || (c == '_')) {
            fields[f].initialValue = 0;
        } else {
            sprintf(buffer, "Error parsing the Sudoku input: illegal character ('%c') at position %d.", c, f);
            logError(buffer);
            return 0;
        }
    }

    // copy original grid
    for (f = 0; f < numberOfFields; f++) {
        fields[f].value = fields[f].initialValue;
    }

    return 1;
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
int parseSudokuString(char *sudoku, int maxNumber) {
    // FIXME not used yet, should be the common function which readSudoku and importSudoku uses
    return 0;
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
unsigned parseGametypeString(char *gametypeString) {
    unsigned gametype;

    if (!strncmp(gametypeString, "standard", strlen(gametypeString))) {
        gametype = GAME_STANDARD_SUDOKU;
        logVerbose("Game type: Standard Sudoku");
    } else if (!strncmp(gametypeString, "x", strlen(gametypeString))) {
        gametype = GAME_X_SUDOKU;
        logVerbose("Game type: X-Sudoku");
    } else if (!strncmp(gametypeString, "color", strlen(gametypeString))) {
        gametype = GAME_COLOR_SUDOKU;
        logVerbose("Game type: Color Sudoku");
    } else {
        sprintf(buffer, "unnknown game type: %s (must be \"standard\", \"x\" or \"color\")", gametypeString);
        logError(buffer);
        exit(EXIT_FAILURE);
    }

    return gametype;
}
