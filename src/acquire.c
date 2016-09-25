/**
 * Acquire Sudoku from any source.
 * E.g. from file.
 * Or from command line string.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "typedefs.h"
#include "logfile.h"
#include "util.h"
#include "acquire.h"
#include "gametype.h"
#include "parameters.h"
#include "util.h"

typedef enum {
    VALUES_DATA, SHAPES_DATA
} DataLineType;

typedef struct {
    Bool dimensioned;
    DataLineType dataLineType;
    unsigned sudokuLinesRead;
    unsigned shapeLinesRead;
    unsigned fileLineNo; // number of the line read (starting with 1). For
    // debugging messages
} ReadStatus;

static void initParameters();
static void processLine(ReadStatus *readStatus, Parameters *parameters, char *line);
static void processControlLine(ReadStatus *readStatus, Parameters *parameters, char *line);
static void processDataLine(ReadStatus *readStatus, Parameters *parameters, char *line);
static void readLineWithValues(ReadStatus *readStatus, Parameters *parameters, char *line);
static void readLineWithShapes(ReadStatus *readStatus, Parameters *parameters, char *line);
static void allocateValues(Parameters *parameters);
static void allocateShapeDefinitions(Parameters *parameters);
static void set(Parameters *parameters, char *name, char *value);
static void validateInput(ReadStatus *readStatus);
static void convertValueChars(Parameters *parameters);
static unsigned getValueFromChar(char *valueChars, char valueChar);

// char list functions
static void sortShapeIds(Parameters *parameters);
static int compareChars(const void * a, const void * b);
static Bool charListContains(const char *charList, char c);
static size_t numberOfChars(const char *charList);
static void appendChar(char *charList, char c);

Parameters parameters;

/**
 * read Sudoku from file.
 * Ignore lines starting with '#'.
 * Space or dot will be interpreted as emtpy fields.
 *
 * @param inputFilename
 * @return the Sudoku parameters or NULL on error
 */
Parameters *readSudoku(char *inputFilename) {
    char line[201];
    FILE *file;
    ReadStatus readStatus;

    assert(inputFilename != NULL);

    sprintf(buffer, "Reading Sudoku from file %s ...", inputFilename);
    logAlways(buffer);

    // open file
    file = fopen(inputFilename, "r");

    if (!file) {
        sprintf(buffer, "Error opening Sudoku file %s", inputFilename);
        logError(buffer);
        return NULL;
    }

    // initialize read mode
    readStatus.dimensioned = FALSE; // we do not know the Sudoku dimension yet
    readStatus.dataLineType = VALUES_DATA; // not in the definition of jigsaw shapes
    readStatus.sudokuLinesRead = 0; // 0 data lines read until now
    readStatus.shapeLinesRead = 0; // 0 lines containing shape IDs read until now
    readStatus.fileLineNo = 0; // no line read from file

    initParameters();

    while (!feof(file)) {

        if (!fgets(line, 200, file)) {
            break;
        }
        readStatus.fileLineNo++;

        // remove trailing CR/LF
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1 ] = '\0';
        }
        if (line[strlen(line) - 1] == '\r') {
            line[strlen(line) - 1 ] = '\0';
        }

        processLine(&readStatus, &parameters, line);
    }
    logVerbose("Sudoku read");

    fclose(file);

    validateInput(&readStatus);

    if (parameters.gameType == JIGSAW_SUDOKU) {
        sortShapeIds(&parameters);
    }

    return &parameters;
}

/**
 * processes one line read from the file
 *
 * @param readStatus
 * @param parameters
 * @param line
 */
void processLine(ReadStatus *readStatus, Parameters *parameters, char *line) {

    if (line[0] == '#') {
        // a comment line => ignore it

    } else if (strchr(line, ':')) {
        processControlLine(readStatus, parameters, line);

    } else {
        processDataLine(readStatus, parameters, line);
    }
}

/**
 * processes a "control line", i.e. a line containing a setting
 *
 * @param readMode
 * @param parameters
 * @param line
 */
void processControlLine(ReadStatus *readStatus, Parameters *parameters, char *line) {
    char *settingName;
    char *settingValue;

    // a control line containing the setting name and the value
    settingName = strtok(line, ":");
    settingValue = strtok(NULL, "\r\n");

    // settingName should be case-insensitive
    toLowerStr(settingName);

    if (settingValue) {

        // skip spaces at the beginning of the value
        while (*settingValue == ' ') {
            settingValue++;
        }

        if (settingValue[0] == '\0') {
            settingValue = NULL;
        }
    }


    // the following settings do not need a settingValue
    if (!strcmp(settingName, "shapes")) {
        // switch on "shapes interpretation mode"
        readStatus->dataLineType = SHAPES_DATA;
        return;
    } else if (!strcmp(settingName, "value")) {
        // switch off "shapes interpretation mode"
        readStatus->dataLineType = VALUES_DATA;
        return;
    }

    // all other settings need a corresponding settingValue
    if (!settingValue) {
        sprintf(buffer, "missing value for setting \"%s\" in line %u", settingName, readStatus->fileLineNo);
        logError(buffer);
        exit(EXIT_FAILURE);
        return;
    }

    set(parameters, settingName, settingValue);
}

/**
 * processes a data line, i.e. a line containing values or shapes
 *
 * @param readMode
 * @param parameters
 * @param line
 */
void processDataLine(ReadStatus *readStatus, Parameters *parameters, char *line) {

    /*
     * the first data line determines intrinsically the geometry of
     * out Sudoku. By reading the first data line, we know how many
     * fields to expect
     */
    if (!readStatus->dimensioned) {
        parameters->maxNumber = strlen(line);
        parameters->numberOfFields = parameters->maxNumber * parameters->maxNumber;
        allocateValues(parameters);
        allocateShapeDefinitions(parameters);

        readStatus->dimensioned = TRUE;
    }

    /*
     * check line length: all data lines must have the same length
     */
    if (strlen(line) != parameters->maxNumber) {
        sprintf(buffer, "Error reading the Sudoku from file: first data row has %u numbers, but line %u has %zu.", parameters->maxNumber, readStatus->fileLineNo, strlen(line));
        logError(buffer);
        exit(EXIT_FAILURE);
    }

    /*
     * depending on the dataLine type, interpret the line as values
     * or shape information
     */

    if (readStatus->dataLineType == VALUES_DATA) {
        readLineWithValues(readStatus, parameters, line);
    } else if (readStatus->dataLineType == SHAPES_DATA) {
        readLineWithShapes(readStatus, parameters, line);
    }
}

/**
 * read a line and parses the values. The read values are used as initial
 * values of the Sudoku
 *
 * @param readStatus
 * @param line
 */
void readLineWithValues(ReadStatus *readStatus, Parameters *parameters, char *line) {
    unsigned x;
    char c;
    unsigned y;
    unsigned maxNumber;

    y = readStatus->sudokuLinesRead;
    maxNumber = parameters->maxNumber;

    if (readStatus->sudokuLinesRead >= maxNumber) {
        sprintf(buffer, "Error reading the Sudoku from file: too many data rows in line %u.", readStatus->fileLineNo);
        logError(buffer);
        exit(EXIT_FAILURE);
    }


    /*
     * go through all chars of the line, should be only digits and
     * spaces
     * // FIXME no: should only be the defined valueChars
     */
    for (x = 0; x < maxNumber; x++) {
        c = line[x];

        // interpret '0' or ' ' or '.' or '_' as initially empty cells
        if ((c == '0') || (c == ' ') || (c == '.') || (c == '_')) {
            c = '0';
        }

        parameters->initialValueChars[y * parameters->maxNumber + x] = c;
    }
    readStatus->sudokuLinesRead++;
}

/**
 * read a line and parses the shapes' IDs. This is only relevant with jigsaw
 * Sudokus.
 *
 * @param readStatus
 * @param line
 */
void readLineWithShapes(ReadStatus *readStatus, Parameters *parameters, char *line) {
    unsigned x;
    char c;
    unsigned y;
    unsigned maxNumber;

    y = readStatus->shapeLinesRead;
    maxNumber = parameters->maxNumber;

    if (readStatus->shapeLinesRead >= maxNumber) {
        sprintf(buffer, "Error reading the Jigsaw Sudoku from file: too many lines with shape definitions in line %u.", readStatus->fileLineNo);
        logError(buffer);
        exit(EXIT_FAILURE);
    }


    /*
     * go through all chars of the line, should be only digits or letters
     */
    for (x = 0; x < maxNumber; x++) {
        c = line[x];

        // allowed chars are 0-9, a-z and A-Z (case sensitive)
        if (isalnum(c)) {
            parameters->shapes[y * parameters->maxNumber + x] = c;

            if (!charListContains(parameters->shapeIds, c)) {
                // add new shape ID to current list of shape IDs
                if (numberOfChars(parameters->shapeIds) < maxNumber) {
                    appendChar(parameters->shapeIds, c);
                } else {
                    sprintf(buffer, "Error reading the Sudoku from file: more than %u different shape IDs. The %u'th one is: %c (in line %d at position %d). The other ones are: %s.", maxNumber, maxNumber + 1, c, readStatus->fileLineNo, x + 1, parameters->shapeIds);
                    logError(buffer);
                    exit(EXIT_FAILURE);
                }
            }

        } else {
            sprintf(buffer, "Error reading the Sudoku from file: illegal character ('%c') in shape definitions in line %d at position %d.", c, readStatus->fileLineNo, x + 1);
            logError(buffer);
            exit(EXIT_FAILURE);
        }
    }
    readStatus->shapeLinesRead++;
}

/**
 * allocate memory for the initial values
 *
 * @param parameters
 */
void allocateValues(Parameters *parameters) {
    int i;
    char *initialValueChars;
    unsigned *initialValues;

    // initialize Sudoku data lines
    initialValueChars = (char *) xmalloc(sizeof (char) * parameters->numberOfFields);
    initialValues = (unsigned *) xmalloc(sizeof (unsigned) * parameters->numberOfFields);

    for (i = parameters->numberOfFields - 1; i >= 0; i--) {
        initialValueChars[i] = '0'; // default for each field: no value given
        initialValues[i] = 0; // default for each field: no value given
    }

    parameters->initialValueChars = initialValueChars;
    parameters->initialValues = initialValues;
}

/**
 * allocate memory for the shape definitions
 *
 * @param parameters
 */
void allocateShapeDefinitions(Parameters *parameters) {
    int i;
    char *shapeDefinitions;
    char *shapeIds;

    // initialize Sudoku data lines
    shapeDefinitions = (char *) xmalloc(sizeof (char) * parameters->numberOfFields);
    shapeIds = (char *) xmalloc(sizeof (char) * (parameters->maxNumber + 1));

    for (i = parameters->numberOfFields - 1; i >= 0; i--) {
        shapeDefinitions[i] = '\0'; // default for each field: no shape ID given
    }

    shapeIds[0] = '\0'; // empty list

    parameters->shapes = shapeDefinitions;
    parameters->shapeIds = shapeIds;
}

/**
 * sets a setting
 *
 * @param parameters
 * @param name the name of the setting
 * @param value the value the setting should be set to or NULL if it is a
 *   parameter without a value (e.g. 'Shapes')
 */
void set(Parameters *parameters, char *name, char *value) {

    // interpret the setting
    if (!strcmp(name, "type")) {
        // specify type of Sudoku
        parameters->gameType = parseGameTypeString(value);

    } else if (!strcmp(name, "box")) {
        // specify box size
        parseBoxDimensionString(value, &(parameters->boxWidth), &(parameters->boxHeight));

    } else if (!strcmp(name, "candidates")) {
        // specify syntax of possible candidates
        parameters->valueChars = strdup(value);
    }
}

/**
 * initialize parameters structure
 */
void initParameters() {
    parameters.gameType = STANDARD_SUDOKU;
    parameters.maxNumber = 0; // uninitialized yet
    parameters.numberOfFields = 0; // uninitialized yet
    parameters.boxWidth = 0; // uninitialized yet
    parameters.boxHeight = 0; // uninitialized yet
    parameters.initialValues = NULL;
    parameters.shapes = NULL;
    parameters.shapeIds = NULL;
    parameters.valueChars = NULL;
}

/**
 * sort shape IDs inline.
 * 
 * @param parameters
 */
void sortShapeIds(Parameters *parameters) {
    char *shapeIds;

    shapeIds = parameters->shapeIds;
    qsort(shapeIds, parameters->maxNumber, sizeof (char), compareChars);
}

/**
 * comparator function for sorting the shape IDs
 * 
 * @param a
 * @param b
 * @return 
 */
int compareChars(const void * a, const void * b) {
    return (*(char *) a > *(char *) b) - (*(char *) a < *(char *) b);
}

/**
 * checks if the list of characters contains the given character
 * 
 * @param charList '\0' terminated list of characters
 * @param c the character to be searched for
 * @return TRUE if the list container the character, FALSE if not
 */
Bool charListContains(const char *charList, char c) {
    return strchr(charList, (int) c) ? TRUE : FALSE;
}

size_t numberOfChars(const char *charList) {
    return strlen(charList);
}

void appendChar(char *charList, char c) {
    char *oneChar;

    oneChar = strdup("X"); // one dummy character
    oneChar[0] = c;
    strcat(charList, oneChar);
}

/**
 * performs some tests to check whether the input is valid
 * 
 * @param readStatus read status structure
 */
void validateInput(ReadStatus *readStatus) {
    char *sorted;
    unsigned ix;

    if (readStatus->sudokuLinesRead != parameters.maxNumber) {
        logError("error reading the Sudoku from file: too few data rows.");
        exit(EXIT_FAILURE);
    }

    if (parameters.gameType == JIGSAW_SUDOKU
            && readStatus->shapeLinesRead != parameters.maxNumber) {
        logError("error reading the Jigsaw Sudoku from file: too few rows with shape definitions.");
        exit(EXIT_FAILURE);
    }

    if (parameters.gameType != JIGSAW_SUDOKU && readStatus->shapeLinesRead) {
        logError("shape definitions found, but game type is not \"Jigsaw\".");
        exit(EXIT_FAILURE);
    }

    if (parameters.gameType == JIGSAW_SUDOKU
            && numberOfChars(parameters.shapeIds) != parameters.maxNumber) {
        logError("error reading the Jigsaw Sudoku from file: too few shape IDs.");
        exit(EXIT_FAILURE);
    }

    if (!parameters.valueChars) {
        if (parameters.maxNumber <= 9) {
            parameters.valueChars = strdup("123456789");
        } else {
            // don't know to represent an internal value of e.g. 10 ...
            logError("Sudoku has more than 9 different candidates, but the option \"candidates\" is missing.");
            exit(EXIT_FAILURE);
        }
    } else {
        // value characters given: must be exactly as many characters as 
        // are possible candidates
        if (strlen(parameters.valueChars) != parameters.maxNumber) {
            sprintf(buffer, "number of candidate characters (%zu) is different from number of possible candidates (%u)", strlen(parameters.valueChars), parameters.maxNumber);
            logError(buffer);
            exit(EXIT_FAILURE);
        }

        sorted = strdup(parameters.valueChars);
        qsort(sorted, strlen(parameters.valueChars), sizeof (char), compareChars);
        for (ix = strlen(parameters.valueChars) - 1; ix > 0; ix--) {
            if (sorted[ix] == sorted[ix - 1]) {
                sprintf(buffer, "error in candidate characters: duplicate entry of character '%c'", sorted[ix]);
                logError(buffer);
                exit(EXIT_FAILURE);
            }
        }
        free(sorted);
    }

    /*
     * check used value characters.
     * Go through all initial values and check whether the given character is
     * one of the defined "value characters"
     */
    convertValueChars(&parameters);

}

/**
 * converts all "value characters" in the initialValues to their internal value
 * 
 * @param parameters the parameters structure
 */
void convertValueChars(Parameters *parameters) {
    unsigned ix;
    unsigned value;
    char c;

    for (ix = 0; ix < parameters->numberOfFields; ix++) {
        c = parameters->initialValueChars[ix];
        if (c == '0') {
            value = 0;
        } else {
            value = getValueFromChar(parameters->valueChars, c);

            if (!value) {
                // don't know to represent an internal value of e.g. 10 ...
                sprintf(buffer, "illegal character on position %u: %c is not in list of possible candidates (which is: %s).", ix, c, parameters->valueChars);
                logError(buffer);
                exit(EXIT_FAILURE);
            }
        }
        // store numerical representation (= internal value)
        parameters->initialValues[ix] = value;
    }
}

/**
 * maps value character (value as a human reader sees it) into the "internal
 * value". For standard Sudokus, the value character and the internal value is
 * the same, but for Sudokus with > 9 numbers, there are letters (instead of or
 * in addition to numbers), but the internal value is nonetheless an unsigned
 * integer starting with 1.
 * 
 * @param valueChars list of characters representing the candidates
 * @param the value character to be mapped to its internal value
 * @return the internal value represented by the given value character or 0
 *   if the character was not found in the value characters string
 */
unsigned getValueFromChar(char *valueChars, char valueChar) {
    char *position;

    position = strchr(valueChars, valueChar);
    if (position != NULL) {
        return 1 + (unsigned int) (position - valueChars);
    } else {
        return 0;
    }
}
