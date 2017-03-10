#include "HardWareTypes.h"

#include "camera\gci_camera.h"
#include "uir_files\gci_camera_ui.h"
#include "string_utils.h"
#include "gci_utils.h"

#include "FreeImage.h"
#include "FreeImageAlgorithms_IO.h"

#include <userint.h>

#ifdef THREADED_CAM_AQ    
#include "asynctmr.h" 
#endif

#include <utility.h>

extern char uir_file_path[GCI_MAX_PATHNAME_LEN];     

int CVICALLBACK Camera_onSnap (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			static int call_in_progress = 0;
			GciCamera *camera = (GciCamera*) callbackData;

			if(call_in_progress == 1)
				return 0;

			call_in_progress = 1;

			SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_SNAP, ATTR_DIMMED, 1);
			SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_LIVE, ATTR_DIMMED, 1);
			ProcessDrawEvents();

			gci_camera_snap_image(camera);
			gci_camera_show_window(camera);

			SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_SNAP, ATTR_DIMMED, 0);
			SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_LIVE, ATTR_DIMMED, 0);
			ProcessDrawEvents();

			//Delay(0.1);
			call_in_progress = 0;

			break;
		}
	}
		
	return 0;
}


int CVICALLBACK Camera_onLive (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
			
			GciCamera *camera = (GciCamera*) callbackData;
			double exposure;
	
			// This is milli seconds from now on
			GetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, &exposure);
			
			if (exposure > 2000) {
				SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
				if (!ConfirmPopup("Warning", "Long exposure - are you sure?"))
					break;
			}
			
			SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_SNAP, ATTR_DIMMED, 0);
			SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_LIVE, ATTR_DIMMED, 1); 
			ProcessSystemEvents();

			gci_camera_set_live_mode(camera);
			gci_camera_activate_live_display(camera);
			gci_camera_show_window(camera);
				
			}break;
	}
		
	return 0;
}


int CVICALLBACK Camera_onGain (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			GciCamera *camera = (GciCamera*) callbackData;
			int live;
			double gain;
	
		
			live = gci_camera_is_live_mode(camera);
    
			GetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN, &gain);
			
			if (camera->_gain_data_type_precision==0) 
			{   // if gain is an integer for this camera, snap to integer
				gain = RoundRealToNearestInteger(gain);
				SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN, gain);
			}
			
			gci_camera_set_gain(camera, CAMERA_ALL_CHANNELS, gain);
	
			if (!live) 
  				gci_camera_snap_image(camera);

			}break;
		}
	return 0;
}


int CVICALLBACK Camera_onExposure (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
		
			GciCamera *camera = (GciCamera*) callbackData;
			int live;
			double exposure;

			GetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, &exposure);
			
			live = gci_camera_is_live_mode(camera);

			gci_camera_set_exposure_time(camera, exposure);
			
			if (!live)
  				gci_camera_snap_image(camera);

			}break;
		}
		
	return 0;
}


int CVICALLBACK Camera_onAutoExposure (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			GciCamera *camera = (GciCamera*) callbackData;
			int live;
			double exposure;

		
			GetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, &exposure);
			
			live = gci_camera_is_live_mode(camera);
		
			gci_camera_autoexposure(camera, 1, 4000);

			if (!live)
  				gci_camera_snap_image(camera);

			}break;
		}
		
	return 0;
}

	
int CVICALLBACK Camera_onReinit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			int live;
			GciCamera *camera = (GciCamera*) callbackData;

			logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING,
				"Re-initialising, please wait...");

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			gci_camera_hide_ui (camera);
			gci_camera_power_down (camera); 
			gci_camera_power_up (camera);  
			gci_camera_initialise(camera);
			gci_camera_display_main_ui (camera);
			
			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL,
				"Re-initialisation complete.");

			}break;
		}
	return 0;
}


int CVICALLBACK Camera_onAverage (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			GciCamera *camera = (GciCamera*) callbackData;
			int frames;
			FIBITMAP *dib;
	
		
			GetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_AVERAGENUMBER, &frames);
			
			dib = gci_camera_get_image_average_for_frames(camera, frames);
			
			gci_camera_display_image_advanced(camera, dib, NULL, frames);
		
			FreeImage_Unload(dib);

			break;
		}
	}
		
	return 0;
}



int CVICALLBACK Camera_onSaveSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			char path[GCI_MAX_PATHNAME_LEN], path_name[GCI_MAX_PATHNAME_LEN];
			GciCamera *camera = (GciCamera*) callbackData;  

			sprintf(path, "%s\\", UIMODULE_GET_DATA_DIR(camera));

			if (FileSelectPopup (path, "*.ini", "",
								 "Save Camera Settings", VAL_SAVE_BUTTON, 0, 1, 1, 1, path_name) <= 0)
				return -1;
				
			gci_camera_save_settings(camera, path_name, "w"); 
			
			}break;
		}

	return 0;
}


int CVICALLBACK Camera_onLoadSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
		 	GciCamera *camera = (GciCamera*) callbackData;
	
			char path[GCI_MAX_PATHNAME_LEN], fname[GCI_MAX_PATHNAME_LEN];

			sprintf(path, "%s\\", UIMODULE_GET_DATA_DIR(camera));

			if (FileSelectPopup (path, "*.ini", "",
								 "Load Camera Settings", VAL_LOAD_BUTTON, 0, 1, 1, 0, fname ) != 1) {
								 
				return CAMERA_ERROR;
			}

			gci_camera_load_settings(camera, fname); 

			}break;
		}
		
	return 0;
}


int CVICALLBACK Camera_onSetDefaults (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
		
			GciCamera *camera = (GciCamera*) callbackData;

			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			
			if (ConfirmPopup ("Confirm Save as Defaults",
				"Are you sure?\n The these values will be used every time\n this application is started.") != 1)
				break;
				
			gci_camera_save_settings_as_default(camera);
			
			}break;
		}

	return 0;
}


int CVICALLBACK Camera_onExtras (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
		
			GciCamera *camera = (GciCamera*) callbackData;
	
			gci_camera_display_extra_ui(camera);
			
			}break;
		}
	return 0;
}


int CVICALLBACK Camera_onQuit(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		
			GciCamera *camera = (GciCamera*) callbackData;
	
			HidePanel(camera->_main_ui_panel);
			
			if(camera->_extra_ui_panel > 0)
				HidePanel(camera->_extra_ui_panel);
			
			gci_camera_deactivate_grab_timer(camera);
			
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "Close", GCI_VOID_POINTER, camera);    
			
			}break;
	}

	return 0;
}


static int CameraDisplayInfoPanel(GciCamera *camera)
{
	int aboutPnl, pnl, ctrl;
	char vendor[64] = "", model[64] = "", bus[64] = "", camID[64] = "";
	char camVersion[64] = "", driverVersion[64] = "";
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);

	if (gci_camera_get_info(camera, vendor, model, bus, camID, camVersion, driverVersion) == CAMERA_ERROR ) {
	
		send_error_text(camera, "Failed to get camera info");
		return CAMERA_ERROR;
	}
	
	aboutPnl = ui_module_add_panel(UIMODULE_CAST(camera), "gci_camera_ui.uir", INFO, 0);
	
	SetCtrlVal(aboutPnl, INFO_VENDOR, vendor);
	SetCtrlVal(aboutPnl, INFO_MODEL, model);
	SetCtrlVal(aboutPnl, INFO_BUS, bus);
	SetCtrlVal(aboutPnl, INFO_CAM_ID, camID);
	SetCtrlVal(aboutPnl, INFO_CAM_VERSION, camVersion);
	SetCtrlVal(aboutPnl, INFO_DRIVER_VERSION, driverVersion);
	InstallPopup (aboutPnl);
	
	while (1) {
		GetUserEvent (1, &pnl, &ctrl);
		if (pnl == aboutPnl) break;
	}
	
	DiscardPanel(aboutPnl);
	
	return 0;
}


int CVICALLBACK Camera_onInfo (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
		
			GciCamera *camera = (GciCamera*) callbackData;
	
			CameraDisplayInfoPanel(camera);
			}break;
		}
	return 0;
}


int CVICALLBACK Camera_onSaveImages (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			char path[GCI_MAX_PATHNAME_LEN], path_prefix[GCI_MAX_PATHNAME_LEN], path_name[GCI_MAX_PATHNAME_LEN];
			GciCamera *camera = (GciCamera*) callbackData;  
			int frames, i;
			FIBITMAP **sequence;

			GetPrivateDataFolder("Microscope Data", path);
			
			if (FileSelectPopup (path, "*.ics", "", "Save Images Prefix",
				VAL_SAVE_BUTTON, 0, 0, 1, 1, path_name) <= 0)
				return -1;
						
			get_file_without_extension(path_name, path_prefix);              
			
			GetCtrlVal(panel, CAMERA_PNL_NO_OF_FRAMES_TO_SAVE, &frames);
			
			sequence = gci_camera_get_images(camera, frames);

			for(i=0; i < frames; i++) {
				
				sprintf(path_name, "%s_%d.ics", path_prefix, i);
		
				//FIA_SaveFIBToFile (sequence[i], path_name, BIT8);

				FreeImageIcs_SaveImage(sequence[i], path_name, 1);

				GCI_ImagingWindow_SaveMetaDataToIcsFilePath(camera->_camera_window, path_name);

				FreeImage_Unload(sequence[i]);
			}
			
			free(sequence);

			}break;
		}

	return 0;
}


int CVICALLBACK Camera_onAverageFramesChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			GciCamera *camera = (GciCamera*) callbackData;
			int frames;

			GetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_AVERAGENUMBER, &frames);

			gci_camera_set_average_frame_number(camera, frames);

			}break;
		}
	return 0;
}
