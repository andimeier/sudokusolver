/* 
 * File:   logfile.h
 * Author: ju
 *
 * Created on 05. April 2016, 00:33
 */

#ifndef LOGFILE_H
#define	LOGFILE_H

#include "typedefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define LOGLEVEL_ERRORS   0
#define LOGLEVEL_SOLVED_CELLS   1
#define LOGLEVEL_CHANGES    2
#define LOGLEVEL_VERBOSE    9

    extern char buffer[1000];

    // one of the LOGLEVEL_* constants
    extern unsigned logLevel;
    void logVerbose(char *text);
    void logError(char *text);
    void logAlways(char *text);
    void logReduction(char *msg);
    void logNewNumber(char *msg);

    void showCandidates(Field *field);
    void showAllCandidates();

    void openLogFile(char *outputFilename);
    void closeLogFile();

    extern char buffer[1000];

#ifdef	__cplusplus
}
#endif

#endif	/* SHOW_H */

