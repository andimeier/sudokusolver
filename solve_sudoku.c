#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

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
void printlog(char *text);
void show();
void printSvg(int index);
int solve();
int getQuadrantNr(int x, int y);
int setUniqueNumber(int x, int y);
int getUniquePositionInRow(int n, int y);
int getUniquePositionInColumn(int n, int x);
int isFinished();
int getAbsoluteX(int q, int qx);
int getAbsoluteY(int q, int qx);
void getQuadrantCell(int n, int *qx, int *qy);
void getQuadrantStart(int q, int *qx, int *qy);
int IsolateColumnTwins(int x, int y3, int y2);
int IsolateRowTwins(int y, int x1, int x2);
int IsolateQuadrantTwins(int q, int y1, int x1, int y2, int x2);
int forbidNumber(int y, int x, int n);
			
// globale Variablen
// das Sudoku-Feld selbst
int fields[9][9];
char possibilities[10][10][10]; // pro Feld die moeglichen Zahlen, als C-String, zB "123000080"
int nrOfPossibilities[9][9]; // Anzahl der verbleibenden Moeglichkeiten pro Zelle
int errors; // number of errors in the algorithm
int verboseLogging; // flag: if truish, switch on verbose logging
char *outputFilename; // filename of printlog file
char *svgFilename; // filename of SVG file

// file handles
FILE *logfile;
char buffer[1000]; // buffer for string operations
int svgIndex;
	
int main(int argc, char **argv) {
	int result;
	int c;

	// read command line arguments
	opterr = 0;
	
	while ((c = getopt (argc, argv, "hvl:s:")) != -1)
		switch (c) {
			case 'v':
				verboseLogging = 1;
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
					fprintf (stderr, "Option -%d requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				abort ();
		}	

		
	if (outputFilename) {
		logfile = fopen(outputFilename, "w");
	}

	if (!readSudoku()) {
		return 1; // Oje ... stopp!
	}
	
	if (verboseLogging) {
		printlog("Initial Sudoku:");
		show();
	}

	result = solve();

	show();
	printSvg(0);

	if (result) {
		printlog("-----------------------------------------------");
		printlog("         FERTIG, SUDOKU WURDE GELOEST!");
		printlog("-----------------------------------------------");
	}	else {
		
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

	
	if (logfile) fclose(logfile);
	
	exit(EXIT_SUCCESS);
}


//-------------------------------------------------------------------
void printlog(char *text) {
	// printlog a message to printlog file or to stdout

	if (logfile) {
		fputs(text, logfile);
	} else {
		// no printlog file => write to stdout
		puts(text);
	}
}


//-------------------------------------------------------------------
void printUsage() {
	// print program usage
	
	puts("Usage: ");
	puts(" solve_sudoku -l LOGFILE -s SVGFILE -v -h");
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
	puts("  -h          this help screen");
}


//-------------------------------------------------------------------
void show() {
	// display sudoku
	int x, y;
	int index;
	
	for (y = 0; y < 9; y++) {
		if (!(y % 3))
			printlog("+-----+-----+-----+");
		
		index = 0;
		for (x = 0; x < 9; x++) {
			if (x % 3)
				buffer[index++] = ' ';
			else
				buffer[index++] = '|';
			if (fields[y][x])
				buffer[index++] = (char)(fields[y][x] + 48);
			else
				// leeres Feld
				buffer[index++] = ' ';
		}
		buffer[index++] = '|';
		buffer[index++] = '\0';
		printlog(buffer);
	}
	printlog("+-----+-----+-----+");
}

//-------------------------------------------------------------------
// if parameter final is truish, the specified svg filename will be used,
// otherwise an indexed file name will be used to store intermediate
// versions of the grid. This will only happen if verboseLogging is turned
// on.
// @param index integer ... 
// The parameter index specifies the suffix to be used in the filename
// so that the log file can reference to a specific SVG intermediate file
// using this suffix. If index is 0, then no suffix will be appended. This
// indicates that this SVG depicts the final, solved version - not some 
// intermediate version
void printSvg(int index) {
	// display sudoku in SVG format
	int x, y;
	FILE *svgfile;
	char *filename;
	char suffix[20];

	if (!svgFilename) return;

	// build filename
	if (!index) {
		filename = svgFilename;
	} else {
		sprintf(suffix, ".%d", index);
		filename = (char *)malloc(sizeof(char)*(strlen(svgFilename)+strlen(suffix)+1)); 
		if (filename == NULL) {
			perror("Not enough memory to allocate memory for SVG filename.");
			exit(EXIT_FAILURE);  
		}
		strcpy(filename, svgFilename);
		strcat(filename, suffix);
	}
	
	sprintf(buffer, "Writing SVG file [%s]", filename);
	printlog(buffer);

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

	for (y = 0; y < 9; y++) {
		for (x = 0; x < 9; x++) {
			if (fields[y][x]) {
				float xPos = x * 9 + 4.5;
				float yPos = y * 9 + 7.65;
				fprintf(svgfile, "<text class=\"final\" x=\"%f\"  y=\"%f\" text-anchor=\"middle\">%d</text>\n", xPos, yPos, fields[y][x]);
			} else {
			    // alle noch moeglichen Zahlen ausgeben
			    int n1;
                for (n1 = 1; n1 <= 9; n1++) {
           		    if (possibilities[y][x][n1-1] == (char)(n1 + 48)) {
           		        float xPos = x * 9 + ((n1 - 1) % 3) * 3 + 1;
           		        float yPos = y * 9 + ((int)((n1 - 1) / 3) * 3 + 2.4);
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
	int x1, x2, y1, y2, qx1, qx2, qy1, qy2, qx, qy;
	int gridVersion;
	
	errors = 0; // noch keine Fehler aufgetreten
	iteration = 0;
	gridVersion = 0;

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
		if (verboseLogging) {
			sprintf(buffer, "----- Beginne Iteration %d -----\n", iteration);
			printlog(buffer);
		}
		
		if (verboseLogging) {
			for (y = 0; y < 9; y++) {
				for (x = 0; x < 9; x++) {
					if (fields[y][x]) {
						sprintf(buffer, "  Feld (%d/%d): %d\n", y+1, x+1, fields[y][x]);
						printlog(buffer);
					} else {
						sprintf(buffer, "  Feld (%d/%d): %s\n", y+1, x+1, possibilities[y][x]);
						printlog(buffer);
					}
				}
			}
		}

		// alle Felder durchgehen und vorkommende Zahlen in der selben
		// Reihe, in der selben Spalte und im selben Quadranten verbieten
		if (verboseLogging) printlog("??? Searching for: unique numbers ... \n");

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
								if (verboseLogging) {
									sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 1a: (Nummer %d verboten wegen %d in (%d/%d))\n", n, n, y+1, x+1);
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
								if (verboseLogging) {
									sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 1b: (Nummer %d verboten wegen %d in (%d/%d))\n", n, n, y+1, x+1);
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
									if (verboseLogging) {
										sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 1c: (Nummer %d verboten wegen %d in (%d/%d))\n", n, n, y+1, x+1);
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

		printSvg(gridVersion++);

		// suche in allen Zeilen nach Zahlen, die nur an einer Position
		// moeglich sind (auch wenn in dieser Zelle mehrere Zahlen moeglich
		// waeren, aber die anderen Moeglichkeiten kann man dann verwerfen)
		if (verboseLogging) {
			sprintf(buffer, "??? Searching for: unique places in rows ... \n");
			printlog(buffer);
		}
		for (y = 0; y < 9; y++) {
			for (n = 1; n <= 9; n++) {
				x = getUniquePositionInRow(n, y);
				if (!fields[y][x] && x) {
					// Zahl n kann nur an der Position x vorkommen in der Zeile y
					if (verboseLogging) {
						sprintf(buffer, "!!! Neue Erkenntnis 2a: In Zeile %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", y+1, n, x+1, y+1, x+1, n);
						printlog(buffer);
					}
					fields[y][x] = n;
					progress = 1; // Flag "neue Erkenntnis" setzen
				}
			}
		}

		printSvg(gridVersion++);

		// suche in allen Spalten nach Zahlen, die nur an einer Position
		// moeglich sind (auch wenn in dieser Zelle mehrere Zahlen moeglich
		// waeren, aber die anderen Moeglichkeiten kann man dann verwerfen)
		if (verboseLogging) {
			sprintf(buffer, "??? Searching for: unique places in cols ... \n");
			printlog(buffer);
		}
		for (x = 0; x < 9; x++) {
			for (n = 1; n <= 9; n++) {
				y = getUniquePositionInColumn(n, x);
				if (!fields[y][x] && y) {
					// Zahl n kann nur an der Position y vorkommen in der Spalte x
					if (verboseLogging) {
						sprintf(buffer, "!!! Neue Erkenntnis 2b: In Spalte %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", x+1, n, y+1, y+1, x+1, n);
						printlog(buffer);
					}
					fields[y][x] = n;
					progress = 1; // Flag "neue Erkenntnis" setzen
				}
			}
		}

		printSvg(gridVersion++);

		// suche in allen Quadranten nach Zahlen, die nur an einer Position
		// moeglich sind (auch wenn in diesem Quadrant mehrere Zahlen moeglich
		// waeren, aber die anderen Moeglichkeiten kann man dann verwerfen)
		{
			int position;
			
			if (verboseLogging) {
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
								sprintf(buffer, "!!! Neue Erkenntnis 2c: In Quadrant %d kann %d nur an Position %d vorkommen => (%d/%d) = %d!\n", q+1, n, 		position+1, y+1, x+1, n);
								printlog(buffer);
							}
							fields[y][x] = n;
							progress = 1; // Flag "neue Erkenntnis" setzen
						}
					}
				}
			}
		}

		printSvg(gridVersion++);

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
		if (verboseLogging) printlog("??? Searching for: twins ... \n");
		
		for (q = 0; q < 9; q++) {
			if (verboseLogging) {
				sprintf(buffer, "Untersuche Quadrant %d auf Zwillinge ...\n", q+1);
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
						if (verboseLogging) {
							sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3a: Zwillinge! Feld (%d/%d) und Feld (%d/%d) sind im gleichen Quadranten und haben beide: %s\n", y1+1, x1+1, y2+1, x2+1, possibilities[y1][x1]);
							printlog(buffer);
						}
   						if (IsolateQuadrantTwins(q, y1, x1, y2, x2))
							progress = 1;
					}
				}
			}
		}

		printSvg(gridVersion++);
		

		// Suche nach Zwillingen in einer Zeile oder einer Spalte (nicht unbedingt in einem Quadranten):
		// ---------------------------------------------------------------------------------------------
		// wenn zwei Felder in der gleichen Zeile die gleichen 2 moeglichen 
		// Zahlen haben, muessen jeweils eine dieser beiden Zahlen in 
		// jeweils einer dieser 2 Zellen stehen => damit koennen beide 
		// Zahlen in der restlichen Zeile nicht mehr vorkommen
		
		// alle Zeilen durchgehen
		for (y = 0; y < 9; y++) {
			if (verboseLogging) {
				sprintf(buffer, "Untersuche Reihe %d auf Zwillinge ...\n", y+1);
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
						if (verboseLogging) {
							sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3b: Zwillinge! Feld (%d/%d) und Feld (%d/%d) haben beide: %s\n", y+1, x1+1, y+1, x2+1, possibilities[y][x1]);
							printlog(buffer);
						}
						if (IsolateRowTwins(y, x1, x2))
							progress = 1;
					}
				}
			}
		}

		printSvg(gridVersion++);

		// alle Spalten durchgehen
		for (x = 0; x < 9; x++) {
			if (verboseLogging) {
				sprintf(buffer, "Untersuche Spalte %d auf Zwillinge ...\n", x+1);
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
						if (verboseLogging) {
							sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 3c: Zwillinge! Feld (%d/%d) und Feld (%d/%d) haben beide: %s\n", y1+1, x+1, y2+1, x+1, possibilities[y1][x]);
							printlog(buffer);
						}
						if (IsolateColumnTwins(x, y1, y2))
							progress = 1;
					}
				}
			}
		}

		printSvg(gridVersion++);

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
			if (verboseLogging) {
				sprintf(buffer, "??? Untersuche Quadrant %d auf Zahlen, die auf eine Zeile eingrenzbar sind ...\n", q+1);
				printlog(buffer);
			}
			getQuadrantStart(q, &qx, &qy);
			// alle Zahlen durchgehen
			for (n = 1; n <= 9; n++) {
				if (verboseLogging) {
					sprintf(buffer, " Untersuche Quadrant %d auf die Zahl %d ...\n", q+1, n);
					printlog(buffer);
				}
				yFound = -1; // noch haben wir fuer diese Zahl keine Zeile gefunden
				for (y = qy; y < qy + 3; y++) {
					for (x = qx; x < qx + 3; x++) {
						if (verboseLogging) {
							sprintf(buffer, "  yFound=%d Feld (%d/%d) %d %s\n", yFound+1, y+1, x+1, fields[y][x], (fields[y][x] ? "" : possibilities[y][x]));
							printlog(buffer);
						}
						// kommt die Zahl in diesem Feld als Moeglichkeit vor?
						if (fields[y][x] == n) {
							// diese Zahl ist bereits fixiert im Quadranten =>
							// nach dieser brauche ich nicht weitersuchen
							if (verboseLogging) {
								sprintf(buffer, "    Zahl %d ist bereits in (%d/%d) identifiziert!\n", n, y+1, x+1);
								printlog(buffer);
							}
							yFound = -1; // nix Tolles gefunden
							y = 99; // auch die aeussere Schleife beenden
							break; // raus aus der Schleife
						}
						if (!fields[y][x] && possibilities[y][x][n-1] != '0') {
							// die Zahl n koennte hier vorkommen
							if (yFound == -1) {
								// wir merken uns diese Zeile, wenn alle anderen
								// Vorkommen auch in dieser Zeile sind, haben wir
								// eine wertvolle Information gewonnen!
								yFound = y;
								if (verboseLogging) {
									sprintf(buffer, "    Zahl %d koennte in Zeile %d vorkommen (%d/%d), merke mir die Zeile ...\n", n, y+1, y+1, x+1);
									printlog(buffer);
								}
							} else if (yFound != y) {
								// oje, das zweite Vorkommen ist in einer
								// anderen Zeile als der gemerkten => Ziel
								// nicht erreicht, das bringt uns nix
								if (verboseLogging) {
									sprintf(buffer, "    Oje, Zahl %d koennte auch in Zeile %d vorkommen (%d/%d), ein Reinfaller.\n", n, y+1, y+1, x+1);
									printlog(buffer);
								}
								yFound = -1; // nix Tolles gefunden
								y = 99;
								break; // diese Zahl war ein Reinfaller
							} else {
								if (verboseLogging) {
									sprintf(buffer, "    Zahl %d koennte auch hier vorkommen, ebenfalls in Zeile %d ...\n", n, y+1);
									printlog(buffer);
								}
							}
						}
					}
				}
				if (yFound != -1) {
					if (verboseLogging) {
						sprintf(buffer, "  Hurra! Zahl %d kann im Quadranten %d nur in Zeile %d vorkommen.\n", n, q+1, yFound+1);
						printlog(buffer);
					}
					for (x = 0; x < 9; x++) {
						// wenn ausserhalb unseren Quadranten: alle Vorkommen der
						// Zahl n verbieten, die muss naemlich im Quadranten q
						// in dieser Zeile vorkommen
						if ((x < qx) || (x >= qx + 3)) {
							if (!fields[yFound][x])
								if (forbidNumber(yFound, x, n)) {
									if (verboseLogging) {
										sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 4a: (Nummer %d in (%d/%d) verboten weil in Zeile %d diese Zahl im Quadranten %d sein muss.\n", n, yFound+1, x+1, yFound+1, q+1);
										printlog(buffer);
									}
									progress = 1;
								}
						}
					}
				}
			}
		}
		
		printSvg(gridVersion++);

		// ... analog in Spalten eines Quadranten suchen
		int xFound;
		for (q = 0; q < 9; q++) {
			if (verboseLogging) {
				sprintf(buffer, "Untersuche Quadrant %d auf Zahlen, die auf eine Spalte eingrenzbar sind ...\n", q+1);
				printlog(buffer);
			}
			getQuadrantStart(q, &qx, &qy);
			// alle Zahlen durchgehen
			for (n = 1; n <= 9; n++) {
				if (verboseLogging) {
					sprintf(buffer, " Untersuche Quadrant %d auf die Zahl %d ...\n", q+1, n);
					printlog(buffer);
				}
				xFound = -1; // noch haben wir fuer diese Zahl keine Spalte gefunden
				for (y = qy; y < qy + 3; y++) {
					for (x = qx; x < qx + 3; x++) {
						if (verboseLogging) {
							sprintf(buffer, "  xFound=%d Feld (%d/%d) %d %s\n", xFound+1, y+1, x+1, fields[y][x], (fields[y][x] ? "" : possibilities[y][x]));
							printlog(buffer);
						}
						// kommt die Zahl in diesem Feld als Moeglichkeit vor?
						if (fields[y][x] == n) {
							// diese Zahl ist bereits fixiert im Quadranten =>
							// nach dieser brauche ich nicht weitersuchen
							if (verboseLogging) {
								sprintf(buffer, "    Zahl %d ist bereits in (%d/%d) identifiziert!\n", n, y+1, x+1);
								printlog(buffer);
							}
							xFound = -1; // nix Tolles gefunden
							y = 99; // auch die aeussere Schleife beenden
							break; // raus aus der Schleife
						}
						if (!fields[y][x] && possibilities[y][x][n-1] != '0') {
							// die Zahl n koennte hier vorkommen
							if (xFound == -1) {
								// wir merken uns diese Zeile, wenn alle anderen
								// Vorkommen auch in dieser Zeile sind, haben wir
								// eine wertvolle Information gewonnen!
								xFound = x;
								if (verboseLogging) {
									sprintf(buffer, "    Zahl %d koennte in Spalte %d vorkommen (%d/%d), merke mir die Spalte ...\n", n, x+1, y+1, x+1);
									printlog(buffer);
								}
							} else if (xFound != x) {
								// oje, das zweite Vorkommen ist in einer
								// anderen Spalte als der gemerkten => Ziel
								// nicht erreicht, das bringt uns nix
								if (verboseLogging) {
									sprintf(buffer, "    Oje, Zahl %d koennte auch in Spalte %d vorkommen (%d/%d), ein Reinfaller.\n", n, x+1, y+1, x+1);
									printlog(buffer);
								}
								xFound = -1; // nix Tolles gefunden
								y = 99;
								break; // diese Zahl war ein Reinfaller
							} else {
								if (verboseLogging) {
									sprintf(buffer, "    Zahl %d koennte auch hier vorkommen, ebenfalls in Spalte %d ...\n", n, x+1);
									printlog(buffer);
								}
							}
						}
					}
				}
				if (xFound != -1) {
					if (verboseLogging) {
						sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 5a: Hurra! Zahl %d kann im Quadranten %d nur in Spalte %d vorkommen.\n", n, q+1, xFound+1);
						printlog(buffer);
					}
					for (y = 0; y < 9; y++) {
						// wenn ausserhalb unseren Quadranten: alle Vorkommen der
						// Zahl n verbieten, die muss naemlich im Quadranten q
						// in dieser Spalte vorkommen
						if ((y < qy) || (y >= qy + 3)) {
							if (!fields[y][xFound])
								if (forbidNumber(y, xFound, n)) {
									if (verboseLogging) {
										sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 5b:  (Nummer %d in (%d/%d) verboten weil in Spalte %d diese Zahl im Quadranten %d sein muss.\n", n, y+1, xFound+1, xFound+1, q+1);
										printlog(buffer);
									}
									progress = 1;
								}
						}
					}
				}
			}
		}

		printSvg(gridVersion++);

		// nach der Iteration den Sudoku-Zwischenstand anzeigen
		if (verboseLogging) show(fields);

	} while(progress);
	
	// wir kommen hierher, weil die letzte Iteration keine einzige Aenderung gebracht
	// hat => wir bleiben stecken mit unserem Algorithmus. Ohne Aenderung in der
	// Implementierung ist dieses Sudoku nicht loesbar
	return 0;
}


//-------------------------------------------------------------------
// Liefert die Nummer des Quadranten, in dem das Feld mit den 
// Koordinaten x/y steht
// Quadranten sind von 0 bis 8 durchnummeriert, dh so angeordnet:
// Q0 Q1 Q3
// Q3 Q4 Q5
// Q6 Q7 Q8
// Jeder Quadrant ist eine 3x3-Matrix
int getQuadrantNr(int x, int y) {

	assert(x >= 0 && x < 9);
	assert(y >= 0 && y < 9);

	return (y / 3) * 3 + (x / 3);		
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
					if (possibilities[y][x][n-1] != '0') {
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
						sprintf(buffer, "!!! Neue Erkenntnis 6a: Juhu, Feld (%d/%d) gefunden (ist eindeutig): %d\n", y+1, x+1, possibility);
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

	assert (x >= 0 && x < 9);
	assert (y >= 0 && y < 9);

	if (fields[y][x]) {
		if (verboseLogging) {
			sprintf(buffer, "FEHLER! HUCH! Obwohl schon ausgefuellt, wird das aufgerufen! (%d/%d) soll gesetzt werden, ist aber bereits %d!\n", y+1, x+1, fields[y][x]);
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
		if (possibilities[y][x][n-1] != '0') {
			if (verboseLogging) {
				sprintf(buffer, "Aha, nur mehr eine Moeglichkeit in Feld (%d/%d) (possibilities: %s): %d\n", y+1, x+1, possibilities[y][x], n);
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
	int xPosition ;
	
	assert (y >= 0 && y < 9);
	assert (n >= 1 && n <= 9);

	unique = 0;
	xPosition = 0;
	for (x = 0; x < 9; x++) {
		if ((fields[y][x] == n) || (!fields[y][x] && (possibilities[y][x][n-1] == (char)(n + 48)))) {
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
	int yPosition ;
	
	assert (x >= 0 && x < 9);
	assert (n >= 1 && n <= 9);

	if (verboseLogging) {
		sprintf(buffer, "Suche nach Moeglichkeiten fuer %d in Spalte %d\n", n, x+1);
		printlog(buffer);
	}
	unique = 0;
	yPosition = 0;
	for (y = 0; y < 9; y++) {
		if ((fields[y][x] == n) || (!fields[y][x] && (possibilities[y][x][n-1] == (char)(n + 48)))) {
			if (verboseLogging) {
				sprintf(buffer, "  %d kann in Zeile %d (%d/%d) vorkommen [%s].\n", n, y+1, y+1, x+1, possibilities[y][x]);
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
	
	assert (q >= 0 && q < 9);
	assert (n >= 1 && n <= 9);

	if (verboseLogging) {
		sprintf(buffer, "Suche nach Moeglichkeiten fuer %d in Quadrant %d\n", n, q+1);
		printlog(buffer);
	}
	unique = 0;
	position = 0;
	for (i = 0; i < 9; i++) {
		getQuadrantField(q, i, &x, &y);
		if ((fields[y][x] == n) || (!fields[y][x] && (possibilities[y][x][n-1] == (char)(n + 48)))) {
			if (verboseLogging) {
				sprintf(buffer, "  %d kann in Quadrant %d (%d/%d) vorkommen [%s].\n", n, q+1, y+1, x+1, possibilities[y][x]);
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
// Rechnet aus Quadrantenkoordinaten in Koordinaten, relativ zum
// Quadrant-Ursprung, um.
// Parameter:
//   n ... Nummer der Zelle innerhalb des Quadranten, in dieser Reihenfolge:
//         012
//         345
//         678
//   qx ... X-Koordinate (0..2), relativ zum Quadranten, wird zurueckgeliefert
//   qy ... Y-Koordinate (0..2), relativ zum Quadranten, wird zurueckgeliefert
// Return-Wert:
//   absolute X-Koordinate (0..8) der Zelle
void getQuadrantCell(int n, int *qx, int *qy) {
	*qx = n % 3;
	*qy = n / 3;
}

//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in absolute Koordinaten um.
// Parameter:
//   q ... Nummer (0..8) des Quadranten
//   qx ... X-Koordinate (0..2), relativ zum Quadranten
// Return-Wert:
//   absolute X-Koordinate (0..8) der Zelle
int getAbsoluteX(int q, int qx) {
	return (q % 3) * 3 + qx;
}


//-------------------------------------------------------------------
// Liefert zu dem x-ten Feld eines Quadranten dessen absolute x- und 
// y-Koordinaten im Sudoku
// Parameter:
//   q ... Nummer des Quadranten (0..8)
//   position ... Position innerhalb des Quadranten (0..8, wobei 0..2
//     in der ersten Zeile des Quadranten sind)
//   x ... absolute X-Koordinate (0..8), wird zurueckgeliefert
//   y ... absolute Y-Koordinate (0..8), wird zurueckgeliefert
int getQuadrantField(int q, int position, int *x, int *y) {
	int qx, qy;

	assert(q >= 0 && q < 9);
	assert(position >= 0 && position < 9);

	getQuadrantStart(q, &qx, &qy);

	*x = qx + (position % 3);
	*y = qy + (position / 3);
}


//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in absolute Koordinaten um.
// Parameter:
//   q ... Nummer (0..8) des Quadranten
//   qy ... Y-Koordinate (0..2), relativ zum Quadranten
// Return-Wert:
//   absolute Y-Koordinate (0..8) der Zelle
int getAbsoluteY(int q, int qy) {
	return (q / 3) * 3 + qy;
}

//-------------------------------------------------------------------
// Rechnet aus Quadrantenkoordinaten in absolute Koordinaten um.
// Parameter:
//   q ... Nummer (0..8) des Quadranten
//   qx ... X-Koordinate des linken oberen Ecks des Quadranten
//   qx ... Y-Koordinate des linken oberen Ecks des Quadranten
void getQuadrantStart(int q, int *qx, int *qy) {
	*qx = (q % 3) * 3;
	*qy = (q / 3) * 3;
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
	int qx, qy;
	int col, row;
	int c;
	
	assert (x >= 0 && x < 9);
	assert (y1 >= 0 && y1 < 9);
	assert (y2 >= 0 && y2 < 9);

	progress = 0; // noch hat sich nichts veraendert
	if (verboseLogging) {
		sprintf(buffer, "Isoliere Zwillinge (%d/%d) und (%d/%d): %s/%s\n", y1+1, x+1, y2+1, x+1, possibilities[y1][x], possibilities[y2][x]);
		printlog(buffer);
	}
	
	// die 2 Zahlen herausfinden
	for (c = 0; c < 9; c++) {
		n = (int)possibilities[y1][x][c] - 48;
		if (n) {
			// diese Zahl n ueberall sonst verbieten im Quadranten und in der Spalte
			
			// in der restlichen Spalte verbieten
			if (verboseLogging) {
				sprintf(buffer, "Gehe Spalte %d durch und verbiete %d ...\n", x, n);
				printlog(buffer);
			}
			for (row = 0; row < 9; row++) {
				if ((row != y1) && (row != y2) && !fields[row][x] && forbidNumber(row, x, n)) {
					if (verboseLogging) {
						sprintf(buffer, " (Nummer %d in der gleichen Spalte %d wie Zwilling (%d/%d) und (%d/%d) verboten)\n", n, x+1, y1+1, x+1, y2+1, x+1);
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
	int qx, qy;
	int col, row;
	int c;
	
	assert (y >= 0 && y < 9);
	assert (x1 >= 0 && x1 < 9);
	assert (x2 >= 0 && x2 < 9);
	
	progress = 0; // noch hat sich nichts veraendert
	if (verboseLogging) {
		sprintf(buffer, "Isoliere Zwillinge (%d/%d) und (%d/%d): %s/%s\n", y+1, x1+1, y+1, x1+1, possibilities[y][x1], possibilities[y][x2]);
		printlog(buffer);
	}
	
	// die 2 Zahlen herausfinden
	for (c = 0; c < 9; c++) {
		n = (int)possibilities[y][x1][c] - 48;
		if (n) {
			// diese Zahl n ueberall sonst verbieten im Quadranten und in der Zeile
			
			// in der restlichen Zeile verbieten
			if (verboseLogging) {
				sprintf(buffer, "Gehe Zeile %d durch und verbiete %d ...\n", y, n);
				printlog(buffer);
			}
			for (col = 0; col < 9; col++) {
				if ((col != x1) && (col != x2) && !fields[y][col] && forbidNumber(y, col, n)) {
					if (verboseLogging) {
						sprintf(buffer, " (Nummer %d in der gleichen Zeile %d wie Zwilling (%d/%d) und (%d/%d) verboten)\n", n, y+1, y+1, x1+1, y+1, x2+1);
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
	
	assert (q >= 0 && q < 9);
	assert (x1 >= 0 && x1 < 9);
	assert (x2 >= 0 && x2 < 9);
	assert (y1 >= 0 && y1 < 9);
	assert (y2 >= 0 && y2 < 9);

	progress = 0; // noch hat sich nichts veraendert
	if (verboseLogging) {
		sprintf(buffer ,"Isoliere Zwillinge (%d/%d) und (%d/%d): %s/%s\n", y1+1, x1+1, y2+1, x1+1, possibilities[y1][x1], possibilities[y2][x2]);
		printlog(buffer);
	}
	
	// die 2 Zahlen herausfinden
	for (c = 0; c < 9; c++) {
		n = (int)possibilities[y1][x1][c] - 48;
		if (n) {
			// diese Zahl n ueberall sonst verbieten im restlichen Quadranten
			
			// im restlichen Quadranten verbieten
			if (verboseLogging) {
				sprintf(buffer, "Gehe Quadrant %d durch und verbiete %d ...\n", q+1, n);
				printlog(buffer);
			}
			getQuadrantStart(q, &qx, &qy);
			for (row = qy; row < qy + 3; row++) {
				for (col = qx; col < qx + 3; col++) {
					if (!(row == y1 && col == x1) && !(row == y2 && col == x2) && !fields[row][col] && forbidNumber(row, col, n)) {
						if (verboseLogging) {
							sprintf(buffer, "!! Neue Moeglichkeiten-Erkenntnis 7c:  (Nummer %d im gleichen Quadranten %d wie Zwilling (%d/%d) und (%d/%d) verboten)\n", n, q+1, y1+1, x1+1, y2+1, x2+1);
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

	assert (x >= 0 && x < 9);
	assert (y >= 0 && y < 9);
	assert (n >= 1 && n <= 9);

	if (possibilities[y][x][n-1] != '0') {
		if (verboseLogging) {
			sprintf(buffer, "Vorher: (%d/%d) possibilities=%s\n", y+1, x+1, possibilities[y][x]);
			printlog(buffer);
		}
		possibilities[y][x][n-1] = '0';
		if (verboseLogging) {
			sprintf(buffer, "Nachher: (%d/%d) possibilities=%s)\n", y+1, x+1, possibilities[y][x]);
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
// Lies das Sudoku aus der Standardeingabe ein.
// Zeilen, die mit '#' beginnen, werden ignoriert.
// Leere Felder werden durch Leerzeichen oder Punkte repraesentiert.
//   1 ... ok, Sudoku wurde eingelesen
//   0 ... Fehler beim Lesen
int readSudoku() {
	int i, j;
	FILE *fp;
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
					fields[y][x] = (int)(c - '0');
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
		
	return ok;
}
