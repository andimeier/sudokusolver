/* 
 * File:   log.h
 * Author: ju
 *
 * Created on 05. April 2016, 00:33
 */

#ifndef LOG_H
#define	LOG_H

#ifdef	__cplusplus
extern "C" {
#endif

    // pointer to "print log" function
    // CAUTION: do not exceed the maximum string size, provided
    //  with the given buffer definition in log.c!
    typedef void (*PrintFunc)(char *msgBuffer, void *);
    
    void initLog();
    void writeLog(PrintFunc printFunc, void *info);
    void printLog();

#ifdef	__cplusplus
}
#endif

#endif	/* LOG_H */

