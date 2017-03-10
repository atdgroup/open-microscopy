#include <userint.h>

#include "Microscope.h"
#include "MicroscopeCommon.h"
#include "MicroscopeUI.h"

#include "OpticalPath.h"
#include "CubeSlider.h"
#include "Lamp.h"

#include "Config.h"

int CVICALLBACK OnMicroscopeClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			
			Microscope *microscope = (Microscope*) callbackData;
	
			microscope_hide_main_ui(microscope);
	
			}break;
	}
	
	return 0;
}


void CVICALLBACK OnMicroscopeMenuHelpAbout(int menubar, int menuItem, void *callbackData, 
										   int panel)
{
	Microscope *microscope = (Microscope*) callbackData;
	char buffer[5000], version_file_path[256];
	FILE *fp;
	int end;
	
	find_resource("version.txt", version_file_path);   
		
	if((fp = fopen (version_file_path, "r")) == NULL)
			return;
		 
	end = fread(buffer, sizeof(char), 5000, fp);;

	fclose(fp);
	
	buffer[end]=0;

	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	GCI_MessagePopup("About", buffer);

	return;
}

void CVICALLBACK OnMicroscopeMenuHelpHelp(int menubar, int menuItem, void *callbackData, 
										   int panel)
{
	Microscope *microscope = (Microscope*) callbackData;
	
	#ifdef RESOURCES_DIRECTORY
	ShowHtmlHelp (RESOURCES_DIRECTORY "\\Help\\Microscopy.chm", HH_DISPLAY_TOC, NULL);
	#endif

	return;
}

void CVICALLBACK OnMicroscopeMenuItemLogClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	UIModule * mod = UIMODULE_CAST(callbackData);	
	
	logger_show(UIMODULE_LOGGER(mod));
}

void CVICALLBACK OnMicroscopeMenuItemClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	UIModule * mod = UIMODULE_CAST(callbackData);	
	
	if(mod != NULL)
		ui_module_display_main_panel(mod); 
}

#ifdef MICROSCOPE_OBJ
int CVICALLBACK OnMicroscopeSetObjective (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int turretPos;
			Microscope* microscope = (Microscope*) callbackData;
	
			GetCtrlVal (microscope->_main_ui_panel, MICROSCOPE_OBJ, &turretPos);
			
			if(objective_manager_move_to_position(microscope->_objective, turretPos) == OBJECTIVE_MANAGER_ERROR)
				return -1; 
			
			// This snaps a new image 
			microscope_update_display(microscope); 
  	
			break; 
		}
	}
	
	return 0;
}
#endif

void CVICALLBACK OnMicroscopeRealTimeOverviewClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	Microscope *ms = (Microscope *) callbackData;	
	int was_checked = 0;

	GetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, &was_checked);

	if(was_checked == 1) {
		realtime_overview_deactivate(ms->_rto);
		realtime_overview_hide(ms->_rto); 
		SetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, 0);
	}
	else {
		realtime_overview_set_image_max_scale_value(ms->_rto, pow(2.0, gci_camera_get_data_mode(MICROSCOPE_MASTER_CAMERA(ms))));
		realtime_overview_init(ms->_rto);
		realtime_overview_activate(ms->_rto);
		realtime_overview_display(ms->_rto);
		SetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, 1);
	}
}

void CVICALLBACK OnMicroscopeMenuItemAutoSnapClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	Microscope *ms = (Microscope *) callbackData;	
	int was_checked = 0;

	GetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, &was_checked);

	if(was_checked == 1) {
		ms->_autosnap = 0;
		SetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, 0);
	}
	else {
		ms->_autosnap = 1;
		SetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, 1);
	}
}

void CVICALLBACK OnMicroscopeMenuItemReInitialiseDevicesClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	Microscope *ms = (Microscope *) callbackData;	

	if(!GCI_ConfirmPopup("Reinitialise", IDI_WARNING, "Are you sure you wish to reinitialise all devices"))
		return;

	microscope_initilise_devices(ms);

	/*
	int i, number_of_devices = hardware_device_number_of_devices();

	HardwareDevice* device = NULL;

	for(i=1; i <= number_of_devices; i++) { 
	
		device = hardware_device_at_index(i);

		if(hardware_device_is_manual(device))
			continue;

		hardware_device_hardware_initialise (device);
	}
	*/

}

void CVICALLBACK OnMicroscopeResetToDefaultSettingsClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	Microscope *microscope = (Microscope*) callbackData;  
	MicroscopeIlluminationMode mode;
	
	microscope_reset_all_mode_settings_to_default(microscope);	
	mode = microscope->illumination_mode;
	microscope->illumination_mode = MICROSCOPE_UNDEFINED; // to force the mode to be reset
	microscope_switch_illumination_mode (microscope, mode, 0);
}

void CVICALLBACK OnMicroscopeSaveSettingsAsClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	Microscope *microscope = (Microscope*) callbackData;  
	char path[GCI_MAX_PATHNAME_LEN], default_path[GCI_MAX_PATHNAME_LEN];
	char *default_extensions = "*.ini;";

	path[0] = '\0';
	default_path[0] = '\0';

	if (LessLameFileSelectPopup (microscope->_main_ui_panel, UIMODULE_GET_DATA_DIR(microscope), "*.ini",
				default_extensions, "Save Settings", VAL_SAVE_BUTTON, 0, 0, 1, 1, path) <= 0)
				return;
			
	microscope_save_settings(microscope, path); 
}

void CVICALLBACK OnMicroscopeLoadSettingsFromFileClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	Microscope *microscope = (Microscope*) callbackData;  
	char path[GCI_MAX_PATHNAME_LEN], default_path[GCI_MAX_PATHNAME_LEN];
	char *default_extensions = "*.ini;";

	path[0] = '\0';
	default_path[0] = '\0';

	if (LessLameFileSelectPopup (microscope->_main_ui_panel, UIMODULE_GET_DATA_DIR(microscope), "*.ini",
			default_extensions, "Load Settings", VAL_OK_BUTTON, 0, 1, 1, 0, path) <= 0)
			return;
			
	microscope_load_settings(microscope, path); 
}

void CVICALLBACK OnMicroscopeSaveSettingsAsDefaultForModeClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	Microscope *microscope = (Microscope*) callbackData;  
	char path[GCI_MAX_PATHNAME_LEN];

	path[0] = '\0';

	microscope_get_filepath_for_default_illumination_settings(microscope, microscope->illumination_mode, path);

	if(GCI_ConfirmPopup("Warning", IDI_WARNING,
		"Are you sure you wish to save these settings as the default for this mode?"))
	{
		microscope_save_settings(microscope, path); 
	}
}

int CVICALLBACK OnMicroscopeSetOpticalPath (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int turretPos;
			Microscope* microscope = (Microscope*) callbackData;
			
			if (microscope->_optical_path == NULL)
				return 0;
	
			GetCtrlVal (microscope->_main_ui_panel, MICROSCOPE_OPTICAL_PATH, &turretPos);
			
			if(optical_path_move_to_position(microscope->_optical_path, turretPos) == OPTICAL_PATH_MANAGER_ERROR)
				return -1;
			
			Delay(0.5);

			// This snaps a new image 
			microscope_update_display(microscope); 
  	
			break; 
		}
	}
	
	return 0;
}

#ifdef MICROSCOPE_JOY_ON
int CVICALLBACK OnMicroscopeJoystickEnable (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int joyOn;
			Microscope* microscope = (Microscope*) callbackData;
			
			if (microscope->_stage == NULL)
				return 0;
	
			GetCtrlVal (microscope->_main_ui_panel, MICROSCOPE_JOY_ON, &joyOn);
			
			if (joyOn)
				stage_set_joystick_off(microscope->_stage);
			else
				stage_set_joystick_on(microscope->_stage); 
			
			break; 
		}
	}
	
	return 0;
}
#endif

int CVICALLBACK OnMicroscopeGotoXYZDatum (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Microscope* microscope = (Microscope*) callbackData;  
			
			if (microscope->_stage){
				stage_goto_xy_position (microscope->_stage, microscope->_stored_stage_x, microscope->_stored_stage_y);   
				stage_set_joystick_on(microscope->_stage);
			}
			
			if (microscope->_z_drive){
				z_drive_set_position (microscope->_z_drive, microscope->_stored_focus_z);
			}	
			
			microscope_update_display(microscope);
		
		    break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnMicroscopeSetDatumXYZ (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			
			Microscope* microscope = (Microscope*) callbackData;  
			
			if (microscope->_stage == NULL)
				return 0;
			
			stage_get_xy_position (microscope->_stage, &(microscope->_stored_stage_x), &(microscope->_stored_stage_y));

			if (microscope->_z_drive == NULL)
				return 0;
	
			z_drive_get_position (microscope->_z_drive, &(microscope->_stored_focus_z));
			
			break;
		}
	}

	return 0;
}

int CVICALLBACK OnMicroscopeSetIlluminationMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int mode;
			Microscope* microscope = (Microscope*) callbackData; 
	
			GetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_MODE, &mode);

			microscope_switch_illumination_mode (microscope, mode, 1);        
			
			break;
		}
	}
	
	return 0;
}



int CVICALLBACK OnMicroscopeSetCube (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int cube;
			Microscope* microscope = (Microscope*) callbackData; 
			
			if (microscope->_fluor_cube == NULL)
				return 0;
	
			GetCtrlVal(microscope->_main_ui_panel, MICROSCOPE_CUBE, &cube);
				
			if (cube_manager_move_to_position(microscope->_fluor_cube, cube) == CUBE_MANAGER_ERROR)
				return -1;
				
			microscope_update_display(microscope);
			
			break;
		}
	}
	
	return 0;
}



int CVICALLBACK OnMicroscopeSnap (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Microscope* microscope = (Microscope*) callbackData;  
			GciCamera *camera;

			camera = microscope_get_camera(microscope);   

			if (camera==NULL)
				return 0;

			SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_SNAP, ATTR_DIMMED, 1);
			SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LIVE, ATTR_DIMMED, 1); 
			ProcessDrawEvents();
			
			gci_camera_snap_image(camera);
			gci_camera_show_window(camera);

			SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_SNAP, ATTR_DIMMED, 0);
			SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LIVE, ATTR_DIMMED, 0); 
			ProcessDrawEvents();

			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnMicroscopeLive (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Microscope* microscope = (Microscope*) callbackData;  
			GciCamera *camera;
			double exposure;
	
			camera = microscope_get_camera(microscope);

			if (camera==NULL)
				return 0;
			
			exposure = gci_camera_get_exposure_time(camera);
			
			// Milli seconds
			if (exposure > 2000) {
				SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
				if (!ConfirmPopup("Warning", "Long exposure - are you sure?"))
					break;
			}
			
			SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_SNAP, ATTR_DIMMED, 0);
			SetCtrlAttribute(microscope->_main_ui_panel, MICROSCOPE_LIVE, ATTR_DIMMED, 1); 
			ProcessDrawEvents();

			gci_camera_set_live_mode(camera);
			gci_camera_activate_live_display(camera);
			gci_camera_show_window(camera);
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnMicroscopeLEDState (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;
			Microscope* microscope = (Microscope*) callbackData; 
			
			if (microscope->_lamp == NULL)
				return 0;
			
			// Off = 1 on == 2
			GetCtrlVal(panel, control, &val);
			
			if(val == 1)
				lamp_off(microscope->_lamp);
			else if (val == 2)
				lamp_on(microscope->_lamp);  
			
			microscope_update_display (microscope);
			
			break;
		}
	}
	
	return 0;
}

#ifdef MICROSCOPE_LEDINTENSITY
int CVICALLBACK OnMicroscopeLEDIntensity (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			double intensity;
			Microscope* microscope = (Microscope*) callbackData; 
			
			GetCtrlVal(panel, MICROSCOPE_LEDINTENSITY, &intensity);    
			
			lamp_set_intensity(microscope->_lamp, intensity);
			
			microscope_update_display (microscope);
			
			break;
		}
		
	}
	
	return 0;
}
#endif

int CVICALLBACK OnMicroscopeCameraExposureChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			double val;
			Microscope* microscope = (Microscope*) callbackData; 
			
			if (MICROSCOPE_MASTER_CAMERA(microscope) == NULL)
				return 0;
			
			GetCtrlVal(panel, control, &val);
			
			gci_camera_set_exposure_time(MICROSCOPE_MASTER_CAMERA(microscope), val);
			
			microscope_update_display(microscope);
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnMicroscopeCameraGainChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			double val;
			Microscope* microscope = (Microscope*) callbackData; 
			
			if (MICROSCOPE_MASTER_CAMERA(microscope) == NULL)
				return 0;
			
			GetCtrlVal(panel, control, &val);
						
			if (MICROSCOPE_MASTER_CAMERA(microscope)->_gain_data_type_precision==0) 
			{   // if gain is an integer for this camera, snap to integer
				val = RoundRealToNearestInteger(val);
				SetCtrlVal(panel, control, val);
			}
			
			gci_camera_set_gain(MICROSCOPE_MASTER_CAMERA(microscope), CAMERA_CHANNEL1, val);
			
			microscope_update_display(microscope);
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnMicroscopeCameraBinningChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;
			Microscope* microscope = (Microscope*) callbackData; 
			
			if (MICROSCOPE_MASTER_CAMERA(microscope) == NULL)
				return 0;

			GetCtrlVal(panel, control, &val);
			
			gci_camera_set_binning_mode(MICROSCOPE_MASTER_CAMERA(microscope), (BinningMode) val);
			
			microscope_update_display(microscope);
			
			break;
		}
	}
	
	return 0;
}
