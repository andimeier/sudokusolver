/* 
 * File:   args.h
 * Author: aurez
 *
 * Created on 01. Mai 2016, 22:04
 */

#ifndef ARGS_H
#define	ARGS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "gametype.h"
#include "logfile.h"

    typedef struct {
        LogLevel logLevel;
        char *gametypeString;
        char *outputFilename;
        char *inputFilename;
        Bool help; // command line help requested
    } CommandLineArgs;

    CommandLineArgs *parseCommandLineArguments(int argc, char **argv);

    void toLowerStr(char *str);
    void parseBoxDimensionString(char *boxDimensionString, unsigned *width, unsigned *height);
    char *parseValueChars(char *valueCharsString, char *errorMsg[]);

#ifdef	__cplusplus
}
#endif

#endif	/* ARGS_H */
