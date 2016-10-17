/* 
 * File:   recorder.h
 * Author: ju
 *
 * Created on 05. April 2016, 00:33
 */

#ifndef RECORDER_H
#define	RECORDER_H

#ifdef	__cplusplus
extern "C" {
#endif

    // pointer to "print solve step" function
    // CAUTION: do not exceed the maximum string size, provided
    //  with the given buffer definition in log.c!
    typedef void (*PrintFunc)(char *msgBuffer, void *);
    
    void initRecorder();
    void recordStep(PrintFunc printFunc, void *info);
    void printStep();
    void releaseRecording();

#ifdef	__cplusplus
}
#endif

#endif	/* RECORDER_H */
