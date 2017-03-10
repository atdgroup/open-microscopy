#include "gci_upix_uc3010_camera.h"
#include "gci_upix_uc3010_camera_ui.h"
#include "gci_upix_uc3010_camera_lowlevel.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "FreeImageAlgorithms_IO.h"

#include "iniparser.h"
#include "ThreadDebug.h"
#include <utility.h>

#define UPIX_BUG_WORK_AROUND

/* Function pointers used as virtual functions */
static struct camera_operations dummy_camera_ops;

static int gci_upix_uc3010_camera_power_up(GciCamera* camera)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if(upix_hardware_initialise (upix_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;

	//gci_upix_uc3010_camera_set_snap_mode(camera);

  	return CAMERA_SUCCESS;
}

static int gci_upix_uc3010_camera_power_down(GciCamera* camera)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if(camera_stop(upix_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
	
	return CAMERA_SUCCESS;
}

static int  gci_upix_uc3010_camera_set_exposure_time(GciCamera* camera, double exposure, double *actual_exposure)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if (set_exposure(upix_camera, exposure) == CAMERA_ERROR){
		return CAMERA_ERROR;
	}

	if (get_exposure(upix_camera, actual_exposure) == CAMERA_ERROR){
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


static int  gci_upix_uc3010_camera_set_gain(GciCamera* camera, CameraChannel channel, double gain)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if (set_gain(upix_camera, (int)gain) == CAMERA_ERROR){
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

static int  gci_upix_uc3010_camera_get_gain(GciCamera* camera, CameraChannel channel, double *gain)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if (get_gain(upix_camera, gain) == CAMERA_ERROR){
		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}

static int  gci_upix_uc3010_camera_set_live_mode(GciCamera* camera)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if(upix_camera->run_mode == UPIX_RUNMODE_PLAY_SNAP || upix_camera->run_mode == UPIX_RUNMODE_PLAY_LIVE) {

		if (camera_stop(upix_camera) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}

		// This is in the demo code so we copied it here.
		Delay(0.05);

		if (camera_play(upix_camera, 1) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}
	}
	else if(upix_camera->run_mode == UPIX_RUNMODE_STOP) {

		if (camera_play(upix_camera, 1) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}
	}

	return CAMERA_SUCCESS;
}


static int  gci_upix_uc3010_camera_set_snap_mode(GciCamera* camera)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if(upix_camera->run_mode == UPIX_RUNMODE_PLAY_LIVE) {

		if (camera_stop(upix_camera) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}

		Delay(0.1);

		if (camera_play(upix_camera, 0) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}
	}
	else if(upix_camera->run_mode == UPIX_RUNMODE_STOP) {

		if (camera_play(upix_camera, 0) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}
	}

	return CAMERA_SUCCESS;
}


static int  gci_upix_uc3010_camera_set_snap_sequence_mode(GciCamera* camera)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if(upix_camera->run_mode == UPIX_RUNMODE_PLAY_LIVE) {

		if (camera_stop(upix_camera) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}

		Delay(0.05);

		if (camera_play(upix_camera, 0) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}
	}
	else if(upix_camera->run_mode == UPIX_RUNMODE_STOP) {

		if (camera_play(upix_camera, 0) == CAMERA_ERROR){
			return CAMERA_ERROR;
		}
	}

	return CAMERA_SUCCESS;
}


static FIBITMAP*  gci_upix_uc3010_camera_get_image(GciCamera* camera, const Rect *rect)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;
	return get_image(upix_camera);
}


static int  gci_upix_uc3010_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = 512;
	*height = 384;

	return CAMERA_SUCCESS;
}


static int  gci_upix_uc3010_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = 2046;
	*height = 1534;

	return CAMERA_SUCCESS;
}


static int gci_upix_uc3010_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus, char *camera_id, char *camera_version, char *driver_version)
{
	if(vendor != NULL)
		strcpy(vendor, "Vineon");
	
	if(model != NULL)
		strcpy(model, "UPix 3010");
	
	if(camera_id != NULL) {
		BYTE serial_number[10];
		char serial_txt[100] = "";
		upix_get_serial_number((Upix3010Camera *) camera, serial_number);
		sprintf(serial_txt, "%d", serial_number);
		strcpy(camera_id, serial_txt);
	}
	
	if(camera_version)
		strcpy(camera_version, "3010");
	
	return CAMERA_SUCCESS;
}


static int gci_upix_uc3010_camera_set_trigger_mode(GciCamera* camera, TriggerMode trig_mode)
{
	// val stored in camera->_trigger_mode by higher level
	return CAMERA_SUCCESS;
}
	
static int gci_upix_uc3010_camera_set_data_mode(GciCamera* camera, DataMode data_mode)
{
	return CAMERA_SUCCESS;
}

int gci_upix_uc3010_camera_destroy(GciCamera* camera)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	if(camera == NULL)
		return CAMERA_ERROR;

	upix_camera->prevent_callback = 1;

	CmtDiscardLock(upix_camera->upix_lock);
	CmtDiscardLock(upix_camera->get_image_lock);

  	return CAMERA_SUCCESS;
}


int gci_upix_uc3010_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
												 unsigned int *width, unsigned int *height, unsigned char *auto_centre)
{
	Upix3010Camera *upix_camera = (Upix3010Camera *) camera;

	*left = 0;
	*top = 0;

	switch(upix_camera->resolution)
	{
		case R2048_1536:
		{
			// This is not 2048 x 1536 as we have to remove a one pixel wide border
			*width = 2046;
			*height = 1534;
			break;
		}

		case R1024_768:
		{
			*width = 1024;
			*height = 768;
			break;
		}

		case R640_480:
		{
			*width = 640;
			*height = 480;
			break;
		}

		case R512_384:
		{
			*width = 512;
			*height = 384;
			break;
		}

	}

	*auto_centre = 0;

	return CAMERA_SUCCESS;
}

static int Upix3010GetIniSectionString(Upix3010Camera* upix_camera, char *buffer)
{
	GciCamera *camera = (GciCamera*) upix_camera;    
	double gain;
	int red_gain, green_gain, blue_gain;	// This is white balance not sure why vinion refer to it as gain

	memset(buffer, 0, 1);   
	
	if (gci_camera_get_gain(camera, CAMERA_CHANNEL1, &gain) == CAMERA_ERROR)
		return CAMERA_ERROR;

	if (get_colour_gain(upix_camera, &red_gain, &green_gain, &blue_gain) == CAMERA_ERROR)
		return CAMERA_ERROR;

	// No longer save this as it causes problems
	//if (upix_get_sensor_mode(upix_camera, &speed) == CAMERA_ERROR)
	//	return CAMERA_ERROR;

	// Write the global section
	sprintf(buffer, "Exposure=%f\nGain=%f\nRedGain=%d\nGreenGain=%d\nBlueGain=%d\n\n",
					camera->_exposure_time, gain, red_gain, green_gain, blue_gain);
	
	str_change_char(buffer, '\n', '\0'); 
	
	return CAMERA_SUCCESS;  
}

static int gci_upix_uc3010_save_settings (GciCamera* camera, const char *filepath, const char *mode)
{
	Upix3010Camera* upix_camera = (Upix3010Camera*) camera;  
	char buffer[2000] = "";
	FILE *fp = NULL;
	
	if(camera == NULL)
		return CAMERA_ERROR;

	if (Upix3010GetIniSectionString(upix_camera, buffer) == CAMERA_ERROR) {
		send_error_text(camera, "Failed to save settings for device %s.", UIMODULE_GET_DESCRIPTION(camera));
		return CAMERA_ERROR;  
	}

	if(!WritePrivateProfileSection(UIMODULE_GET_NAME(camera), buffer, filepath)) {
		send_error_text(camera, "Failed to save settings for device %s.", UIMODULE_GET_DESCRIPTION(camera));
		return CAMERA_ERROR;  
	}
		
	return CAMERA_SUCCESS;
}

static void loadFromIniDictionary(Upix3010Camera* upix_camera, dictionary* ini)
{
	char buffer[500];
	double d_tmp = 0.0;
	int red = 0, green = 0, blue = 0, speed;
	GciCamera *camera = (GciCamera*) upix_camera;     
	
	sprintf (buffer, "%s:Exposure", UIMODULE_GET_NAME(camera));
	d_tmp = dictionary_getdouble(ini, buffer, -1.0);

	if (d_tmp>0.0)
		if (gci_camera_set_exposure_time(camera, d_tmp) == CAMERA_ERROR)
			goto Error;

	sprintf (buffer, "%s:Gain", UIMODULE_GET_NAME(camera));
	d_tmp = dictionary_getdouble(ini, buffer, -1.0);
	if (d_tmp>=0.0)
		if (gci_camera_set_gain(camera, CAMERA_CHANNEL1, d_tmp) == CAMERA_ERROR)
			goto Error;    

	sprintf (buffer, "%s:RedGain", UIMODULE_GET_NAME(camera));
	red = dictionary_getint(ini, buffer, -1);

	sprintf (buffer, "%s:GreenGain", UIMODULE_GET_NAME(camera));
	green = dictionary_getint(ini, buffer, -1);

	sprintf (buffer, "%s:BlueGain", UIMODULE_GET_NAME(camera));
	blue = dictionary_getint(ini, buffer, -1);

	red = max(20, min(red, 200));
	green = max(20, min(green, 200));
	blue = max(20, min(blue, 200));

	if (set_colour_gain(upix_camera, red, green, blue) == CAMERA_ERROR)
		goto Error;

	sprintf (buffer, "%s:SensorMode", UIMODULE_GET_NAME(camera));
	speed = dictionary_getint(ini, buffer, -1);

	// This doesn't work it causes the next snap callback to not be called.
	//speed = max(0, min(speed, 1));

	//if(speed >= 0) {
	//	if (upix_set_sensor_mode(upix_camera, speed) == CAMERA_ERROR)
	//		goto Error;
	//}

	return;

Error:
	send_error_text(camera, "Failed to load settings for device %s.", UIMODULE_GET_DESCRIPTION(camera));
	return;  
}

static int gci_upix_uc3010_load_settings (GciCamera* camera, const char *filepath) 
{
	Upix3010Camera* upix_camera = (Upix3010Camera*) camera;  
	dictionary* ini = iniparser_load  (filepath);    
	
	if (camera == NULL)
		return CAMERA_ERROR;

	if (ini == NULL)
		return CAMERA_ERROR;
	
	loadFromIniDictionary(upix_camera, ini);
		
	iniparser_freedict(ini);

	return CAMERA_SUCCESS;
}

static int gci_upix_uc3010_camera_save_state(GciCamera* camera, CameraState *state)
{
	Upix3010Camera* upix_camera = (Upix3010Camera*) camera;  

	state->data_mode = camera->_data_mode;
	state->exposure_time = camera->_exposure_time;
    state->gain1 = camera->_gain;
	state->live = gci_camera_is_live_mode(camera);

	state->binning = NO_BINNING;
	state->bpp = 8;
	state->dual_tap = 0;
	state->blacklevel1 = 0;
	state->blacklevel2 = 0;
	state->sensitivity = 0;
	state->gain2 = 0;
	state->light_mode = 0;
	state->photon_mode = 0;
	state->trigger_mode = CAMERA_NO_TRIG;
	
	switch(upix_camera->resolution)
	{
		case R2048_1536:
			state->resolution = CAM_RES_2048_1536;
			break;
		case R1024_768:
			state->resolution = CAM_RES_1024_768;
			break;
		case R512_384:
			state->resolution = CAM_RES_512_384;
			break;
	}

	return CAMERA_SUCCESS;
}

static int gci_upix_uc3010_camera_restore_state(GciCamera* camera, CameraState *state)
{
	Upix3010Camera* upix_camera = (Upix3010Camera*) camera;  

	// I had to move this above gci_camera_set_snap_mode WTF ?
	// Otherwise the next snap after gci_upix_uc3010_camera_restore_state would return errors from the camera.
	// Another Dll bug I guess.
	if (camera->_exposure_time != state->exposure_time)
		if (gci_camera_set_exposure_time(camera, state->exposure_time) == CAMERA_ERROR)
			goto Error;

	if (gci_camera_set_snap_mode(camera) == CAMERA_ERROR)
		goto Error;

// No such function on this camera
//	if (gci_camera_set_data_mode(camera, state->data_mode) == CAMERA_ERROR)
//		goto Error;

	if (camera->_gain != state->gain1)
		if (gci_camera_set_gain(camera, CAMERA_ALL_CHANNELS, state->gain1) == CAMERA_ERROR)
			goto Error;

	if (upix_camera->resolution != state->resolution) {
		switch(state->resolution)
		{
			case CAM_RES_2048_1536:
				upix_set_resolution(upix_camera, R2048_1536);
				break;
			case CAM_RES_1024_768:
				upix_set_resolution(upix_camera, R1024_768);
				break;
			case CAM_RES_512_384:
				upix_set_resolution(upix_camera, R512_384);
				break;
		}
	}

	if(state->live) {
		gci_camera_set_live_mode(camera);
		gci_camera_activate_live_display(camera);
	}
		
	return CAMERA_SUCCESS;

Error:
	send_error_text(camera, "Failed to restore state for device %s.", UIMODULE_GET_DESCRIPTION(camera));
	return CAMERA_ERROR;  
}

static int gci_upix_uc3010_camera_set_default_settings (GciCamera* camera)
{
	//double actual_exposure;

	//gci_upix_uc3010_camera_set_exposure_time(camera, 50.0, &actual_exposure);
	//gci_upix_uc3010_camera_set_gain(camera, CAMERA_CHANNEL1, 1.0);

	return CAMERA_SUCCESS; 
}

static int gci_upix_uc3010_camera_get_highest_datamode (GciCamera* camera, DataMode *data_mode)  
{
	*data_mode = BPP24;
	
	return CAMERA_SUCCESS; 
}

static int gci_upix_uc3010_camera_get_lowest_datamode (GciCamera* camera, DataMode *data_mode)  
{
	*data_mode = BPP24;
	
	return CAMERA_SUCCESS; 
}

static int gci_upix_uc3010_supports_fast_mode (GciCamera* camera)
{
	return 1;
}

static int gci_upix_uc3010_camera_set_fast_mode (GciCamera* camera, int enable)
{
// An empty set_fast_mode will not do anything to the camera, the higher level will leave it too the camera and not do anything

	Upix3010Camera* upix_camera = (Upix3010Camera*) camera;  
/*
	if(enable)
		upix_set_resolution (upix_camera, R512_384);
	else
		upix_set_resolution (upix_camera, R2048_1536);
*/

	return CAMERA_SUCCESS;
}

static int gci_upix_uc3010_get_colour_type(GciCamera* camera)
{
	return RGB_TYPE;
}

static int gci_upix_uc3010_camera_initialise (GciCamera* camera)
{
	Upix3010Camera* upix_camera = (Upix3010Camera*) camera; 
//	int r,g,b;

	#ifndef UPIX_BUG_WORK_AROUND
	upix_camera->window_hwnd = GCI_ImagingWindow_GetImageViewHandle(camera->_camera_window);
	#endif

	gci_camera_set_extra_panel_uir(camera, "gci_upix_uc3010_camera_ui.uir", EXTRA_PNL); 

	gci_camera_set_exposure_range(camera, 0.12, 359.42, "ms", VAL_DOUBLE);   

	gci_camera_set_gain_range(camera, 1.0, 80.0, VAL_UNSIGNED_INTEGER);
	//gci_camera_set_gain_range_text(camera, "(1)", "(80)");

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_QUIT, UPix3010OnExtraPanelClose, upix_camera) < 0)
		return CAMERA_ERROR;	

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_RES, UPix3010OnResolutionChanged, upix_camera) < 0)
		return CAMERA_ERROR;	

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_RED_GAIN, UPix3010OnColourChanged, upix_camera) < 0)
		return CAMERA_ERROR;	

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_GREEN_GAIN, UPix3010OnColourChanged, upix_camera) < 0)
		return CAMERA_ERROR;	

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_BLUE_GAIN, UPix3010OnColourChanged, upix_camera) < 0)
		return CAMERA_ERROR;	

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_CE_ENGINE, UPix3010OnColourEnhancementChanged, upix_camera) < 0)
		return CAMERA_ERROR;	
	
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_AUTO_WB, UPix3010OnOnOnePushWhiteBalancePressed, upix_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SENSOR_MODE, UPix3010OnOnSensorModeChanged, upix_camera) < 0)
		return CAMERA_ERROR;

	//get_colour_gain(upix_camera, &r, &g, &b);

	//SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_RED_GAIN, r);
	//SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_GREEN_GAIN, g);
	//SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLUE_GAIN, b);	

	return CAMERA_SUCCESS;
}

static int gci_upix_uc3010_save_settings_as_default (GciCamera* camera)
{
	char path[GCI_MAX_PATHNAME_LEN];
	Upix3010Camera* upix_camera = (Upix3010Camera*) camera;     
				
	if (ConfirmPopup ("Confirm Save as Defaults",
		"Are you sure?\n The these values will be used every time\n this application is started.") != 1) {
		
		return CAMERA_ERROR;
	}
		
	GetPrivateDataFolder("Microscope Data", path);
	strcat(path, upix_camera->default_settings_file_path);
	gci_camera_save_settings(camera, path, "w");  
	
	return CAMERA_SUCCESS;
}

static int upix_supports_focal_indicator_settings(GciCamera* camera)
{
	return 1;
}

static FocusSettings upix_get_focal_indicator_settings (GciCamera* camera)
{
	FocusSettings settings;

	settings.sample_type = FOCUS_SETTINGS_SAMPLE_DETAILED;
	settings.crop_type = FOCUS_SETTINGS_CROP_8;
	settings.resample_type = FOCUS_SETTINGS_RESAMPLE_2;

	return settings;
}

static int upix_attempt_recovery(GciCamera* camera)
{
	Upix3010Camera* upix_camera = (Upix3010Camera*) camera;     

//	camera_play(upix_camera, 1);
//	Delay(0.1);
//	camera_play( upix_camera, 0);

//	gci_upix_uc3010_camera_power_down(camera);
//	Delay(1.0);
//	gci_upix_uc3010_camera_power_up(camera);

	return CAMERA_SUCCESS;
}

GciCamera* gci_upix_uc3010_camera_new(const char *name, const char* description)
{
	Upix3010Camera* upix_camera = (Upix3010Camera*) malloc(sizeof(Upix3010Camera));
	GciCamera *camera = (GciCamera*) upix_camera;

	memset(upix_camera, 0, sizeof(Upix3010Camera));

	gci_camera_constructor(camera, name, description);

	GciCmtNewLock ("UPixCamera", 0, &(upix_camera->upix_lock));
	GciCmtNewLock ("UPixCameraGetImageLock", 0, &(upix_camera->get_image_lock));
	GciCmtNewLock ("UPixCameraFlagLock", 0, &(upix_camera->upix_flag_lock));

	camera->_data_mode = BPP24;
	upix_camera->window_hwnd = 0;
	upix_camera->run_mode = UPIX_RUNMODE_STOP;
	upix_camera->initialised = 0;
	upix_camera->camera_stopped = 1;
	upix_camera->camera_number = 1; //camera_number++;
	upix_camera->image_callback_completed = 0;
	upix_camera->resolution = R2048_1536;   // default resolution

	strcpy(upix_camera->default_settings_file_path, "UPix3010DefaultSettings.cam");

	//GetPanelAttribute (camera->_main_ui_panel, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	//upix_camera->window_hwnd = (HWND) window_handle;

	#ifdef UPIX_BUG_WORK_AROUND
	if(upix_camera->window_hwnd == 0)
	{
		int handle;
		upix_camera->hidden_panel = NewPanel(0, "HiddenPanel", 0, 0, 200, 200);
		
		//DisplayPanel(upix_camera->hidden_panel);

		GetPanelAttribute(upix_camera->hidden_panel, ATTR_SYSTEM_WINDOW_HANDLE, &handle);

		upix_camera->window_hwnd = (HWND) handle;
	}
	#endif

	CAMERA_VTABLE_PTR(camera).initialise = gci_upix_uc3010_camera_initialise;
	CAMERA_VTABLE_PTR(camera).set_default_settings = gci_upix_uc3010_camera_set_default_settings;
	CAMERA_VTABLE_PTR(camera).destroy = gci_upix_uc3010_camera_destroy;
	CAMERA_VTABLE_PTR(camera).power_up = gci_upix_uc3010_camera_power_up;
	CAMERA_VTABLE_PTR(camera).power_down = gci_upix_uc3010_camera_power_down;
	CAMERA_VTABLE_PTR(camera).set_exposure_time = gci_upix_uc3010_camera_set_exposure_time;
	CAMERA_VTABLE_PTR(camera).set_gain = gci_upix_uc3010_camera_set_gain; 
	CAMERA_VTABLE_PTR(camera).get_gain = gci_upix_uc3010_camera_get_gain;
	CAMERA_VTABLE_PTR(camera).set_live_mode = gci_upix_uc3010_camera_set_live_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_mode = gci_upix_uc3010_camera_set_snap_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_sequence_mode = gci_upix_uc3010_camera_set_snap_sequence_mode;
	CAMERA_VTABLE_PTR(camera).get_colour_type = gci_upix_uc3010_get_colour_type;
	CAMERA_VTABLE_PTR(camera).get_image = gci_upix_uc3010_camera_get_image; 
	CAMERA_VTABLE_PTR(camera).get_size_position = gci_upix_uc3010_get_size_position;
	CAMERA_VTABLE_PTR(camera).get_info =	gci_upix_uc3010_camera_get_info;
	CAMERA_VTABLE_PTR(camera).set_datamode =	gci_upix_uc3010_camera_set_data_mode;
	CAMERA_VTABLE_PTR(camera).set_trigger_mode =	gci_upix_uc3010_camera_set_trigger_mode; 
	CAMERA_VTABLE_PTR(camera).save_settings = gci_upix_uc3010_save_settings;
	CAMERA_VTABLE_PTR(camera).load_settings = gci_upix_uc3010_load_settings;
	CAMERA_VTABLE_PTR(camera).save_settings_as_default =  gci_upix_uc3010_save_settings_as_default;
	CAMERA_VTABLE_PTR(camera).save_state = gci_upix_uc3010_camera_save_state;
	CAMERA_VTABLE_PTR(camera).restore_state = gci_upix_uc3010_camera_restore_state;
	CAMERA_VTABLE_PTR(camera).get_highest_datamode = gci_upix_uc3010_camera_get_highest_datamode;
	CAMERA_VTABLE_PTR(camera).get_lowest_datamode = gci_upix_uc3010_camera_get_lowest_datamode;
	CAMERA_VTABLE_PTR(camera).set_fast_mode = gci_upix_uc3010_camera_set_fast_mode;
	CAMERA_VTABLE_PTR(camera).supports_fast_mode = gci_upix_uc3010_supports_fast_mode;
	CAMERA_VTABLE_PTR(camera).get_focal_indicator_settings = upix_get_focal_indicator_settings;
	CAMERA_VTABLE_PTR(camera).supports_focal_indicator_settings = upix_supports_focal_indicator_settings;
	CAMERA_VTABLE_PTR(camera).attempt_recovery = upix_attempt_recovery;

	return camera;
}



