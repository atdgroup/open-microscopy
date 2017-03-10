#include <userint.h>
#include "gci_C9100_camera.h"
#include "gci_C9100_camera_ui.h"
#include "gci_orca_camera_lowlevel.h" 

int CVICALLBACK C9100_Camera_onSetSizePosition (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	
	return Orca_Camera_onSetSizePosition (panel, control, event, callbackData, eventData1, eventData2);
}

int CVICALLBACK C9100_Camera_onPresetSubWindow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	return Orca_Camera_onPresetSubWindow (panel, control, event, callbackData, eventData1, eventData2);
}


int CVICALLBACK C9100_Camera_onBinning (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	return Orca_Camera_onBinning (panel, control, event, callbackData, eventData1, eventData2);
}
 		

int CVICALLBACK C9100_Camera_onDataMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	return Orca_Camera_onDataMode (panel, control, event, callbackData, eventData1, eventData2);
}

int CVICALLBACK C9100_Camera_onLightMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	return Orca_Camera_onLightMode (panel, control, event, callbackData, eventData1, eventData2);
}


int CVICALLBACK C9100_Camera_onBlackLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	return Orca_Camera_onBlackLevel (panel, control, event, callbackData, eventData1, eventData2); 
}

int CVICALLBACK C9100_Camera_onExtrasQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	return Orca_Camera_onExtrasQuit (panel, control, event, callbackData, eventData1, eventData2);
}
