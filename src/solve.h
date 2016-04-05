/* 
 * File:   solve.h
 * Author: aurez
 *
 * Created on 04. April 2016, 21:22
 */

#ifndef SOLVE_H
#define	SOLVE_H

#ifdef	__cplusplus
extern "C" {
#endif

  int isFinished();
  int forbidNumber(int y, int x, int n);
  
  // strategies
  int checkForSolvedCells();
  int findHiddenSingles();
  int findNakedPairsAndTriples(); // not implemented yet
  int findHiddenPairsAndTriples(); // not implemented yet
  int findNakedQuads(); // not implemented yet
  int findHiddenQuads(); // not implemented yet
  int findPointingPairsAndTriples(); // not implemented yet
  int boxLineReduction(); // not implemented yet

  // auxiliary functions
  int setUniqueNumbers();
  int setUniqueNumber(int x, int y);

  int getUniquePositionInRow(int n, int y);
  int getUniquePositionInColumn(int n, int x);
  int getUniquePositionInBox(int n, int q);

  int IsolateColumnTwins(int x, int y3, int y2);
  int IsolateRowTwins(int y, int x1, int x2);
  int IsolateQuadrantTwins(int q, int y1, int x1, int y2, int x2);

#ifdef	__cplusplus
}
#endif


#endif	/* SOLVE_H */

