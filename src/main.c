#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "solve.h"
#include "global.h"
#include "util.h"
#include "show.h"
#include "log.h"

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
int solve();

char buffer[1000]; // buffer for string operations

int main(int argc, char **argv) {
    int result;
    int c;
    char *outputFilename = NULL; // filename of printlog file
    char *inputFilename = NULL;
    char *sudoku = NULL;

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

    for (int f = 0; f < NUMBER_OF_FIELDS; f++) {
        printf("[1234-1] field #%d: in row %d, col %d, box %d\n", f, fields[f].unitPositions[ROWS], fields[f].unitPositions[COLS], fields[f].unitPositions[BOXES]);
    }

    if (inputFilename && !readSudoku(inputFilename)) {
        return 1; // Oje ... stopp!
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
// Return-Wert
//   1 ... Sudoku wurde erfolgreich geloest
//   0 ... Algorithmus bleibt stecken, Endlositeration abgebrochen

int solve() {
    int iteration;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    iteration = 0;
    errors = 0; // noch keine Fehler aufgetreten

    printf("[4sf]\n");

    printSvg(0);

    printf("[4s65f]\n");

    do {
        iteration++;
        progress = 0; // noch kein neuen Erkenntnis in dieser Runde (hat ja erst begonnen)
        if (verboseLogging == 2) {
            sprintf(buffer, "----- Beginne Iteration %d -----\n", iteration);
            printlog(buffer);
        }

        // TODO:
        //        if (verboseLogging == 2) {
        //            for (y = 0; y < 9; y++) {
        //                for (x = 0; x < 9; x++) {
        //                    if (fields[y][x]) {
        //                        sprintf(buffer, "  Feld (%d/%d): %d\n", y + 1, x + 1, fields[y][x]);
        //                        printlog(buffer);
        //                    } else {
        //                        // TODO sprintf(buffer, "  Feld (%d/%d): %s\n", y + 1, x + 1, possibilities[y][x]);
        //                        printlog(buffer);
        //                    }
        //                }
        //            }
        //        }

        // alle Felder durchgehen und vorkommende Zahlen in der selben
        // Reihe, in der selben Spalte und im selben Quadranten verbieten
        if (verboseLogging == 2)
            printlog("??? Searching for: unique numbers ... \n");

        progress |= checkForSolvedCells();


        if (verboseLogging) {
            printSvg(0);
        }

        progress |= findHiddenSingles();


        if (verboseLogging) {
            printSvg(0);
        }

        //? FIXME FEHLT hier nicht, das nicht nur fuer Spalten und Zeile, sondern auch fuer Quadranten anzuwenden?




        // wenn alle Felder ausgefuellt sind, sind wir wohl fertig!
        if (isFinished())
            return 1;

        progress |= findNakedTuples(2); // find naked pairs

        if (isFinished())
            return 1;

        //progress |= findNakedTuples(3); // find naked triples

        progress |= findPointingTupels(); // find pointing pairs/triples


        if (verboseLogging) {
            printSvg(0);
        }

        // Suche nach lokaler Eingrenzung einer Zahl in einem Quadranten:
        // --------------------------------------------------------------
        // wenn in einem Quadranten eine Zahl nur in Zellen in der gleichen
        // Zeile vorkommen kann, muss sie in diesem Quadranten in dieser
        // Zeile stehen und kann daher fuer die restliche Zeile (ausserhalb
        // des Quadranten) verboten werden.
        // Analog fuer Spalten.

        /*
                // gehe alle Quadranten durch
                int yFound;
                for (q = 0; q < 9; q++) {
                    if (verboseLogging == 2) {
                        sprintf(buffer, "??? Untersuche Quadrant %d auf Zahlen, die auf eine Zeile eingrenzbar sind ...\n", q + 1);
                        printlog(buffer);
                    }
                    getQuadrantStart(q, &qx, &qy);
                    // alle Zahlen durchgehen
                    for (n = 1; n <= 9; n++) {
                        if (verboseLogging == 2) {
                            sprintf(buffer, " Untersuche Quadrant %d auf die Zahl %d ...\n", q + 1, n);
                            printlog(buffer);
                        }
                        yFound = -1; // noch haben wir fuer diese Zahl keine Zeile gefunden
                        for (y = qy; y < qy + 3; y++) {
                            for (x = qx; x < qx + 3; x++) {
                                if (verboseLogging == 2) {
                                    // TODO sprintf(buffer, "  yFound=%d Feld (%d/%d) %d %s\n", yFound + 1, y + 1, x + 1, fields[y][x], (fields[y][x] ? "" : possibilities[y][x]));
                                    printlog(buffer);
                                }
                                // kommt die Zahl in diesem Feld als Moeglichkeit vor?
                                if (fields[y][x] == n) {
                                    // diese Zahl ist bereits fixiert im Quadranten =>
                                    // nach dieser brauche ich nicht weitersuchen
                                    if (verboseLogging == 2) {
                                        // TODO sprintf(buffer, "    Zahl %d ist bereits in (%d/%d) identifiziert!\n", n, y + 1, x + 1);
                                        printlog(buffer);
                                    }
                                    yFound = -1; // nix Tolles gefunden
                                    y = 99; // auch die aeussere Schleife beenden
                                    break; // raus aus der Schleife
                                }
                                if (!fields[y][x] && possibilities[y][x][n - 1] != '0') {
                                    // die Zahl n koennte hier vorkommen
                                    if (yFound == -1) {
                                        // wir merken uns diese Zeile, wenn alle anderen
                                        // Vorkommen auch in dieser Zeile sind, haben wir
                                        // eine wertvolle Information gewonnen!
                                        yFound = y;
                                        if (verboseLogging == 2) {
                                            sprintf(buffer, "    Zahl %d koennte in Zeile %d vorkommen (%d/%d), merke mir die Zeile ...\n", n, y + 1, y + 1, x + 1);
                                            printlog(buffer);
                                        }
                                    } else if (yFound != y) {
                                        // oje, das zweite Vorkommen ist in einer
                                        // anderen Zeile als der gemerkten => Ziel
                                        // nicht erreicht, das bringt uns nix
                                        if (verboseLogging == 2) {
                                            sprintf(buffer, "    Oje, Zahl %d koennte auch in Zeile %d vorkommen (%d/%d), ein Reinfaller.\n", n, y + 1, y + 1, x + 1);
                                            printlog(buffer);
                                        }
                                        yFound = -1; // nix Tolles gefunden
                                        y = 99;
                                        break; // diese Zahl war ein Reinfaller
                                    } else {
                                        if (verboseLogging == 2) {
                                            sprintf(buffer, "    Zahl %d koennte auch hier vorkommen, ebenfalls in Zeile %d ...\n", n, y + 1);
                                            printlog(buffer);
                                        }
                                    }
                                }
                            }
                        }
                        if (yFound != -1) {
                            if (verboseLogging == 2) {
                                sprintf(buffer, "  Hurra! Zahl %d kann im Quadranten %d nur in Zeile %d vorkommen.\n", n, q + 1, yFound + 1);
                                printlog(buffer);
                            }
                            for (x = 0; x < 9; x++) {
                                // wenn ausserhalb unseren Quadranten: alle Vorkommen der
                                // Zahl n verbieten, die muss naemlich im Quadranten q
                                // in dieser Zeile vorkommen
                                if ((x < qx) || (x >= qx + 3)) {
                                    if (!fields[yFound][x])
                                        if (forbidNumber(yFound, x, n)) {
                                            if (verboseLogging == 2) {
                                                sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 4a: (Nummer %d in (%d/%d) verboten weil in Zeile %d diese Zahl im Quadranten %d sein muss.\n", n, yFound + 1, x + 1, yFound + 1, q + 1);
                                                printlog(buffer);
                                            }
                                            progress = 1;
                                        }
                                }
                            }
                        }
                    }
                }

                if (verboseLogging) {
                    printSvg(0);
                }

                // ... analog in Spalten eines Quadranten suchen
                int xFound;
                for (q = 0; q < 9; q++) {
                    if (verboseLogging == 2) {
                        sprintf(buffer, "Untersuche Quadrant %d auf Zahlen, die auf eine Spalte eingrenzbar sind ...\n", q + 1);
                        printlog(buffer);
                    }
                    getQuadrantStart(q, &qx, &qy);
                    // alle Zahlen durchgehen
                    for (n = 1; n <= 9; n++) {
                        if (verboseLogging == 2) {
                            sprintf(buffer, " Untersuche Quadrant %d auf die Zahl %d ...\n", q + 1, n);
                            printlog(buffer);
                        }
                        xFound = -1; // noch haben wir fuer diese Zahl keine Spalte gefunden
                        for (y = qy; y < qy + 3; y++) {
                            for (x = qx; x < qx + 3; x++) {
                                if (verboseLogging == 2) {
                                    sprintf(buffer, "  xFound=%d Feld (%d/%d) %d %s\n", xFound + 1, y + 1, x + 1, fields[y][x], (fields[y][x] ? "" : possibilities[y][x]));
                                    printlog(buffer);
                                }
                                // kommt die Zahl in diesem Feld als Moeglichkeit vor?
                                if (fields[y][x] == n) {
                                    // diese Zahl ist bereits fixiert im Quadranten =>
                                    // nach dieser brauche ich nicht weitersuchen
                                    if (verboseLogging == 2) {
                                        sprintf(buffer, "    Zahl %d ist bereits in (%d/%d) identifiziert!\n", n, y + 1, x + 1);
                                        printlog(buffer);
                                    }
                                    xFound = -1; // nix Tolles gefunden
                                    y = 99; // auch die aeussere Schleife beenden
                                    break; // raus aus der Schleife
                                }
                                if (!fields[y][x] && possibilities[y][x][n - 1] != '0') {
                                    // die Zahl n koennte hier vorkommen
                                    if (xFound == -1) {
                                        // wir merken uns diese Zeile, wenn alle anderen
                                        // Vorkommen auch in dieser Zeile sind, haben wir
                                        // eine wertvolle Information gewonnen!
                                        xFound = x;
                                        if (verboseLogging == 2) {
                                            sprintf(buffer, "    Zahl %d koennte in Spalte %d vorkommen (%d/%d), merke mir die Spalte ...\n", n, x + 1, y + 1, x + 1);
                                            printlog(buffer);
                                        }
                                    } else if (xFound != x) {
                                        // oje, das zweite Vorkommen ist in einer
                                        // anderen Spalte als der gemerkten => Ziel
                                        // nicht erreicht, das bringt uns nix
                                        if (verboseLogging == 2) {
                                            sprintf(buffer, "    Oje, Zahl %d koennte auch in Spalte %d vorkommen (%d/%d), ein Reinfaller.\n", n, x + 1, y + 1, x + 1);
                                            printlog(buffer);
                                        }
                                        xFound = -1; // nix Tolles gefunden
                                        y = 99;
                                        break; // diese Zahl war ein Reinfaller
                                    } else {
                                        if (verboseLogging == 2) {
                                            sprintf(buffer, "    Zahl %d koennte auch hier vorkommen, ebenfalls in Spalte %d ...\n", n, x + 1);
                                            printlog(buffer);
                                        }
                                    }
                                }
                            }
                        }
                        if (xFound != -1) {
                            if (verboseLogging == 2) {
                                sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 5a: Hurra! Zahl %d kann im Quadranten %d nur in Spalte %d vorkommen.\n", n, q + 1, xFound + 1);
                                printlog(buffer);
                            }
                            for (y = 0; y < 9; y++) {
                                // wenn ausserhalb unseren Quadranten: alle Vorkommen der
                                // Zahl n verbieten, die muss naemlich im Quadranten q
                                // in dieser Spalte vorkommen
                                if ((y < qy) || (y >= qy + 3)) {
                                    if (!fields[y][xFound])
                                        if (forbidNumber(y, xFound, n)) {
                                            if (verboseLogging == 2) {
                                                sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 5b:  (Nummer %d in (%d/%d) verboten weil in Spalte %d diese Zahl im Quadranten %d sein muss.\n", n, y + 1, xFound + 1, xFound + 1, q + 1);
                                                printlog(buffer);
                                            }
                                            progress = 1;
                                        }
                                }
                            }
                        }
                    }
                }

                if (verboseLogging) {
                    printSvg(0);
                }

                // nach der Iteration den Sudoku-Zwischenstand anzeigen
                if (verboseLogging) show(0);
         */
    } while (progress);

    showAllCandidates();

    // wir kommen hierher, weil die letzte Iteration keine einzige Aenderung gebracht
    // hat => wir bleiben stecken mit unserem Algorithmus. Ohne Aenderung in der
    // Implementierung ist dieses Sudoku nicht loesbar
    return 0;
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
