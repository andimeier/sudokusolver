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

  extern char buffer[1000]; // general buffer for string operations

  extern int fields[9][9]; // current grid state [y][x]
  extern int initfields[9][9]; // initial grid state, does not change during solving
  extern char possibilities[10][10][10]; // for each cell: the candidates as C string, e.g. "123000080"
  extern int nrOfPossibilities[9][9]; // for each cell: number of candidates left

  extern int errors; // number of errors in the algorithm
  extern int verboseLogging; // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations

  extern char *svgFilename; // filename of SVG file


#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_H */

