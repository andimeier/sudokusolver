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

    void initUnits();
    void initGrid();
    void freeUnits();
    void freeGrid();
    int isFinished();
    int forbidNumber(int y, int x, int n);

    // strategies
    int checkForSolvedCells();
    int findHiddenSingles();
    int findNakedPairs(); // not implemented yet
    int findNakedTriples(); // not implemented yet
    int findHiddenPairsAndTriples(); // not implemented yet
    int findNakedQuads(); // not implemented yet
    int findHiddenQuads(); // not implemented yet
    int findPointingPairsAndTriples(); // not implemented yet
    int boxLineReduction(); // not implemented yet


#ifdef	__cplusplus
}
#endif


#endif	/* SOLVE_H */

