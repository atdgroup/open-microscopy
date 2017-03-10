#include "gci_upix_uc3010_camera.h"
#include "gci_upix_uc3010_camera_ui.h"

int CVICALLBACK UPix3010OnResolutionChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Upix3010Camera *upix_camera = (Upix3010Camera*) callbackData;
	GciCamera *camera = (GciCamera*) upix_camera;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int res;

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_RES, &res);
			upix_set_resolution (upix_camera, res);
			gci_camera_snap_image(camera);

			break;
		}
	}

	return 0;
}

int CVICALLBACK UPix3010OnColourChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Upix3010Camera *upix_camera = (Upix3010Camera*) callbackData;
	GciCamera *camera = (GciCamera*) upix_camera;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int r, g, b;

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_RED_GAIN, &r);
			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_GREEN_GAIN, &g);
			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLUE_GAIN, &b);
			
			set_colour_gain(upix_camera, r, g, b);

			gci_camera_snap_image(camera);

			break;
		}
	}

	return 0;
}

int CVICALLBACK UPix3010OnOnOnePushWhiteBalancePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Upix3010Camera *upix_camera = (Upix3010Camera*) callbackData;
	GciCamera *camera = (GciCamera*) upix_camera;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int r,g,b;

			set_one_time_auto_white_balance(upix_camera);

			get_colour_gain(upix_camera, &r, &g, &b);

			SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_RED_GAIN, r);
			SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_GREEN_GAIN, g);
			SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLUE_GAIN, b);	

			gci_camera_snap_image(camera);

			break;
		}
	}

	return 0;
}

int CVICALLBACK UPix3010OnOnSensorModeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Upix3010Camera *upix_camera = (Upix3010Camera*) callbackData;
	GciCamera *camera = (GciCamera*) upix_camera;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int mode;

			GetCtrlVal(panel, control, &mode);

			upix_set_sensor_mode(upix_camera, mode);

			gci_camera_snap_image(camera);

			break;
		}
	}

	return 0;
}

int CVICALLBACK UPix3010OnColourEnhancementChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Upix3010Camera *upix_camera = (Upix3010Camera*) callbackData;
	GciCamera *camera = (GciCamera*) upix_camera;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int checkbox;

			GetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_RES, &checkbox);

			set_colour_enhancement(upix_camera, checkbox);
	
			gci_camera_snap_image(camera);

			break;
		}
	}

	return 0;
}

int CVICALLBACK UPix3010OnExtraPanelClose (int panel, int control, int event,
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
