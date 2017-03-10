#include "stage\stage.h"
#include "RS232CorvusXY.h"
#include "RS232CorvusXY_UserInterface.h" 

int CVICALLBACK OnParamsLoad (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			XYStage* stage = (XYStage *) callbackData;
			char path[MAX_PATHNAME_LEN] = "", file[MAX_FILENAME_LEN];

		    sprintf(path, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(stage), UIMODULE_GET_NAME(stage), DEFAULT_STAGE_FILENAME_SUFFIX);
		    sprintf(file, "%s_%s", UIMODULE_GET_NAME(stage), DEFAULT_STAGE_FILENAME_SUFFIX);

			if (FileSelectPopup (path, file, "*.ini", "Load",
							 	 VAL_LOAD_BUTTON, 1, 1, 1, 0, path) != 1) return 0;
	
			if (!FileExists(path, NULL))
                return 0;

            stage_load_settings(stage, path);

			}break;
		}
	return 0;
}

int CVICALLBACK OnParamsSave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			XYStage* stage = (XYStage *) callbackData;
			int ret;

			ret = stage_save_settings_as_default(stage);

			if (STAGE_SUCCESS==ret) {
				GCIDialog(0, "Configuration Saved", IDI_INFORMATION, GCI_OK_BUTTON, "Configuration saved successfully.");
			}
			else {
				GCIDialog(0, "Configuration Save Error", IDI_EXCLAMATION, GCI_OK_BUTTON, "Configuration FAILED to save.");
			}
		}
		break;

	}
	return 0;
}

int CVICALLBACK OnResetSafeRegion (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			XYStage* stage = (XYStage *) callbackData;
	
			stage_set_safe_region_rectangle_percentage_smaller_than_limits(stage, 1.0);

			}break;
		}
	return 0;
}

int CVICALLBACK OnToggleClosedLoop (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			XYStage* stage = (XYStage *) callbackData;
	
			if (stage->_closed_loop_enabled)
				stage->_closed_loop_enabled = 0;
			else
				stage->_closed_loop_enabled = 1;
			
			corvus_rs232_xy_send_closed_loop_params (stage);

			}break;
		}
	return 0;
}

int CVICALLBACK OnParamsClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage= (XYStage *) callbackData;

			
			stage_hide_params_ui (stage);

			}break;
		}
	return 0;
}

int CVICALLBACK OnPitch (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			XYStage* stage= (XYStage *) callbackData;
			double pitch;
	
			//We make the X and Y pitch values the same
			GetCtrlVal(panel, control, &pitch);
	
		//	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_X_PITCH, stage->_pitch[XAXIS]);
		//	SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_Y_PITCH, stage->_pitch[YAXIS]);
			
			stage_set_pitch(stage, XAXIS, pitch);
			stage_set_pitch(stage, YAXIS, pitch);
			
			SetCtrlAttribute (stage->_params_ui_panel, XY_PARAMS_X_SPEED, ATTR_MAX_VALUE, 45.0*pitch);
			SetCtrlAttribute (stage->_params_ui_panel, XY_PARAMS_Y_SPEED, ATTR_MAX_VALUE, 45.0*pitch);

			}break;
		}
	
	return 0;
}

int CVICALLBACK OnSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage= (XYStage *) callbackData;
			double speed;
	
			
			//We make the X and Y speed values the same
			GetCtrlVal(panel, control, &speed);
			SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_X_SPEED, speed);
			SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_Y_SPEED, speed);
			
			stage_set_speed(stage, ALL_AXIS, speed);
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnAcceleration (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage= (XYStage *) callbackData;
			double acceleration;
	
			//We make the X and Y acceleration values the same
			GetCtrlVal(panel, control, &acceleration);

			SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_X_ACC, acceleration);
			SetCtrlVal(stage->_params_ui_panel, XY_PARAMS_Y_ACC, acceleration);
			
			stage_set_acceleration(stage, ALL_AXIS, acceleration);
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnXenabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage= (XYStage *) callbackData;

			GetCtrlVal(stage->_params_ui_panel, XY_PARAMS_X_ENABLED, &stage->_enabled_axis[XAXIS]);
			
			if (corvus_rs232_set_axis_enable(stage, XAXIS, stage->_enabled_axis[XAXIS])==STAGE_ERROR) return 0;
			
			}break;
		}
	
	return 0;
}

int CVICALLBACK OnYenabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage = (XYStage *) callbackData;

			GetCtrlVal(stage->_params_ui_panel, XY_PARAMS_Y_ENABLED, &stage->_enabled_axis[YAXIS]);
			
			if (corvus_rs232_set_axis_enable(stage, YAXIS, stage->_enabled_axis[YAXIS])==STAGE_ERROR) return 0;
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnXbacklash (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage = (XYStage *) callbackData;
			CorvusXYStage *this = (CorvusXYStage *) stage;  

			GetCtrlVal(stage->_params_ui_panel, XY_PARAMS_BACKLASH_X, &this->_backlash[XAXIS]);
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnYbacklash (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage = (XYStage *) callbackData;
			CorvusXYStage *this = (CorvusXYStage *) stage;  

			GetCtrlVal(stage->_params_ui_panel, XY_PARAMS_BACKLASH_Y, &this->_backlash[YAXIS]);
			
			}break;
		}
	return 0;
}
