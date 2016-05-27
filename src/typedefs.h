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

    // forward declaration
    typedef struct Container Container;
    
    typedef struct Field {
        unsigned x; // X position on the game board, starting with 0 (doubled
          // by the position in the container set "COLS")
        unsigned y; // Y position on the game board, starting with 0 (doubled
          // by the position in the container set "ROWS")
        int *containerIndexes; // indexes of the several containers. In any 
        // container, an index of -1 means that this number is not part of any 
        // instance of this container type (this can be the case in the unit 
        // "diagonal"))
        Container **containers; // containers containing this field. The order 
          // of containers corresponds to the order of the "container sets".
          // Note that any member of this vector can be NULL, indicating that
          // this field is not part of any container of this container type.
          // Possible in e.g. diagonal containers.
        unsigned *candidates;
        unsigned initialValue;
        unsigned candidatesLeft;
        unsigned value;
        char *name; // "name" of field = coordinates, e.g. "A2"
    } Field;

    typedef struct Container {
        char *name; // the name of the container, e.g. "row 3" or "slash diagonal"
        unsigned type; // the container type (e.g. 0==row), corresponds with the
        // "unit index" in unitDefs
        Field **fields; // the fields in this container, in "order of 
        // appearance". This is a vector of MAX_NUMBER fields
    } Container;

    /**
     * function for determining the index of the container a specified field
     * (specified by X and Y coordinates) resides in. If no container contains
     * this field, the function is supposed to return -1. This can be the case
     * with diagonal containers, where only 2 containers are defined (the
     * diagonals) and all other fields are not part of any container of the
     * "diagonal" type.
     */
    typedef int (*getContainerIndexFunc)(unsigned x, unsigned y);

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

        // the function to determine the index of the container a specified
        // field is placed in (or -1 if the field is in no container of this 
        // type)
        getContainerIndexFunc *getContainerIndex;

        // number of containers of this type in 
        // the Sudoku. Will normally be 9, but can also be something else, 
        // e.g. 2 for "diagonal"
        size_t numberOfContainers;

        // the containers of this container type,
        // in "order of appearance", terminated by NULL
        Container **containers;

    } ContainerSet;

    // TODO can be ditched, use *ContainerType instead (vector of container types))

    typedef struct UnitDefs {
        struct ContainerSet *containerSets;
        size_t count;
    } UnitDefs;

    typedef Field * FieldsVector;

#ifdef	__cplusplus
}
#endif

#endif	/* TYPEDEFS_H */

