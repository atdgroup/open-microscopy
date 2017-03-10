
#include <stdio.h>
#include "asynctmr.h"
#include <userint.h>
#include <utility.h>

double start;

int CVICALLBACK TimeLapseSnapTimerCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    
	switch (event)
	{
		case EVENT_TIMER_TICK:

			printf("callback at %.0f\n", Timer()-start);

			break;
	}

	return 0;
}


int main (void)
{
	int timer = NewAsyncTimer(10.0, -1, 0, TimeLapseSnapTimerCallback, 0);

	SetAsyncTimerAttribute (timer, ASYNC_ATTR_ENABLED, 1);
	SetAsyncTimerAttribute (timer, ASYNC_ATTR_INTERVAL, 600.0);

	start = Timer();
	printf("started\n");
	
	RunUserInterface();

	getchar();

	return 0;
}
