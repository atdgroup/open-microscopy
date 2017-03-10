#include <userint.h>
#include "ATD_LedLighting_A.h"
#include "ATD_LedLightingUI_A.h"

int CVICALLBACK OnLedLightingStateChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            int mode;
   
			GetCtrlVal(panel, control, &mode);
			
			if(mode == 1)
				lamp_on(lamp);	
			else
				lamp_off(lamp);	

            break;
		}
	}
    
	return 0;
}

int CVICALLBACK OnLedLightingQuit (int panel, int control, int event,
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