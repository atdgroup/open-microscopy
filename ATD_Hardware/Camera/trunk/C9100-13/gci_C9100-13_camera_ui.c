#include <userint.h>
#include "gci_C9100-13_camera.h"
#include "gci_C9100-13_camera_ui.h"


int CVICALLBACK C9100_13_Camera_onSetSizePosition (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, auto_centre, left, top, width, height;
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) callbackData;
	GciCamera *camera = (GciCamera *) C9100_13_camera;
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, &auto_centre); 
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT,   &left);
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP,    &top);
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH,  &width);
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, &height);
	
			gci_camera_set_size_position(camera, left, top, width, height, auto_centre);

			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
	return 0;
}


int CVICALLBACK C9100_13_Camera_onPresetSubWindow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, left, top, width, height, auto_centre;
	char val[256];
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) callbackData;
	GciCamera *camera = (GciCamera *) C9100_13_camera;

	switch (event)
		{
		case EVENT_COMMIT:
		
			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);
		
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, &auto_centre);
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT,   &left);
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP,    &top);
			
			GetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_PRSETWINDOW, val);
			
			sscanf(val, "%d %d", &width, &height);
			
			if (width > 0 && height > 0)
				gci_camera_set_size_position(camera, left, top, width, height, auto_centre);
				
			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}
				
			break;
		}
	return 0;
}


int CVICALLBACK C9100_13_Camera_onBinning (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live;
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) callbackData;
	GciCamera *camera = (GciCamera *) C9100_13_camera;
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BINNING, &(C9100_13_camera->_binning) );
			gci_camera_set_binning_mode(camera, C9100_13_camera->_binning);

			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
	return 0;
}
 		

int CVICALLBACK C9100_13_Camera_onDataMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live;
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) callbackData; 
	GciCamera *camera = (GciCamera *) C9100_13_camera; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, &(camera->_data_mode) );
			gci_camera_set_data_mode(camera, camera->_data_mode);

			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
	return 0;
}

int CVICALLBACK C9100_13_Camera_onExtrasQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	GciCamera *camera = (GciCamera *) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			gci_camera_hide_extra_ui(camera);

			break;
		}
	return 0;
}

int CVICALLBACK C9100_13_Camera_SetPhotonMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	
	GciCamera *camera = (GciCamera *) callbackData;  
	
	switch (event)
	{
		case EVENT_VAL_CHANGED:

			GetCtrlVal(panel, control, &val);
			
			gci_C9100_13_camera_set_photon_mode(camera, val);
			
			break;
	}
	return 0;
}

int CVICALLBACK C9100_13_Camera_TimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			double temp;
			GciCamera *camera = (GciCamera *) callbackData;    
			
			temp = gci_C9100_13_camera_get_temperature(camera);       
				
			SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_DEGS_C, temp);            
			
			break;
		}
	}
	
	return 0;
}
