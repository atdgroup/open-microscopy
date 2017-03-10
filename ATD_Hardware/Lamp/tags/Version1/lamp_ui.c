#include <userint.h>

#include "hardware.h"
#include "lamp.h"
#include "lamp_ui.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Brightness control.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK OnLampOnOffToggle (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Lamp *lamp = (Lamp *) callbackData;
	int status;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (lamp->_main_ui_panel, LAMP_PNL_ONOFF, &status);
			if (status == 1)
				lamp_on(lamp);
			else
				lamp_off(lamp);
			break;
		}
	return 0;
}

int CVICALLBACK OnLampIntensity (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Lamp *lamp = (Lamp *) callbackData;
	double intensity;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (lamp->_main_ui_panel, LAMP_PNL_INTENSITY, &intensity);
			intensity = max(intensity, lamp->_min_intensity);	//get error messages if voltage sent is less than 1
			lamp_set_intensity(lamp, intensity);
			break;
		}
	return 0;
}

int CVICALLBACK OnLampClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Lamp *lamp = (Lamp*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
			lamp_hide_main_ui(lamp);
			//GCI_Signal_Emit(&(lamp->signal_table), "Hide", GCI_VOID_POINTER, lamp);
	
			break;
	}
	
	return 0;
}
