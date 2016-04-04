/* 
 * File:   solve.c
 * Author: aurez
 *
 * Created on 04. April 2016, 21:01
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "global.h"
#include "util.h"


int fields[9][9];
int initfields[9][9];
char possibilities[10][10][10]; // pro Feld die moeglichen Zahlen, als C-String, zB "123000080"
int nrOfPossibilities[9][9]; // Anzahl der verbleibenden Moeglichkeiten pro Zelle

int errors; // number of errors in the algorithm
int verboseLogging; // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations


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
// Versucht, Felder zu finden, in denen nur eine einzige Zahl mehr
// moeglich ist. Diese wird dort ausgefuellt.
// Return-Wert:
//   1 ... es wurde etwas veraendert
//   0 ... keine Unique Numbers gefunden

int setUniqueNumbers() {
  int x, y, n;
  int unique; // eindeutiges Feld?
  int possibility;
  int found;

  found = 0; // pessimistische Grundannahme: keine unique number gefunden
  for (y = 0; y < 9; y++) {
    for (x = 0; x < 9; x++) {

      // wenn Feld nicht schon ausgefuellt ist
      if (!fields[y][x]) {
        unique = 0;
        for (n = 1; n <= 9; n++) {
          if (possibilities[y][x][n - 1] != '0') {
            // eine moegliche Zahl gefunden, haben wir eigentlich schon eine andere moegliche?
            if (unique == 1) {
              // ups, schon die zweite Moeglichkeit fuer dieses Feld
              unique = 2;
              break; // ok, dieses Feld ist nicht eindeutig => ab zum naechsten
            } else {
              unique = 1;
              possibility = n; // die (moeglicherweise eindeutige) Zahl n merken
            }
          }
        }
        // war dieses Feld eindeutig?
        if (unique == 1) {
          if (verboseLogging) {
            sprintf(buffer, "!!! Neue Erkenntnis 6a: Juhu, Feld (%d/%d) gefunden (ist eindeutig): %d\n", y + 1, x + 1, possibility);
            printlog(buffer);
          }
          fields[y][x] = possibility;
          found = 1;
        }
      }
    }
  }
  return found;
}

//-------------------------------------------------------------------
// Setzt in dem Feld x/y (das nur mehr eine Moeglichkeit aufweisen muss)
// die Zahl.
// Es muss sichergestellt sein, dass nur mehr eine Zahl moeglich ist,
// hier wird das nicht mehr ueberprueft!
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
//   0 ... Zahl koennte in der Zeile an mehreren Positionen vorkommen

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
        return 0; // war wohl nix
      }
    }
  }
  if (unique)
    return xPosition;
  
  return 0;
}

//-------------------------------------------------------------------
// Checkt die Anzahl der moeglichen Vorkommnisse einer Zahl in der
// Spalte x.
// Liefert:
//   y ... y-Position des Feldes, in dem die Zahl n als einziges Feld
//         der ganzen Spalte vorkommen koennte oder
//   0 ... Zahl koennte in der Spalte an mehreren Positionen vorkommen

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
        return 0; // war wohl nix
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
  return 0;
}

//-------------------------------------------------------------------
// Checkt die Anzahl der moeglichen Vorkommnisse einer Zahl im 
// Quadrant q.
// Liefert:
//   position ... Position des Feldes, in dem die Zahl n als einziges Feld
//         des ganzen Quadranten vorkommen koennte oder [0..8]
//   0 ... Zahl koennte im Quadranten an mehreren Positionen vorkommen

int getUniquePositionInQuadrant(int n, int q) {
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
        return 0; // war wohl nix
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
  return 0;
}

//-------------------------------------------------------------------
// "Isoliert" Zwillinge in einer Spalte: die beiden Zahlenpaare, die
// in diesen beiden Zellen moeglich sind, koennen im Rest der Spalte 
// nicht mehr vorkommen
// Return-Wert:
//   1 ... mind. 1 Nummer in der restlichen Spalte oder dem restlichen
//         Quadranten wurde verboten, wir "sind weitergekommen"
//   0 ... Isolieren der Zwillinge hat keine Aenderung im Sudoku bewirkt

int IsolateColumnTwins(int x, int y1, int y2) {
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

int IsolateRowTwins(int y, int x1, int x2) {
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
// "Isoliert" Zwillinge in einem Quadranten: die beiden Zahlenpaare, die
// in diesen beiden Zellen moeglich sein, koennen im restlichen
// Quadranten nicht mehr vorkommen
// Return-Wert:
//   1 ... mind. 1 Nummer in der restlichen Spalte oder dem restlichen
//         Quadranten wurde verboten, wir "sind weitergekommen"
//   0 ... Isolieren der Zwillinge hat keine Aenderung im Sudoku bewirkt

int IsolateQuadrantTwins(int q, int y1, int x1, int y2, int x2) {
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
    sprintf(buffer, "Isoliere Zwillinge (%d/%d) und (%d/%d): %s/%s\n", y1 + 1, x1 + 1, y2 + 1, x1 + 1, possibilities[y1][x1], possibilities[y2][x2]);
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

