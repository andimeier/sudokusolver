/* 
 * File:   typedefs.h
 * Author: ju
 *
 * Created on 04. April 2016, 21:31
 */

#ifndef TYPEDEFS_H
#define	TYPEDEFS_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct Field {
        int *unitPositions; // position in the several units. In any unit, a 
          // position of -1 means that this number is not part of any instance
          // of this unit (this can be the case in the unit "diagonal"))
        unsigned *candidates;
        unsigned initialValue;
        unsigned candidatesLeft;
        unsigned value;
        char *name; // "name" of field = coordinates, e.g. "A2"
    } Field;
    
    typedef struct Container {
        char *name; // the name of the container, e.g. "row 3" or "slash diagonal"
        Field **fields; // the fields in this container, in "order of 
          // appearance". This is a vector of MAX_NUMBER fields
    } Container;
    
    typedef struct Unit {
        char *name; // the name of the unit used for log output in singular 
          // form, e.g. 'row'
        size_t containers; // number of units of this type in the Sudoku. 
          // Will normally be 9, but can also be something else, e.g. 2 for 
          // "diagonal"
        Container *theContainers; // the fields in this unit, in "order of 
          // appearance". The two dimensions are [UnitDefs.count][9], 9 standing
          // for 9 possible numbers. So, normally this will be a [9][9] array,
          // but could also be e.g. a [2][9] array (unit "diagonal" having only
          // 2 containers)
    } Unit;
    
    typedef struct UnitDefs {
        struct Unit *units;
        size_t count;
    } UnitDefs;

    typedef Field * FieldsVector;
    
#ifdef	__cplusplus
}
#endif

#endif	/* TYPEDEFS_H */

