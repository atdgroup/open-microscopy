#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

static MMRESULT (__stdcall *timeSetEvent_Ptr)(UINT uDelay,
                                              UINT uResolution,
                                              LPTIMECALLBACK fptc,
                                              DWORD dwUser, UINT fuEvent);

//static MMRESULT (__stdcall *timeBeginPeriod_Ptr)(UINT uPeriod);
//static MMRESULT (__stdcall *timeEndPeriod_Ptr)(UINT uPeriod);

time_t start;

static void CALLBACK LocalTimerCallbackFunc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 )
{
    time_t clock = time(NULL);
	char buffer[256];
	
    printf("Called  at %safter %d s \n", ctime(&clock), clock-start);
	start = clock;
}

int main (void)
{
	UINT interval = 600000;

	HINSTANCE DLLHandle = LoadLibrary("winmm.dll");

    start = time(NULL);
	printf("Set timer for %d seconds.\nStarted at %s\n", interval/1000, ctime(&start));

	timeSetEvent_Ptr = (void *) GetProcAddress(DLLHandle, "timeSetEvent");
//	timeBeginPeriod_Ptr = (void*) GetProcAddress(DLLHandle, "timeBeginPeriod");
//	timeEndPeriod_Ptr = (void*) GetProcAddress(DLLHandle, "timeEndPeriod");

//	(*timeBeginPeriod_Ptr)(1);		
	(*timeSetEvent_Ptr)(interval, 1, LocalTimerCallbackFunc, 0, TIME_PERIODIC);

	getchar();

//	(*timeEndPeriod_Ptr)(1);		

	return 0;
}