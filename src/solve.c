/* 
 * File:   solve.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "solve.h"
#include "show.h"
#include "global.h"
#include "util.h"

static int getUniquePositionInRow(int n, int y);
static int getUniquePositionInColumn(int n, int x);
static int getUniquePositionInBox(int n, int q);

static int isolateColumnTwins(int x, int y3, int y2);
static int isolateRowTwins(int y, int x1, int x2);
static int isolateBoxTuples(int q, int y1, int x1, int y2, int x2);

// auxiliary functions
static int setUniqueNumber(int x, int y);



int fields[9][9];
int initfields[9][9];
char possibilities[10][10][10]; // pro Feld die moeglichen Zahlen, als C-String, zB "123000080"
int nrOfPossibilities[9][9]; // Anzahl der verbleibenden Moeglichkeiten pro Zelle

int errors; // number of errors in the algorithm
int verboseLogging; // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations

void initGrid() {
    int x, y;

    // Initialisierung:
    // zunaechst sind ueberall alle Zahlen moeglich
    for (y = 0; y < 9; y++) {
        for (x = 0; x < 9; x++) {
            strcpy(possibilities[y][x], "123456789");
            nrOfPossibilities[y][x] = 9;
        }
    }
}


//-------------------------------------------------------------------
// Checkt, ob alle Zellen mit einer Zahl befuellt sind, dann sind 
// wir naemlich fertig!
// Return-Wert:
//   1 ... fertig (in jeder Zelle steht eine Zahl)
//   0 ... noch nicht fertig

int isFinished() {
    int x, y;

    for (y = 0; y < 9; y++) {
        for (x = 0; x < 9; x++) {
            if (!fields[y][x])
                // ein leeres Feld gefunden => wir sind noch nicht fertig!
                return 0;
        }
    }
    return 1;
}


//-------------------------------------------------------------------
// Setzt in dem Feld x/y (das nur mehr eine Moeglichkeit aufweisen muss)
// die einzige Zahl, die in den Candidates gefunden wird.
// Es muss sichergestellt sein, dass nur mehr eine Zahl moeglich ist,
// hier wird das nicht mehr ueberprueft - der erste Candidate wird als 
// "einzig moegliche Zahl" behandelt.
// Return-Wert:
//   die fixierte Zahl

int setUniqueNumber(int x, int y) {
    int n;

    assert(x >= 0 && x < 9);
    assert(y >= 0 && y < 9);

    if (fields[y][x]) {
        if (verboseLogging == 2) {
            sprintf(buffer, "FEHLER! HUCH! Obwohl schon ausgefuellt, wird das aufgerufen! (%d/%d) soll gesetzt werden, ist aber bereits %d!\n", y + 1, x + 1, fields[y][x]);
            printlog(buffer);
            sprintf(buffer, "Fehler vor inc: %d\n", errors); //?DEBUG
            printlog(buffer);
        }
        errors++;
        if (verboseLogging) {
            sprintf(buffer, "Fehler nach inc: %d\n", errors); //?DEBUG
            printlog(buffer);
        }
    }

    for (n = 1; n <= 9; n++)
        if (possibilities[y][x][n - 1] != '0') {
            if (verboseLogging == 2) {
                sprintf(buffer, "Aha, nur mehr eine Moeglichkeit in Feld (%d/%d) (possibilities: %s): %d\n", y + 1, x + 1, possibilities[y][x], n);
                printlog(buffer);
            }
            fields[y][x] = n;
            break;
        }

    return n;
}

//-------------------------------------------------------------------
// Checkt die Anzahl der moeglichen Vorkommnisse einer Zahl in der
// Reihe y.
// Liefert:
//   x ... x-Position des Feldes, in dem die Zahl n als einziges Feld
//         der ganzen Reihe vorkommen koennte oder
//   -1 ... Zahl koennte in der Zeile an mehreren Positionen vorkommen

int getUniquePositionInRow(int n, int y) {
    int x;
    int unique;
    int xPosition;

    assert(y >= 0 && y < 9);
    assert(n >= 1 && n <= 9);

    unique = 0;
    xPosition = 0;
    for (x = 0; x < 9; x++) {
        if ((fields[y][x] == n) || (!fields[y][x] && (possibilities[y][x][n - 1] == (char) (n + 48)))) {
            if (!unique) {
                unique = 1; // erstes gefundenes Vorkommen in der Reihe
                xPosition = x; // Position merken, falls sie eindeutig ist
            } else {
                // oje, das waere schon das 2. Vorkommen der Zahl in dieser Reihe
                return -1; // war wohl nix
            }
        }
    }
    if (unique)
        return xPosition;

    return -1;
}

//-------------------------------------------------------------------
// Checkt die Anzahl der moeglichen Vorkommnisse einer Zahl in der
// Spalte x.
// Liefert:
//   y ... y-Position des Feldes, in dem die Zahl n als einziges Feld
//         der ganzen Spalte vorkommen koennte oder
//   -1 ... Zahl koennte in der Spalte an mehreren Positionen vorkommen

int getUniquePositionInColumn(int n, int x) {
    int y;
    int unique;
    int yPosition;

    assert(x >= 0 && x < 9);
    assert(n >= 1 && n <= 9);

    if (verboseLogging == 2) {
        sprintf(buffer, "Suche nach Moeglichkeiten fuer %d in Spalte %d\n", n, x + 1);
        printlog(buffer);
    }
    unique = 0;
    yPosition = 0;
    for (y = 0; y < 9; y++) {
        if ((fields[y][x] == n) || (!fields[y][x] && (possibilities[y][x][n - 1] == (char) (n + 48)))) {
            if (verboseLogging == 2) {
                sprintf(buffer, "  %d kann in Zeile %d (%d/%d) vorkommen [%s].\n", n, y + 1, y + 1, x + 1, possibilities[y][x]);
                printlog(buffer);
            }
            if (!unique) {
                unique = 1; // erstes gefundenes Vorkommen in der Spalte
                yPosition = y; // Position merken, falls sie eindeutig ist
            } else {
                // oje, das waere schon das 2. Vorkommen der Zahl in dieser Spalte
                return -1; // war wohl nix
            }
        }
    }
    if (unique)
        return yPosition;
    else {
        if (verboseLogging) {
            sprintf(buffer, "3: Nanu, Zahl %d kann nie vorkommen in der Spalte %d??\n", n, x);
            printlog(buffer);
        }
    }
    return -1;
}

//-------------------------------------------------------------------
// Checkt die Anzahl der moeglichen Vorkommnisse einer Zahl im 
// Quadrant q.
// Liefert:
//   position ... Position des Feldes, in dem die Zahl n als einziges Feld
//         des ganzen Quadranten vorkommen koennte oder [0..8]
//   -1 ... Zahl koennte im Quadranten an mehreren Positionen vorkommen

int getUniquePositionInBox(int n, int q) {
    int x, y;
    int unique;
    int position;
    int i;

    assert(q >= 0 && q < 9);
    assert(n >= 1 && n <= 9);

    if (verboseLogging == 2) {
        sprintf(buffer, "Suche nach Moeglichkeiten fuer %d in Quadrant %d\n", n, q + 1);
        printlog(buffer);
    }
    unique = 0;
    position = 0;
    for (i = 0; i < 9; i++) {
        getQuadrantField(q, i, &x, &y);
        if ((fields[y][x] == n) || (!fields[y][x] && (possibilities[y][x][n - 1] == (char) (n + 48)))) {
            if (verboseLogging == 2) {
                sprintf(buffer, "  %d kann in Quadrant %d (%d/%d) vorkommen [%s].\n", n, q + 1, y + 1, x + 1, possibilities[y][x]);
                printlog(buffer);
            }
            if (!unique) {
                unique = 1; // erstes gefundenes Vorkommen im Quadranten
                position = i; // Position merken, falls sie eindeutig ist
            } else {
                // oje, das waere schon das 2. Vorkommen der Zahl in diesem Quadranten
                return -1; // war wohl nix
            }
        }
    }
    if (unique)
        return position;
    else {
        if (verboseLogging) {
            sprintf(buffer, "3: Nanu, Zahl %d kann nie vorkommen im Quadranten %d??\n", n, x);
            printlog(buffer);
        }
    }
    return -1;
}

//-------------------------------------------------------------------
// "Isoliert" Zwillinge in einer Spalte: die beiden Zahlenpaare, die
// in diesen beiden Zellen moeglich sind, koennen im Rest der Spalte 
// nicht mehr vorkommen
// Return-Wert:
//   1 ... mind. 1 Nummer in der restlichen Spalte oder dem restlichen
//         Quadranten wurde verboten, wir "sind weitergekommen"
//   0 ... Isolieren der Zwillinge hat keine Aenderung im Sudoku bewirkt

int isolateColumnTwins(int x, int y1, int y2) {
    int n;
    int progress;
    int row;
    int c;

    assert(x >= 0 && x < 9);
    assert(y1 >= 0 && y1 < 9);
    assert(y2 >= 0 && y2 < 9);

    progress = 0; // noch hat sich nichts veraendert
    if (verboseLogging == 2) {
        sprintf(buffer, "Isoliere Zwillinge (%d/%d) und (%d/%d): %s/%s\n", y1 + 1, x + 1, y2 + 1, x + 1, possibilities[y1][x], possibilities[y2][x]);
        printlog(buffer);
    }

    // die 2 Zahlen herausfinden
    for (c = 0; c < 9; c++) {
        n = (int) possibilities[y1][x][c] - 48;
        if (n) {
            // diese Zahl n ueberall sonst verbieten im Quadranten und in der Spalte

            // in der restlichen Spalte verbieten
            if (verboseLogging == 2) {
                sprintf(buffer, "Gehe Spalte %d durch und verbiete %d ...\n", x, n);
                printlog(buffer);
            }
            for (row = 0; row < 9; row++) {
                if ((row != y1) && (row != y2) && !fields[row][x] && forbidNumber(row, x, n)) {
                    if (verboseLogging == 2) {
                        sprintf(buffer, " (Nummer %d in der gleichen Spalte %d wie Zwilling (%d/%d) und (%d/%d) verboten)\n", n, x + 1, y1 + 1, x + 1, y2 + 1, x + 1);
                        printlog(buffer);
                    }
                    progress = 1;
                }
            }
        }
    }
    return progress;
}

//-------------------------------------------------------------------
// "Isoliert" Zwillinge in einer Zeile: die beiden Zahlenpaare, die
// in diesen beiden Zellen moeglich sein, koennen im Rest der Zeile 
// nicht mehr vorkommen
// Return-Wert:
//   1 ... mind. 1 Nummer in der restlichen Zeile oder dem restlichen
//         Quadranten wurde verboten, wir "sind weitergekommen"
//   0 ... Isolieren der Zwillinge hat keine Aenderung im Sudoku bewirkt

int isolateRowTwins(int y, int x1, int x2) {
    int n;
    int progress;
    int col;
    int c;

    assert(y >= 0 && y < 9);
    assert(x1 >= 0 && x1 < 9);
    assert(x2 >= 0 && x2 < 9);

    progress = 0; // noch hat sich nichts veraendert
    if (verboseLogging == 2) {
        sprintf(buffer, "Isoliere Zwillinge (%d/%d) und (%d/%d): %s/%s\n", y + 1, x1 + 1, y + 1, x1 + 1, possibilities[y][x1], possibilities[y][x2]);
        printlog(buffer);
    }

    // die 2 Zahlen herausfinden
    for (c = 0; c < 9; c++) {
        n = (int) possibilities[y][x1][c] - 48;
        if (n) {
            // diese Zahl n ueberall sonst verbieten im Quadranten und in der Zeile

            // in der restlichen Zeile verbieten
            if (verboseLogging == 2) {
                sprintf(buffer, "Gehe Zeile %d durch und verbiete %d ...\n", y, n);
                printlog(buffer);
            }
            for (col = 0; col < 9; col++) {
                if ((col != x1) && (col != x2) && !fields[y][col] && forbidNumber(y, col, n)) {
                    if (verboseLogging == 2) {
                        sprintf(buffer, " (Nummer %d in der gleichen Zeile %d wie Zwilling (%d/%d) und (%d/%d) verboten)\n", n, y + 1, y + 1, x1 + 1, y + 1, x2 + 1);
                        printlog(buffer);
                    }
                    progress = 1;
                }
            }
        }
    }
    return progress;
}

//-------------------------------------------------------------------
// "Isoliert" pairs/triples/quads in einem Quadranten: die candidates, die
// in diesen beiden Zellen moeglich sein, koennen im restlichen
// Quadranten nicht mehr vorkommen
// Return-Wert:
//   1 ... mind. 1 Nummer in der restlichen Spalte oder dem restlichen
//         Quadranten wurde verboten, wir "sind weitergekommen"
//   0 ... Isolieren der Zwillinge hat keine Aenderung im Sudoku bewirkt
// TODO im Moment gehen nur Zwillinge, trotz des Funktionsnamens!

int isolateBoxTuples(int q, int y1, int x1, int y2, int x2) {
    int n;
    int progress;
    int qx, qy;
    int col, row;
    int c;

    assert(q >= 0 && q < 9);
    assert(x1 >= 0 && x1 < 9);
    assert(x2 >= 0 && x2 < 9);
    assert(y1 >= 0 && y1 < 9);
    assert(y2 >= 0 && y2 < 9);

    progress = 0; // noch hat sich nichts veraendert
    if (verboseLogging == 2) {
        sprintf(buffer, "Isoliere Tupel (%d/%d) und (%d/%d): %s/%s\n", y1 + 1, x1 + 1, y2 + 1, x1 + 1, possibilities[y1][x1], possibilities[y2][x2]);
        printlog(buffer);
    }

    // die 2 Zahlen herausfinden
    for (c = 0; c < 9; c++) {
        n = (int) possibilities[y1][x1][c] - 48;
        if (n) {
            // diese Zahl n ueberall sonst verbieten im restlichen Quadranten

            // im restlichen Quadranten verbieten
            if (verboseLogging == 2) {
                sprintf(buffer, "Gehe Quadrant %d durch und verbiete %d ...\n", q + 1, n);
                printlog(buffer);
            }
            getQuadrantStart(q, &qx, &qy);
            for (row = qy; row < qy + 3; row++) {
                for (col = qx; col < qx + 3; col++) {
                    if (!(row == y1 && col == x1) && !(row == y2 && col == x2) && !fields[row][col] && forbidNumber(row, col, n)) {
                        if (verboseLogging == 2) {
                            sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 7c:  (Nummer %d im gleichen Quadranten %d wie Zwilling (%d/%d) und (%d/%d) verboten)\n", n, q + 1, y1 + 1, x1 + 1, y2 + 1, x2 + 1);
                            printlog(buffer);
                        }
                        progress = 1;
                    }
                }
            }
        }
    }
    return progress;
}

//-------------------------------------------------------------------
// Verbiete eine Zahl in einer bestimmten Zelle
// Return-Wert:
//   1 ... Nummer wurde verboten
//   0 ... keine Aenderung, Nummer war bereits verboten

int forbidNumber(int y, int x, int n) {

    assert(x >= 0 && x < 9);
    assert(y >= 0 && y < 9);
    assert(n >= 1 && n <= 9);

    if (possibilities[y][x][n - 1] != '0') {
        if (verboseLogging == 2) {
            sprintf(buffer, "Vorher: (%d/%d) possibilities=%s\n", y + 1, x + 1, possibilities[y][x]);
            printlog(buffer);
        }
        possibilities[y][x][n - 1] = '0';
        if (verboseLogging == 2) {
            sprintf(buffer, "Nachher: (%d/%d) possibilities=%s)\n", y + 1, x + 1, possibilities[y][x]);
            printlog(buffer);
        }
        nrOfPossibilities[y][x]--;
        if (nrOfPossibilities[y][x] == 1) {
            // nur noch eine einzige Zahl ist moeglich => ausfuellen!
            setUniqueNumber(x, y);
        }
        return 1;
    }
    return 0;
}

//-------------------------------------------------------------------
// check for solved cells and remove candidates from neighbor cells
// @return 1 ... something has changed, 0 ... nothing changed

int checkForSolvedCells() {
    int x, y, i;
    int n;
    int q;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    progress = 0;

    for (y = 0; y < 9; y++) {
        for (x = 0; x < 9; x++) {
            n = fields[y][x];
            if (n) {
                // hier steht bereits eine Zahl n drin => diese Zahl darf
                // sonst nicht mehr vorkommen ...
                // ... in der selben Zeile:
                for (i = 0; i < 9; i++) {
                    // alle "zweifelhaften" Zellen durchgehen
                    if (!fields[y][i]) {
                        if (forbidNumber(y, i, n)) {
                            if (verboseLogging == 2) {
                                sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 1a: (Nummer %d verboten wegen %d in (%d/%d))\n", n, n, y + 1, x + 1);
                                printlog(buffer);
                            }
                            progress = 1; // Flag "neue Erkenntnis" setzen
                        }
                    }
                }
                // ... in der selben Spalte:
                for (i = 0; i < 9; i++) {
                    // alle "zweifelhaften" Zellen durchgehen
                    if (!fields[i][x]) {
                        if (forbidNumber(i, x, n)) {
                            if (verboseLogging == 2) {
                                sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 1b: (Nummer %d verboten wegen %d in (%d/%d))\n", n, n, y + 1, x + 1);
                                printlog(buffer);
                            }
                            progress = 1; // Flag "neue Erkenntnis" setzen
                        }
                    }
                }
                // ... im selben Quadranten:
                q = getQuadrantNr(x, y);
                // x0 ... x-Koordinate des linken oberen Feldes des Quadranten
                // y0 ... y-Koordinate des linken oberen Feldes des Quadranten
                // x1 ... x innerhalb des Quadranten
                // y1 ... y innerhalb des Quadranten
                int x0 = (q % 3) * 3;
                int y0 = (q / 3) * 3;
                int x1, y1;
                for (y1 = 0; y1 < 3; y1++) {
                    for (x1 = 0; x1 < 3; x1++) {
                        // alle "zweifelhaften" Zellen durchgehen
                        if (!fields[y0 + y1][x0 + x1]) {
                            if (forbidNumber(y0 + y1, x0 + x1, n)) {
                                if (verboseLogging == 2) {
                                    sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 1c: (Nummer %d verboten wegen %d in (%d/%d))\n", n, n, y + 1, x + 1);
                                    printlog(buffer);
                                }
                                progress = 1; // Flag "neue Erkenntnis" setzen
                            }
                        }
                    }
                }
            }
        }
    }

    return progress;
}

int findHiddenSingles() {
    int x, y;
    int n;
    int q;
    int progress; // flag: something has changed

    progress = 0;

    // suche in allen Zeilen nach Zahlen, die nur an einer Position
    // moeglich sind (auch wenn in dieser Zelle mehrere Zahlen moeglich
    // waeren, aber die anderen Moeglichkeiten kann man dann verwerfen)
    if (verboseLogging == 2) {
        sprintf(buffer, "??? Searching for: unique places in rows ... \n");
        printlog(buffer);
    }
    for (y = 0; y < 9; y++) {
        for (n = 1; n <= 9; n++) {
            x = getUniquePositionInRow(n, y);
            if (x != -1 && !fields[y][x]) {
                // Zahl n kann nur an der Position x vorkommen in der Zeile y
                if (verboseLogging) {
                    sprintf(buffer, "!!! Neue Erkenntnis 2a: In Zeile %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", y + 1, n, x + 1, y + 1, x + 1, n);
                    printlog(buffer);
                }
                fields[y][x] = n;
                progress = 1; // Flag "neue Erkenntnis" setzen
            }
        }
    }

    // suche in allen Spalten nach Zahlen, die nur an einer Position
    // moeglich sind (auch wenn in dieser Zelle mehrere Zahlen moeglich
    // waeren, aber die anderen Moeglichkeiten kann man dann verwerfen)
    if (verboseLogging == 2) {
        sprintf(buffer, "??? Searching for: unique places in cols ... \n");
        printlog(buffer);
    }
    for (x = 0; x < 9; x++) {
        for (n = 1; n <= 9; n++) {
            y = getUniquePositionInColumn(n, x);
            if (y != -1 && !fields[y][x]) {
                // Zahl n kann nur an der Position y vorkommen in der Spalte x
                if (verboseLogging) {
                    sprintf(buffer, "!!! Neue Erkenntnis 2b: In Spalte %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", x + 1, n, y + 1, y + 1, x + 1, n);
                    printlog(buffer);
                }
                fields[y][x] = n;
                progress = 1; // Flag "neue Erkenntnis" setzen
            }
        }
    }

    // suche in allen Quadranten nach Zahlen, die nur an einer Position
    // moeglich sind (auch wenn in diesem Quadrant mehrere Zahlen moeglich
    // waeren, aber die anderen Moeglichkeiten kann man dann verwerfen)
    {
        int position;

        if (verboseLogging == 2) {
            sprintf(buffer, "??? Searching for: unique places in quadrants ... \n");
            printlog(buffer);
        }
        for (q = 0; q < 9; q++) {
            for (n = 1; n <= 9; n++) {
                position = getUniquePositionInBox(n, q);
                if (position != -1) {
                    getQuadrantField(q, position, &x, &y);
                    if (!fields[y][x]) {
                        // Zahl n kann nur an der Position y vorkommen in der Spalte x
                        if (verboseLogging) {
                            sprintf(buffer, "!!! Neue Erkenntnis 2c: In Quadrant %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", q + 1, n, position + 1, y + 1, x + 1, n);
                            printlog(buffer);
                        }
                        fields[y][x] = n;
                        progress = 1; // Flag "neue Erkenntnis" setzen
                    }
                }
            }
        }
    }

    return progress;
}

int findNakedPairs() {
    int x, y, i, j;
    int q;
    int x1, x2, y1, y2, qx, qy;
    int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen

    progress = 0;

    // Suche nach Zwillingen in einem Quadranten (nicht unbedingt in der gleichen Zeile oder Spalte):
    // ----------------------------------------------------------------------------------------------
    // wenn zwei Felder in der gleichen Zeile die gleichen 2 moeglichen 
    // Zahlen haben, muessen jeweils eine dieser beiden Zahlen in 
    // jeweils einer dieser 2 Zellen stehen => damit koennen beide 
    // Zahlen im restlichen Quadranten nicht mehr vorkommen.
    // Wenn die beiden auch noch in der selben Zeile sind, kann auch 
    // in der restlichen Zeile keine dieser Zahlen mehr vorkommen.
    // Analog fuer Spalten.
    if (verboseLogging == 2) printlog("??? Searching for: twins ... \n");

    for (q = 0; q < 9; q++) {
        if (verboseLogging == 2) {
            sprintf(buffer, "Untersuche Quadrant %d auf Zwillinge ...\n", q + 1);
            printlog(buffer);
        }
        getQuadrantStart(q, &qx, &qy);
        // Vergleiche jedes Feld im Quadranten mit jedem anderen im selben Quadranten
        for (i = 0; i < 9; i++) {
            getQuadrantCell(i, &x1, &y1);
            x1 += qx;
            y1 += qy;
            for (j = i + 1; j < 9; j++) {
                getQuadrantCell(j, &x2, &y2);
                x2 += qx;
                y2 += qy;

                if ((nrOfPossibilities[y1][x1] == 2 && nrOfPossibilities[y2][x2] == 2)
                        && !strcmp(possibilities[y1][x1], possibilities[y2][x2])) {
                    // ja, wird haben Quadranten-Zwillinge => im restlichen Quadranten 
                    // koennen diese 2 Zahlen nicht mehr vorkommen!
                    if (verboseLogging == 2) {
                        sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3a: Zwillinge! Feld (%d/%d) und Feld (%d/%d) sind im gleichen Quadranten und haben beide: %s\n", y1 + 1, x1 + 1, y2 + 1, x2 + 1, possibilities[y1][x1]);
                        printlog(buffer);
                    }
                    if (isolateBoxTuples(q, y1, x1, y2, x2))
                        progress = 1;
                }
            }
        }
    }

    if (verboseLogging) {
        printSvg(0);
    }


    // Suche nach Zwillingen in einer Zeile oder einer Spalte (nicht unbedingt in einem Quadranten):
    // ---------------------------------------------------------------------------------------------
    // wenn zwei Felder in der gleichen Zeile die gleichen 2 moeglichen 
    // Zahlen haben, muessen jeweils eine dieser beiden Zahlen in 
    // jeweils einer dieser 2 Zellen stehen => damit koennen beide 
    // Zahlen in der restlichen Zeile nicht mehr vorkommen

    // alle Zeilen durchgehen
    for (y = 0; y < 9; y++) {
        if (verboseLogging == 2) {
            sprintf(buffer, "Untersuche Reihe %d auf Zwillinge ...\n", y + 1);
            printlog(buffer);
        }
        // suche Zwillinge in dieser Reihe
        for (x1 = 0; x1 < 9; x1++) {
            for (x2 = x1 + 1; x2 < 9; x2++) {
                // vergleiche die beiden Zellen: sind es Zwillinge?
                if ((nrOfPossibilities[y][x1] == 2 && nrOfPossibilities[y][x2] == 2)
                        && !strcmp(possibilities[y][x1], possibilities[y][x2])) {
                    // ja, x1, x2 sind Zwillinge => in der restlichen Zeile
                    // koennen diese 2 Zahlen nicht mehr vorkommen!
                    if (verboseLogging == 2) {
                        sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3b: Zwillinge! Feld (%d/%d) und Feld (%d/%d) haben beide: %s\n", y + 1, x1 + 1, y + 1, x2 + 1, possibilities[y][x1]);
                        printlog(buffer);
                    }
                    if (isolateRowTwins(y, x1, x2))
                        progress = 1;
                }
            }
        }
    }

    if (verboseLogging) {
        printSvg(0);
    }

    // alle Spalten durchgehen
    for (x = 0; x < 9; x++) {
        if (verboseLogging == 2) {
            sprintf(buffer, "Untersuche Spalte %d auf Zwillinge ...\n", x + 1);
            printlog(buffer);
        }
        // suche Zwillinge in dieser Spalte
        for (y1 = 0; y1 < 9; y1++) {
            for (y2 = y1 + 1; y2 < 9; y2++) {
                // vergleiche die beiden Zellen: sind es Zwillinge?
                if ((nrOfPossibilities[y1][x] == 2 && nrOfPossibilities[y2][x] == 2)
                        && !strcmp(possibilities[y1][x], possibilities[y2][x])) {
                    // ja, y1, y2 sind Zwillinge => in der restlichen Spalte
                    // koennen diese 2 Zahlen nicht mehr vorkommen!
                    if (verboseLogging == 2) {
                        sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3c: Zwillinge! Feld (%d/%d) und Feld (%d/%d) haben beide: %s\n", y1 + 1, x + 1, y2 + 1, x + 1, possibilities[y1][x]);
                        printlog(buffer);
                    }
                    if (isolateColumnTwins(x, y1, y2))
                        progress = 1;
                }
            }
        }
    }

    return progress;
}

int findHiddenPairs() {
    int y;
    int cand;
    int progress;

    // http://programmers.stackexchange.com/questions/270930/sudoku-hidden-sets-algorithm

    progress = 0;

    // hidden pairs in rows
    for (y = 0; y < 9; y++) {
        for (cand = 1; cand <= 9; cand++) {
            // countCandidateInRow(cand, y);
        }

    }

    return progress;
}