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

    // the qualities of solve steps
    typedef enum {
        STEP_REMOVE_CANDIDATE,
        STEP_SET_VALUE,
        STEP_STRATEGY_FINDING_START,
        STEP_STRATEGY_FINDING_END
    } STEP_TYPE;

    // pointer to "print solve step" function
    // CAUTION: do not exceed the maximum string size, provided
    //  with the given buffer definition in log.c!
    typedef void (*PrintFunc)(char *msgBuffer, STEP_TYPE stepType, void *info);

    void initRecorder(void);
    void recordStep(STEP_TYPE stepType, PrintFunc printFunc, void *info);
    void recordStartOfStrategyFinding(PrintFunc printFunc, void *info);
    void recordEndOfStrategyFinding(void);
    void printStep(void);
    void releaseRecording(void);

#ifdef	__cplusplus
}
#endif

#endif	/* RECORDER_H */
