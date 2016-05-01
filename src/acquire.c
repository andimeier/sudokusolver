#include <stdio.h>
#include "typedefs.h"
#include "grid.h"
#include "log.h"
#include "acquire.h"


//-------------------------------------------------------------------
// Lies das Sudoku aus einem File ein.
// Zeilen, die mit '#' beginnen, werden ignoriert.
// Leere Felder werden durch Leerzeichen oder Punkte repraesentiert.
//   1 ... ok, Sudoku wurde eingelesen
//   0 ... Fehler beim Lesen

int readSudoku(char *inputFilename) {
    char line[201];
    int linecount;
    char c;
    int ok;
    int x, y;
    int f;
    FILE *file;

    printf("Reading Sudoku from file %s ...\n", inputFilename);

    // open file
    file = fopen(inputFilename, "r");

    if (!file)
        return 0;

    // Sudoku initialisieren
    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        fields[f].initialValue = 0;
    }

    // Sudoku einlesen
    ok = 1; // optimistische Grundannahme

    linecount = 0;
    y = 0;
    while (ok && !feof(file)) {

        if (!fgets(line, 200, file)) {
            break;
        }
        linecount++;

        printf("Next line read: %s ...\n", line);

        if (line[0] != '#') {
            printf("... is a data line and contains row %d ...\n", y);

            // alle Zeichen der Zeile durchgehen, das sollten nur Ziffern 
            // und Leerzeichen sein
            if (y >= MAX_NUMBER) {
                printlog("Error reading the Sudoku from file: too many data rows.");
                ok = 0; // oops
                break;
            }
            printf("Storing line %d ...\n", y);
            for (x = 0; x < MAX_NUMBER; x++) {
                c = line[x];
                if ((c >= '0') && (c <= (char) (MAX_NUMBER + (int) '0'))) {
                    fields[y * MAX_NUMBER + x].initialValue = (int) (c - '0');
                } else if ((c == ' ') || (c == '.') || (c == '_')) {
                    fields[y * MAX_NUMBER + x].initialValue = 0;
                } else {
                    sprintf(buffer, "Error reading the Sudoku from file: illegal character ('%c') in line %d at position %d.\n", c, x + 1, linecount);
                    printlog(buffer);
                    ok = 0; // oops, this was no number
                    break;
                }
            }
            y++;
        } else {
            // eine Kommentarzeile
        }
    }
    printf("Sudoku read\n");

    fclose(file);

    if (ok && y != MAX_NUMBER) {
        printlog("Error reading the Sudoku from file: too few data rows.");
        ok = 0;
    }

    printf("Copy original grid ...\n");

    // copy original grid
    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        fields[f].value = fields[f].initialValue;
    }

    printf("Initial values filled.\n");

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

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        c = sudoku[f];
        if (c == '\0') {
            sprintf(buffer, "Error parsing the Sudoku input: unexpected end of Sudoku data after character #%d", f);
            printlog(buffer);
            return 0;
        }

        if ((c >= '0') && (c <= (char) (MAX_NUMBER + (int) '0'))) {
            fields[f].initialValue = (int) (c - '0');
        } else if ((c == ' ') || (c == '.') || (c == '_')) {
            fields[f].initialValue = 0;
        } else {
            sprintf(buffer, "Error parsing the Sudoku input: illegal character ('%c') at position %d.\n", c, f);
            printlog(buffer);
            return 0;
        }
    }

    // copy original grid
    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        fields[f].value = fields[f].initialValue;
    }

    return 1;
}
