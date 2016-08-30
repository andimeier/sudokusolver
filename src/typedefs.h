/* 
 * File:   typedefs.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:31
 */

#ifndef TYPEDEFS_H
#define	TYPEDEFS_H

#include <stddef.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef int Bool;

    // ID values of the container types

    typedef enum {
        ROWS = 1, COLS, BOXES, DIAGONALS
    } ContainerType;


    // forward declarations
    typedef struct Container Container;
    typedef struct Field Field;

    typedef Field * FieldsVector;

    typedef struct Field {
        unsigned x; // X position on the game board, starting with 0 (doubled
        // by the position in the container set "COLS")
        unsigned y; // Y position on the game board, starting with 0 (doubled
        // by the position in the container set "ROWS")

        /* indexes of the several containers. In any 
         * container, an index of -1 means that this number is not part of any 
         * instance of this container type (this can be the case in the unit 
         * "diagonal"))
         * This member is an array representing the container sets. Each item
         * in turn is a -1-terminated list of container indexes. For example, 
         * if the item #4 is a list of [ 2, -1 ], it means that this field
         * is member of the container #2 in the container set #4. If the same
         * item would be a list of [ -1 ], it indicates that this field is not
         * a part of any container of this type (which could be the case with
         * diagonal containers - not all fields are member of a diagonal 
         * container). If the same item would be a list of [ 0, 1 -1 ], then 
         * this field is member in two containers of this container type (again,
         * this could be the case with diagonal containers, where the center 
         * field would be member of both diagonals)
         */
        int **containerIndexes;

        /* containers containing this field. The order 
         * of containers corresponds to the order of the "container sets".
         * This is a vector of lists, where every list can be NULL, indicating 
         * that
         * this field is not part of any container of this container type.
         * Possible in e.g. diagonal containers.
         */
        Container ***containers;

        unsigned *candidates;
        unsigned initialValue;
        unsigned correctSolution; // if known in advance, can be filled out
        // to test the algorithms
        unsigned candidatesLeft;
        unsigned value;
        char name[4]; // "name" of field = coordinates, e.g. "A2"
    } Field;

    typedef struct Container {
        char *name; // the name of the container, e.g. "row 3" or "slash diagonal"
        ContainerType type; // the container type (e.g. ROWS)
        Field **fields; // the fields in this container, in "order of 
        // appearance". This is a vector of maxNumber fields
    } Container;

    /**
     * function for filling a container with its fields. The fields must
     * be prepared (allocated) beforehand, this function does not allocate,
     * but only *use* memory provided by its parametes
     */
    typedef void (*fillContainerFieldsFunc)(unsigned containerIndex, FieldsVector *fields);

    /**
     * creates and returns the name of a child container instance in a 
     * container set. Depending on the given index, the result can be a name
     * like 'row 1' or 'box 9'.
     * 
     * @param index container index of the child (starting with 0). For example,
     *   for row containers, the index specifies the number of the row.
     */
    typedef char * (*getContainerNameFunc)(unsigned index);

    /**
     * a container set is a set of (a class of) areas containing all Sudoku
     * numbers uniquely. Examples of a container set are: row, column, box,
     * diagonal, etc.
     * 
     * An *child* of such a container set would be a specific row, a 
     * specific column, etc. This would be called a 'container', then.
     */
    typedef struct ContainerSet {
        // the name of the container type used for log output in 
        // singular form, e.g. 'row'
        char *name;

        // the container type (e.g. 1==row)
        ContainerType type;

        /* the function to determine the fields of the specified container */
        fillContainerFieldsFunc fillContainerFields;

        /* the function to retrieve the container instance name (e.g. 'row 2') */
        getContainerNameFunc getContainerName;

        // number of containers of this type in 
        // the Sudoku. Will normally be 9, but can also be something else, 
        // e.g. 2 for "diagonal"
        size_t numberOfContainers;

        // the containers of this container type,
        // in "order of appearance", terminated by NULL
        Container **containers;

    } ContainerSet;

#ifdef	__cplusplus
}
#endif

#endif	/* TYPEDEFS_H */

