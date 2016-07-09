#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>


void *xmalloc(size_t size);


int main(int argc, char **argv) {
    int result;
    int c;
    char *outputFilename = NULL; // filename of printlog file
    char *inputFilename = NULL;
    char *sudoku = NULL;

    // FIXME debugging test code
    char *alex;
    alex = (char *) xmalloc(sizeof(char) * 5);
    strcpy(alex, "Alex");
    printf("alex1: [%s]\n", alex);


//    printf("und ...\n");
//    getContainerTypes(1);
//    printf("Vorbei\n");

    initFields();
    initContainers();


    freeContainers();
    //freeGrid();

    exit(EXIT_SUCCESS);
}


/**
 * init empty fields, just the memory for the field and candidates, but
 * with no data, except the field name. This will be done in initGrid().
 */
void initFields() {
    unsigned *candidates;

    for (int f = 0; f < 81; f++) {
        // allocate candidates
        candidates = (unsigned *) xmalloc(sizeof (unsigned) * 9);
        //        field->candidates = candidates;
        //        field->candidates = NULL;

        //        // use the ROWS and COLS coordinates as the "name" of the field
        //        // reserve space for coordinates up to "Z26" (a theoretical limit of
        //        // a 26-number-Sudoku)
//        char *name = (char *) xmalloc(sizeof (char) * 4);
//        sprintf(field->name, "%c%u", (char) (field->y + (int) 'A'), field->x + 1);
//        field->name = name;
    }
//    printlog("Finished initializing fields");
}


void initContainers() {
    getContainerTypes();
}

/**
 * free units memory
 */
void freeContainers() {
    //Container *containerPtr;

    //    for (int i = 0; i < unitDefs.count; i++) {
    //        free(unitDefs.containerTypes[i].name);
    //
    //        containerPtr = &(unitDefs.containerTypes[i].containers);
    //        while (*containerPtr) {
    //            free(containerPtr->name);
    //            free(containerPtr->fields);
    //        }
    //        free(unitDefs.containerTypes[i].containers);
    //    }
    //    free(unitDefs.containerTypes);

    //free(allContainers);

}

/**
 * free fields memory
 */
void freeFields() {

    // for (int f = 0; f < 81; f++) {
    //     free(fields[f].candidates);
    // }

//    free(fields);
}

void getContainerTypes(void) {

    // FIXME debugging test code
    char *alex;
    
    alex = (char *) xmalloc(sizeof (char) * 5);
    strcpy(alex, "Alex");
    printf("alex2: [%s]\n", alex);
}

void *xmalloc(size_t size) {
    void *value = malloc(size);
    if (value == NULL) {
        printf("virtual memory exhausted\n");
        exit(EXIT_FAILURE);
    }

    return value;
}
