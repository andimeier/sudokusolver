/* 
 * File:   gridutils.h
 * Author: aurez
 *
 * Created on 04. April 2016, 21:57
 */

#ifndef GRIDUTILS_H
#define	GRIDUTILS_H

#ifdef	__cplusplus
extern "C" {
#endif

    void setValue(Field *field, unsigned value);
    void forbidNumberInNeighbors(Field *field, unsigned n);
    int forbidNumbersInOtherFields(Field **container, unsigned *n, Field **dontTouch);
    int forbidNumber(Field *field, unsigned n);
    int fieldHasCandidate(Field *field, unsigned n);
    int setUniqueNumber(Field *field);
    FieldsVector *fieldsWithCandidate(FieldsVector *container, unsigned n);
    unsigned equalNumberOfFieldsAndCandidates(FieldsVector *fieldsVector, unsigned *numbers);
    int getUniquePositionInContainer(Field **container, unsigned n);
    int fieldCandidatesSubsetOf(Field *field, unsigned *numbers);
    int containsField(Field **list, Field * field);
    int isFinished();
    void initCandidates();


#ifdef	__cplusplus
}
#endif

#endif	/* GRIDUTILS_H */

