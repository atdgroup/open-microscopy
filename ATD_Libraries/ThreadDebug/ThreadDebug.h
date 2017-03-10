#ifndef __THREAD_DEBUG__
#define __THREAD_DEBUG__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "Toolbox.h"

typedef int (CVICALLBACK *ThreadFunctionPtr) (void *functionData);

int CVIFUNC GciCmtNewLock (const char lockName[], unsigned int options, int *lockHandle);

int CVIFUNC GciCmtGetLock (int lockHandle);

int CVIFUNC GciCmtReleaseLock (int lockHandle);

int CVIFUNC     GciCmtScheduleThreadPoolFunction (int poolHandle, ThreadFunctionPtr threadFunction, 
												  void *threadFunctionData, int *threadFunctionID);


#ifdef __cplusplus
}
#endif

#endif
