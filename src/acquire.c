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

typedef enum {
    VALUES, SHAPES
} DataLineType;

typedef struct {
    Bool dimensioned;
    DataLineType dataLineType;
    unsigned sudokuLinesRead;
    unsigned shapeLinesRead;
    unsigned fileLineNo; // number of the line read (starting with 1). For
    // debugging messages
} ReadStatus;

static void processLine(ReadStatus *readStatus, Parameters *parameters, char *line);
static void processControlLine(ReadStatus *readStatus, Parameters *parameters, char *line);
static void processDataLine(ReadStatus *readStatus, Parameters *parameters, char *line);
static void readLineWithValues(ReadStatus *readStatus, Parameters *parameters, char *line);
static void readLineWithShapes(ReadStatus *readStatus, Parameters *parameters, char *line);
static void allocateValues(Parameters *parameters);
static void allocateShapeDefinitions(Parameters *parameters);
static void set(Parameters *parameters, char *name, char *value);

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
    readStatus.dataLineType = VALUES; // not in the definition of jigsaw shapes
    readStatus.sudokuLinesRead = 0; // 0 data lines read until now
    readStatus.shapeLinesRead = 0; // 0 lines containing shape IDs read until now
    readStatus.fileLineNo = 0; // no line read from file

    parameters.maxNumber = 0; // undimensioned

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

    if (readStatus.sudokuLinesRead != parameters.maxNumber) {
        logError("Error reading the Sudoku from file: too few data rows.");
        exit(EXIT_FAILURE);
    }

    if (parameters.gameType == JIGSAW_SUDOKU 
            && readStatus.shapeLinesRead != parameters.maxNumber) {
        logError("Error reading the Jigsaw Sudoku from file: too few rows with shape definitions.");
        exit(EXIT_FAILURE);
    }

    return &parameters;
 
    if (parameters.gameType != JIGSAW_SUDOKU && readStatus.shapeLinesRead) {
        logError("shape definitions found, but game type is not \"Jigsaw\".");
        exit(EXIT_FAILURE);
    }

    return &parameters;
}

#ifdef READ_SUDOKU_STRING

/**
 * import a sudoku as a sequence of characters representing the initial numbers.
 * Empty fields can be defined by using 0, _ or .
 *
 * @param sudoku the Sudoku string
 * @result success flag: 1 if the Sudoku could be read successfully, 0 if not
 */
Bool parseSudokuString(char *sudoku) {
    int f;
    char c;

    for (f = 0; f < numberOfFields; f++) {
        c = sudoku[f];
        if (c == '\0') {
            sprintf(buffer, "Error parsing the Sudoku input: unexpected end of Sudoku data after character #%d", f);
            logError(buffer);
            return FALSE;
        }

        if ((c >= '0') && (c <= (char) (maxNumber + (int) '0'))) {
            fields[f].initialValue = (int) (c - '0');
        } else if ((c == ' ') || (c == '.') || (c == '_')) {
            fields[f].initialValue = 0;
        } else {
            sprintf(buffer, "Error parsing the Sudoku input: illegal character ('%c') at position %d.", c, f);
            logError(buffer);
            return FALSE;
        }
    }

    // copy original grid
    for (f = 0; f < numberOfFields; f++) {
        fields[f].value = fields[f].initialValue;
    }

    return TRUE;
}

/**
 * parse Sudoku string 409176000610389040...
 *
 * @param sudoku the Sudoku string: all numbers concatenated in one string
 *   or - if maxNumber > 9 - characters beginning with 'A', concatenated in
 *   one string // FIXME characters not implenented
 * @param maxNumber maximum number in the Sudoku = width of Sudoku = height
 *   of Sudoku (will be 9 for a standard Sudoku)
 * @return
 */
Bool parseSudokuString(char *sudoku, int maxNumber) {
    // FIXME not used yet, should be the common function which readSudoku and importSudoku uses
    return FALSE;
}
#endif

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
        readStatus->dataLineType = SHAPES;
        return;
    } else if (!strcmp(settingName, "value")) {
        // switch off "shapes interpretation mode"
        readStatus->dataLineType = VALUES;
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

    if (readStatus->dataLineType == VALUES) {
        readLineWithValues(readStatus, parameters, line);
    } else if (readStatus->dataLineType == SHAPES) {
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
     */
    for (x = 0; x < maxNumber; x++) {
        c = line[x];

        if ((c >= '0') && (c <= (char) (maxNumber + (int) '0'))) {
            parameters->initialValues[y * parameters->maxNumber + x] = (int) (c - '0');

        } else if ((c == ' ') || (c == '.') || (c == '_')) {
            parameters->initialValues[y * parameters->maxNumber + x] = 0;

        } else {
            sprintf(buffer, "Error reading the Sudoku from file: illegal character ('%c') in line %d at position %d.", c, readStatus->fileLineNo, x + 1);
            logError(buffer);
            exit(EXIT_FAILURE);
        }
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
     * go through all chars of the line, should be only digits
     */
    for (x = 0; x < maxNumber; x++) {
        c = line[x];

        if ((c >= '0') && (c <= (char) (maxNumber + (int) '0'))) {
            parameters->initialValues[y * parameters->maxNumber + x] = (int) (c - '0');

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
    unsigned *initialValues;

    // initialize Sudoku data lines
    initialValues = (unsigned *) xmalloc(sizeof (unsigned) * parameters->numberOfFields);

    for (i = parameters->numberOfFields - 1; i >= 0; i--) {
        initialValues[i] = 0; // default for each field: no value given
    }

    parameters->initialValues = initialValues;
}

/**
 * allocate memory for the shape definitions
 *
 * @param parameters
 */
void allocateShapeDefinitions(Parameters *parameters) {
    int i;
    unsigned *shapeDefinitions;

    // initialize Sudoku data lines
    shapeDefinitions = (unsigned *) xmalloc(sizeof (unsigned) * parameters->numberOfFields);

    for (i = parameters->numberOfFields - 1; i >= 0; i--) {
        shapeDefinitions[i] = 0; // default for each field: no shape ID given
    }

    parameters->shapes = shapeDefinitions;
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
        parameters->gameType = parseGametypeString(value);

    } else if (!strcmp(name, "box")) {
        // specify box size
        parseBoxDimensionString(value, &(parameters->boxWidth), &(parameters->boxHeight));
    }
}
