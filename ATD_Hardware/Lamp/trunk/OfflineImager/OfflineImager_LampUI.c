#include <userint.h>
#include "OfflineImager_Lamp.h"
#include "OfflineImager_LampUI.h"

////////////////////////////////////////////////////////////////////////////
//RJL/RGN April 2007
//GCI HTS Microscope system. 
//LED sequence control - ui callbacks
////////////////////////////////////////////////////////////////////////////

//Settings panel controls

int CVICALLBACK offline_imager_lamp_closesettingpnl (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
            Lamp *lamp = (Lamp *) callbackData;
            
			offline_imager_lamp_hide_settings_ui(lamp);
            
			break;
        }
	}
    
	return 0;
}


int CVICALLBACK offline_imager_lamp_intensity (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
        {
            Lamp *lamp = (Lamp *) callbackData;
            double intensity;
            
            GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), OLLAMP_PNL_INTENSITY, &intensity);
			lamp_set_intensity (lamp, intensity);
            
            break;
		}
	}
	return 0;
}

int CVICALLBACK offline_imager_lamp_ledmode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            
            int mode;
  
			GetCtrlVal(panel, control, &mode);
				 
			if(mode == OFFLINE_IMAGER_LAMP_OFF)
				lamp_off(lamp);
			else
				lamp_on(lamp);

            break;
		}
	}
    
	return 0;
}

int CVICALLBACK offline_imager_lamp_loadsettingpanel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;

			offline_imager_lamp_display_settings_ui(lamp);
            
            break;
		}
	}
    
	return 0;
}


int CVICALLBACK offline_imager_lamp_quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
			lamp_hide_main_ui(lamp);
            
            break;
		}
	}
    
	return 0;
}

int CVICALLBACK offline_imager_lamp_ledenable (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            int val = 0;
            
            GetCtrlVal(panel, control, &val);
            
            if(val)
                offline_imager_lamp_enable (lamp);
            else
                offline_imager_lamp_disable (lamp);
               
            break;
		}
	}
    
	return 0;
}