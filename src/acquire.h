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

    /*
     * entire set of the Sudoku input parameters. All parameters which are
     * read from the Sudoku file or from other sources (e.g. as switches from
     * the command line) are included in this structure.
     */
    typedef struct {
        GameType gameType;
        unsigned maxNumber;
        unsigned boxWidth;
        unsigned boxHeight;
        unsigned *initialValues;
        unsigned *shapes;
    } Parameters;
    
    int readSudoku(char *inputFilename);
    int importSudoku(char *sudoku);

    unsigned parseGametypeString(char *gametypeString);
    void parseBoxDimensionString(char *boxDimensionString, unsigned *width, unsigned *height);

#ifdef	__cplusplus
}
#endif

#endif	/* ACQUIRE_H */

