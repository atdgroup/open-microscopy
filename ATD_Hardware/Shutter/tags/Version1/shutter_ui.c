#include <utility.h>
#include <userint.h>

#include "hardware.h"
#include "shutter.h"
#include "shutter_ui.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Shutter control.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK cbShutterOpenButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Shutter *shutter = (Shutter *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			shutter_open(shutter);
			break;
		}
	return 0;
}

int CVICALLBACK cbShutterCloseButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Shutter *shutter = (Shutter *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			shutter_close(shutter);
			break;
		}
	return 0;
}

int CVICALLBACK cbShutterTriggerButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Shutter *shutter = (Shutter *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			shutter_trigger(shutter);
			break;
		}
	return 0;
}

int CVICALLBACK cbShutterOpenTime (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Shutter *shutter = (Shutter *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (shutter->_main_ui_panel, SHUTTER_OPEN_TIME, &shutter->_open_time);  //ms
			shutter_set_open_time(shutter, shutter->_open_time);
			break;
		}
	return 0;
}

int CVICALLBACK cbShutterClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Shutter *shutter = (Shutter *) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
			shutter_hide_main_ui(shutter);
			//GCI_Signal_Emit(&(shutter->signal_table), "Hide", GCI_VOID_POINTER, shutter);
	
			break;
	}
	
	return 0;
}

int CVICALLBACK cbShutterTest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Shutter *shutter = (Shutter *) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:

			shutter_open(shutter);
			//Delay(0.01);
			shutter_close(shutter);
			break;
	}
	return 0;
}
