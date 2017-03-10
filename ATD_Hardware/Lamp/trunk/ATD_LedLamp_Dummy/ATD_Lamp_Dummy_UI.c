#include <userint.h>

#include "lamp.h"
#include "ATD_Lamp_Dummy_UI.h"

#include "gci_utils.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Brightness control.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK OnLampOnOffToggle (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			Lamp *lamp = (Lamp *) callbackData;
			int status;
	
			GetCtrlVal (lamp->_main_ui_panel, LAMP_PNL_ONOFF, &status);
			if (status == 1)
				lamp_on(lamp);
			else
				lamp_off(lamp);
			}break;
		}
	return 0;
}

int CVICALLBACK OnLampIntensity (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Lamp *lamp = (Lamp *) callbackData;
			double intensity;
	
			GetCtrlVal (lamp->_main_ui_panel, LAMP_PNL_INTENSITY, &intensity);

			lamp_set_intensity(lamp, intensity);
			
			break;		
		}
	}
	
	return 0;
}

int CVICALLBACK OnLampClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			Lamp *lamp = (Lamp*) callbackData;
	
			lamp_hide_main_ui(lamp);
			//GCI_Signal_Emit(&(lamp->signal_table), "Hide", GCI_VOID_POINTER, lamp);
	
			}break;
	}
	
	return 0;
}
