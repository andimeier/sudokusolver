#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "solve.h"
#include "global.h"
#include "util.h"
#include "show.h"

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

// file handles
int svgIndex;

int main(int argc, char **argv) {
  int result;
  int c;
  char *outputFilename; // filename of printlog file

  // read command line arguments
  opterr = 0;

  while ((c = getopt(argc, argv, "hvVl:s:")) != -1)
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


  if (outputFilename) {
    openLogFile(outputFilename);
  }

  if (!readSudoku()) {
    return 1; // Oje ... stopp!
  }

  if (verboseLogging) {
    printlog("Initial Sudoku:");
    show(0);
  }

  result = solve();

  show(1);
  printSvg(0);

  if (result) {
    printlog("-----------------------------------------------");
    printlog("         FERTIG, SUDOKU WURDE GELOEST!");
    printlog("-----------------------------------------------");
  } else {

    int x, y;
    int numbersFound = 0;
    for (y = 0; y < 9; y++)
      for (x = 0; x < 9; x++)
        if (fields[y][x])
          numbersFound++;

    printlog("-----------------------------------------------");
    printlog("      Sudoku konnte nicht geloest werden!");
    sprintf(buffer, "      %d von 81 Zellen wurden gefunden.", numbersFound);
    printlog(buffer);
    printlog("-----------------------------------------------");
  }

  if (errors) {
    sprintf(buffer, "Es sind %d FEHLER aufgetreten!\n", errors);
    printlog(buffer);
  }


  closeLogFile();

  exit(EXIT_SUCCESS);
}




//-------------------------------------------------------------------

void printUsage() {
  // print program usage

  puts("Usage: ");
  puts(" sudoku-solver -l LOGFILE -s SVGFILE -v -h");
  puts("");
  puts("Parameters:");
  puts("");
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
}


//-------------------------------------------------------------------
// Return-Wert
//   1 ... Sudoku wurde erfolgreich geloest
//   0 ... Algorithmus bleibt stecken, Endlositeration abgebrochen

int solve() {
  int x, y, i, j;
  int n;
  int q;
  int iteration;
  int progress; // Flag: in einer Iteration wurde zumindest eine Erkenntnis gewonnen
  int x1, x2, y1, y2, qx, qy;
  int gridVersion;

  errors = 0; // noch keine Fehler aufgetreten
  iteration = 0;
  gridVersion = 1;

  printSvg(gridVersion++);

  // Initialisierung:
  // zunaechst sind ueberall alle Zahlen moeglich
  for (y = 0; y < 9; y++) {
    for (x = 0; x < 9; x++) {
      strcpy(possibilities[y][x], "123456789");
      nrOfPossibilities[y][x] = 9;
    }
  }

  do {
    iteration++;
    progress = 0; // noch kein neuen Erkenntnis in dieser Runde (hat ja erst begonnen)
    if (verboseLogging == 2) {
      sprintf(buffer, "----- Beginne Iteration %d -----\n", iteration);
      printlog(buffer);
    }

    if (verboseLogging == 2) {
      for (y = 0; y < 9; y++) {
        for (x = 0; x < 9; x++) {
          if (fields[y][x]) {
            sprintf(buffer, "  Feld (%d/%d): %d\n", y + 1, x + 1, fields[y][x]);
            printlog(buffer);
          } else {
            sprintf(buffer, "  Feld (%d/%d): %s\n", y + 1, x + 1, possibilities[y][x]);
            printlog(buffer);
          }
        }
      }
    }

    // alle Felder durchgehen und vorkommende Zahlen in der selben
    // Reihe, in der selben Spalte und im selben Quadranten verbieten
    if (verboseLogging == 2) printlog("??? Searching for: unique numbers ... \n");

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

    if (verboseLogging) {
      printSvg(gridVersion++);
    }

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
        if (!fields[y][x] && x) {
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

    if (verboseLogging) {
      printSvg(gridVersion++);
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
        if (!fields[y][x] && y) {
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

    if (verboseLogging) {
      printSvg(gridVersion++);
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
          position = getUniquePositionInQuadrant(n, q);
          if (position) {
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

    if (verboseLogging) {
      printSvg(gridVersion++);
    }

    //? FIXME FEHLT hier nicht, das nicht nur fuer Spalten und Zeile, sondern auch fuer Quadranten anzuwenden?




    /* nicht mehr noetig
                    if (setUniqueNumbers(fields, possibilities)) {
                            progress = 1; // Flag "neue Erkenntnis" setzen
                            printf("Neue Erkenntnis 6\n");
                    }
     */

    // wenn alle Felder ausgefuellt sind, sind wir wohl fertig!
    if (isFinished())
      return 1;

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
        for (j = i + 1; j < 9; j++) {
          getQuadrantCell(i, &x1, &y1);
          x1 += qx;
          y1 += qy;
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
            if (IsolateQuadrantTwins(q, y1, x1, y2, x2))
              progress = 1;
          }
        }
      }
    }

    if (verboseLogging) {
      printSvg(gridVersion++);
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
            if (IsolateRowTwins(y, x1, x2))
              progress = 1;
          }
        }
      }
    }

    if (verboseLogging) {
      printSvg(gridVersion++);
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
            if (IsolateColumnTwins(x, y1, y2))
              progress = 1;
          }
        }
      }
    }

    if (verboseLogging) {
      printSvg(gridVersion++);
    }

    // Suche nach lokaler Eingrenzung einer Zahl in einem Quadranten:
    // --------------------------------------------------------------
    // wenn in einem Quadranten eine Zahl nur in Zellen in der gleichen
    // Zeile vorkommen kann, muss sie in diesem Quadranten in dieser
    // Zeile stehen und kann daher fuer die restliche Zeile (ausserhalb
    // des Quadranten) verboten werden.
    // Analog fuer Spalten.

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
              sprintf(buffer, "  yFound=%d Feld (%d/%d) %d %s\n", yFound + 1, y + 1, x + 1, fields[y][x], (fields[y][x] ? "" : possibilities[y][x]));
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
      printSvg(gridVersion++);
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
      printSvg(gridVersion++);
    }

    // nach der Iteration den Sudoku-Zwischenstand anzeigen
    if (verboseLogging) show(0);

  } while (progress);

  // wir kommen hierher, weil die letzte Iteration keine einzige Aenderung gebracht
  // hat => wir bleiben stecken mit unserem Algorithmus. Ohne Aenderung in der
  // Implementierung ist dieses Sudoku nicht loesbar
  return 0;
}



//-------------------------------------------------------------------
// Lies das Sudoku aus der Standardeingabe ein.
// Zeilen, die mit '#' beginnen, werden ignoriert.
// Leere Felder werden durch Leerzeichen oder Punkte repraesentiert.
//   1 ... ok, Sudoku wurde eingelesen
//   0 ... Fehler beim Lesen

int readSudoku() {
  char line[201];
  int linecount;
  char c;
  int ok;
  int x, y;

  // Sudoku initialisieren
  for (y = 0; y < 9; y++) {
    for (x = 0; x < 9; x++) {
      fields[y][x] = 0;
    }
  }

  // Sudoku einlesen
  ok = 1; // optimistische Grundannahme

  linecount = 0;
  y = 0;
  while (ok && !feof(stdin)) {
    if (!fgets(line, 200, stdin)) {
      break;
    }
    linecount++;

    if (line[0] != '#') {
      // alle Zeichen der Zeile durchgehen, das sollten nur Ziffern 
      // und Leerzeichen sein
      if (y > 9) {
        printlog("Fehler beim Einlesen des Sudokus: zu viele Datenzeilen.");
        ok = 0; // oje, das war keine Ziffer!
        break;
      }
      for (x = 0; x < 9; x++) {
        c = line[x];
        if ((c >= '0') && (c <= '9')) {
          fields[y][x] = (int) (c - '0');
        } else if ((c == ' ') || (c == '.')) {
          fields[y][x] = 0;
        } else {
          //sprintf(buffer, "Fehler beim Einlesen des Sudokus: illegales Zeichen ('%c') in Zeile %d an Position %d.\n", c, x+1, linecount);
          //printlog(buffer);
          ok = 0; // oje, das war keine Ziffer!
          break;
        }
      }
      y++;
    } else {
      // eine Kommentarzeile
    }
  }

  if (ok && y != 9) {
    printlog("Fehler beim Einlesen des Sudokus: es muessen genau 9 Datenzeilen sein.");
    ok = 0;
  }

  // copy original grid
  for (y = 0; y < 9; y++) {
    for (x = 0; x < 9; x++) {
      initfields[y][x] = fields[y][x];
    }
  }

  return ok;
}
