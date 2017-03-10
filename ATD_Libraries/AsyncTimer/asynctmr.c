//------------------------------------------------------------------------------
// Asynchronous Timer Instrument Driver for LW/CVI 4.x for Windows 95/NT
//------------------------------------------------------------------------------
// Note:  The multimedia timer functions block until all outstanding timer
// callbacks are complete.
//------------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Include files
//----------------------------------------------------------------------------
// Windows header files
#include <windows.h>
#include <mmsystem.h>

// CVI header files
#include <ansi_c.h>
#include <libsupp.h>
#include <userint.h>
#include <utility.h>

// Driver header file
#include "asynctmr.h"

//#define STAND_ALONE_DEBUG
#ifndef STAND_ALONE_DEBUG
#include "gci_utils.h"
#endif

#define ASYNC_TIMER_ENABLED  1
#define ASYNC_TIMER_DISABLED 0

#define MMTIMER_RESOLUTION_55     55        /* millisecs */
#define MMTIMER_RESOLUTION_1      1         /* millisecs */

static int number_of_timers = 0;
static int debug_panel = -1;
static int debug_tree = -1;

//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef struct _tAsyncTimerRec {
    int userHandle;
    AsyncTimerCallbackPtr mmTimerCallbackFunc;
    int count;
    int interval;
    int status;
    MMRESULT mmTimerId;
    void * callbackData;
    double lastTime;

	#ifdef ONE_THREAD_PER_CALLBACK
	int thread_id;
	long missed_tick;
	#endif

	long number_of_calls;
	int  has_name;
	char name[30];

    #ifdef _CVI_
    BYTE asyncBuffer[ASYNC_CALLBACK_ENV_SIZE];
    #endif 
    struct _tAsyncTimerRec *prevTimer;    
    struct _tAsyncTimerRec *nextTimer;    

} tAsyncTimerRec;


#ifdef ONE_THREAD_PER_CALLBACK
int thread_pool_id = -1;
#endif

//----------------------------------------------------------------------------
// Local variables
//----------------------------------------------------------------------------
static HANDLE globalMutexHandle = NULL;
static int lastHandle = 1;

static tAsyncTimerRec *asyncTimerListHead = NULL;
static tAsyncTimerRec *asyncTimerListTail = NULL;
static tAsyncTimerRec *asyncDiscardListHead = NULL; // Use this list to store discarded timers for later reuse

static int asyncTimerStatus = ASYNC_TIMER_ENABLED;
static CRITICAL_SECTION asyncCriticalSection = {0};
static int asyncCriticalSectionInitialized = 0;
																
//----------------------------------------------------------------------------
// Static functions
//----------------------------------------------------------------------------
static void CALLBACK LocalTimerCallbackFunc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 );
static tAsyncTimerRec *FindAsyncTimer(int timerId);
static MMRESULT RemoveAsyncTimer(tAsyncTimerRec *asyncTimerPtr);
static double MMTimer(void);
static unsigned int GetResolution(void);
static int InitAsyncLibrary(void);

//----------------------------------------------------------------------------
// Static functions for WINMM.DLL
//----------------------------------------------------------------------------
static int UnLoadDLLIfNecessary(void);
static int LoadDLLIfNeeded(void);

/* The two macros below are used as error return codes */
/* in case the DLL does not load, or is missing one or */
/* more functions, respectively.  You must define them */
/* to whatever values are meaningful for your DLL.     */
#define kFailedToLoadDLLError     ASYNC_DLL_LOAD_ERR
#define kCouldNotFindFunction     ASYNC_DLL_LOAD_ERR

static HINSTANCE DLLHandle;

/* Declare the variables that hold the addresses of the function   */
/* pointers.                                                       */
static MMRESULT (__stdcall *timeGetSystemTime_Ptr)(LPMMTIME pmmt, UINT cbmmt);
static DWORD (__stdcall *timeGetTime_Ptr)(void);
static MMRESULT (__stdcall *timeSetEvent_Ptr)(UINT uDelay,
                                              UINT uResolution,
                                              LPTIMECALLBACK fptc,
                                              DWORD dwUser, UINT fuEvent);
static MMRESULT (__stdcall *timeKillEvent_Ptr)(UINT uTimerID);
static MMRESULT (__stdcall *timeGetDevCaps_Ptr)(LPTIMECAPS ptc, UINT cbtc);
static MMRESULT (__stdcall *timeBeginPeriod_Ptr)(UINT uPeriod);
static MMRESULT (__stdcall *timeEndPeriod_Ptr)(UINT uPeriod);


//------------------------------------------------------------------------------
// LoadDLLIfNeeded
//------------------------------------------------------------------------------
static int LoadDLLIfNeeded(void)
{
    static int messageDisplayed = 0;
    
    if (DLLHandle)
        return 0;

    DLLHandle = LoadLibrary("winmm.dll");
    if (DLLHandle == NULL) {
        if (!messageDisplayed) {
#ifndef STAND_ALONE_DEBUG
            GCI_MessagePopup("asynctmr", "Failed to load winmm.dll");
#endif			
            messageDisplayed = 1;
        }   
        return kFailedToLoadDLLError;
        }

    if (!(timeGetSystemTime_Ptr = (void*) GetProcAddress(DLLHandle, 
         "timeGetSystemTime")))
        goto FunctionNotFoundError;

    if (!(timeGetTime_Ptr = (void*) GetProcAddress(DLLHandle, "timeGetTime")))
        goto FunctionNotFoundError;

    if (!(timeSetEvent_Ptr = (void*) GetProcAddress(DLLHandle, "timeSetEvent")))
        goto FunctionNotFoundError;

    if (!(timeKillEvent_Ptr = (void*) GetProcAddress(DLLHandle, "timeKillEvent")))
        goto FunctionNotFoundError;

    if (!(timeGetDevCaps_Ptr = (void*) GetProcAddress(DLLHandle, 
         "timeGetDevCaps")))
        goto FunctionNotFoundError;

    if (!(timeBeginPeriod_Ptr = (void*) GetProcAddress(DLLHandle, 
         "timeBeginPeriod")))
        goto FunctionNotFoundError;

    if (!(timeEndPeriod_Ptr = (void*) GetProcAddress(DLLHandle, "timeEndPeriod")))
        goto FunctionNotFoundError;

    return 0;

FunctionNotFoundError:
    FreeLibrary(DLLHandle);
    DLLHandle = 0;
    return kCouldNotFindFunction;
}

//------------------------------------------------------------------------------
// UnLoadDLLIfNecessary
//------------------------------------------------------------------------------
#if 0 /* Not used */
static int UnLoadDLLIfNecessary(void)
{
    if (DLLHandle) {
        FreeLibrary(DLLHandle);
        DLLHandle = NULL;
    }   
    return 0;
}    
#endif

//------------------------------------------------------------------------------
// Glue Code for each of the DLL functions
//------------------------------------------------------------------------------
static MMRESULT __stdcall ASYNCtimeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
{
    int dllLoadError;

    if (dllLoadError = LoadDLLIfNeeded())
        return dllLoadError;
    return (*timeGetSystemTime_Ptr)(pmmt, cbmmt);
}


static DWORD __stdcall ASYNCtimeGetTime(void)
{
    int dllLoadError;

    if (dllLoadError = LoadDLLIfNeeded())
        return dllLoadError;
    return (*timeGetTime_Ptr)();
}


static MMRESULT __stdcall ASYNCtimeSetEvent(UINT uDelay, UINT uResolution, 
                                LPTIMECALLBACK fptc, DWORD dwUser, 
                                UINT fuEvent)
{
    int dllLoadError;

    if (dllLoadError = LoadDLLIfNeeded())
        return dllLoadError;
    return (*timeSetEvent_Ptr)(uDelay, uResolution, fptc, dwUser, fuEvent);
}


static MMRESULT __stdcall ASYNCtimeKillEvent(UINT uTimerID)
{
    int dllLoadError;

    if (dllLoadError = LoadDLLIfNeeded())
        return dllLoadError;
    return (*timeKillEvent_Ptr)(uTimerID);
}


static MMRESULT __stdcall ASYNCtimeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
{
    int dllLoadError;

    if (dllLoadError = LoadDLLIfNeeded())
        return dllLoadError;
    return (*timeGetDevCaps_Ptr)(ptc, cbtc);
}


static MMRESULT __stdcall ASYNCtimeBeginPeriod(UINT uPeriod)
{
    int dllLoadError;

    if (dllLoadError = LoadDLLIfNeeded())
        return dllLoadError;
    return (*timeBeginPeriod_Ptr)(uPeriod);
}


#if 0 /* Not used */
static MMRESULT __stdcall ASYNCtimeEndPeriod(UINT uPeriod)
{
    int dllLoadError;

    if (dllLoadError = LoadDLLIfNeeded())
        return dllLoadError;
    return (*timeEndPeriod_Ptr)(uPeriod);
}
#endif


//------------------------------------------------------------------------------
// Init routine for Async Timer
//------------------------------------------------------------------------------
int CVIFUNC NewAsyncTimer(double doubleInterval, int count, int status, 
    AsyncTimerCallbackPtr callbackFunc, void *callbackData)
{
    int interval = (int) (doubleInterval*1000);
    int dllLoadError = 0;
    int timerId = 0;
    unsigned int resolution; //mS
    tAsyncTimerRec *asyncTimerPtr;
    MMRESULT mmResult;
    
	int tree_item_id;
	char buffer[50] = "";

	#ifdef ONE_THREAD_PER_CALLBACK
	// First call so we create the thread pool
	if(thread_pool_id == -1) {

		int err = 0;

		if ((err = CmtNewThreadPool (30, &thread_pool_id)) < 0) {

			return err;
		}
	}
    #endif 

    // Load DLL if necessary
    if (dllLoadError = LoadDLLIfNeeded())
        return dllLoadError;

    // Call InitAsyncLibrary here instead of in runstate change callback
    if (!InitAsyncLibrary())
        return ASYNC_INTERNAL_ERR;

    // Need to verify paramters
    if (status != ASYNC_TIMER_ENABLED && status != ASYNC_TIMER_DISABLED)
        return ASYNC_INVALID_PARAMETER_ERR;
    
    if (interval<0)
        return ASYNC_INVALID_PARAMETER_ERR;

    if (count==0)
        return ASYNC_INVALID_PARAMETER_ERR;

    // Enter critical section
    EnterCriticalSection(&asyncCriticalSection);
    
    // check if there are any discarded timers
    if (asyncDiscardListHead)
    {
        // Make the first discarded timer the new timer
        asyncTimerPtr = asyncDiscardListHead;
        asyncDiscardListHead->prevTimer = NULL;
        asyncDiscardListHead = asyncDiscardListHead->nextTimer;
    }
    else
    {
        // Out of timer handles?
        if (lastHandle <= 0)
        {
            // Leave critical section
            LeaveCriticalSection(&asyncCriticalSection);
            return ASYNC_NO_MORE_HANDLES_ERR;
        }
        
        // Allocate a new record for the new timer     
        if (!(asyncTimerPtr = (tAsyncTimerRec *) malloc(sizeof(tAsyncTimerRec))))
        {
            // Leave critical section
            LeaveCriticalSection(&asyncCriticalSection);
            return ASYNC_OUT_OF_MEMORY_ERR;  // Could not allocate memory 
        }
        
        // Assign a new handle
        asyncTimerPtr->userHandle = lastHandle++;
    }   
    
    // Attach the record to the end of the list
    asyncTimerPtr->nextTimer = NULL;

    if (asyncTimerListHead == NULL) 
    {
        asyncTimerListHead = asyncTimerListTail = asyncTimerPtr;
        asyncTimerPtr->prevTimer = NULL;
    }   
    else
    {
        asyncTimerListTail->nextTimer = asyncTimerPtr;
        asyncTimerPtr->prevTimer = asyncTimerListTail;
        asyncTimerListTail = asyncTimerPtr;
    }
    
    resolution = GetResolution();
    if (resolution == 0)
        goto Error;
            
    // Set new entry values
	number_of_timers++;
    asyncTimerPtr->status              = ASYNC_TIMER_DISABLED;
    asyncTimerPtr->count               = count;
    asyncTimerPtr->interval            = (interval > (int)resolution) ? interval : (int)resolution;
    asyncTimerPtr->mmTimerCallbackFunc = callbackFunc;
    asyncTimerPtr->callbackData        = callbackData;

    // Leave critical section
    LeaveCriticalSection(&asyncCriticalSection);

    // Start multimedia timer callback   
    mmResult = ASYNCtimeSetEvent((UINT)interval, (UINT)resolution, LocalTimerCallbackFunc, (DWORD) asyncTimerPtr, TIME_PERIODIC); /* TIME_ONESHOT */
//    mmResult = ASYNCtimeSetEvent((UINT)interval, (UINT)interval/100, LocalTimerCallbackFunc, (DWORD) asyncTimerPtr, TIME_PERIODIC); /* TIME_ONESHOT */
    
    // Enter critical section
    EnterCriticalSection(&asyncCriticalSection);
    
    // If timer is not available then remove timer from list, exit critical section and return error
    if (!mmResult)
        goto Error;

    // Otherwise store the timer handle
    asyncTimerPtr->mmTimerId = mmResult;

    asyncTimerPtr->status    = status;
    
    timerId = asyncTimerPtr->userHandle;
    
	asyncTimerPtr->has_name = 0;

	#ifdef ONE_THREAD_PER_CALLBACK
	asyncTimerPtr->thread_id		   = -1;
	asyncTimerPtr->missed_tick		   = 0;
	#endif

#ifndef STAND_ALONE_DEBUG
	if(gci_get_debug_level() == 1) 
#endif	
	{			
		asyncTimerPtr->number_of_calls	   = 0;

		if(debug_panel == -1)
		{
			debug_panel = NewPanel(0, "Async Thread Debugger", 50, 50, 300, 600);
			debug_tree = NewCtrl(debug_panel, CTRL_TREE, "", 0, 0);

			SetPanelAttribute(debug_panel, ATTR_SIZABLE, 0);
			SetCtrlAttribute(debug_panel, debug_tree, ATTR_HEIGHT, 300);
			SetCtrlAttribute(debug_panel, debug_tree, ATTR_WIDTH, 600);
			SetCtrlAttribute(debug_panel, debug_tree, ATTR_SHOW_MARKS, 0);
			SetCtrlAttribute(debug_panel, debug_tree, ATTR_SHOW_PLUS_MINUS, 0);
			SetCtrlAttribute(debug_panel, debug_tree, ATTR_SHOW_CONNECTION_LINES, 0);
			SetCtrlAttribute(debug_panel, debug_tree, ATTR_COLUMN_LABELS_VISIBLE, 1);

			InsertTreeColumn (debug_panel, debug_tree, 0, "Id / Name");
			InsertTreeColumn (debug_panel, debug_tree, 1, "Interval");
			InsertTreeColumn (debug_panel, debug_tree, 2, "Number Of Calls");
			InsertTreeColumn (debug_panel, debug_tree, 3, "Call Time");
			InsertTreeColumn (debug_panel, debug_tree, 4, "Time Diff");
		    
			#ifdef ONE_THREAD_PER_CALLBACK
			InsertTreeColumn (debug_panel, debug_tree, 5, "Thread");
			InsertTreeColumn (debug_panel, debug_tree, 6, "Missed Ticks");
			#endif
		}

		sprintf(buffer, "timer id: %d", timerId);

		tree_item_id = InsertTreeItem (debug_panel, debug_tree, VAL_SIBLING, 0, VAL_LAST, buffer, 0, 0, 0);

		sprintf(buffer, "%.2f", asyncTimerPtr->interval / 1000.0);
		SetTreeCellAttribute (debug_panel, debug_tree, tree_item_id, 1, ATTR_LABEL_TEXT, buffer);

		DisplayPanel(debug_panel);
	}

    // Leave critical section
    LeaveCriticalSection(&asyncCriticalSection);
    
    return timerId;

Error:  
    asyncTimerPtr->mmTimerId           = 0;
    asyncTimerPtr->status              = ASYNC_TIMER_DISABLED;
    asyncTimerPtr->count               = 0;
    asyncTimerPtr->mmTimerCallbackFunc = NULL;
    asyncTimerPtr->callbackData        = NULL;
    
    // Remove the timer from the main timer list
    if (asyncTimerPtr->prevTimer) 
    {
        asyncTimerPtr->prevTimer->nextTimer = asyncTimerPtr->nextTimer;
        if (asyncTimerPtr->nextTimer)
            asyncTimerPtr->nextTimer->prevTimer = asyncTimerPtr->prevTimer;
    }
    else 
    {
        asyncTimerListHead = asyncTimerPtr->nextTimer;
        if (asyncTimerPtr->nextTimer)
            asyncTimerPtr->nextTimer->prevTimer = NULL;
    }
    
    if (asyncTimerListTail == asyncTimerPtr)
        asyncTimerListTail = asyncTimerPtr->prevTimer;
    
    // Add it to the top discarded timer list. 
    asyncTimerPtr->prevTimer = NULL;
    asyncTimerPtr->nextTimer = asyncDiscardListHead;
    if (asyncDiscardListHead)
        asyncDiscardListHead->prevTimer = asyncTimerPtr;
    asyncDiscardListHead = asyncTimerPtr;

    // Leave critical section
    LeaveCriticalSection(&asyncCriticalSection);
    return ASYNC_TIMER_FAIL_ERR;
}

void CVIFUNC SetAsyncTimerName(int timerId, const char *name)
{
	tAsyncTimerRec *asyncTimerPtr = FindAsyncTimer(timerId);
	char buffer[50] = "";

	if(asyncTimerPtr!=NULL && name!=NULL) {
		asyncTimerPtr->has_name = 1;
		strncpy(asyncTimerPtr->name, name, 29);
		sprintf(buffer, "%s", asyncTimerPtr->name);
		SetTreeCellAttribute (debug_panel, debug_tree, asyncTimerPtr->userHandle - 1, 0, ATTR_LABEL_TEXT, buffer);	
	}
}

//------------------------------------------------------------------------------
// This routine finds the Async Timer from the list given the timerId
//------------------------------------------------------------------------------
static tAsyncTimerRec *FindAsyncTimer(int timerId)
{
    tAsyncTimerRec *asyncTimerPtr = asyncTimerListHead;
    
    // Search for the timerId among all the timers
    for(;asyncTimerPtr && asyncTimerPtr->userHandle <= timerId;asyncTimerPtr = asyncTimerPtr->nextTimer)
    {
        // If found return the pointer
        if (asyncTimerPtr->userHandle == timerId)
            return asyncTimerPtr;
    }
    // No timer with the given timerId found
    return NULL;
}
    
//------------------------------------------------------------------------------
// This routine removes the Async Timer from the list given the pointer to it
// This routine asssume you are in the critical section
//------------------------------------------------------------------------------
static MMRESULT RemoveAsyncTimer(tAsyncTimerRec *asyncTimerPtr)
{
    MMRESULT mmResult;
    double currentTime;
    double deltaTime; 

    // Disable the timer
    asyncTimerPtr->status = ASYNC_TIMER_DISABLED;

    // Remove the timer from the main timer list
    if (asyncTimerPtr->prevTimer) 
    {
        asyncTimerPtr->prevTimer->nextTimer = asyncTimerPtr->nextTimer;
        if (asyncTimerPtr->nextTimer)
            asyncTimerPtr->nextTimer->prevTimer = asyncTimerPtr->prevTimer;
    }
    else 
    {
        asyncTimerListHead = asyncTimerPtr->nextTimer;
        if (asyncTimerPtr->nextTimer)
            asyncTimerPtr->nextTimer->prevTimer = NULL;
    }
        
    if (asyncTimerListTail == asyncTimerPtr)
        asyncTimerListTail = asyncTimerPtr->prevTimer;
        
    // Add it to the top discarded timer list. 
    asyncTimerPtr->prevTimer = NULL;
    asyncTimerPtr->nextTimer = asyncDiscardListHead;
    if (asyncDiscardListHead)
        asyncDiscardListHead->prevTimer = asyncTimerPtr;
    asyncDiscardListHead = asyncTimerPtr;
    
    // Update time for DISCARD
    currentTime = MMTimer();
    deltaTime = currentTime - asyncTimerPtr->lastTime;
    asyncTimerPtr->lastTime = currentTime; 
    
    // Disable timer
    if (asyncTimerPtr->mmTimerId)
    {
        // Leave critical section
        LeaveCriticalSection(&asyncCriticalSection);

        mmResult = ASYNCtimeKillEvent(asyncTimerPtr->mmTimerId);

        // Call user's callback with EVENT_DISCARD
        if (asyncTimerPtr->mmTimerCallbackFunc)
            (*asyncTimerPtr->mmTimerCallbackFunc) ((int)0, asyncTimerPtr->userHandle, 
                (int)EVENT_DISCARD, asyncTimerPtr->callbackData, (int) &currentTime, (int) &deltaTime);
            
        // Enter critical section
        EnterCriticalSection(&asyncCriticalSection);

        asyncTimerPtr->mmTimerId           = 0;
        asyncTimerPtr->count               = 0;
        asyncTimerPtr->mmTimerCallbackFunc = NULL;
        asyncTimerPtr->callbackData        = NULL;
    }
    else
        mmResult = 0;
        
    // If no timers are left, free discard list, so users do not see unfreed memory
    if (!asyncTimerListHead)
    {
        while(asyncDiscardListHead) 
        {
            asyncTimerPtr = asyncDiscardListHead;
            asyncDiscardListHead = asyncTimerPtr->nextTimer;
            
            free (asyncTimerPtr);
        }
        lastHandle = 1;
    }
        
    return mmResult;
}

//------------------------------------------------------------------------------
// Clean up routine for Async Timer
//------------------------------------------------------------------------------
int CVIFUNC DiscardAsyncTimer(int timerId)
{   
    tAsyncTimerRec *asyncTimerPtr;
    MMRESULT mmResult;

    // Call InitAsyncLibrary here instead of in runstate change callback
    if (!InitAsyncLibrary())
        return ASYNC_INTERNAL_ERR;
        
    // Enter critical section
    EnterCriticalSection(&asyncCriticalSection);
    
    // Lookup timerId in list, if timerId is -1, disable all timers
    if (timerId == -1)
    {
        while(asyncTimerListHead) {
            // Remove the timer from the list - RemoveAsyncTimer will leave and reenter the critical section
            mmResult = RemoveAsyncTimer(asyncTimerListHead);
        }
        
        lastHandle = 1;

		if(debug_panel != -1) {
			DiscardPanel(debug_panel);
			debug_panel = 1;
		}

		number_of_timers = 0;

    } else {    
        asyncTimerPtr = FindAsyncTimer(timerId);
    
        if (!asyncTimerPtr)
        {
            // Leave critical section
            LeaveCriticalSection(&asyncCriticalSection);
            return ASYNC_TIMER_NOT_FOUND_ERR;
        }
        
        // Remove the timer from the list - RemoveAsyncTimer will leave and reenter the critical section
        mmResult = RemoveAsyncTimer(asyncTimerPtr);
		number_of_timers--;

		if(number_of_timers <= 0)
		{
			if(debug_panel != -1) {
				DiscardPanel(debug_panel);
				debug_panel = 1;
			}
		}
    }    

    // Leave critical section
    LeaveCriticalSection(&asyncCriticalSection);
    
    return 0;
}

//------------------------------------------------------------------------------
// Suspend all enabled timers
//------------------------------------------------------------------------------
int CVIFUNC SuspendAsyncTimerCallbacks(void)
{
    // Call InitAsyncLibrary here instead of in runstate change callback
    if (!InitAsyncLibrary())
        return ASYNC_INTERNAL_ERR;
        
    // Enter critical section
    EnterCriticalSection(&asyncCriticalSection);
    
    asyncTimerStatus = ASYNC_TIMER_DISABLED;
    
    // Leave critical section
    LeaveCriticalSection(&asyncCriticalSection);
    return 0;
}

//------------------------------------------------------------------------------
// Resume all enabled timers 
//------------------------------------------------------------------------------
int CVIFUNC ResumeAsyncTimerCallbacks(void)
{
    // Call InitAsyncLibrary here instead of in runstate change callback
    if (!InitAsyncLibrary())
        return ASYNC_INTERNAL_ERR;
        
    // Enter critical section
    EnterCriticalSection(&asyncCriticalSection);
    
    asyncTimerStatus = ASYNC_TIMER_ENABLED;
    
    // Leave critical section
    LeaveCriticalSection(&asyncCriticalSection);
    
    return 0;
}

//------------------------------------------------------------------------------
// This function sets the various settable attributes of the timer
//------------------------------------------------------------------------------
int CVIFUNC SetAsyncTimerAttribute(int timerId, int attribute, ...)
{
    va_list parmInfo;
    int error;
    
    va_start(parmInfo, attribute);
    error = SetAsyncTimerAttributeFromParmInfo(timerId,attribute, parmInfo);
    va_end(parmInfo);
    
    return error;
}

//------------------------------------------------------------------------------
// This function sets the various settable attributes of the timer from 
// the given parmInfo
//------------------------------------------------------------------------------
int CVIFUNC SetAsyncTimerAttributeFromParmInfo(int timerId, int attribute,va_list parmInfo)
{
    tAsyncTimerRec *asyncTimerPtr;
    int error = UIENoError;
    
    // Call InitAsyncLibrary here instead of in runstate change callback
    if (!InitAsyncLibrary())
        return ASYNC_INTERNAL_ERR;
        
    if (attribute < ASYNC_ATTR_MIN || attribute > ASYNC_ATTR_MAX)
        return ASYNC_INVALID_PARAMETER_ERR;
        
                
    // Enter critical section
    EnterCriticalSection(&asyncCriticalSection);
    
    asyncTimerPtr = FindAsyncTimer(timerId);
    if (!asyncTimerPtr)
        error =  ASYNC_TIMER_NOT_FOUND_ERR;
    else
    {
        switch(attribute)
        {
            case ASYNC_ATTR_ENABLED :
            {
                int status = va_arg(parmInfo,int);
                if (status == ASYNC_TIMER_ENABLED || status == ASYNC_TIMER_DISABLED)
                    asyncTimerPtr->status = status;
                else
                    error = ASYNC_INVALID_PARAMETER_ERR;
                break;
            }
            
            case ASYNC_ATTR_COUNT :
            {
                int count = va_arg(parmInfo,int);
                if (count != 0)
                    asyncTimerPtr->count = count;
                else
                    error = ASYNC_INVALID_PARAMETER_ERR;    
                break;
            }
            
            case ASYNC_ATTR_INTERVAL :
            {
                double doubleInterval = va_arg(parmInfo,double);
                int interval = (int)(doubleInterval*1000);
                if (interval >= 0)
                {
                    MMRESULT mmResult;
                    UINT uResolution;
                    TIMECAPS timeCaps;
                    
                    // Get Timer resolution information
                    mmResult = ASYNCtimeGetDevCaps(&timeCaps, sizeof(TIMECAPS));
                    uResolution = timeCaps.wPeriodMin;
    
                    if (interval < (int)uResolution)
                        interval = (int)uResolution;
                    
                    // Create a new timer
                    mmResult = ASYNCtimeSetEvent((UINT)interval, (UINT)uResolution, LocalTimerCallbackFunc, (DWORD) asyncTimerPtr, TIME_PERIODIC); /* TIME_ONESHOT */
                //    mmResult = ASYNCtimeSetEvent((UINT)interval, (UINT)interval/100, LocalTimerCallbackFunc, (DWORD) asyncTimerPtr, TIME_PERIODIC); /* TIME_ONESHOT */
    
                    // If timer is not available 
                    if (!mmResult)
                        error = ASYNC_TIMER_FAIL_ERR;
                    else
                    {
                        // else kill old timer
                        if (ASYNCtimeKillEvent(asyncTimerPtr->mmTimerId) == TIMERR_NOERROR)
                        {
                            asyncTimerPtr->interval = interval;
                            asyncTimerPtr->mmTimerId = mmResult;
                        }
                        else
                        {
                            ASYNCtimeKillEvent(mmResult);
                            error = ASYNC_TIMER_FAIL_ERR;
                        }
                    }
                }
                else
                    error = ASYNC_INVALID_PARAMETER_ERR;
                break;
            }
            
            case ASYNC_ATTR_CALLBACK_DATA :
            {
                void * callbackData = va_arg(parmInfo,void *);
                asyncTimerPtr->callbackData = callbackData;
                break;
            }
            
            case ASYNC_ATTR_CALLBACK_FUNCTION_POINTER :
            {
                AsyncTimerCallbackPtr callbackFn = va_arg(parmInfo,AsyncTimerCallbackPtr);
                asyncTimerPtr->mmTimerCallbackFunc = callbackFn;
                break;
            }
            
        }
    }       
    // Leave critical section
    LeaveCriticalSection(&asyncCriticalSection);
    
    return error;
}           

//------------------------------------------------------------------------------
// This function gets the various attributes of the timer
//------------------------------------------------------------------------------
int CVIFUNC GetAsyncTimerAttribute(int timerId, int attribute, void *value)
{
    tAsyncTimerRec *asyncTimerPtr;
    int error = UIENoError;
    
    // Call InitAsyncLibrary here instead of in runstate change callback
    if (!InitAsyncLibrary())
        return ASYNC_INTERNAL_ERR;
        
    if (attribute < ASYNC_ATTR_MIN || attribute > ASYNC_ATTR_MAX)
        return ASYNC_INVALID_PARAMETER_ERR;
        
    // Enter critical section
    EnterCriticalSection(&asyncCriticalSection);
    
    asyncTimerPtr = FindAsyncTimer(timerId);
    if (!asyncTimerPtr)
        error =  ASYNC_TIMER_NOT_FOUND_ERR;
    else
    {
        switch(attribute)
        {
            case ASYNC_ATTR_ENABLED :
            {
                *((int *) value) = asyncTimerPtr->status;
                break;
            }
            
            case ASYNC_ATTR_COUNT :
            {
                *((int *) value) = asyncTimerPtr->count;
                break;
            }
            
            case ASYNC_ATTR_INTERVAL :
            {
                *((double *) value) = (double)((double)asyncTimerPtr->interval)/1000.0;
                break;
            }
            
            case ASYNC_ATTR_CALLBACK_DATA :
            {
                *((void **) value) = asyncTimerPtr->callbackData;
                break;
            }
            
            case ASYNC_ATTR_CALLBACK_FUNCTION_POINTER :
            {
                *((AsyncTimerCallbackPtr *) value) = asyncTimerPtr->mmTimerCallbackFunc;
                break;
            }
        }
    }       
    // Leave critical section
    LeaveCriticalSection(&asyncCriticalSection);
    
    return error;
}   

//------------------------------------------------------------------------------
// This function gets the multi-media timer resolution used by async timers
//------------------------------------------------------------------------------
int CVIFUNC GetAsyncTimerResolution(double *resolutionInSecs)
{
    int error = UIENoError;
    unsigned int resolution;
    
    *resolutionInSecs = 0;
    
    if (!InitAsyncLibrary())
        return ASYNC_INTERNAL_ERR;

    EnterCriticalSection(&asyncCriticalSection);
    resolution = GetResolution();
    LeaveCriticalSection(&asyncCriticalSection);
    
    if (resolution == 0)
        return ASYNC_INTERNAL_ERR;
    *resolutionInSecs = ((double)resolution) / 1000;
    
    return error;
}

		
static int CVICALLBACK timer_callback_thread(void *callback)
{
	tAsyncTimerRec *asyncTimerPtr = (tAsyncTimerRec *) callback;
	
	double currentTime = MMTimer();
    double deltaTime = currentTime - asyncTimerPtr->lastTime;  
    void *callbackData = asyncTimerPtr->callbackData;

	asyncTimerPtr->number_of_calls++;

#ifndef STAND_ALONE_DEBUG
	if(gci_get_debug_level() == 1) 
#endif			
	{

		char buffer[50] = "";

		sprintf(buffer, "%d", CmtGetCurrentThreadID());

		SetTreeCellAttribute (debug_panel, debug_tree, asyncTimerPtr->userHandle - 1, 5, ATTR_LABEL_TEXT, buffer);

		#ifdef ONE_THREAD_PER_CALLBACK

		sprintf(buffer, "%d", asyncTimerPtr->missed_tick);

		SetTreeCellAttribute (debug_panel, debug_tree, asyncTimerPtr->userHandle - 1, 6, ATTR_LABEL_TEXT, buffer);

		#endif
	}

	asyncTimerPtr->lastTime = currentTime; 

	if(asyncTimerPtr->mmTimerCallbackFunc != NULL && asyncTimerStatus == ASYNC_TIMER_ENABLED) {

		(*asyncTimerPtr->mmTimerCallbackFunc) ((int)0, asyncTimerPtr->userHandle, (int)EVENT_TIMER_TICK,
			callbackData, (int) &currentTime, (int) &deltaTime);
	}

	#ifdef ONE_THREAD_PER_CALLBACK
	asyncTimerPtr->thread_id = -1;
	#endif

	return 0;
}

//------------------------------------------------------------------------------
// This is the Multi Media Timer callback function
//------------------------------------------------------------------------------
//Parameters
//    uID    - Identifier of the timer event. This identifier was returned by the timeSetEvent function 
//             when the timer event was set up.
//    uMsg   - Reserved; do not use.
//    dwUser - User instance data supplied to the dwUser parameter of timeSetEvent.
//    dw1    - Reserved; do not use.
//    dw2    - Reserved; do not use.
static void CALLBACK LocalTimerCallbackFunc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 )
{
    MMRESULT mmResult;
    double currentTime;
    double deltaTime; 
    void *callbackData = NULL;
	char buffer[100] = "";

    tAsyncTimerRec *asyncTimerPtr = (tAsyncTimerRec *)dwUser;

    // Enter critical section
    EnterCriticalSection(&asyncCriticalSection);

    // If at a breakpoint then don't do anything
    if (!asyncTimerStatus)
    {
        // Leave critical section
        LeaveCriticalSection(&asyncCriticalSection);
        return;
    }
    
    // See if we need to leave
    if ((!asyncTimerPtr) || (asyncTimerPtr->status == ASYNC_TIMER_DISABLED))
    {
        // Leave critical section
        LeaveCriticalSection(&asyncCriticalSection);
        return;
    }
       
    // If count is zero, leave; otherwise decrement count if not less than zero
    if (!asyncTimerPtr->count) 
    {
        // Remove the timer from the list - RemoveAsyncTimer will leave and reenter the critical section
        mmResult = RemoveAsyncTimer(asyncTimerPtr);
    
        // Leave critical section
        LeaveCriticalSection(&asyncCriticalSection);
        return;
    }    
    else if (asyncTimerPtr->count>0)
        asyncTimerPtr->count--;

    // No need to make this thread safe, it will not be reentered
    currentTime = MMTimer();
    deltaTime = currentTime - asyncTimerPtr->lastTime;
    asyncTimerPtr->lastTime = currentTime; 
    callbackData = asyncTimerPtr->callbackData;
    
#ifndef STAND_ALONE_DEBUG
	if(gci_get_debug_level() == 1) 
#endif			
	{
	
		if (asyncTimerPtr->mmTimerCallbackFunc) {

			if(asyncTimerPtr->has_name == 0) {
				sprintf(buffer, "timer id: %d", asyncTimerPtr->userHandle);
			}
			else {
				sprintf(buffer, "%s", asyncTimerPtr->name);
			}

			SetTreeCellAttribute (debug_panel, debug_tree, asyncTimerPtr->userHandle - 1, 0, ATTR_LABEL_TEXT, buffer);

			sprintf(buffer, "%.2f", asyncTimerPtr->interval / 1000.0);
			SetTreeCellAttribute (debug_panel, debug_tree, asyncTimerPtr->userHandle - 1, 1, ATTR_LABEL_TEXT, buffer);

			sprintf(buffer, "%d", asyncTimerPtr->number_of_calls);
			SetTreeCellAttribute (debug_panel, debug_tree, asyncTimerPtr->userHandle - 1, 2, ATTR_LABEL_TEXT, buffer);
			
			sprintf(buffer, "%.0f", currentTime);
			SetTreeCellAttribute (debug_panel, debug_tree, asyncTimerPtr->userHandle - 1, 3, ATTR_LABEL_TEXT, buffer);
			
			sprintf(buffer, "%.0f", deltaTime);
			SetTreeCellAttribute (debug_panel, debug_tree, asyncTimerPtr->userHandle - 1, 4, ATTR_LABEL_TEXT, buffer);
		}		 
	}

    // We want to release Critical Section before callback so user's callback can take their time to do what they need to do 
    LeaveCriticalSection(&asyncCriticalSection);

#if defined(_CVI_) && (_CVI_DEBUG_==0)
    //--------------------------------------------------------------------
    // The callback function and any code that is executed by it must be 
    // called inside the EnterAsyncCallback/ExitAsyncCallback.  
    //--------------------------------------------------------------------
    // Call CVI Library function
    //--------------------------------------------------------------------
     EnterAsyncCallback(asyncTimerPtr->asyncBuffer);
#endif    
    
    // Call user's callback
	 if (asyncTimerPtr->mmTimerCallbackFunc) {

		#ifdef ONE_THREAD_PER_CALLBACK

		if(asyncTimerPtr->thread_id != -1)   {
			asyncTimerPtr->missed_tick++;
			return;
		}

		CmtScheduleThreadPoolFunction (thread_pool_id, timer_callback_thread, asyncTimerPtr, &(asyncTimerPtr->thread_id)); 
		
		#else
		(*asyncTimerPtr->mmTimerCallbackFunc) ((int)0, asyncTimerPtr->userHandle, (int)EVENT_TIMER_TICK, callbackData, (int) &currentTime, (int) &deltaTime);
		asyncTimerPtr->number_of_calls++;
		#endif
	 }

#if defined(_CVI_) && (_CVI_DEBUG_==0)
    //--------------------------------------------------------------------
    // The callback function and any code that is executed by it must be 
    // called inside the EnterAsyncCallback/ExitAsyncCallback.  
    //--------------------------------------------------------------------
    // Call CVI Library function
    //--------------------------------------------------------------------
    ExitAsyncCallback(asyncTimerPtr->asyncBuffer);
#endif

    return;
}


//------------------------------------------------------------------------------
// Local Timer() function that is thread safe when initialized the first time
//------------------------------------------------------------------------------
static double MMTimer(void)
{
    static DWORD startTime = 0;
    DWORD currentTime;
    
    currentTime = ASYNCtimeGetTime();
    
    if (!startTime) {
        startTime = currentTime;
        return 0.0;    
    } else {
        return  (((double)(currentTime - startTime))/1000.0);
    }    
}



//------------------------------------------------------------------------------
// InitAsyncLibrary
//------------------------------------------------------------------------------
static int InitAsyncLibrary(void)   
{                           
    int holdingMutex = FALSE;
      
    // The purpose of the mutex is to ensure that:                 
    //   - the critical section(s) are initialized only once         
    
    // CreateMutex only creates the mutex the first time   
    // it is called.  After that it only opens it (but it does       
    // return a separate handle, which must be closed separately.)   
    // If the globalMutexHandle is already non-NULL, then probably   
    // we already tried and failed.  We need to wait on the mutex 
    // to be sure that we are the only ones.  
    // Otherwise, we need to create or open the mutex.  
    // (We would not be creating it only if another thread   
    // got there just before this thread.)  Once we open or create   
    // the mutex, we need to store the handle in a local variable.   
    // We then wait on the mutex so that only one thread can store   
    // its handle as the globalMutexHandle.  The other thread must,  
    // once it continues, release the mutex, close its handle, and   
    // wait on the globalMutexHandle.

    /* may have been created in previous execution */
    if (globalMutexHandle != NULL) {
         WaitForSingleObject (globalMutexHandle, INFINITE);
    }
    else {
         DWORD processId;
         char mutexName[MAX_PATH+1];
         HANDLE tmpMutexHandle;
         
         // Create or Open existing Mutex as a temporary mutex
         processId = GetCurrentProcessId ();
         sprintf (mutexName, "National Instruments LabWindows/CVI Async Timer Library %d", processId);
         tmpMutexHandle = CreateMutex (NULL, FALSE, mutexName);
         if (tmpMutexHandle == NULL) {
             return FALSE; 
         }
         
         // Acquire Mutex
         WaitForSingleObject (tmpMutexHandle, INFINITE);
         
         // If global mutex handle was already initialized, release it
         if (globalMutexHandle != NULL) {
            ReleaseMutex (tmpMutexHandle);
            CloseHandle (tmpMutexHandle);
            WaitForSingleObject (globalMutexHandle, INFINITE);
         }
         // Otherwise Create critical sections 
         else {
             globalMutexHandle = tmpMutexHandle;
         }
    }
    holdingMutex = TRUE;
    
    // Init critical section if not already done
    if (!asyncCriticalSectionInitialized) {
        InitializeCriticalSection (&asyncCriticalSection);
        asyncCriticalSectionInitialized = 1;
    }    
    ReleaseMutex (globalMutexHandle);
  
    return TRUE;

}  

//------------------------------------------------------------------------------
// Gets the resolution of the multimedia timer
//------------------------------------------------------------------------------
static unsigned int GetResolution(void)
{
    static unsigned int resolution = 0;

    MMRESULT mmResult;
    TIMECAPS timeCaps;
    
    //------------------------------------------------------------------------
    // Using the multimedia timer causes the resolution returned by          
    // timeGetTime() to change to 1ms regardless of what we pass to          
    // timeBeginPeriod().  timeBeginPeriod() does reprogram the timer chip   
    // so interrupts are generated more often if the value passed to         
    // timeBeginPeriod is smaller.                                           
    //
    // We are using 55 as the default timer resolution to avoid any problems 
    // with the high speed HW(DAQ).  This means that the chip should not     
    // be generating interrupts at any higher rate than normal, but the      
    // resolution returned by timeGetTime() is 1 ms.                         
    //
    // The resolution returned by GetTickCount() is always 55 ms.
    
    // On window NT, you don't get the 1 ms resolution unless you set the    
    // multimedia resolution to 1ms. (This does not appear to be true on WinNT 4)
    //------------------------------------------------------------------------
    
    if (resolution != 0)
        return resolution;
        
    // Get Timer resolution information
    mmResult = ASYNCtimeGetDevCaps(&timeCaps, sizeof(TIMECAPS));
    if (mmResult)
        goto Error;

    if (GetCurrentPlatform ()== kPlatformWin95) {
         resolution = MMTIMER_RESOLUTION_55;
    } else {
        resolution = MMTIMER_RESOLUTION_1;
    }
        
    if (timeCaps.wPeriodMin > resolution)
        resolution = timeCaps.wPeriodMin;
    else if (timeCaps.wPeriodMax < resolution)
        resolution = timeCaps.wPeriodMax;

    mmResult = ASYNCtimeBeginPeriod(resolution);
    if (mmResult)
        goto Error;

    return resolution;
    
Error:
    resolution = 0;
    return 0;
}


static int CVICALLBACK OnAsyncTestTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			double random_secs = (5.0 * ((double) rand() / RAND_MAX));
			//Delay((int) random_secs);
			Delay(1);
		}
	}
	return (0);
}


void CVIFUNC TestAsyncTimer(void)
{
	NewAsyncTimer(0.05, -1, 1, OnAsyncTestTimerTick, NULL);	
	NewAsyncTimer(0.05, -1, 1, OnAsyncTestTimerTick, NULL);	
	NewAsyncTimer(5.0, -1, 1, OnAsyncTestTimerTick, NULL);	
	NewAsyncTimer(2.0, -1, 1, OnAsyncTestTimerTick, NULL);	
	NewAsyncTimer(0.5, -1, 1, OnAsyncTestTimerTick, NULL);	
	NewAsyncTimer(1.5, -1, 1, OnAsyncTestTimerTick, NULL);	
	NewAsyncTimer(3.0, -1, 1, OnAsyncTestTimerTick, NULL);	
	NewAsyncTimer(2.0, -1, 1, OnAsyncTestTimerTick, NULL);	
	NewAsyncTimer(4.2, -1, 1, OnAsyncTestTimerTick, NULL);	
}