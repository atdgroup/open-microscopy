#include <userint.h>
#include "gci_twain_camera_ui.h"
#include "gci_twain_camera.h"  

int CVICALLBACK TwainCameraOnClosePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK TwainCameraOnCapturePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			GciCamera *camera = (GciCamera *) callbackData;
			
			gci_camera_snap_image(camera);
			
			gci_camera_show_window(camera);   
			
			break;
		}
	}
	
	return 0;
}
