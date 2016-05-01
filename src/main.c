#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include "solve.h"
#include "global.h"
#include "util.h"
#include "show.h"
#include "log.h"
#include "init.h"

/*
Fehlende Strategien:

1. versteckte Paare finden
wenn in einer row/col/quadrant nur zwei Zellen gibt, wo x und y vorkommen koennen, sind das Paare - auch, wenn
diese beiden Zellen auch fuer einen Haufen anderer Zahlen vorgesehen sind (standard.1232a.sudoku)

2. Wenn in einer Reihe x und y nur an 2 Positionen im gleichen Quadranten vorkommen koennen, koennen diese im restlichen
Quadranten nicht mehr vorkommen. (standard.1424a.sudoku)

Umwandlung des Formats "sudokusolver" ("_3___1___+__6____5_+5_____983+_8___63_2+____5____+9_38___6_+714_____9+_2____8__+___4___3_")
in das hier erwartete:
  
  cat FILE.txt | sed -e 's/+/\n/g' -e 's/_/0/g' | ./solve_sudoku.exe

 */

void printUsage();
int readSudoku();
int importSudoku(char *sudoku);

int main(int argc, char **argv) {
    int result;
    int c;
    char *outputFilename = NULL; // filename of printlog file
    char *inputFilename = NULL;
    char *sudoku = NULL;

    // if the Sudoku is wider than 26 numbers, we have a memory allocation issue
    // with the field->name (what is right of "Z26"?)
    assert(MAX_NUMBER <= 26);
    
    // read command line arguments
    opterr = 0;

    while ((c = getopt(argc, argv, "f:hvVl:s:")) != -1)
        switch (c) {
            case 'v':
                verboseLogging = 1;
                break;
            case 'V':
                verboseLogging = 2;
                break;
            case 's':
                svgFilename = optarg;
                break;
            case 'l':
                outputFilename = optarg;
                break;
            case 'f':
                inputFilename = optarg;
                break;
            case 'h':
                printUsage();
                return 0;
                break;
            case '?':
                if (optopt == 'l' || optopt == 's')
                    fprintf(stderr, "Option -%d requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }

    // first positional parameter is a Sudoku string
    if (optind < argc) {
        sudoku = argv[optind];
    }

    for (int i = optind; i < argc; i++) {
        //  process positional parameters
    }

    // if no sudoku is given
    if (!sudoku && !inputFilename) {
        fprintf(stderr, "No Sudoku data given. Please either specify a file with -f or a Sudoku string.\n");
        fprintf(stderr, "See usage page for details: -h");
        exit(EXIT_FAILURE);
    }

    if (outputFilename) {
        openLogFile(outputFilename);
    }

    initFields();
    initUnits();
    initGrid();

    if (inputFilename && !readSudoku(inputFilename)) {
        return 1; // Oje ... stopp!
    }

    if (sudoku) {
        importSudoku(sudoku);
    }

    if (verboseLogging) {
        printlog("Initial Sudoku:");
        show(0);
    }

    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        printf("[1234-2] field #%d: in row %d, col %d, box %d\n", f, fields[f].unitPositions[ROWS], fields[f].unitPositions[COLS], fields[f].unitPositions[BOXES]);
    }

    result = solve();

    show(1);
    printSvg(1);

    if (result) {
        printlog("-----------------------------------------------");
        printlog("         FERTIG, SUDOKU WURDE GELOEST!");
        printlog("-----------------------------------------------");
    } else {

        int numbersFound = 0;
        for (int f = 0; f < NUMBER_OF_FIELDS; f++)
            if (fields[f].value)
                numbersFound++;

        printlog("-----------------------------------------------");
        printlog("      Sudoku konnte nicht geloest werden!");
        sprintf(buffer, "      %d von %d Zellen wurden gefunden.", numbersFound, NUMBER_OF_FIELDS);
        printlog(buffer);
        printlog("-----------------------------------------------");
        sudokuString();
    }

    if (errors) {
        sprintf(buffer, "Es sind %d FEHLER aufgetreten!\n", errors);
        printlog(buffer);
    }


    closeLogFile();

    freeUnits();
    freeGrid();

    exit(EXIT_SUCCESS);
}




//-------------------------------------------------------------------

void printUsage() {
    // print program usage

    puts("Usage: ");
    puts(" sudoku-solver -l LOGFILE -s SVGFILE -v -h [ SUDOKU_STRING ]");
    puts("");
    puts("Parameters:");
    puts("");
    puts("  -f SUDOKUFILE  the Sudoku input file containing 9 lines of 9 number each.");
    puts("              This represents the starting state of the Sudoku.");
    puts("  -l LOGFILE  printlog into LOGFILE (filename) instead of stdout");
    puts("  -s SVGFILE  write SVG representation of Sudoku grid into SVG files, each iteration will write");
    puts("              another SVG file with a numeric suffix appended. The SVG file without an additional");
    puts("              numeric suffix is the final, solved grid.");
    puts("              For example, when the parameter -s test.svg is specified, you will end up with SVG");
    puts("              files of test.svg.1, test.svg.2, test.svg.3 etc. plus the final grid, stored in the");
    puts("              file test.svg (without additional suffix).");
    puts("  -v          verbose logging");
    puts("  -V          very verbose logging");
    puts("  -h          this help screen");
    puts("  SUDOKU_STRING a Sudoku in the one-string format. If given, overrides the -f setting.");
}


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
    printf("REALLY!\n");

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
 */
int importSudoku(char *sudoku) {
    int f;
    char c;

    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        c = sudoku[f];
        if (c == '\0') {
            sprintf(buffer, "Error parsing the Sudoku input: unexpected end of Sudoku data after character #%d", f);
            printlog(buffer);
            exit(EXIT_FAILURE);
        }

        if ((c >= '0') && (c <= (char) (MAX_NUMBER + (int) '0'))) {
            fields[f].initialValue = (int) (c - '0');
        } else if ((c == ' ') || (c == '.') || (c == '_')) {
            fields[f].initialValue = 0;
        } else {
            sprintf(buffer, "Error parsing the Sudoku input: illegal character ('%c') at position %d.\n", c, f);
            printlog(buffer);
            exit(EXIT_FAILURE);
        }
    }

    // copy original grid
    for (f = 0; f < NUMBER_OF_FIELDS; f++) {
        fields[f].value = fields[f].initialValue;
    }

    return 1;
}