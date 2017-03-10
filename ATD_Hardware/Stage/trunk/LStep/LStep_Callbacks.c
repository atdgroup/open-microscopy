#include "stage\stage.h"
#include "LStep\LStepXY.h"
#include "LStep\LStep_UserInterface.h" 

int CVICALLBACK OnLStepParamsLoad (int panel, int control, int event,
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

int CVICALLBACK OnLStepParamsSave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
        
			XYStage* stage = (XYStage *) callbackData;
			int ret;

			ret = stage_save_settings_as_default(stage);
            
			if (STAGE_SUCCESS==ret) {
				GCIDialog(0, "Configuration Saved", IDI_INFORMATION, GCI_OK_BUTTON, "Configuration saved successfully.");
			}
			else {
				GCIDialog(0, "Configuration Save Error", IDI_EXCLAMATION, GCI_OK_BUTTON, "Configuration FAILED to save.");
			}

			}break;
		}
	return 0;
}


int CVICALLBACK OnLStepParamsClose (int panel, int control, int event,
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

int CVICALLBACK OnLStepPitch (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			XYStage* stage= (XYStage *) callbackData;
			double pitch;
	
			//We make the X and Y pitch values the same
			GetCtrlVal(panel, control, &pitch);
	
		//	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_X_PITCH, stage->_pitch[XAXIS]);
		//	SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_Y_PITCH, stage->_pitch[YAXIS]);
			
			stage_set_pitch(stage, XAXIS, pitch);
			stage_set_pitch(stage, YAXIS, pitch);
			
			SetCtrlAttribute (stage->_params_ui_panel, LS_PARAMS_X_SPEED, ATTR_MAX_VALUE, 45.0*pitch);
			SetCtrlAttribute (stage->_params_ui_panel, LS_PARAMS_Y_SPEED, ATTR_MAX_VALUE, 45.0*pitch);

			}break;
		}
	
	return 0;
}

int CVICALLBACK OnLStepSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage= (XYStage *) callbackData;
			double speed;
	
			
			//We make the X and Y speed values the same
			GetCtrlVal(panel, control, &speed);
			SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_X_SPEED, speed);
			SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_Y_SPEED, speed);
			
			stage_set_speed(stage, ALL_AXIS, speed);
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnLStepAcceleration (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage= (XYStage *) callbackData;
			double acceleration;
	
			//We make the X and Y acceleration values the same
			GetCtrlVal(panel, control, &acceleration);

			SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_X_ACC, acceleration);
			SetCtrlVal(stage->_params_ui_panel, LS_PARAMS_Y_ACC, acceleration);
			
			stage_set_acceleration(stage, ALL_AXIS, acceleration);
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnLStepXenabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage= (XYStage *) callbackData;

			GetCtrlVal(stage->_params_ui_panel, LS_PARAMS_X_ENABLED, &stage->_enabled_axis[XAXIS]);
			lstep_set_axis_enable(stage, XAXIS, stage->_enabled_axis[XAXIS]);
		//	if (corvus_rs232_set_axis_enable(stage, XAXIS, stage->_enabled_axis[XAXIS])==STAGE_ERROR) return 0;
			
			}break;
		}
	
	return 0;
}

int CVICALLBACK OnLStepYenabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage = (XYStage *) callbackData;

			GetCtrlVal(stage->_params_ui_panel, LS_PARAMS_Y_ENABLED, &stage->_enabled_axis[YAXIS]);
			lstep_set_axis_enable(stage, YAXIS, stage->_enabled_axis[YAXIS]);
	//		if (corvus_rs232_set_axis_enable(stage, YAXIS, stage->_enabled_axis[YAXIS])==STAGE_ERROR) return 0;
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnLStepXbacklash (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage = (XYStage *) callbackData;
			LStepXYStage *this = (LStepXYStage *) stage;  

			GetCtrlVal(stage->_params_ui_panel, LS_PARAMS_BACKLASH_X, &this->_backlash[XAXIS]);
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnLStepYbacklash (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			XYStage* stage = (XYStage *) callbackData;
			LStepXYStage *this = (LStepXYStage *) stage;  

			GetCtrlVal(stage->_params_ui_panel, LS_PARAMS_BACKLASH_Y, &this->_backlash[YAXIS]);
			
			}break;
		}
	return 0;
}
