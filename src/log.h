/* 
 * File:   log.h
 * Author: ju
 *
 * Created on 05. April 2016, 00:33
 */

#ifndef LOG_H
#define	LOG_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

    void showCandidates(Field *field);
    void showAllCandidates();

    void logReduction(char *msg);
    void logNewNumber(char *msg);

    void openLogFile(char *outputFilename);
    void printlog(char *text);
    void closeLogFile();


    extern char buffer[1000];

    // 0 ... no verbose logging, 1 ... log changes, 2 ... log even considerations
    extern int verboseLogging;



#ifdef	__cplusplus
}
#endif

#endif	/* SHOW_H */

