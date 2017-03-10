#include <userint.h>
#include "toolbox.h"
#include "jvc\gci_jvc_camera.h"
#include "uir_files\gci_camera_jvc_ui.h"
#include "uir_files\gci_camera_ui.h"
#include "string_utils.h"
#include "dictionary.h"
#include "iniparser.h"

#include "FreeImageAlgorithms_IO.h"

#include "shellapi.h"

#include "KYF75_CTRL.h"
#include "constkyf75.h"

#include "easytab.h"

#include "wingdi.h"
#include <utility.h>

#define IM_MAX_WIDTH 1360
#define IM_MAX_HEIGHT 1024
#define IM_MIN_WIDTH 680
#define IM_MIN_HEIGHT 384

#define DEFAULT_SETTINGS_FILE "KYF75CameraDefaultSettings.cam"

#define INVALID_VALUE -999999

static int jvc_check_param(GciJvcCamera* jvc_camera, long item, long value)
{
	long min = 0, max = 0;

	KYF75_IKYF75GetMinValue (jvc_camera->handle, &(jvc_camera->ErrorInfo), item, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, &(jvc_camera->ErrorInfo), item, &max);

	if(value < min || value > max)
		return CAMERA_ERROR;

	return CAMERA_SUCCESS;
}

#define VALIDATE_PARAM(camera, item, value) \
	if (jvc_check_param(((GciJvcCamera*)(camera)), (item), (value)) == CAMERA_ERROR) { \
		return CAMERA_ERROR; }

static void RestartIEEE1394HostController(GciJvcCamera *jvc_camera)
{
	GciCamera *camera = (GciCamera *) jvc_camera;
	int val, window_handle;
	double start_time = Timer();

	GetPanelAttribute (camera->_main_ui_panel, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);
 
	// Opening the file in the default app will cause it to be opened in the python interpreter
	// opens MyFile with MyEditor program
	//val = (int) ShellExecute((HWND) window_handle, NULL, "devcon.exe", "disable PCI\\VEN_1106*", NULL, SW_HIDE);
	val = (int) ShellExecute((HWND) window_handle, NULL, "devcon.exe", "disable PCI\\VEN_1033*", NULL, SW_HIDE);

	if(val == ERROR_FILE_NOT_FOUND)
		GCI_MessagePopup("Error", "Error Running Devcon.exe");

	while((Timer() - start_time) < 5.0) {
		ProcessSystemEvents();
		Delay(0.3);
	}

	//ShellExecute((HWND) window_handle, NULL, "devcon.exe", "enable PCI\\VEN_1106*", NULL, SW_HIDE);
	ShellExecute((HWND) window_handle, NULL, "devcon.exe", "enable PCI\\VEN_1033*", NULL, SW_HIDE);

	start_time = Timer();

	while((Timer() - start_time) < 5.0) {
		ProcessSystemEvents();
		Delay(0.3);
	}
}

static void send_jvc_error_text(GciJvcCamera *jvc_camera, HRESULT error, ERRORINFO ErrorInfo)
{
	GciCamera *camera = (GciCamera *) jvc_camera;
	int msg;
	char temp_dir[500] = "", path[500] = "";

	jvc_camera->_set_live = 0;
	
	if(strstr(ErrorInfo.description, "bus band") != NULL) {

		// HACK

		gci_camera_power_down(camera);

		// There seems to be a bug in the JVC camera where bus bandwidth errors occur.
		// We have to restart the Firewire host controller.
		// This is computer specific.
		msg = GCIDialogNoButtons (0, "Camera Error", IDI_INFORMATION, "JVC Bandwidth Errors. Restarting IEEE 1394 Host Controller.");

		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "JVC Bandwidth Errors. Restarting IEEE 1394 Host Controller");  

		RestartIEEE1394HostController(jvc_camera);

		DiscardPanel(msg);

		gci_camera_power_up(camera);
		gci_camera_initialise(camera); 

		get_temp_directory(temp_dir);
		sprintf(path, "%s\\%s", temp_dir, "camera_settings.ini");

		gci_camera_load_settings(camera, path);

		return;
	}	

	send_error_text(camera, "%s", ErrorInfo.description);
}

static unsigned long number_of_jvc_cameras = 0;


static int gci_jvc_camera_set_default_settings (GciCamera* camera)
{
	int shutter_val;
	
	GciJvcCamera* jvc_camera = (GciJvcCamera*) camera;

	gci_jvc_camera_set_shutter_mode  (jvc_camera, SHUT_STEP);
	gci_jvc_camera_get_shutter_mode  (jvc_camera, &(jvc_camera->_shutter_mode));
	
	gci_jvc_camera_get_shutter_level (jvc_camera, &shutter_val); 
	gci_jvc_camera_set_shutter_level (jvc_camera, shutter_val);  // Updates the UI
	
	gci_jvc_camera_set_gain_mode  (jvc_camera, GAIN_VGAIN);   
	gci_jvc_camera_get_gain_mode (jvc_camera, &(jvc_camera->_gain_mode));
	
	gci_jvc_camera_get_white_balance_mode (jvc_camera, &(jvc_camera->_white_balance_mode));
	gci_jvc_camera_set_white_balance_mode (jvc_camera, jvc_camera->_white_balance_mode);

	gci_jvc_camera_set_colour_matrix_mode (jvc_camera, COLOUR_MATRIX_MODE_OFF);

	gci_jvc_camera_set_iris_mode (jvc_camera, MODE_AUTO);

	gci_jvc_camera_set_detail_mode (jvc_camera, DETAIL_MODE_OFF); 
	
	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_save_settings_as_default (GciCamera* camera)
{
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			
	if (ConfirmPopup ("Confirm Save as Defaults",
		"Are you sure?\n The these values will be used every time\n this application is started.") != 1) {
		
		return CAMERA_ERROR;
	}
		
	gci_camera_save_settings(camera, DEFAULT_SETTINGS_FILE, "a");  
	
	return CAMERA_SUCCESS;
}

static int gci_jvc_camera_power_up(GciCamera* camera)
{
	char *model;
	char data_dir[500] = "";
	char temp_dir[500] = "";
	char path[500] = "";
	int exists = 0, fsize = 0;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) camera;

	gci_camera_get_data_dir(camera, data_dir); 
	sprintf(jvc_camera->settings_filepath, "%s\\JvcCameraSettings.ini", data_dir);

	jvc_camera->_last_error = KYF75_NewIKYF75 (NULL, 1, LOCALE_NEUTRAL, 0, &(jvc_camera->handle));

	//CHECK_FOR_JVC_ERROR(jvc_camera, "JVC KY-F75 NewIKYF75 failed", jvc_camera->_last_error);

	// register the license key
	jvc_camera->_last_error = KYF75_IKYF75SetLicenceString (jvc_camera->handle, &(jvc_camera->ErrorInfo), "KYF75Ctrl [8C97-CC6AC] Copyright (c) 2002 JVC");
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	// initialise the camera
	jvc_camera->_last_error = KYF75_IKYF75Initialize (jvc_camera->handle, &(jvc_camera->ErrorInfo));
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	// number of cameras
	jvc_camera->_last_error = KYF75_IKYF75GetDeviceCount (jvc_camera->handle, NULL, &number_of_jvc_cameras);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	if (number_of_jvc_cameras == 0) {
		
		MessagePopup("jvc_camera->_last_error", "No JVC 1394 cameras detected.");
		return CAMERA_ERROR;
	}
	else if (number_of_jvc_cameras > 1 ) {
	
		MessagePopup("Warning", "Several JVC 1394 cameras detected.\nSelecting device 'Zero'");
	}

	jvc_camera->_last_error = KYF75_IKYF75ReadDeviceList (jvc_camera->handle, &(jvc_camera->ErrorInfo), 0, &model);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);  

	jvc_camera->_last_error = KYF75_IKYF75SetTargetIndex (jvc_camera->handle, &(jvc_camera->ErrorInfo), 0);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	exists = FileExists(jvc_camera->settings_filepath, &fsize);

	if(exists) {
		jvc_camera->_last_error = KYF75_IKYF75LoadFile(jvc_camera->handle, &(jvc_camera->ErrorInfo), jvc_camera->settings_filepath);
	
		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	}

	jvc_camera->_last_error = KYF75_IKYF75Setlive_aspect (jvc_camera->handle, &(jvc_camera->ErrorInfo), F75_ASPECT_4x3);  
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	jvc_camera->_last_error = KYF75_IKYF75Setlive_color (jvc_camera->handle, &(jvc_camera->ErrorInfo), 0);  
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	jvc_camera->_last_error = KYF75_IKYF75Setlive_step (jvc_camera->handle, &(jvc_camera->ErrorInfo), F75_LIVE_XL2
);  
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	jvc_camera->_last_error = KYF75_IKYF75Setstill_aspect (jvc_camera->handle, &(jvc_camera->ErrorInfo), F75_ASPECT_4x3);  
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	jvc_camera->_last_error = KYF75_IKYF75Setstill_color (jvc_camera->handle, &(jvc_camera->ErrorInfo), 0);  
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	jvc_camera->_last_error = KYF75_IKYF75Setstill_step (jvc_camera->handle, &(jvc_camera->ErrorInfo), F75_STILL_XL2);  
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	jvc_camera->_last_error = KYF75_IKYF75StopIsoc (jvc_camera->handle, &jvc_camera->ErrorInfo);      
	jvc_camera->_set_live = 0;

	gci_camera_set_data_mode(camera, BPP24);

	gci_jvc_camera_get_colour_type(jvc_camera, &(jvc_camera->_colour_type));

	get_temp_directory(temp_dir);
	sprintf(path, "%s\\%s", temp_dir, "camera_settings.ini");

	gci_camera_save_settings(camera, path , "w");

  	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_power_down(GciCamera* camera)
{
	GciJvcCamera* jvc_camera = (GciJvcCamera*) camera;

	jvc_camera->_last_error =
		KYF75_IKYF75SaveFile(jvc_camera->handle, &jvc_camera->ErrorInfo, jvc_camera->settings_filepath);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	jvc_camera->_last_error = KYF75_IKYF75StopIsoc (jvc_camera->handle, &jvc_camera->ErrorInfo);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	CA_DiscardObjHandle(jvc_camera->handle);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_save_state(GciCamera* camera, CameraState *state)
{
	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_restore_state(GciCamera* camera, CameraState *state)
{
	return CAMERA_SUCCESS;
}


static int convert_jvc_camera_exposure_string_to_double(char *exposure_string, double *exposure)
{
	char *string; //, *num_string, *dom_string;
	double num, dom;

	if(!exposure_string)
		return CAMERA_ERROR;
		
	PROFILE_START("convert_jvc_camera_exposure_string_to_double");
	
	str_remove_char(exposure_string, 's');	

	if(str_contains_ch(exposure_string, '/')) {

		string = strtok(exposure_string, "/");
		sscanf(string, "%lf", &num);
	
		string = strtok (NULL, "/");
		sscanf(string, "%lf", &dom);
	
		*exposure = num / dom;
	}
	else {
	
		// No / in the string
	
		sscanf(exposure_string, "%lf", exposure);
	
	}
	
	PROFILE_STOP("convert_jvc_camera_exposure_string_to_double");            

	return CAMERA_SUCCESS;
}


static int  gci_jvc_camera_set_exposure_time(GciCamera* camera, double exposure)
{
	// Have not got a good way to set the exposure in this camera, need to translate ms to a int val depending on the mode
	// get the user to do it!
	
	//	camera->_exposure_time = exposure;
	GCI_MessagePopup("JVC Camera", "Please change the exposure on the camera to %.2f.", exposure);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_step (GciJvcCamera* jvc_camera, JVC_STEP_MODE val)
{
	GciCamera *camera = (GciCamera*) jvc_camera; 

	if(val < LIVE_L1 || val > LIVE_XL2)
		return CAMERA_ERROR;

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75Setlive_step (jvc_camera->handle, &(jvc_camera->ErrorInfo), val); 
	}
	else if(camera->_aquire_mode == SNAP) { 
	
		jvc_camera->_last_error = KYF75_IKYF75Setstill_step (jvc_camera->handle, &(jvc_camera->ErrorInfo), val); 
	}
	else {
	
		return CAMERA_ERROR;
	}
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_test_pattern_mode (GciJvcCamera* jvc_camera, JVC_TEST_PATTERN_MODE val)
{
	if(val < PATTERN_OFF || val > PATTERN_IMPULSE)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Settest_pattern_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_test_pattern_mode (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Gettest_pattern_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_test_pattern_level (GciJvcCamera* jvc_camera, int val)
{
	if(val != 0 && val != 1)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Settest_pattern_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_test_pattern_level (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Gettest_pattern_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_freeze_cancel (GciJvcCamera* jvc_camera, FREEZE_CANCEL_MODE val)
{
	if(val < CANCEL_MANUAL || val > CANCEL_OFF)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setfreeze_cancel_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	SetCtrlVal(jvc_camera->_system_panel, SYSTEM_FREEZECANCEL, val);
	
	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_memory_save (GciJvcCamera* jvc_camera)
{
	jvc_camera->_last_error = KYF75_IKYF75MemorySave (jvc_camera->handle, &(jvc_camera->ErrorInfo), 1);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_resolution (GciJvcCamera* jvc_camera, JVC_STEP_MODE val)
{
	GciCamera *camera = (GciCamera*) jvc_camera;

	if(val < LIVE_L1 || val > LIVE_XL2)
		return CAMERA_ERROR;

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75Setlive_step (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	}
	else if(camera->_aquire_mode == SNAP) {
	
		jvc_camera->_last_error = KYF75_IKYF75Setstill_step (jvc_camera->handle, &(jvc_camera->ErrorInfo), val); 
	}
	else {
	
		return CAMERA_ERROR;
	}

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	SetCtrlVal(jvc_camera->_image_panel, IMAGE_RESOLUTION, val);
	
	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_get_resolution (GciJvcCamera* jvc_camera, long *val)
{
	GciCamera *camera = (GciCamera*) jvc_camera;

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getlive_step (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	}
	else if(camera->_aquire_mode == SNAP) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getstill_step (jvc_camera->handle, &(jvc_camera->ErrorInfo), val); 
	}
	else {
	
		return CAMERA_ERROR;
	}

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_aspect (GciJvcCamera* jvc_camera, JVC_ASPECT_MODE val)
{
	GciCamera *camera = (GciCamera*) jvc_camera;

	if(val < ASPECT_4X3 || val > ASPECT_14X9)
		return CAMERA_ERROR;

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75Setlive_aspect (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	}
	else if(camera->_aquire_mode == SNAP) {
	
		jvc_camera->_last_error = KYF75_IKYF75Setstill_aspect (jvc_camera->handle, &(jvc_camera->ErrorInfo), val); 
	}
	else {
	
		return CAMERA_ERROR;
	}

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	SetCtrlVal(jvc_camera->_image_panel, IMAGE_ASPECT, val);
	
	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_get_aspect (GciJvcCamera* jvc_camera, long *val)
{
	GciCamera *camera = (GciCamera*) jvc_camera;

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getlive_aspect (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	}
	else if(camera->_aquire_mode == SNAP) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getstill_aspect (jvc_camera->handle, &(jvc_camera->ErrorInfo), val); 
	}
	else {
	
		return CAMERA_ERROR;
	}

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_colour_matrix_mode (GciJvcCamera* jvc_camera, JVC_COLOUR_MATRIX_MODE val)  
{
	int dim, min, max;
	
	if(val != COLOUR_MATRIX_MODE_ON && val != COLOUR_MATRIX_MODE_OFF)
		return CAMERA_ERROR;

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_colour_matrix_mode %d\n", val);
		fflush(gfp);
	#endif
	
	jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	SetCtrlVal(jvc_camera->_colour_matrix_panel, COLMAT_COLOURMATRIX, val);
	
	switch (val)
		{
		case COLOUR_MATRIX_MODE_ON:
		
			dim       = 0;
			
			// Get the max and min of the gain control
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL0, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL0, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL0, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL0, ATTR_MAX_VALUE, max);
			
			
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL1, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL1, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL1, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL1, ATTR_MAX_VALUE, max);
			
			
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL2, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL2, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL2, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL2, ATTR_MAX_VALUE, max);
			
			
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL3, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL3, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL3, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL3, ATTR_MAX_VALUE, max);
			
			
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL4, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL4, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL4, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL4, ATTR_MAX_VALUE, max);
			
			
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL5, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL5, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL5, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL5, ATTR_MAX_VALUE, max);
			
			
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL6, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL6, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL6, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL6, ATTR_MAX_VALUE, max);
			
			
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL7, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL7, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL7, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL7, ATTR_MAX_VALUE, max);
			
			
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL8, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_COLOR_MAT_LEVEL8, &max);
			
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL8, ATTR_MIN_VALUE, min); 
			SetCtrlAttribute(jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL8, ATTR_MAX_VALUE, max);
			
			break;
			
		case COLOUR_MATRIX_MODE_OFF:
		
			dim       = 1;
			break;
		}

	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL0,  ATTR_DIMMED, dim);
	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL1,  ATTR_DIMMED, dim);
	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL2,  ATTR_DIMMED, dim);
	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL3,  ATTR_DIMMED, dim);
	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL4,  ATTR_DIMMED, dim);
	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL5,  ATTR_DIMMED, dim);
	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL6,  ATTR_DIMMED, dim);
	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL7,  ATTR_DIMMED, dim);
	SetCtrlAttribute (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL8,  ATTR_DIMMED, dim);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_colour_matrix_mode (GciJvcCamera* jvc_camera, long *val)  
{
	jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_colour_matrix (GciJvcCamera* jvc_camera, JVC_COLOUR_MATRIX_NUMBER matrix, int level)
{
	int ctrl = 0;

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_colour_matrix, %d %d\n", matrix, level);
		fflush(gfp);
	#endif

	if(level < 0 || level > 99)
		return CAMERA_ERROR;

	switch(matrix)
	{
		case MATRIX0:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level0 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL0;

			break;
	
		case MATRIX1:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level1 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL1;

			break;
			
		case MATRIX2:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level2 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL2;

			break;
			
		case MATRIX3:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level3 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL3;

			break;
			
		case MATRIX4:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level4 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL4;

			break;
			
		case MATRIX5:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level5 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL5;

			break;
			
		case MATRIX6:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level6 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL6;

			break;
			
		case MATRIX7:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level7 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL7;

			break;
			
		case MATRIX8:
		
			jvc_camera->_last_error = KYF75_IKYF75Setcolor_mat_level8 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			ctrl = COLMAT_CMATLEVEL8;

			break;
			
		default:
		
			return CAMERA_ERROR;
	
	}

 	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_colour_matrix_panel, ctrl, level);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_get_colour_matrix (GciJvcCamera* jvc_camera, JVC_COLOUR_MATRIX_NUMBER matrix, int *level)
{

	switch(matrix)
	{
		case MATRIX0:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level0 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
	
		case MATRIX1:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level1 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
			
		case MATRIX2:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level2 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
			
		case MATRIX3:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level3 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
			
		case MATRIX4:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level4 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
			
		case MATRIX5:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level5 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
			
		case MATRIX6:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level6 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
			
		case MATRIX7:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level7 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
			
		case MATRIX8:
		
			jvc_camera->_last_error = KYF75_IKYF75Getcolor_mat_level8 (jvc_camera->handle, &(jvc_camera->ErrorInfo), level);
			
			break;
			
		default:
		
			return CAMERA_ERROR;
	
	}

 	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_iris_mode (GciJvcCamera* jvc_camera, JVC_IRIS_MODE val)
{
	jvc_camera->_last_error = KYF75_IKYF75Setiris_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_iris_mode %d\n", val);
		fflush(gfp);
	#endif
	
	if(val != MODE_AUTO && val != MODE_MANUAL)
		return CAMERA_ERROR;

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	switch (val)
	{
		case MODE_AUTO:
		
			SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_IRISLEVEL, ATTR_DIMMED, TRUE);
			
			break;
		
		case MODE_MANUAL:
		{
			int min, max;
	
			KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_IRIS_LEVEL, &min); 
			KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_IRIS_LEVEL, &max);
			SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_IRISLEVEL, ATTR_MIN_VALUE, min);
			SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_IRISLEVEL, ATTR_MAX_VALUE, max);
			SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_IRISLEVEL, ATTR_DIMMED, FALSE);
	
			break;
		}
	}
	
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_IRISMODE, val);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_iris_mode (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getiris_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_iris_level (GciJvcCamera* jvc_camera, int val)
{
	if(val < -10 || val > 10)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setiris_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_IRISLEVEL, val);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_iris_level (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getiris_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_IRISLEVEL, *val);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_ae_level (GciJvcCamera* jvc_camera, long val)
{
	if(val < -128 || val > 127)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setiris_ae_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_AELEVEL, val);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_ae_level (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getiris_ae_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_draw_ae_area (GciJvcCamera* jvc_camera, int val)
{
	// Possibly display the area on the image
	// The SDK Docs say 0 Not drawn - 1 Drawn
	// This is wrong and the other way around hence the !
	jvc_camera->_last_error =  KYF75_IKYF75DrawAeArea (jvc_camera->handle, &(jvc_camera->ErrorInfo), !val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_ae_area (GciJvcCamera* jvc_camera, JVC_AE_AREA_TYPE val, int draw)
{
	if(val < AREA_SQUARE || val > AREA_CIRCLE)
		return CAMERA_ERROR;

    jvc_camera->_last_error = KYF75_IKYF75Setiris_ae_area (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	if(draw) {
	
		gci_jvc_camera_draw_ae_area (jvc_camera, draw);
		
		// start timer to display this for some time
		SetCtrlAttribute(jvc_camera->_exposure_panel, jvc_camera->_ae_area_timer, ATTR_ENABLED, 1);
	}

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_AEAREA, val);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_ae_area (GciJvcCamera* jvc_camera, long *val)
{
    jvc_camera->_last_error = KYF75_IKYF75Getiris_ae_area (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_ae_detect  (GciJvcCamera* jvc_camera, JVC_IRIS_AE_DETECT val)
{
	if(val < DETECT_NORMAL || val > DETECT_AVG)
		return CAMERA_ERROR;

	jvc_camera->_last_error =  KYF75_IKYF75Setiris_ae_detect (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_AEDETECT, val);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_ae_detect (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error =  KYF75_IKYF75Getiris_ae_detect (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_shutter_mode (GciJvcCamera* jvc_camera, JVC_SHUTTER_MODE val)
{
	int dim, levelCtrl, level, *pLevel, min, max;
	char *pLevelText;
	
	GciCamera *camera = (GciCamera *) jvc_camera;

	// Validate parameters
	if(val < SHUT_STEP || val > SHUT_RANDOM)
		return CAMERA_ERROR;

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_shutter_mode %d\n", val);
		fflush(gfp);
	#endif	
	
	pLevel = &level;  // by default, we may change this to min for certain conditions
	
	jvc_camera->_last_error =  KYF75_IKYF75Setshutter_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_SHUTTER, val);

	switch (val)
	{
		case SHUT_STEP:
		
			dim       = 0;
			levelCtrl = F75_SHUT_STEP_LEVEL;
			level     = jvc_camera->_shutter_step_level;
			
			break;
			
		case SHUT_VSCAN:
		
			dim       = 0;
			levelCtrl = F75_SHUT_VSCAN_LEVEL;
			level     = jvc_camera->_shutter_vscan_level;
			
			break;
			
		case SHUT_EEI:
		
			dim       = 1;
			levelCtrl = F75_INVALID_LEVEL;
			pLevel    = &min;

			break;
			
		case SHUT_OFF: 
		
			dim       = 1;
			levelCtrl = F75_INVALID_LEVEL;
			pLevel    = &min;
			
			break;
			
		case SHUT_RANDOM:
		
			dim       = 0;
			levelCtrl = F75_SHUT_RANDOM_LEVEL;
			level     = jvc_camera->_shutter_random_level;

			break;
	}
	
	SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_SPEED,  ATTR_DIMMED, dim);

	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrl, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrl, &max);
	SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_SPEED, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_SPEED, ATTR_MAX_VALUE, max);
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_SPEED, *pLevel);
		
	KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, levelCtrl, &pLevelText);
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_SPEEDDISPLAY, pLevelText);

	jvc_camera->_shutter_mode = val;

	return CAMERA_SUCCESS;
}

int gci_jvc_camera_get_shutter_mode (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error =  KYF75_IKYF75Getshutter_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}

int gci_jvc_camera_set_shutter_level (GciJvcCamera* jvc_camera, int val)
{
	char *pLevelText;
	int can_restart_shutter;
	double exposure;
	
	GciCamera *camera = (GciCamera*) jvc_camera; 
	
	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_shutter_level %d\n", val);
		fflush(gfp);
	#endif	
	
	switch (jvc_camera->_shutter_mode)
	{
		case SHUT_STEP:
		
			VALIDATE_PARAM(camera, F75_SHUT_STEP_LEVEL, val)

			jvc_camera->_last_error = KYF75_IKYF75Setshutter_step_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			if(jvc_camera->_last_error < 0)
				send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
		
			KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_SHUT_STEP_LEVEL, &pLevelText);
			jvc_camera->_shutter_step_level = val;
			
			break;
			
		case SHUT_VSCAN:
		
			VALIDATE_PARAM(camera, F75_SHUT_VSCAN_LEVEL, val)

			jvc_camera->_last_error = KYF75_IKYF75Setshutter_vscan_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			if(jvc_camera->_last_error < 0)
				send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
		
			KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_SHUT_VSCAN_LEVEL, &pLevelText);
			
			jvc_camera->_last_error = KYF75_IKYF75InqRestartShutter (jvc_camera->handle, &(jvc_camera->ErrorInfo), &can_restart_shutter);
			
			if(jvc_camera->_last_error < 0)
				send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

			SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_RESTART,  ATTR_DIMMED, !can_restart_shutter);
			
			jvc_camera->_shutter_vscan_level = val;
			
			break;
			
		case SHUT_RANDOM:
		
			VALIDATE_PARAM(camera, F75_SHUT_RANDOM_LEVEL, val)

			jvc_camera->_last_error = KYF75_IKYF75Setshutter_random_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			if(jvc_camera->_last_error < 0)
				send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
		
			KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_SHUT_RANDOM_LEVEL, &pLevelText);
			jvc_camera->_shutter_random_level = val;
			
			break;
			
		case F75_SHUT_EEI:
		
			val=0;
			KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_INVALID_LEVEL, &pLevelText);
			
			break;
			
		case F75_SHUT_OFF: 
		
			val=0;
			KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_INVALID_LEVEL, &pLevelText);
			
			break;
		}
		
	
		
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_SPEED, val);
	
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_SPEEDDISPLAY, pLevelText);

	convert_jvc_camera_exposure_string_to_double(pLevelText, &exposure); 
	camera->_exposure_time = exposure * 1000;	  // convert to ms

	if(camera->_main_ui_panel != -1)
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, camera->_exposure_time);

	return 0;
}


int gci_jvc_camera_get_shutter_level (GciJvcCamera* jvc_camera, int *val)
{
	switch (jvc_camera->_shutter_mode)
	{
		case SHUT_STEP:
		
			jvc_camera->_last_error = KYF75_IKYF75Getshutter_step_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			if(jvc_camera->_last_error < 0)
				send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
		
			break;
			
		case SHUT_VSCAN:
		
			jvc_camera->_last_error = KYF75_IKYF75Getshutter_vscan_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			if(jvc_camera->_last_error < 0)
				send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
		
			break;
			
		case SHUT_RANDOM:
		
			jvc_camera->_last_error = KYF75_IKYF75Getshutter_random_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			if(jvc_camera->_last_error < 0)
				send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
		
			break;
			
		case F75_SHUT_EEI:
		
			val=0;
			
			break;
			
		case F75_SHUT_OFF: 
		
			val=0;
			
			break;
		}

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_restart_shutter (GciJvcCamera* jvc_camera)
{
	jvc_camera->_last_error =  KYF75_IKYF75RestartShutter (jvc_camera->handle, &(jvc_camera->ErrorInfo));
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_alc_max (GciJvcCamera* jvc_camera, int val)
{
	jvc_camera->_last_error =  KYF75_IKYF75Setalc_max_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	// Validate parameters
	if(val != 0 && val != 1)
		return CAMERA_ERROR;

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_ALCMAX, val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_alc_max (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error =  KYF75_IKYF75Getalc_max_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_ALCMAX, *val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_eei_limit (GciJvcCamera* jvc_camera, int val)
{
	jvc_camera->_last_error = KYF75_IKYF75Seteei_limit_level  (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	// Validate parameters
	if(val != 0 && val != 1)
		return CAMERA_ERROR;

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_EEILIMIT, val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_eei_limit (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Geteei_limit_level  (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_EEILIMIT, *val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_gain_mode(GciJvcCamera* jvc_camera, JVC_GAIN_MODE mode)
{
	GciCamera *camera = (GciCamera*) jvc_camera;
	
	// Validate parameters
	if(mode < GAIN_STEP || mode > GAIN_VGAIN)
		return CAMERA_ERROR;

	jvc_camera->_gain_mode = mode;
	
	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_gain_mode %d\n", mode);
		fflush(gfp);
	#endif	
	
	jvc_camera->_last_error = KYF75_IKYF75Setgain_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), mode);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	if(mode == GAIN_ALC) {
	
		// Disable gain control
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_DIMMED, 1);
		
		SetCtrlAttribute(jvc_camera->_exposure_panel, EXPOSURE_GAINLEVEL, ATTR_DIMMED, 1);
		
		return CAMERA_SUCCESS;
	}
	else if (mode == GAIN_STEP) {
	
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_DIMMED, 1);
	
		SetCtrlAttribute(jvc_camera->_exposure_panel, EXPOSURE_GAINLEVEL, ATTR_DIMMED, 0);
	
		// Get the max and min of the gain control
		KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_GAIN_STEP_LEVEL, &(jvc_camera->_min_gain_value)); 
		KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_GAIN_STEP_LEVEL, &(jvc_camera->_max_gain_value));
	}
	else if (mode == GAIN_VGAIN) {
	
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_DIMMED, 0);
	
		SetCtrlAttribute(jvc_camera->_exposure_panel, EXPOSURE_GAINLEVEL, ATTR_DIMMED, 0);
	
		// Get the max and min of the gain control
		KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, F75_GAIN_VGAIN_LEVEL, &(jvc_camera->_min_gain_value)); 
		KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, F75_GAIN_VGAIN_LEVEL, &(jvc_camera->_max_gain_value));
	}
	else {
	
		return CAMERA_ERROR;
	}
	
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_GAIN, mode);
	
	SetCtrlAttribute(jvc_camera->_exposure_panel, EXPOSURE_GAINLEVEL, ATTR_MIN_VALUE, jvc_camera->_min_gain_value); 
	SetCtrlAttribute(jvc_camera->_exposure_panel, EXPOSURE_GAINLEVEL, ATTR_MAX_VALUE, jvc_camera->_max_gain_value);
	
	SetCtrlAttribute (camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_MIN_VALUE, (double) jvc_camera->_min_gain_value);
	SetCtrlAttribute (camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_MAX_VALUE, (double) jvc_camera->_max_gain_value);
	
	gci_camera_set_gain_range_text(camera, "", "");
	SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN_TEXT_MIN, camera->_min_gain_text);
	SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN_TEXT_MAX, camera->_max_gain_text);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_gain_mode(GciJvcCamera* jvc_camera, long *mode)
{
	jvc_camera->_last_error = KYF75_IKYF75Getgain_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), mode);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_gain_level(GciJvcCamera* jvc_camera, int gain)
{
	char *pLevelText;  
	GciCamera *camera = (GciCamera*) jvc_camera; 
	
	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_gain_level %d\n", gain);
		fflush(gfp);
	#endif	

	if(jvc_camera->_gain_mode == GAIN_ALC) {
	
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN, 0.0); 
		return CAMERA_SUCCESS;
	}

	if(gain > jvc_camera->_max_gain_value)
		gain = jvc_camera->_max_gain_value;

	if(gain < jvc_camera->_min_gain_value)
		gain = jvc_camera->_min_gain_value;

	if (jvc_camera->_gain_mode == GAIN_VGAIN) { 
	
		VALIDATE_PARAM(camera, F75_GAIN_VGAIN_LEVEL, gain)

		jvc_camera->_last_error = KYF75_IKYF75Setgain_vgain_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), gain);
		
		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
		
		KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_GAIN_VGAIN_LEVEL, &pLevelText);
	}
	else if (jvc_camera->_gain_mode == GAIN_STEP) {
	
		VALIDATE_PARAM(camera, F75_GAIN_STEP_LEVEL, gain)

		jvc_camera->_last_error = KYF75_IKYF75Setgain_step_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), gain);
		
		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
		
		KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_GAIN_STEP_LEVEL, &pLevelText);
	}
			
	SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN, (double) gain);
					
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_GAINLEVEL, (int) gain);
	
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_GAINLEVELDISPLAY, pLevelText);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_gain(GciCamera* camera, CameraChannel c, double *gain)
{
	GciJvcCamera *jvc_camera = (GciJvcCamera*) camera;  
	long mode, val;

	gci_jvc_camera_get_gain_mode(jvc_camera, &mode);

	if(mode == GAIN_ALC)
		return CAMERA_ERROR;

	if (mode == GAIN_VGAIN) { 
	
		jvc_camera->_last_error = KYF75_IKYF75Getgain_vgain_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), &val);
		
		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	}
	else if (mode == GAIN_STEP) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getgain_step_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), &val);
		
		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	}

	*gain = (double) val;
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_gain(GciCamera* camera, CameraChannel c, double gain)
{
	GciJvcCamera *jvc_camera = (GciJvcCamera*) camera;

	// Validate parameters
	if( gain < 0 || gain > 127.0)
		return CAMERA_ERROR;

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_gain %f\n", gain);
		fflush(gfp);
	#endif	

	gci_jvc_camera_set_gain_mode(jvc_camera, GAIN_VGAIN);

	gci_jvc_camera_set_gain_level(jvc_camera, (int) gain);

	return CAMERA_SUCCESS;
}


/////////////////////////// White Balance ///////////////////////////////////////////////////////////

int gci_jvc_camera_set_colour_temp (GciJvcCamera* jvc_camera, int val)
{
	if(val != 0 && val != 1)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setcolor_temp  (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_COLORTEMP, val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_colour_temp (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getcolor_temp  (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_COLORTEMP, *val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_white_balance_mode  (GciJvcCamera* jvc_camera, JVC_WHITE_BALANCE_MODE val)
{
	int dim = 0, levelCtrlR, levelCtrlB, min, max, auto_balance;

	if(val < WB_PRESET || val > WB_MANUAL)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_mode  (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_WHITEBAL, val);
	
	switch (val)
	{
		case WB_PRESET:
		
			dim = 1;
			levelCtrlR = F75_INVALID_LEVEL;
			levelCtrlB = F75_INVALID_LEVEL;
			break;
			
		case WB_AUTO1:
		
			levelCtrlR = F75_WB_AUTO1_LEVEL_R;
			levelCtrlB = F75_WB_AUTO1_LEVEL_B;
			break;
			
		case WB_AUTO2:
		
		
			levelCtrlR = F75_WB_AUTO2_LEVEL_R;
			levelCtrlB = F75_WB_AUTO2_LEVEL_B;
			break;
			
		case WB_MANUAL:
		
			levelCtrlR = F75_WB_MANUAL_LEVEL_R;
			levelCtrlB = F75_WB_MANUAL_LEVEL_B;
			
			break;
	}

	jvc_camera->_white_balance_mode = val;
	
	// R
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrlR, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrlR, &max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_LEVELR, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_LEVELR, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_LEVELR,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_white_balance_red_level  (jvc_camera, 0); 
	
	// B
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrlB, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrlB, &max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_LEVELB, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_LEVELB, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_LEVELB,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_white_balance_blue_level  (jvc_camera, 0);

	jvc_camera->_last_error = KYF75_IKYF75InqAutoWhiteBalance (jvc_camera->handle, &(jvc_camera->ErrorInfo), &auto_balance);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_AUTOWHITE,  ATTR_DIMMED, !auto_balance);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_white_balance_mode  (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_mode  (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	return CAMERA_SUCCESS;
}



int gci_jvc_camera_set_white_balance_red_level  (GciJvcCamera* jvc_camera, int val)
{
	switch (jvc_camera->_white_balance_mode)
	{
		case WB_AUTO1:
		
			VALIDATE_PARAM(jvc_camera, F75_WB_AUTO1_LEVEL_R, val)

			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_auto_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_AUTO2:
		
			VALIDATE_PARAM(jvc_camera, F75_WB_AUTO2_LEVEL_R, val)

			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_auto2_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_MANUAL:
		
			VALIDATE_PARAM(jvc_camera, F75_WB_MANUAL_LEVEL_R, val)

			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_manual_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
	}
		
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_LEVELR, val);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_white_balance_red_level  (GciJvcCamera* jvc_camera, int *val)
{
	long mode;

	gci_jvc_camera_get_white_balance_mode(jvc_camera, &mode);

	switch (mode)
	{
		case WB_AUTO1:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_auto_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_AUTO2:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_auto2_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_MANUAL:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_manual_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

			break;
	}
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_white_balance_blue_level  (GciJvcCamera* jvc_camera, int val)
{
	switch (jvc_camera->_white_balance_mode)
	{
		case WB_AUTO1:
		
			VALIDATE_PARAM(jvc_camera, F75_WB_AUTO1_LEVEL_B, val)

			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_auto_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_AUTO2:

			VALIDATE_PARAM(jvc_camera, F75_WB_AUTO2_LEVEL_B, val)
		
			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_auto2_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

			break;
			
		case WB_MANUAL:

			VALIDATE_PARAM(jvc_camera, F75_WB_MANUAL_LEVEL_B, val)
		
			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_manual_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
	}
		
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_LEVELB, val);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_white_balance_blue_level  (GciJvcCamera* jvc_camera, int *val)
{
	long mode;

	gci_jvc_camera_get_white_balance_mode(jvc_camera, &mode);

	switch (mode)
	{
		case WB_AUTO1:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_auto_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

			break;
			
		case WB_AUTO2:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_auto2_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_MANUAL:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_manual_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
	}
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_white_balance_base_red  (GciJvcCamera* jvc_camera, int val)
{
	if(val < -32 || val > 31)
		return CAMERA_ERROR;

	switch (jvc_camera->_white_balance_mode)
	{
		case WB_AUTO1:
		
			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_auto_base_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_AUTO2:
		
			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_auto2_base_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
	}
		
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_BASER, val);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_white_balance_base_red  (GciJvcCamera* jvc_camera, int *val)
{
	*val = 0;

	switch (jvc_camera->_white_balance_mode)
	{
		case WB_AUTO1:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_auto_base_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_AUTO2:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_auto2_base_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

			break;
	}

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}



int gci_jvc_camera_set_white_balance_base_blue  (GciJvcCamera* jvc_camera, int val)
{
	if(val < -32 || val > 31)
		return CAMERA_ERROR;

	switch (jvc_camera->_white_balance_mode)
	{
		case WB_AUTO1:
		
			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_auto_base_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_AUTO2:
		
			jvc_camera->_last_error = KYF75_IKYF75Setwhite_bal_auto2_base_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
	}
		
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_BASEB, val);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_white_balance_base_blue  (GciJvcCamera* jvc_camera, int *val)
{
	*val = 0;

	switch (jvc_camera->_white_balance_mode)
	{
		case WB_AUTO1:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_auto_base_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
			
		case WB_AUTO2:
		
			jvc_camera->_last_error = KYF75_IKYF75Getwhite_bal_auto2_base_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
			break;
	}
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_auto_whitebalance  (GciJvcCamera* jvc_camera) 
{
	jvc_camera->_last_error = KYF75_IKYF75AutoWhiteBalance (jvc_camera->handle, &(jvc_camera->ErrorInfo));
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_shading_level_red  (GciJvcCamera* jvc_camera, int val) 
{
	if(val < -128 || val > 128)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setshading_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELR, val);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_get_shading_level_red  (GciJvcCamera* jvc_camera, int *val) 
{
	jvc_camera->_last_error = KYF75_IKYF75Getshading_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_shading_level_green (GciJvcCamera* jvc_camera, int val) 
{
	if(val < -128 || val > 128)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setshading_level_g (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELG, val);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_get_shading_level_green  (GciJvcCamera* jvc_camera, int *val) 
{
	jvc_camera->_last_error = KYF75_IKYF75Getshading_level_g (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_shading_level_blue  (GciJvcCamera* jvc_camera, int val) 
{
	if(val < -128 || val > 128)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setshading_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELB, val);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_get_shading_level_blue  (GciJvcCamera* jvc_camera, int *val) 
{
	jvc_camera->_last_error = KYF75_IKYF75Getshading_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_shading_mode (GciJvcCamera* jvc_camera, JVC_SHADING_MODE val)
{
	int dim, levelCtrlR, levelCtrlG, levelCtrlB, min, max;

	if(val != SHADING_MODE_ON && val != SHADING_MODE_OFF)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setshading_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_SHADING, val);
	
	switch (val)
	{
		case SHADING_MODE_ON:
		
			dim       = 0;
			levelCtrlR = F75_SHADING_LEVEL_R;
			levelCtrlG = F75_SHADING_LEVEL_G;
			levelCtrlB = F75_SHADING_LEVEL_B;
			
			break;
			
		case SHADING_MODE_OFF:
		
			dim       = 1;
			levelCtrlR = F75_INVALID_LEVEL;
			levelCtrlG = F75_INVALID_LEVEL;
			levelCtrlB = F75_INVALID_LEVEL;
			
			break;
	}

	// R
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrlR, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrlR, &max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELR, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELR, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELR,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_shading_level_red(jvc_camera, 0);
	
	// G
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrlG, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrlG, &max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELG, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELG, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELG,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_shading_level_green(jvc_camera, 0);
	
	// B
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrlB, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrlB, &max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELB, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELB, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELB,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_shading_level_blue(jvc_camera, 0);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_shading_mode (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getshading_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


/////////////////////////// Process ///////////////////////////////////////////////////////////

int gci_jvc_camera_set_detail_level (GciJvcCamera* jvc_camera, int val)
{
	if(val < -7 || val > 7)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setdetail_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_DETAILLEVEL, val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_detail_level (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getdetail_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}



int gci_jvc_camera_set_detail_level_depend (GciJvcCamera* jvc_camera, int val)
{
	char *pLevelText;

	if(val < 0 || val > 127)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setdetail_level_dep (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

    if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_DETAIL_LEVEL_DEP, &pLevelText);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_DETAILLEVELDEP, val);
	SetCtrlVal(jvc_camera->_process_panel, PROCESS_LEVELDEPDISPLAY, pLevelText);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_detail_level_depend (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getdetail_level_dep (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_detail_mode (GciJvcCamera* jvc_camera, JVC_DETAIL_MODE val)
{
	int dim, levelCtrl, depCtrl, min, max;
	
	if(val != DETAIL_MODE_ON && val != DETAIL_MODE_OFF)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setdetail_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_DETAIL, val);
	
	switch (val)
	{
		case DETAIL_MODE_ON:
		
			dim       = 0;
			levelCtrl = F75_DETAIL_LEVEL;
			depCtrl= F75_DETAIL_LEVEL_DEP;
			break;
			
		case DETAIL_MODE_OFF:
		
			dim       = 1;
			levelCtrl = F75_INVALID_LEVEL;
			depCtrl= F75_INVALID_LEVEL;
			
			break;
	}

	// level
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrl, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrl, &max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_DETAILLEVEL, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_DETAILLEVEL, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_DETAILLEVEL,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_detail_level (jvc_camera, 0);
	
	// depend
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, depCtrl, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, depCtrl, &max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_DETAILLEVELDEP, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_DETAILLEVELDEP, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_DETAILLEVELDEP,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_detail_level_depend (jvc_camera, 0); 
	
	// Noise Sup.
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_NOISESUP,  ATTR_DIMMED, dim);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_detail_mode (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getdetail_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_gamma_level (GciJvcCamera* jvc_camera, long val)
{
	char *pLevelText; 

	if(val < 0 || val > 100)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setgamma_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, F75_GAMMA_LEVEL, &pLevelText);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_GAMMALEVEL, val);
	SetCtrlVal(jvc_camera->_process_panel, PROCESS_GAMMALEVELDISPLAY, pLevelText);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_get_gamma_level (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getgamma_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}



int gci_jvc_camera_set_gamma_mode (GciJvcCamera* jvc_camera, JVC_GAMMA_MODE val)
{
	int dim, levelCtrl, level, *pLevel, min, max;
	
	if(val != GAMMA_MODE_NORMAL && val != GAMMA_MODE_ADJUST)
		return CAMERA_ERROR;

	pLevel = &level;  // by default, we may change this to min for certain conditions
	
	jvc_camera->_last_error = KYF75_IKYF75Setgamma_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_GAMMA, val);
	
	// If ADJUST=OFF then GAMMA=NORMAL(value=10, display=0.45) 
	switch (val)
	{
		case GAMMA_MODE_ADJUST:
		
			dim       = 0;
			levelCtrl = F75_GAMMA_LEVEL;
			level     = 10;
			break;
			
		case GAMMA_MODE_NORMAL:
		
			dim       = 1;
			levelCtrl = F75_GAMMA_LEVEL;
			level     = 10;
			
			break;
	}

	// level
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrl, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrl, &max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_GAMMALEVEL, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_GAMMALEVEL, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_GAMMALEVEL,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_gamma_level(jvc_camera, *pLevel);
	

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_gamma_mode (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getgamma_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_flare_level_red (GciJvcCamera* jvc_camera, int val)
{
	if(val < -32 || val > 31)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setflare_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_FLARELEVELR, val);
	
	return CAMERA_SUCCESS;
}

int gci_jvc_camera_get_flare_level_red (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getflare_level_r (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_flare_level_blue (GciJvcCamera* jvc_camera, int val)
{
	if(val < -32 || val > 31)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setflare_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_FLARELEVELB, val);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_flare_level_blue (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getflare_level_b (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_flare_mode (GciJvcCamera* jvc_camera, JVC_FLARE_MODE val)
{
	int dim, levelCtrlR, levelCtrlB, level, *pLevel, min, max;

	if(val != FLARE_MODE_ON && val != FLARE_MODE_OFF)
		return CAMERA_ERROR;

	pLevel = &level;  // by default, we may change this to min for certain conditions
	
	jvc_camera->_last_error = KYF75_IKYF75Setflare_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_FLARE, val);
	
	switch (val)
	{
		case FLARE_MODE_ON:
		
			dim       = 0;
			levelCtrlR = F75_FLARE_LEVEL_R;
			levelCtrlB = F75_FLARE_LEVEL_B;
			level     = 0;
			break;
			
		case FLARE_MODE_OFF:
		
			dim       = 1;
			levelCtrlR = F75_INVALID_LEVEL;
			levelCtrlB = F75_INVALID_LEVEL;
			level     = 0;
			
			break;
	}

	// R
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrlR, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrlR, &max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_FLARELEVELR, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_FLARELEVELR, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_FLARELEVELR,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_flare_level_red(jvc_camera, *pLevel);
	
	// B
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrlB, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrlB, &max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_FLARELEVELB, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_FLARELEVELB, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_FLARELEVELB,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_flare_level_blue(jvc_camera, *pLevel);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_flare_mode (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getflare_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_abl_level (GciJvcCamera* jvc_camera, int val)
{
	if(val < -32 || val > 31)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setabl_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_ABLLEVEL, val); 
	
	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_get_abl_level (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getabl_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_abl_mode (GciJvcCamera* jvc_camera, JVC_ABL_MODE val)
{
	int dim, levelCtrl, level, *pLevel, min, max;

	if(val != ABL_MODE_ON && val != ABL_MODE_OFF)
		return CAMERA_ERROR;

	pLevel = &level;  // by default, we may change this to min for certain conditions
	
	jvc_camera->_last_error = KYF75_IKYF75Setabl_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_ABL, val);
	
	switch (val)
	{
		case F75_MODE_ON:
		
			dim       = 0;
			levelCtrl = F75_ABL_LEVEL;
			level     = 0;
			
			break;
			
		case F75_MODE_OFF:
		
			dim       = 1;
			levelCtrl = F75_INVALID_LEVEL;
			level     = 0;
			
			break;
	}

	// level
	KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrl, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrl, &max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_ABLLEVEL, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_ABLLEVEL, ATTR_MAX_VALUE, max);
	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_ABLLEVEL,  ATTR_DIMMED, dim);
	gci_jvc_camera_set_abl_level (jvc_camera, *pLevel);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_abl_mode (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getabl_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS; 
}


int gci_jvc_camera_set_detail_noise_suppression (GciJvcCamera* jvc_camera, JVC_DETAIL_NOISE_SUPPRESSION val)
{
	if(val < NOISE_SUPP_LOW || val > NOISE_SUPP_OFF)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setdetail_noise_supp (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_NOISESUP, val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_detail_noise_suppression (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getdetail_noise_supp (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}

int gci_jvc_camera_set_dsp_bypass (GciJvcCamera* jvc_camera, JVC_DSP_MODE val)
{
	if(val != DSP_OFF && val != DSP_BYPASS)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setdsp_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_DSPBYPASS, val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_dsp_bypass (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getdsp_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}

int gci_jvc_camera_set_nega_mode (GciJvcCamera* jvc_camera, JVC_NEGA_MODE val)
{
	if(val != NEGA_MODE_ON && val != NEGA_MODE_OFF)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setnega_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_NEGA, val); 
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_nega_mode (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getnega_mode (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_master_black_level (GciJvcCamera* jvc_camera, int val)
{
	if(val < -99 || val > 99)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setmaster_black_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_MASTERBLACK, val);
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_master_black_level (GciJvcCamera* jvc_camera, int *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getmaster_black_level (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_pixel_compensation (GciJvcCamera* jvc_camera, JVC_PIXEL_COMPENSATION_MODE val)
{
	int pixel_checked;

	if(val != PIXEL_COMPENSATION_MODE_ON && val != PIXEL_COMPENSATION_MODE_OFF)
		return CAMERA_ERROR;

	jvc_camera->_last_error = KYF75_IKYF75Setpixel_compen (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_process_panel, PROCESS_PIXELCOMP, val);
	
	jvc_camera->_last_error = KYF75_IKYF75InqPixelCheck (jvc_camera->handle, &(jvc_camera->ErrorInfo), &pixel_checked);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlAttribute (jvc_camera->_process_panel, PROCESS_PIXELCHECK,  ATTR_DIMMED, !pixel_checked);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_pixel_compensation (GciJvcCamera* jvc_camera, long *val)
{
	jvc_camera->_last_error = KYF75_IKYF75Getpixel_compen (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_pixel_check (GciJvcCamera* jvc_camera)
{
	jvc_camera->_last_error =KYF75_IKYF75PixelCheck (jvc_camera->handle, &(jvc_camera->ErrorInfo));
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

static int gci_jvc_camera_set_live_mode(GciCamera* camera)
{		 
	GciJvcCamera *jvc_camera = (GciJvcCamera*) camera; 

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_live_mode: ");
		fflush(gfp);
	#endif

	if(jvc_camera->_set_live == 1)
		return CAMERA_SUCCESS;

	jvc_camera->_last_error = KYF75_IKYF75StartIsoc (jvc_camera->handle, &jvc_camera->ErrorInfo);

	//Delay(1.0);

	jvc_camera->_set_live = 1;  

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_set_snap_sequence_mode(GciCamera* camera)
{
	GciJvcCamera *jvc_camera = (GciJvcCamera*) camera;

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_snap_sequence_mode\n");
		fflush(gfp);
	#endif

	jvc_camera->_set_live = 0;
	

	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_set_snap_mode(GciCamera* camera)
{
	GciJvcCamera *jvc_camera = (GciJvcCamera*) camera;

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_set_snap_mode\n");
		fflush(gfp);
	#endif

	if(jvc_camera->_set_live == 0)
		return CAMERA_SUCCESS;

	jvc_camera->_last_error = KYF75_IKYF75StopIsoc (jvc_camera->handle, &jvc_camera->ErrorInfo);
	
	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	jvc_camera->_set_live = 0; 

	//Delay(1.0);

	return CAMERA_SUCCESS;
}


int gci_jvc_camera_get_colour_type(GciJvcCamera* jvc_camera, long *val)
{
	GciCamera *camera = (GciCamera*) jvc_camera;
	int tmp;

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_get_colour_type\n");
		fflush(gfp);
	#endif

	*val = F75_COL_RGB;

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getlive_color (jvc_camera->handle, &(jvc_camera->ErrorInfo), &tmp);

	}
	else if(camera->_aquire_mode == SNAP) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getstill_color (jvc_camera->handle, &(jvc_camera->ErrorInfo), &tmp); 
	}
	else {
	
		return CAMERA_ERROR;
	}

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	switch(tmp) {
	
		case 0:
		
			*val = F75_COL_RGB;
			return CAMERA_SUCCESS;
			
		case 1:
		
			*val = F75_COL_R;
			return CAMERA_SUCCESS;
			
		case 2:
		
			*val = F75_COL_G;
			return CAMERA_SUCCESS;
			
		case 3:
		
			*val = F75_COL_B;
			return CAMERA_SUCCESS;
	
	}
	
	return CAMERA_ERROR;
}


int gci_jvc_camera_set_colour_type(GciJvcCamera* jvc_camera, JVC_COLOUR_TYPE val)
{
	GciCamera *camera = (GciCamera*) jvc_camera;

	if(val < COL_RGB || val > COL_B)
		return CAMERA_ERROR;

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75Setlive_color (jvc_camera->handle, &(jvc_camera->ErrorInfo), val);

	}
	else if(camera->_aquire_mode == SNAP) {
	
		jvc_camera->_last_error = KYF75_IKYF75Setstill_color (jvc_camera->handle, &(jvc_camera->ErrorInfo), val); 
	}
	else {
	
		return CAMERA_ERROR;
	}

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	
	SetCtrlVal(jvc_camera->_image_panel, IMAGE_COLOUR, val);
	jvc_camera->_colour_type = val;

	return CAMERA_SUCCESS;
}


///////////////////////////////////////////////////////////////////
//
// get the size of DIB in bytes
//
///////////////////////////////////////////////////////////////////
static DWORD GetDibBytes(BITMAPINFOHEADER* pbmi)
{
	DWORD DibBytes;
	
	DWORD bytes1  = pbmi->biSize;
	DWORD bytes2  = pbmi->biSizeImage;
	DWORD bytes3  = 0;
	
	if( pbmi->biBitCount == 8 ){
		bytes3 = sizeof(RGBQUAD) * 256;
	}
	
	DibBytes = bytes1 + bytes2 + bytes3;

	return DibBytes;
}

static FIBITMAP * gci_jvc_camera_get_image(GciCamera* camera, const Rect *rect)
{
	long pBitMapInfo;
	BITMAPINFO *BitMapInfo;
	char *pData, aspect_text[30];
	FIBITMAP *dib = NULL, *channel_dib;
	FREE_IMAGE_COLOR_CHANNEL channel;
	TriggerMode trigger_mode;
	
	GciJvcCamera *jvc_camera = (GciJvcCamera *) camera;

	#ifdef JVC_DEBUG
		fprintf (gfp, "gci_jvc_camera_get_image\n");
		fflush(gfp);
	#endif

	gci_camera_get_trigger_mode(camera, &trigger_mode); 

	if (trigger_mode == CAMERA_EXTERNAL_TRIG)   
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "TriggerNow", GCI_VOID_POINTER, camera);   

	PROFILE_START("gci_jvc_camera_get_image");

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75GetLiveImage (jvc_camera->handle, &(jvc_camera->ErrorInfo), &pBitMapInfo);

		if(jvc_camera->_last_error < 0) {
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
			goto FINISH;
		}   
	}
	else if(camera->_aquire_mode == SNAP) {
	
		jvc_camera->_last_error = KYF75_IKYF75GetStillImage (jvc_camera->handle, &(jvc_camera->ErrorInfo), &pBitMapInfo);

		if(jvc_camera->_last_error < 0) {
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
			goto FINISH;   
		}   
	}
	else {
	
		goto FINISH;    
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "PostCapture", GCI_VOID_POINTER, jvc_camera);   

	BitMapInfo = ((BITMAPINFO *)pBitMapInfo);
	pData = (char *)BitMapInfo + sizeof(BITMAPINFOHEADER);
	
	if(pData) {

		switch (jvc_camera->_colour_type)
		{
			case COL_RGB:
				
				dib = FIA_LoadColourFIBFromArrayData (pData, 24, BitMapInfo->bmiHeader.biWidth, BitMapInfo->bmiHeader.biHeight, 1, 0, COLOUR_ORDER_RGB); 
				
				break;

			case COL_R:
			case COL_G:
			case COL_B:

				if ( (channel_dib = FreeImage_Allocate (BitMapInfo->bmiHeader.biWidth, BitMapInfo->bmiHeader.biHeight, 8, 0, 0, 0)) == NULL )
					goto FINISH; 

				// There seems to be an extra header of 1024 bytes
				FIA_CopyBytesToFBitmap (channel_dib, pData + 1024, 0, 0, COLOUR_ORDER_RGB);

				if ( (dib = FreeImage_Allocate (BitMapInfo->bmiHeader.biWidth, BitMapInfo->bmiHeader.biHeight, 24, 0, 0, 0)) == NULL )
					goto FINISH;    

				if(jvc_camera->_colour_type == COL_R) {
				
					channel = FICC_RED;
				}
				else if(jvc_camera->_colour_type == COL_G) {
				
					channel = FICC_GREEN;
				}
				else if(jvc_camera->_colour_type == COL_B) {
				
					channel = FICC_BLUE;
				}
				else {
				
					FreeImage_Unload(channel_dib);
					FreeImage_Unload(dib);
				
					goto FINISH;    
				}

				FreeImage_SetChannel(dib, channel_dib, channel);

				FreeImage_Unload(channel_dib);

				break;
		}
	}
	
	sprintf (aspect_text, "%d by %d pixels", FreeImage_GetWidth(dib), FreeImage_GetHeight(dib));
	SetCtrlVal(jvc_camera->_image_panel, IMAGE_ASPECTTEXT, aspect_text);
	
	FINISH:
	
	PROFILE_STOP("gci_jvc_camera_get_image"); 
	
	return dib;
}


static int gci_jvc_camera_set_size_position(GciCamera* camera, unsigned int left, unsigned int top,
																unsigned int width, unsigned int height, unsigned char auto_centre)
{
	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
																unsigned int *width, unsigned int *height, unsigned char *auto_centre)
{
	int camera_width, camera_height;
	GciJvcCamera *jvc_camera = (GciJvcCamera *) camera;

	if(camera->_aquire_mode == LIVE) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getlive_width (jvc_camera->handle, &(jvc_camera->ErrorInfo), &camera_width);

		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

		jvc_camera->_last_error = KYF75_IKYF75Getlive_height (jvc_camera->handle, &(jvc_camera->ErrorInfo), &camera_height);

		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	}
	else if(camera->_aquire_mode == SNAP) {
	
		jvc_camera->_last_error = KYF75_IKYF75Getstill_width (jvc_camera->handle, &(jvc_camera->ErrorInfo), &camera_width);

		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

		jvc_camera->_last_error = KYF75_IKYF75Getstill_height (jvc_camera->handle, &(jvc_camera->ErrorInfo), &camera_height);

		if(jvc_camera->_last_error < 0)
			send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);
	}
	else {
	
		return CAMERA_ERROR;
	}
	
	*width = camera_width;
	*height = camera_height;
	*left = 0;
	*top = 0;
    
	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = IM_MIN_WIDTH;
	*height = IM_MIN_HEIGHT;

	return CAMERA_SUCCESS;
}


static int  gci_jvc_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = IM_MAX_WIDTH;
	*height = IM_MAX_HEIGHT;

	return CAMERA_SUCCESS;
}


static int gci_jvc_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus, char *camera_id, char *camera_version, char *driver_version)
{
	if (vendor != NULL) {
	
		strcpy(vendor, "Jvc");	
	}
	
	if (model != NULL) {
	
		strcpy(model, "KY-F75U");	
	}
	
	if (bus != NULL) {
	
		strcpy(bus, "");	
	}
	
	if (camera_id != NULL) {
	
		strcpy(camera_id, "");		
	}
	
	if (camera_version != NULL) {
	
		strcpy(camera_version, "");		
	}
	
	if (driver_version != NULL) {
	
		strcpy(driver_version, "");		
	}
	
	return CAMERA_SUCCESS;
}


int gci_jvc_camera_set_data_mode(GciCamera* camera, DataMode data_mode)
{
	return CAMERA_SUCCESS;
}

int gci_jvc_camera_get_highest_data_mode(GciCamera* camera, DataMode *data_mode)
{
	*data_mode = BPP24;
	
	return CAMERA_SUCCESS; 
}

static int gci_jvc_camera_init (GciCamera* camera)
{
	GciJvcCamera *jvc_camera = (GciJvcCamera *) camera;
	char path[500] = "";
	long pVal;

	#ifdef JVC_DEBUG
		gfp = fopen ("c:\\jvc.log", "a");
		fprintf (gfp, "gci_jvc_camera_init\n");
		fflush(gfp);
	#endif
	gci_camera_set_extra_panel_uir(camera, "gci_camera_jvc_ui.uir", EXTRA_PNL);
	gci_camera_set_exposure_range(camera, 1, 5000, "ms", VAL_DOUBLE);       

	jvc_camera->_tab_control = EasyTab_ConvertFromCanvas (camera->_extra_ui_panel, EXTRA_PNL_CANVAS);

	if(find_resource("gci_camera_jvc_ui.uir", path) < 0) {
		return -1;  
	}
	
	EasyTab_LoadPanels (camera->_extra_ui_panel, jvc_camera->_tab_control, 1, path, __CVIUserHInst,
		IMAGE, &(jvc_camera->_image_panel), EXPOSURE, &(jvc_camera->_exposure_panel), WHITEBAL, &(jvc_camera->_white_balance_panel), 
		PROCESS, &(jvc_camera->_process_panel), COLMAT, &(jvc_camera->_colour_matrix_panel), SYSTEM, &(jvc_camera->_system_panel), 0);

	// System Panel
	
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_CLOSE, JvcCamera_onExtrasQuit, jvc_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (jvc_camera->_system_panel, SYSTEM_TESTPATTERN, JvcCamera_onTestPatternDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (jvc_camera->_system_panel, SYSTEM_PATTERNLEVEL, JvcCamera_onTestPatternLevelDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_system_panel, SYSTEM_FREEZECANCEL, JvcCamera_onFreezeCancelDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;	
		
	if ( InstallCtrlCallback (jvc_camera->_system_panel, SYSTEM_MEMORYSAVE, JvcCamera_onCameraMemorySave, jvc_camera) < 0)
		return CAMERA_ERROR;	
		
		
	// Image Panel
	if ( InstallCtrlCallback (jvc_camera->_image_panel, IMAGE_COLOUR, JvcCamera_onColourDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
	
	if ( InstallCtrlCallback (jvc_camera->_image_panel, IMAGE_RESOLUTION, JvcCamera_onResolutionDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
	
	if ( InstallCtrlCallback (jvc_camera->_image_panel, IMAGE_ASPECT, JvcCamera_onAspectDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		

	// Exposure Panel

	jvc_camera->_ae_area_timer = NewCtrl(jvc_camera->_exposure_panel, CTRL_TIMER, "", 0, 0); 
	SetCtrlAttribute(jvc_camera->_exposure_panel, jvc_camera->_ae_area_timer, ATTR_INTERVAL, 3.0);
	SetCtrlAttribute(jvc_camera->_exposure_panel, jvc_camera->_ae_area_timer, ATTR_ENABLED, 0);

	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_GAIN, JvcCamera_onGainModeDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_GAINLEVEL, JvcCamera_onGainLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_IRISLEVEL, JvcCamera_onIrisLevel, jvc_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_IRISMODE, JvcCamera_onIrisModeDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_SHUTTER, JvcCamera_onShutterModeDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_SPEED, JvcCamera_onShutterSpeedLevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_AEAREA, JvcCamera_onAEAreaDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, jvc_camera->_ae_area_timer, JvcCamera_onAEAreaTimer, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_AEDETECT, JvcCamera_onAEDetectDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_AELEVEL, JvcCamera_onAELevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_RESTART, JvcCamera_onShutterRestart, jvc_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_ALCMAX, JvcCamera_onAlcMaxDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (jvc_camera->_exposure_panel, EXPOSURE_EEILIMIT, JvcCamera_onEeilimitDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;



	// Proccess Panel
	
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_FLARE, JvcCamera_onFlareDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_FLARELEVELB, JvcCamera_onFlareBlueLevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_MASTERBLACK, JvcCamera_onMasterBlackLevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_ABLLEVEL, JvcCamera_onAblLevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_FLARELEVELR, JvcCamera_onFlareRedLevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_GAMMA, JvcCamera_onGammaDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_GAMMALEVEL, JvcCamera_onGammaLevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_ABL, JvcCamera_onABLDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_DSPBYPASS, JvcCamera_onDspBypass, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_PIXELCHECK, JvcCamera_onPixelCheck, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_PIXELCOMP, JvcCamera_onPixelCompLevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_NEGA, JvcCamera_onNegaDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
																			 
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_NOISESUP, JvcCamera_onNoiseSupDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_DETAIL, JvcCamera_onProcessDetailDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_DETAILLEVEL,JvcCamera_onProcessDetailLevel, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_process_panel, PROCESS_DETAILLEVELDEP, JvcCamera_onProccessDetailDepLevel, jvc_camera) < 0)
		return CAMERA_ERROR;


	// Colour Matrix Panel
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL0, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
	
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL1, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL2, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL3, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL4, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL5, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL6, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL7, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_CMATLEVEL8, JvcCamera_onColourMatrixLevelChanged, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_colour_matrix_panel, COLMAT_COLOURMATRIX, JvcCamera_onColourMatrixModeDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;	
		


	// White Balance Panel
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_SHADING, JvcCamera_onShadingDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;
	
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELB, JvcCamera_onShadingLevelBlue, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELG, JvcCamera_onShadingLevelGreen, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_SHADLEVELR, JvcCamera_onShadingLevelRed, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_AUTOWHITE, JvcCamera_onWhiteBalanceAuto, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_BASEB, JvcCamera_onWhiteBalanceBaseLevelBlue, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_BASER, JvcCamera_onWhiteBalanceBaseLevelRed, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_COLORTEMP, JvcCamera_onWhiteBalanceColourTemp, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_LEVELB, JvcCamera_onWhiteBalanceLevelBlue, jvc_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_LEVELR, JvcCamera_onWhiteBalanceLevelRed, jvc_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (jvc_camera->_white_balance_panel, WHITEBAL_WHITEBAL, JvcCamera_onWhiteBalanceModeDropDown, jvc_camera) < 0)
		return CAMERA_ERROR;

	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, ATTR_CTRL_MODE, VAL_INDICATOR);  
	
	// TODO setup all the panel ctrls.
	KYF75_IKYF75Getshading_mode(jvc_camera->handle, &(jvc_camera->ErrorInfo), &pVal);

	if(jvc_camera->_last_error < 0)
		send_jvc_error_text(jvc_camera, jvc_camera->_last_error, jvc_camera->ErrorInfo);

	SetCtrlVal(jvc_camera->_white_balance_panel, WHITEBAL_SHADING, pVal);

	/*
	int dim, levelCtrl, level, *pLevel, min, max;
	char *pLevelText;

KYF75_IKYF75GetMinValue (jvc_camera->handle, NULL, levelCtrl, &min); 
	KYF75_IKYF75GetMaxValue (jvc_camera->handle, NULL, levelCtrl, &max);
	SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_SPEED, ATTR_MIN_VALUE, min);
	SetCtrlAttribute (jvc_camera->_exposure_panel, EXPOSURE_SPEED, ATTR_MAX_VALUE, max);
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_SPEED, *pLevel);
	
	
	KYF75_IKYF75GetValueText (jvc_camera->handle, NULL, levelCtrl, &pLevelText);
	SetCtrlVal(jvc_camera->_exposure_panel, EXPOSURE_SPEEDDISPLAY, pLevelText);
	*/

	return CAMERA_SUCCESS;
}

static int GetIniSectionString(GciCamera* camera, char *buffer)
{
	GciJvcCamera *jvc_camera = (GciJvcCamera *) camera;

	int shutter_level, white_bal_blue, white_bal_red, master_black_level;
	long white_bal_blue_base, white_bal_red_base;
	long gain_mode, shutter_mode, white_bal_mode, aspect, colour_type;
	long iris_mode, iris_level, ae_level, ae_area, ae_detect, alc_max;
	long eei_limit, detail_mode, detail_level, detail_level_dep;
	long gamma_mode, gamma_level, flare_mode, flare_level_r, flare_level_b;
	long abl_mode, abl_level, noise_sup, nega, dsp_bypass, pix_comp;
	long colour_temp, shading_mode, shading_level_red, shading_level_green;
	long shading_level_blue, colour_matrix_mode, cmat_level0, cmat_level1;
	long cmat_level2, cmat_level3, cmat_level4, cmat_level5, cmat_level6;
	long cmat_level7, cmat_level8;
	double gain;
	
	memset(buffer, 0, 1);   
	
	gci_jvc_camera_get_shutter_mode(jvc_camera, &shutter_mode);
	gci_jvc_camera_get_gain_mode(jvc_camera, &gain_mode);
	gci_jvc_camera_get_shutter_level (jvc_camera, &shutter_level);
	gci_jvc_camera_get_gain((GciCamera*) jvc_camera, CAMERA_CHANNEL1, &gain);
	gci_jvc_camera_get_white_balance_mode(jvc_camera, &white_bal_mode);
	gci_jvc_camera_get_white_balance_red_level(jvc_camera, &white_bal_red);
	gci_jvc_camera_get_white_balance_blue_level(jvc_camera, &white_bal_blue);
	gci_jvc_camera_get_white_balance_base_red(jvc_camera, &white_bal_red_base);
	gci_jvc_camera_get_white_balance_base_blue(jvc_camera, &white_bal_blue_base);
	gci_jvc_camera_get_master_black_level(jvc_camera, &master_black_level);
	gci_jvc_camera_get_aspect(jvc_camera, &aspect);
	gci_jvc_camera_get_colour_type(jvc_camera, &colour_type);
	gci_jvc_camera_get_iris_mode(jvc_camera, &iris_mode);
	gci_jvc_camera_get_iris_level(jvc_camera, &iris_level);
	gci_jvc_camera_get_ae_level(jvc_camera, &ae_level);
	gci_jvc_camera_get_ae_area(jvc_camera, &ae_area);
	gci_jvc_camera_get_ae_detect(jvc_camera, &ae_detect);
	gci_jvc_camera_get_alc_max(jvc_camera, &alc_max);
	gci_jvc_camera_get_eei_limit(jvc_camera, &eei_limit);
	gci_jvc_camera_get_detail_mode(jvc_camera, &detail_mode);
	gci_jvc_camera_get_detail_level(jvc_camera, &detail_level);
	gci_jvc_camera_get_detail_level_depend(jvc_camera, &detail_level_dep);
	gci_jvc_camera_get_gamma_mode(jvc_camera, &gamma_mode);
	gci_jvc_camera_get_gamma_level(jvc_camera, &gamma_level);
	gci_jvc_camera_get_flare_mode(jvc_camera, &flare_mode);
	gci_jvc_camera_get_flare_level_red(jvc_camera, &flare_level_r);
	gci_jvc_camera_get_flare_level_blue(jvc_camera, &flare_level_b);
	gci_jvc_camera_get_abl_mode(jvc_camera, &abl_mode);
	gci_jvc_camera_get_abl_level(jvc_camera, &abl_level);
	gci_jvc_camera_get_detail_noise_suppression(jvc_camera, &noise_sup); 
	gci_jvc_camera_get_nega_mode(jvc_camera, &nega);
	gci_jvc_camera_get_dsp_bypass (jvc_camera, &dsp_bypass);
	gci_jvc_camera_get_pixel_compensation (jvc_camera, &pix_comp);
	gci_jvc_camera_get_colour_temp (jvc_camera, &colour_temp);
	gci_jvc_camera_get_shading_mode (jvc_camera, &shading_mode);
	gci_jvc_camera_get_shading_level_red (jvc_camera, &shading_level_red);
	gci_jvc_camera_get_shading_level_green (jvc_camera, &shading_level_green);
	gci_jvc_camera_get_shading_level_blue (jvc_camera, &shading_level_blue);
	gci_jvc_camera_get_colour_matrix_mode(jvc_camera, &colour_matrix_mode);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX0, &cmat_level0);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX1, &cmat_level1);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX2, &cmat_level2);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX3, &cmat_level3);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX4, &cmat_level4);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX5, &cmat_level5);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX6, &cmat_level6);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX7, &cmat_level7);
	gci_jvc_camera_get_colour_matrix (jvc_camera, MATRIX8, &cmat_level8);

	
	// Write the global section
	sprintf(buffer, "Shutter Mode=%d\nShutter Level=%d\nGain Mode=%d\nGain Level=%f\n"
					"White Balance Mode=%d\nWhite Balance Red=%d\nWhite Balance Blue=%d\n"
					"White Balance Red Base=%d\nWhite Balance Blue Base=%d\n"
					"Master Black Level=%d\nAspect=%d\nColour Type=%d\nIris Mode=%d\n"
					"Iris Level=%d\nAE Level=%d\nAE Area=%d\nAE Detect=%d\nALC Max=%d\n"
					"EEI Limit=%d\nDetail Mode=%d\nDetail Level=%d\nDetail Level Dep=%d\n"
					"Gamma Mode=%d\nGamma Level=%d\nFlare Mode=%d\nFlare Level Red=%d\n"
					"Flare Level Blue=%d\nABL Mode=%d\nABL Level=%d\nNoise Suppression=%d\n"
					"Nega=%d\nDSP Bypass=%d\nPixel Compensation=%d\nColour Temp=%d\n"
					"Shading Mode=%d\nShading Level Red=%d\nShading Level Green=%d\n"
					"Shading Level Blue=%d\nColur Matrix Mode=%d\nCMAT Level 0=%d\n"
					"CMAT Level 1=%d\nCMAT Level 2=%d\nCMAT Level 3=%d\nCMAT Level 4=%d\n"
					"CMAT Level 5=%d\nCMAT Level 6=%d\nCMAT Level 7=%d\nCMAT Level 8=%d\n",
		shutter_mode, shutter_level, gain_mode, gain, white_bal_mode,
		white_bal_red, white_bal_blue, white_bal_red_base, white_bal_blue_base,
		master_black_level, aspect, colour_type,
		iris_mode, iris_level, ae_level, ae_area, ae_detect, alc_max,
		eei_limit, detail_mode, detail_level, detail_level_dep, gamma_mode, gamma_level,
		flare_mode, flare_level_r, flare_level_b, abl_mode, abl_level, noise_sup, nega,
		dsp_bypass, pix_comp, colour_temp,shading_mode, shading_level_red, shading_level_green,
		shading_level_blue, colour_matrix_mode, cmat_level0, cmat_level1, cmat_level2,
		cmat_level3, cmat_level4, cmat_level5, cmat_level6, cmat_level7, cmat_level8);
	
	str_change_char(buffer, '\n', '\0'); 
	
	return CAMERA_SUCCESS;  
}


static int CameraSaveIniCameraSettings(GciCamera* camera, const char * filepath, const char *mode)  
{
	GciJvcCamera *jvc_camera = (GciJvcCamera *) camera;
  
	char buffer[2000] = "";
	FILE *fp = NULL;
	
	GetIniSectionString(camera, buffer);
	
	// clear the file
	fp = fopen(filepath, mode);
	fclose(fp);
	
	if(!WritePrivateProfileSection(UIMODULE_GET_NAME(camera), buffer, filepath))
		return CAMERA_ERROR;  
		
	return CAMERA_SUCCESS; 
}

static int gci_jvc_save_settings (GciCamera* camera, const char *filepath, const char* flags)
{
	GciJvcCamera *jvc_camera = (GciJvcCamera *) camera;

	// Save in ini file for changing microscope modes.
	if(CameraSaveIniCameraSettings(camera, filepath, flags) == CAMERA_ERROR) {
	
		send_error_text(camera, "Failed to save settings for device %s.", UIMODULE_GET_DESCRIPTION(camera));
		return CAMERA_ERROR;
	}

	//jvc_camera->_last_error = KYF75_IKYF75SaveFile (jvc_camera->handle, &(jvc_camera->ErrorInfo), filepath);
	//CHECK_FOR_JVC_ERROR(jvc_camera, "JVC KY-F75 SaveFile Error", jvc_camera->_last_error);

	return CAMERA_SUCCESS;
}


static void LoadFromIniDictionary(GciCamera* camera, dictionary* ini)
{
	char buffer[64];
	int tmp;
	double d_tmp;
	GciJvcCamera *jvc_camera = (GciJvcCamera *) camera;
	
	sprintf (buffer, "%s:Shutter Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_shutter_mode(jvc_camera, tmp);  

	sprintf (buffer, "%s:Shutter Level", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_shutter_level(jvc_camera, tmp);  
 
	sprintf (buffer, "%s:Gain Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_gain_mode(jvc_camera, tmp);  

	sprintf (buffer, "%s:Gain Level", UIMODULE_GET_NAME(camera));
	d_tmp = iniparser_getdouble(ini, buffer, INVALID_VALUE);
	gci_camera_set_gain(camera, CAMERA_CHANNEL1, d_tmp);  

	sprintf (buffer, "%s:White Balance Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_white_balance_mode(jvc_camera, tmp);  

	sprintf (buffer, "%s:White Balance Red", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_white_balance_red_level(jvc_camera, tmp);  

	sprintf (buffer, "%s:White Balance Blue", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_white_balance_blue_level(jvc_camera, tmp);  

	sprintf (buffer, "%s:White Balance Red Base", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_white_balance_base_red(jvc_camera, tmp);  

	sprintf (buffer, "%s:White Balance Blue Base", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_white_balance_base_blue(jvc_camera, tmp);  

	sprintf (buffer, "%s:Master Black Level", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_master_black_level(jvc_camera, tmp);  

	sprintf (buffer, "%s:Aspect", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_aspect(jvc_camera, tmp); 

	sprintf (buffer, "%s:Colour Type", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_type(jvc_camera, tmp); 

	sprintf (buffer, "%s:Iris Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_iris_mode(jvc_camera, tmp); 

	sprintf (buffer, "%s:Iris Level", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_iris_level(jvc_camera, tmp); 

	sprintf (buffer, "%s:AE Level", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_ae_level(jvc_camera, tmp); 

	sprintf (buffer, "%s:AE Area", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_ae_area(jvc_camera, tmp, 0); 

	sprintf (buffer, "%s:AE Detect", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_ae_detect(jvc_camera, tmp); 

	sprintf (buffer, "%s:ALC Max", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_alc_max(jvc_camera, tmp); 

	sprintf (buffer, "%s:EEI Limit", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_eei_limit(jvc_camera, tmp); 

	sprintf (buffer, "%s:Detail Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_detail_mode(jvc_camera, tmp); 

	sprintf (buffer, "%s:Detail Level", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_detail_level(jvc_camera, tmp); 

	sprintf (buffer, "%s:Detail Level Dep", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_detail_level_depend(jvc_camera, tmp); 

	sprintf (buffer, "%s:Gamma Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_gamma_mode(jvc_camera, tmp); 

	sprintf (buffer, "%s:Gamma Level", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_gamma_level(jvc_camera, tmp); 

	sprintf (buffer, "%s:Flare Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_flare_mode(jvc_camera, tmp); 

	sprintf (buffer, "%s:Flare Level Red", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_flare_level_red(jvc_camera, tmp); 

	sprintf (buffer, "%s:Flare Level Blue", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_flare_level_blue(jvc_camera, tmp); 

	sprintf (buffer, "%s:ABL Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_abl_mode(jvc_camera, tmp); 

	sprintf (buffer, "%s:ABL Level", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_abl_level(jvc_camera, tmp); 

	sprintf (buffer, "%s:Noise Suppression", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_detail_noise_suppression(jvc_camera, tmp); 

	sprintf (buffer, "%s:Nega", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_nega_mode(jvc_camera, tmp); 

	sprintf (buffer, "%s:DSP Bypass", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_dsp_bypass(jvc_camera, tmp); 

	sprintf (buffer, "%s:Pixel Compensation", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_pixel_compensation(jvc_camera, tmp); 

	sprintf (buffer, "%s:Colour Temp", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_temp(jvc_camera, tmp); 

	sprintf (buffer, "%s:Shading Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_shading_mode(jvc_camera, tmp); 

	sprintf (buffer, "%s:Shading Level Red", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_shading_level_red(jvc_camera, tmp); 

	sprintf (buffer, "%s:Shading Level Green", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_shading_level_green(jvc_camera, tmp); 

	sprintf (buffer, "%s:Shading Level Blue", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_shading_level_blue(jvc_camera, tmp); 

	sprintf (buffer, "%s:Colur Matrix Mode", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix_mode(jvc_camera, tmp); 

	sprintf (buffer, "%s:CMAT Level 0", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX0, tmp); 

	sprintf (buffer, "%s:CMAT Level 1", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX1, tmp); 

	sprintf (buffer, "%s:CMAT Level 2", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX2, tmp); 

	sprintf (buffer, "%s:CMAT Level 3", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX3, tmp); 

	sprintf (buffer, "%s:CMAT Level 4", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX4, tmp); 

	sprintf (buffer, "%s:CMAT Level 5", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX5, tmp); 

	sprintf (buffer, "%s:CMAT Level 6", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX6, tmp); 

	sprintf (buffer, "%s:CMAT Level 7", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX7, tmp); 

	sprintf (buffer, "%s:CMAT Level 8", UIMODULE_GET_NAME(camera));
	tmp = iniparser_getint(ini, buffer, -1);
	gci_jvc_camera_set_colour_matrix(jvc_camera, MATRIX8, tmp); 
}


static int CameraLoadIniCameraSettings(GciCamera* camera, const char * filepath)  
{
	dictionary* ini = iniparser_load  (filepath);    
	
	LoadFromIniDictionary(camera, ini);
		
	iniparser_freedict(ini);
	
	return CAMERA_SUCCESS; 
}


static int gci_jvc_load_settings (GciCamera* camera, const char *filepath) 
{
	GciJvcCamera *jvc_camera = (GciJvcCamera *) camera;

	if( CameraLoadIniCameraSettings(camera, filepath) == CAMERA_ERROR) {
	
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

static int gci_jvc_camera_set_trigger_mode (GciCamera* camera, TriggerMode trig_mode)
{
	return CAMERA_SUCCESS;
}


static int gci_jvc_get_colour_type (GciCamera* camera)
{
	return RGB_TYPE;
}


int gci_jvc_camera_destroy(GciCamera* camera)
{
	GciJvcCamera* jvc_camera = (GciJvcCamera*) camera;

	gci_camera_power_down(camera);

	--number_of_jvc_cameras;

  	return CAMERA_SUCCESS;
}

static int gci_jvc_supports_binning(GciCamera* camera)
{
	return 0;
}

void gci_jvc_constructor(GciCamera *camera, const char *name, const char *description)
{
	gci_camera_constructor (camera, name, description);
	
	CAMERA_VTABLE_PTR(camera).initialise = gci_jvc_camera_init;
	CAMERA_VTABLE_PTR(camera).power_up = gci_jvc_camera_power_up;
	CAMERA_VTABLE_PTR(camera).power_down = gci_jvc_camera_power_down;
	CAMERA_VTABLE_PTR(camera).set_exposure_time = gci_jvc_camera_set_exposure_time;
	CAMERA_VTABLE_PTR(camera).set_gain = gci_jvc_camera_set_gain; 
	CAMERA_VTABLE_PTR(camera).get_gain = gci_jvc_camera_get_gain;  
	CAMERA_VTABLE_PTR(camera).set_live_mode = gci_jvc_camera_set_live_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_mode = gci_jvc_camera_set_snap_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_sequence_mode = gci_jvc_camera_set_snap_sequence_mode;
	CAMERA_VTABLE_PTR(camera).get_image = gci_jvc_camera_get_image; 
	CAMERA_VTABLE_PTR(camera).set_size_position = gci_jvc_camera_set_size_position;
	CAMERA_VTABLE_PTR(camera).get_size_position = gci_jvc_camera_get_size_position;
	CAMERA_VTABLE_PTR(camera).get_info =	gci_jvc_camera_get_info;
	CAMERA_VTABLE_PTR(camera).save_settings = gci_jvc_save_settings;
	CAMERA_VTABLE_PTR(camera).load_settings = gci_jvc_load_settings;
	CAMERA_VTABLE_PTR(camera).set_datamode = gci_jvc_camera_set_data_mode;
	CAMERA_VTABLE_PTR(camera).get_highest_datamode = gci_jvc_camera_get_highest_data_mode;

	CAMERA_VTABLE_PTR(camera).get_colour_type = gci_jvc_get_colour_type;
	CAMERA_VTABLE_PTR(camera).set_default_settings = gci_jvc_camera_set_default_settings;
	CAMERA_VTABLE_PTR(camera).save_settings_as_default = gci_jvc_camera_save_settings_as_default;
	CAMERA_VTABLE_PTR(camera).save_state = gci_jvc_camera_save_state;
	CAMERA_VTABLE_PTR(camera).restore_state = gci_jvc_camera_restore_state;
	CAMERA_VTABLE_PTR(camera).destroy = gci_jvc_camera_destroy;
	CAMERA_VTABLE_PTR(camera).supports_binning = gci_jvc_supports_binning;
	CAMERA_VTABLE_PTR(camera).set_trigger_mode = gci_jvc_camera_set_trigger_mode; 
}


GciCamera* gci_jvc_camera_new(const char *name, const char *description)
{
	GciJvcCamera* jvc_camera = (GciJvcCamera*) malloc(sizeof(GciJvcCamera));
	GciCamera *camera = (GciCamera*) jvc_camera; 

	memset(jvc_camera, 0, sizeof(GciJvcCamera));
	
	gci_jvc_constructor(camera, name, description);
	
	return camera;
}
