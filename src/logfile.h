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

    typedef enum {
        LOGLEVEL_ERRORS,
        LOGLEVEL_SOLVED_CELLS,
        LOGLEVEL_CHANGES,
        LOGLEVEL_VERBOSE
    } LogLevel;

    extern char buffer[1000];

    void setLogLevel(LogLevel logLevel);
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

