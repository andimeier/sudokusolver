/* 
 * File:   acquire.h
 * Author: aurez
 *
 * Created on 01. Mai 2016, 22:04
 */

#ifndef ACQUIRE_H
#define	ACQUIRE_H

#ifdef	__cplusplus
extern "C" {
#endif

    int readSudoku(char *inputFilename);
    int importSudoku(char *sudoku);

#ifdef	__cplusplus
}
#endif

#endif	/* ACQUIRE_H */

