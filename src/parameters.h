/* 
 * File:   parameters.h
 * Author: aurez
 *
 * Created on 01. Mai 2016, 22:04
 */

#ifndef PARAMETERS_H
#define	PARAMETERS_H

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

#ifdef	__cplusplus
}
#endif

#endif	/* PARAMETERS_H */

