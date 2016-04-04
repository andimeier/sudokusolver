/* 
 * File:   global.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:31
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#ifdef	__cplusplus
extern "C" {
#endif

  extern char buffer[1000]; // buffer for string operations

  extern int fields[9][9];
  extern int initfields[9][9];
  extern char possibilities[10][10][10]; // pro Feld die moeglichen Zahlen, als C-String, zB "123000080"
  extern int nrOfPossibilities[9][9]; // Anzahl der verbleibenden Moeglichkeiten pro Zelle

  extern int errors; // number of errors in the algorithm
  extern int verboseLogging; // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations


#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_H */

