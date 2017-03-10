#include "intensilight_ui.h"
#include "intensilight.h"

#include <userint.h>
#include <utility.h>

////////////////////////////////////////////////////////////////////////////
//RJL January 2007
//Nikon Intensilight control.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK cbIntensilightClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Intensilight* intensilight = (Intensilight*) callbackData;

	switch (event)
	{
		case EVENT_COMMIT:
			ui_module_hide_all_panels(UIMODULE_CAST(intensilight));

			break;
	}

	return 0;
}

int CVICALLBACK cbIntensilightSetNdfilter (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int nd_filter;
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &nd_filter);
			if (intensilight_set_nd_filter(intensilight, nd_filter) == INTENSILIGHT_ERROR)
				break;
			if (intensilight_get_nd_filter(intensilight, &nd_filter) == INTENSILIGHT_ERROR)
				break;
				
			SetCtrlVal(panel, control, intensilight->_nd_filter);
			break;
		}
	return 0;
}

int CVICALLBACK cbIntensilightShutterOpen (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int shutter_status;
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			if (intensilight_open_shutter(intensilight) == INTENSILIGHT_ERROR)
				break;
			if (intensilight_read_shutter(intensilight, &shutter_status) == INTENSILIGHT_ERROR)
				break;
				
			SetCtrlVal(panel, I_MAIN_OPEN_LED, intensilight->_shutter_status);
		}
	return 0;
}

int CVICALLBACK cbIntensilightShutterClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int shutter_status;
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			if (intensilight_close_shutter(intensilight) == INTENSILIGHT_ERROR)
				break;
			if (intensilight_read_shutter(intensilight, &shutter_status) == INTENSILIGHT_ERROR)
				break;
				
			intensilight->_shutter_status = shutter_status;
			SetCtrlVal(panel, I_MAIN_OPEN_LED, intensilight->_shutter_status);
		}
	return 0;
}

int CVICALLBACK cbIntensilightShutterTrigger (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			if (intensilight_trigger_shutter(intensilight) == INTENSILIGHT_ERROR)
				break;
			intensilight->_shutter_status = 1;
			SetCtrlVal(panel, I_MAIN_OPEN_LED, intensilight->_shutter_status);
			while(intensilight->_shutter_status) {
				if (intensilight_read_shutter(intensilight, &intensilight->_shutter_status) == INTENSILIGHT_ERROR)
					break;
			}
			SetCtrlVal(panel, I_MAIN_OPEN_LED, intensilight->_shutter_status);
		}
	return 0;
}

int CVICALLBACK cbIntensilightShutterOpenTime (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &val);
			//Round to nearest 100 ms
			val = (val+99)/100;
			val *= 100;
			SetCtrlVal(panel, control, val);
			intensilight->_shutter_open_time = val/100;
			break;
		}
	return 0;
}

int CVICALLBACK cbIntensilightShutterTest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Intensilight* intensilight = (Intensilight*) callbackData;
	int i, delay;
	double t1;
	
	switch (event)
		{
		case EVENT_COMMIT:
			intensilight_stop_timer(intensilight);

			t1 = Timer();
			//for (i=0; i<10; i++) {
				GetCtrlVal(panel, I_MAIN_EXPOSURE, &delay);
				if (intensilight_open_shutter(intensilight) == INTENSILIGHT_ERROR)
					break;
				Delay((double)delay/1000.0);
				//Delay(0.05);
				if (intensilight_close_shutter(intensilight) == INTENSILIGHT_ERROR)
					break;
				//if (intensilight_trigger_shutter(intensilight) == INTENSILIGHT_ERROR)
				//	break;
				Delay(0.01);	
			//}
			//printf("%.2f\n", Timer()-t1);
			
			intensilight_start_timer(intensilight);
			break;
		}
	return 0;
}

int CVICALLBACK cbIntensilightRemoteEnable (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int enabled;
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &enabled);
			if (intensilight_set_remote_enable(intensilight, enabled) == INTENSILIGHT_ERROR)
				break;
			if (intensilight_get_remote_enable(intensilight, &enabled) == INTENSILIGHT_ERROR)
				break;
			
			SetCtrlVal(panel, control, intensilight->_remote_enabled);
			break;
		}
	return 0;
}

int CVICALLBACK cbIntensilightRemotebrightness (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int led;
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &led);
			if (intensilight_set_remote_led(intensilight, led) == INTENSILIGHT_ERROR)
				break;
			if (intensilight_get_remote_led(intensilight, &led) == INTENSILIGHT_ERROR)
				break;
			
			SetCtrlVal(panel, control, intensilight->_remote_led);
			break;
		}
	return 0;
}

int CVICALLBACK cb_IntensilightBounceDelay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &intensilight->_bounce_delay);
			break;
	}
	return 0;
}

int CVICALLBACK cb_IntensilightSignalDelay (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &intensilight->_signal_delay);
			break;
	}
	return 0;
}

int CVICALLBACK cb_IntensilightExposure (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int exposure, val;
	Intensilight* intensilight = (Intensilight*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:

			//Calculate how long the shutter signal must be in order for the
			//shutter to be open for the required exposure
			GetCtrlVal(panel, control, &exposure);
			if (exposure >= 40)
				val = exposure + intensilight->_signal_delay + intensilight->_bounce_delay - 40;
			else
				val = intensilight->_signal_delay + intensilight->_bounce_delay;

			SetCtrlVal(panel, I_MAIN_DELAY, val);
			
			//Round up to nearest 100 ms
			//val = ceil((double)val/100.0);
			//val *= 100;
			//SetCtrlVal(panel, I_MAIN_SHUTTER_TIME, val);
			//intensilight->_shutter_open_time = val/100;

			break;
	}
	return 0;
}

int CVICALLBACK cbIntensilightExpTest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Intensilight* intensilight = (Intensilight*) callbackData;
	int delay;
	
	switch (event)
		{
		case EVENT_COMMIT:
			intensilight_stop_timer(intensilight);

			GetCtrlVal(panel, I_MAIN_DELAY, &delay);
			if (intensilight_open_shutter(intensilight) == INTENSILIGHT_ERROR)
				break;
			Delay((double)delay/1000.0);
			if (intensilight_close_shutter(intensilight) == INTENSILIGHT_ERROR)
				break;
			Delay(0.01);	
			
			intensilight_start_timer(intensilight);
			break;
		}
	return 0;
}
