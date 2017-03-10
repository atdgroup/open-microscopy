/*------------------------------------------------------------------------------*/
/* Asynchronous Timer Instrument Driver for LW/CVI for Windows 95/NT            */
/*------------------------------------------------------------------------------*/
#ifndef ASYNCTMR_HEADER
#define ASYNCTMR_HEADER

#include <stdarg.h>
#include "cvidef.h"
#include "cvirte.h"

#ifdef __cplusplus
    extern "C" {
#endif

/* Typedefs */
typedef int (CVICALLBACK * AsyncTimerCallbackPtr) (int reserved, int timerId, int event, void *callbackData, int eventData1, int eventData2);

/* Defines */
/* Attribute names */

#define ASYNC_ATTR_INTERVAL                         1
#define ASYNC_ATTR_COUNT                            2
#define ASYNC_ATTR_ENABLED                          3
#define ASYNC_ATTR_CALLBACK_DATA                    4       
#define ASYNC_ATTR_CALLBACK_FUNCTION_POINTER        5

/* Error codes */
#define ASYNC_DLL_LOAD_ERR                         -1
#define ASYNC_TIMER_FAIL_ERR                       -2
#define ASYNC_NO_MORE_HANDLES_ERR                  -3
#define ASYNC_OUT_OF_MEMORY_ERR                    -4
#define ASYNC_TIMER_NOT_FOUND_ERR                  -5
#define ASYNC_INTERNAL_ERR                         -6
#define ASYNC_INVALID_PARAMETER_ERR                -7
                                                   
/* For ease of comparison */                        
#define ASYNC_ATTR_MIN                              1
#define ASYNC_ATTR_MAX                              5


/* Prototypes */
int CVIFUNC NewAsyncTimer(double interval, int count, int status, AsyncTimerCallbackPtr callbackFunc, void *callbackData);
int CVIFUNC DiscardAsyncTimer(int timerId);

int CVIFUNC SuspendAsyncTimerCallbacks(void);
int CVIFUNC ResumeAsyncTimerCallbacks(void);

int CVIFUNC SetAsyncTimerAttribute(int timerId, int attribute, ...);
int CVIFUNC SetAsyncTimerAttributeFromParmInfo(int timerId, int attribute, va_list parmInfo);
int CVIFUNC GetAsyncTimerAttribute(int timerId, int attribute, void *value);
int CVIFUNC GetAsyncTimerResolution(double *resolution);

void CVIFUNC SetAsyncTimerName(int timerId, const char *name);
void CVIFUNC TestAsyncTimer(void);

#ifdef __cplusplus
    }
#endif

#endif /* ASYNCTMR */
