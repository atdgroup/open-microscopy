#include "gci_dcam_camera.h"
#include "uir_files\gci_camera_ui.h"
#include "gci_dcam_camera_ui.h" 
#include "gci_dcam_camera_settings.h"
#include "gci_dcam_camera_lowlevel.h" 
#include "string_utils.h"
#include "gci_utils.h"

#include "dcamapi.h"
#include "features.h"
#include "dcamapix.h"
#include "dcamprop.h"

#include "toolbox.h"

#include "FreeImageAlgorithms_IO.h"
#include "icsviewer_signals.h"

#ifdef POWER_VIA_I2C
#include "power.h"
#endif

#include <utility.h>
#include <formatio.h>

#define DONT_PROFILE
#include "profile.h"

//////////////////////////////////////////////////////////////////////////////////
// Camera module for the Hamatsu DCam camera
// GP/RJL 2004 - 2006
//////////////////////////////////////////////////////////////////////////////////
// RJ Locke, 18 Nov 2004
// Change macro MICROFOCUS_DATA_DIR to GetProjectDir() etc. 
//////////////////////////////////////////////////////////////////////////////////

static unsigned char number_of_gci_dcam_cameras = 0;

static int ROUND_TO_FACTOR(int value, int factor) {
	
	volatile int tmp;

	tmp = value / factor;
	return tmp * factor;
}

int gci_dcam_camera_set_default_settings (GciCamera* camera)
{
	int file_size;
	char filepath[GCI_MAX_PATHNAME_LEN] = "", data_dir[GCI_MAX_PATHNAME_LEN] = "";
	
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;

	gci_camera_get_data_dir(camera, data_dir);

	sprintf(filepath, "%s\\%s", data_dir, gci_dcam_camera->default_settings_file_path);

	if(FileExists(filepath, &file_size)) {

		if(gci_camera_load_settings(camera, gci_dcam_camera->default_settings_file_path) == CAMERA_ERROR)
			return CAMERA_ERROR;

		gci_dcam_idle(gci_dcam_camera);

		camera->_data_mode = BPP8;
		gci_camera_set_data_mode(camera, camera->_data_mode);
	
		if(gci_camera_feature_enabled(camera, CAMERA_FEATURE_LIGHTMODE)) {
			gci_dcam_camera->_light_mode = LIGHT_MODE_HI;
			gci_camera_set_light_mode(camera, gci_dcam_camera->_light_mode);
		}
	
		gci_camera_set_exposure_time(camera, 40);  // 40 ms     
		
		gci_camera_set_gain(camera, CAMERA_ALL_CHANNELS, 0);
	
		gci_dcam_camera->_binning = 1;
	
		if( gci_camera_set_binning_mode(camera, gci_dcam_camera->_binning) == CAMERA_ERROR)
			return CAMERA_ERROR;
	
		if( gci_camera_set_size_position(camera, 0, 0, camera->_max_width, camera->_max_height, 0) == CAMERA_ERROR)  
			return CAMERA_ERROR;
	
		gci_dcam_camera->_black_level = 255.0;   // 255 = 0%
	
		if( gci_camera_set_blacklevel(camera, CAMERA_ALL_CHANNELS, gci_dcam_camera->_black_level) == CAMERA_ERROR)
			return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}


static int gci_dcam_camera_save_settings_as_default (GciCamera* camera)
{
	char path[GCI_MAX_PATHNAME_LEN];
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;     
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			
	if (ConfirmPopup ("Confirm Save as Defaults",
		"Are you sure?\n The these values will be used every time\n this application is started.") != 1) {
		
		return CAMERA_ERROR;
	}
		
	GetPrivateDataFolder("Microscope Data", path);
	strcat(path, gci_dcam_camera->default_settings_file_path);
	gci_camera_save_settings(camera, path, "w");  
	
	return CAMERA_SUCCESS;
}


static int  gci_dcam_camera_set_exposure_time(GciCamera* camera, double exposure, double *actual_exposure)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	double exposure_time;

	if(camera == NULL)
		return CAMERA_ERROR;
	
	// Passed Milli Seconds
	if ((exposure < 0.05) || (exposure > 100000))
		return CAMERA_ERROR;
	
	// Takes seconds
	if (!dcam_setexposuretime(gci_dcam_camera->_hCam, exposure / 1000.0))
		return CAMERA_ERROR;
	
	if (!dcam_getexposuretime(gci_dcam_camera->_hCam, &exposure_time))
		return CAMERA_ERROR;

	*actual_exposure = (double) RoundRealToNearestInteger(exposure_time * 1000.0);
	gci_dcam_camera->_timeout = (int)(exposure_time * 1000.0) + 2000;
	
	//We have to delay before sending another command to the camera
	//The delay is proportional to the exposure with a max of 1 second
	//Is this still true
//	Delay( min(1.0, (exposure * 1000.0) /3.0) ); 
	
	return CAMERA_SUCCESS;
}

static int gci_dcam_camera_power_up(GciCamera* camera)
{
	int ret;
	char path[GCI_MAX_PATHNAME_LEN] = "";
	double actual_exposure_time = 0.0;

    ret = DCamPowerUp(camera); 
  
	if(ret == CAMERA_ERROR)
		return CAMERA_ERROR;

	// Create ad find properties for the camera.
//	properties_dialog_new((GciDCamCamera *)camera);

	// Set a default exposure
	gci_dcam_camera_set_exposure_time(camera, 50.0, &actual_exposure_time);

	return ret;
}


static int gci_dcam_camera_power_down(GciCamera* camera)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	// Make sure the sensitity of the camera is 0 for cameras that support it.
	// Or the camera can be damaged even if turned off.
	if(gci_camera_feature_enabled (camera, CAMERA_FEATURE_SENSITIVITY))	 {
		if(gci_camera_set_sensitivity(camera, CAMERA_ALL_CHANNELS, 0.0) == CAMERA_ERROR) {

			GCIDialogNoButtons (0, "Camera Warning", IDI_WARNING,
				"Failed to set camera sensitivity to 0. This could damage the camera even if it is off");   
		}
	}

	if (!dcam_close(gci_dcam_camera->_hCam)) {
	
		send_error_text(camera, "Failed to close camera");
		return CAMERA_ERROR;
	}
	
	if(!dcam_uninit(gci_dcam_camera->_ghInstance, NULL)) {
	
		send_error_text(camera, "gci_dcam_uninit Failed");
		return CAMERA_ERROR;
	}
	
	camera->_powered_up = 0;
	
	#ifdef POWER_VIA_I2C

		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		
		if (GCI_ConfirmPopup("", "Do you want to switch the camera off?")) {
			
			GCI_PowerCameraOff();
		}
		
	#endif
	
	return CAMERA_SUCCESS;
}


static int gci_dcam_camera_save_state(GciCamera* camera, CameraState *state)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	
	state->data_mode = camera->_data_mode;
	state->exposure_time = camera->_exposure_time;
    state->gain1 = camera->_gain;
    state->binning = gci_dcam_camera->_binning;   
    state->blacklevel1 = gci_dcam_camera->_black_level;   
	state->sensitivity = gci_dcam_camera->_sensitivity;
	state->light_mode = gci_dcam_camera->_light_mode;
	state->live = gci_camera_is_live_mode(camera);
	
	return CAMERA_SUCCESS;
}


static int gci_dcam_camera_restore_state(GciCamera* camera, CameraState *state)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if(gci_dcam_camera->saved_settings == NULL) {

		send_error_text(camera, "gci_dcam_camera_restore_state failed (previous state not saved)");
		return CAMERA_ERROR;	
	}
	
	gci_camera_set_snap_mode(camera);
	gci_camera_set_data_mode(camera, state->data_mode);
	gci_camera_set_exposure_time(camera, state->exposure_time);
	gci_camera_set_gain(camera, CAMERA_ALL_CHANNELS, state->gain1);
	gci_camera_set_binning_mode(camera, state->binning);
	gci_camera_set_blacklevel(camera, CAMERA_ALL_CHANNELS, state->blacklevel1);
	gci_camera_set_sensitivity(camera, CAMERA_ALL_CHANNELS, state->sensitivity);
	gci_camera_set_light_mode(camera, state->light_mode);

	if(state->live) {
		gci_camera_set_live_mode(camera);
		gci_camera_activate_live_display(camera);
	}
		
	return CAMERA_SUCCESS;
}


static BinningMode gci_dcam_camera_get_binning_mode(GciCamera* camera)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	return gci_dcam_camera->_binning;
}


static int  gci_dcam_camera_set_gain(GciCamera* camera, CameraChannel c, double gain)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	double sensitivity;

	if ((gain < 0) || (gain > 255))
		return CAMERA_ERROR;
	
	if(gci_camera_feature_enabled (camera, CAMERA_FEATURE_SENSITIVITY))	 {

		gci_dcam_camera_get_sensitivity(camera, c, &sensitivity);

		if(sensitivity > 0 && gain < (double) gci_dcam_camera->_gain_safe_preset_for_sensitivity) {

			logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING,
				"User set gain below safe threshold - setting sensitivity to 0.0");  
	
			if(gci_dcam_camera_set_sensitivity(camera, c, 0.0) == CAMERA_ERROR)
				return CAMERA_ERROR;
		}	
	}

	if (DCamSetFeature (gci_dcam_camera, DCAM_IDFEATURE_GAIN, (double)RoundRealToNearestInteger(gain))==CAMERA_ERROR)
		return CAMERA_ERROR;

	camera->_gain = gain;
	
	return CAMERA_SUCCESS;
}


static int  gci_dcam_camera_get_gain(GciCamera* camera, CameraChannel channel, double *gain)
{
	// setting the gain this way does not specify which tap - so set both
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;  

	*gain = camera->_gain;
	
	return CAMERA_SUCCESS;
}

static int gci_dcam_camera_set_live_mode(GciCamera* camera)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	
	if( gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR) 
		return CAMERA_ERROR; 
	
	if (gci_dcam_precapture(gci_dcam_camera, ccCapture_Sequence) == CAMERA_ERROR) 
		return CAMERA_ERROR;
			
	// gci_dcam_set_to_ready_mode contains the slow gci_dcam_allocframe function.
	if (gci_dcam_set_to_ready_mode(gci_dcam_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
		
	// We have waited for the READY state by now
	gci_dcam_capture(gci_dcam_camera);
					
	return CAMERA_SUCCESS;
}


static int gci_dcam_camera_set_snap_sequence_mode(GciCamera* camera)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if( gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR) 
			return CAMERA_ERROR; 

	if (gci_dcam_precapture(gci_dcam_camera, ccCapture_Snap) == CAMERA_ERROR)
		return CAMERA_ERROR;

	// gci_dcam_set_to_ready_mode contains the slow gci_dcam_allocframe function.
	if (gci_dcam_set_to_ready_mode(gci_dcam_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
		
	return CAMERA_SUCCESS;
}


static int gci_dcam_camera_set_snap_mode(GciCamera* camera)
{
	gci_dcam_camera_set_snap_sequence_mode(camera);
	
	return CAMERA_SUCCESS;
}

static FIBITMAP * gci_dcam_camera_get_image(GciCamera* camera, const Rect *rect)
{
	return DCamGetImage(camera, rect);   
}

int gci_dcam_camera_get_nearest_subwindow_rect(GciCamera* camera, Rect *rect)
{
	unsigned int binning;
	int binned_image_width, binned_image_height;
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	binning = gci_camera_get_binning_mode(camera);
    
	binned_image_width = camera->_max_width / binning;
	binned_image_height = camera->_max_height / binning;

	if(rect->width > binned_image_width)
		rect->width = binned_image_width;

	if(rect->height > binned_image_height)
		rect->height = binned_image_height;

	if(camera->_horz_flip) {		
		rect->left = binned_image_width - rect->left - rect->width;	
	}

	if(camera->_vert_flip) {		
		rect->top = binned_image_height - rect->top - rect->height;	
	}

	if(binning == NO_BINNING) {
		rect->left = ROUND_TO_FACTOR(rect->left, 8);
		rect->top = ROUND_TO_FACTOR(rect->top, 8);
		rect->height = ROUND_TO_FACTOR(rect->height, 8);
	}
	else if(binning == BINNING2X2){
		rect->left = ROUND_TO_FACTOR(rect->left, 4);
		rect->top = ROUND_TO_FACTOR(rect->top, 4);
		rect->height = ROUND_TO_FACTOR(rect->height, 4);
	}
	else if(binning == BINNING4X4){
		rect->left = ROUND_TO_FACTOR(rect->left, 2);
		rect->top = ROUND_TO_FACTOR(rect->top, 2);
		rect->height = ROUND_TO_FACTOR(rect->height, 2);
	}
	else if(binning == BINNING8X8){
		rect->left = ROUND_TO_FACTOR(rect->left, 1);
		rect->top = ROUND_TO_FACTOR(rect->top, 1);
		rect->height = ROUND_TO_FACTOR(rect->height, 1);
	}

	// Force width to be th required size
	rect->width = ROUND_TO_FACTOR(rect->width, 32 / binning);
    
	return CAMERA_SUCCESS;
}

static int gci_dcam_camera_set_size_position(GciCamera* camera, unsigned int left, unsigned int top,
																unsigned int width, unsigned int height, unsigned char auto_centre)
{
	Rect rect;
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

  	gci_dcam_free_camera_resources(gci_dcam_camera);
    
	RectSet (&rect, top, left, height, width);

	gci_dcam_camera_get_nearest_subwindow_rect(camera, &rect);

    if ( DCamSetSubarray(gci_dcam_camera, rect.left, rect.top, rect.width, rect.height) == CAMERA_ERROR ) {

		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "gci_dcam_camera_set_size_position failed");  

		return CAMERA_ERROR;
    }
    
	return CAMERA_SUCCESS;
}


static int gci_dcam_camera_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
																unsigned int *width, unsigned int *height, unsigned char *auto_centre)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	// SubWindow vals may have been changed by GCI_DCamSetSubarray
    if ( DCamGetSubarray(gci_dcam_camera, left, top, width, height) == CAMERA_ERROR ) {
    
		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "gci_dcam_camera_get_size_position failed");  

		return CAMERA_ERROR;
    }
	
	if(camera->_horz_flip) {
		
		*left = camera->_max_width - *left - *width;	
	}

	return CAMERA_SUCCESS;
}

static int gci_dcam_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = camera->_min_width;
	*height = camera->_min_height;

	return CAMERA_SUCCESS;
}


static int  gci_dcam_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = camera->_max_width;
	*height = camera->_max_height;

	return CAMERA_SUCCESS;
}


static int gci_dcam_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus, char *camera_id, char *camera_version, char *driver_version)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if (vendor != NULL) {
	
		if (!dcam_getmodelinfo(gci_dcam_camera->_index, DCAM_IDSTR_VENDOR, vendor, 64)) {
			send_error_text(camera, "Failed to get vendor info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (model != NULL) {
	
		if (!dcam_getmodelinfo(gci_dcam_camera->_index, DCAM_IDSTR_MODEL, model, 64)) {
			send_error_text(camera, "Failed to get model info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (bus != NULL) {
	
		if (!dcam_getmodelinfo(gci_dcam_camera->_index, DCAM_IDSTR_BUS, bus, 64)) {
			send_error_text(camera, "Failed to get bus info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (camera_id != NULL) {
	
		if (!dcam_getmodelinfo(gci_dcam_camera->_index, DCAM_IDSTR_CAMERAID, camera_id, 64)) {
			send_error_text(camera, "Failed to get camera ID info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (camera_version != NULL) {
	
		if (!dcam_getmodelinfo(gci_dcam_camera->_index, DCAM_IDSTR_CAMERAVERSION, camera_version, 64)) {
			send_error_text(camera, "Failed to get camera version info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (driver_version != NULL) {
	
		if (!dcam_getmodelinfo(gci_dcam_camera->_index, DCAM_IDSTR_DRIVERVERSION, driver_version, 64)) {
			send_error_text(camera, "Failed to get driver version info.");
			return CAMERA_ERROR;
		}	
	}
	
	return CAMERA_SUCCESS;
}


int gci_dcam_camera_set_data_mode(GciCamera* camera, DataMode data_mode)
{
	int was_live = 0;
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if(gci_dcam_camera == NULL)
		return CAMERA_ERROR;

	was_live = gci_camera_is_live_mode(camera);
	
	if(was_live)
		gci_camera_set_snap_mode(camera);

	if (gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
	
	if( DCamSetDataType(gci_dcam_camera, data_mode) == CAMERA_ERROR ) {
	
		send_error_text(camera, "DCamSetDataType function failed");
		return CAMERA_ERROR;
	}
  
  	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, data_mode);
  
	if(was_live)
		gci_camera_set_live_mode(camera);

	return CAMERA_SUCCESS;
}

int gci_dcam_camera_get_lowest_data_mode(GciCamera* camera, DataMode *data_mode)
{
	int i;

	*data_mode = BPP32;
	for (i=0; i<camera->number_of_datamodes; i++){
		if (camera->supported_datamodes[i] < *data_mode) *data_mode = camera->supported_datamodes[i];
	}
	
	return CAMERA_SUCCESS;
}

int gci_dcam_camera_get_highest_data_mode(GciCamera* camera, DataMode *data_mode)
{
	int i;

	*data_mode = BPP8;
	for (i=0; i<camera->number_of_datamodes; i++){
		if (camera->supported_datamodes[i] > *data_mode) *data_mode = camera->supported_datamodes[i];
	}
	
	return CAMERA_SUCCESS;
}

int gci_dcam_camera_get_highest_binning_mode(GciCamera* camera, BinningMode *binning)
{
	int i;

	*binning = NO_BINNING;
	for (i=0; i<camera->number_of_binning_modes; i++){
		if (camera->supported_binning_modes[i] > *binning) *binning = camera->supported_binning_modes[i];
	}
	
	return CAMERA_SUCCESS;
}

static int gci_dcam_camera_set_light_mode(GciCamera* camera, LightMode light_mode)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if(gci_dcam_camera == NULL)
		return CAMERA_ERROR;

	if(!gci_camera_feature_enabled(camera, CAMERA_FEATURE_LIGHTMODE))
		return CAMERA_SUCCESS;

	if( (light_mode != LIGHT_MODE_LO) && (light_mode != LIGHT_MODE_HI) ) {
	
		send_error_text(camera, "Light mode not valid");
		return CAMERA_ERROR;
	}
	
	if (DCamSetFeature (gci_dcam_camera, DCAM_IDFEATURE_LIGHTMODE, (double)light_mode) == CAMERA_ERROR ) {
		send_error_text(camera, "DCamSetLightMode function failed");
		return CAMERA_ERROR;
	}
  
	gci_dcam_camera->_light_mode = light_mode;

  	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_LIGHTMODE, light_mode);
  
	return CAMERA_SUCCESS;
}

int  gci_dcam_camera_get_light_mode(GciCamera* camera)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	return gci_dcam_camera->_light_mode;
}

int gci_dcam_camera_set_external_trigger_mode(GciDCamCamera* gci_dcam_camera) 
{
	GciCamera *camera = (GciCamera *) gci_dcam_camera; 

	if(gci_dcam_camera->_trigger_mode == CAMERA_EXTERNAL_TRIG)
		return CAMERA_SUCCESS;

// setting trigger can be done in any state
	if (gci_dcam_camera->_require_free_resources==1)
		if (gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR)
			return CAMERA_ERROR;

	if (!dcam_settriggermode(gci_dcam_camera->_hCam, DCAM_TRIGMODE_EDGE)) {
	
        send_error_text(camera, "gci_dcam_settriggermode failed");
        return CAMERA_ERROR;
    }
	
	if (!dcam_settriggerpolarity(gci_dcam_camera->_hCam, DCAM_TRIGPOL_NEGATIVE)) {
	
        send_error_text(camera, "gci_dcam_settriggerpolarity failed");
        return CAMERA_ERROR;
    }
	
	gci_dcam_camera->_trigger_mode = CAMERA_EXTERNAL_TRIG;
	
	return CAMERA_SUCCESS;
}


int gci_dcam_camera_set_internal_trigger_mode(GciDCamCamera* gci_dcam_camera) 
{
	unsigned long error;
	char buffer[255], string_buffer[255];
	GciCamera *camera = (GciCamera *) gci_dcam_camera; 

	if(gci_dcam_camera->_trigger_mode == CAMERA_INTERNAL_TRIG || gci_dcam_camera->_trigger_mode == CAMERA_NO_TRIG)
		return CAMERA_SUCCESS;

// setting trigger can be done in any state
	if (gci_dcam_camera->_require_free_resources==1)
		if (gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR)
			return CAMERA_ERROR; 

	if (!dcam_settriggermode(gci_dcam_camera->_hCam, DCAM_TRIGMODE_INTERNAL)) {
	
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
	
		gci_dcam_error_code_to_string(error, string_buffer);  
	
		send_error_text(camera, "gci_dcam_settriggermode failed: %s", string_buffer);

        return CAMERA_ERROR;
    }
	
	gci_dcam_camera->_trigger_mode = CAMERA_INTERNAL_TRIG;
	
	return CAMERA_SUCCESS;
}

static int gci_dcam_camera_fire_internal_trigger(GciCamera* camera) 
{
	unsigned long error;
	char buffer[255] = "", string_buffer[255];
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	//Software trigger
	if (!dcam_firetrigger(gci_dcam_camera->_hCam)) {
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "gci_dcam_settriggermode failed: %s", string_buffer);
        return CAMERA_ERROR;
    }
	return CAMERA_SUCCESS;
}

int gci_dcam_camera_is_external_trigger_mode(GciDCamCamera* gci_dcam_camera)
{
	return gci_dcam_camera->_trigger_mode;
}

static int gci_dcam_camera_set_trigger_mode(GciCamera* camera, TriggerMode trig_mode)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if (trig_mode == CAMERA_EXTERNAL_TRIG)
		return gci_dcam_camera_set_external_trigger_mode(gci_dcam_camera);
	
	return gci_dcam_camera_set_internal_trigger_mode(gci_dcam_camera);
}


int gci_dcam_supports_binning(GciCamera* camera)
{
	return 1;
}


static int gci_dcam_camera_set_binning_mode(GciCamera* camera, BinningMode binning)
{
	int was_live = 0;
	long error;
	char error_string[255] = "";
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if ((binning != 1) && (binning != 2) && (binning != 4) && (binning != 8))
		return CAMERA_ERROR;
   	
	was_live = gci_camera_is_live_mode(camera);
	
	if(was_live)
		gci_camera_set_snap_mode(camera);

   	if( gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR) {

		send_error_text(camera, "gci_dcam_free_camera_resources failed");
		return CAMERA_ERROR;	
	}
   
	gci_dcam_camera->_binning = binning;
	
	if (!dcam_setbinning(gci_dcam_camera->_hCam, binning)) {
	
		error = dcam_getlasterror(gci_dcam_camera->_hCam, error_string, 255);
	
		send_error_text(camera, "Failed to set binning mode: erro: %d, %s", error, error_string);
		return CAMERA_ERROR;
	}	
	
	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BINNING, binning);
	
	GCI_ImagingWindow_SetBinningSize(camera->_camera_window, binning);  
  
	if(was_live)
		gci_camera_set_live_mode(camera);

	return CAMERA_SUCCESS;
}


int gci_dcam_camera_set_blacklevel(GciCamera* camera, CameraChannel c, double black_level)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
		
	if ((black_level < 0) || (black_level > 255))
		return CAMERA_ERROR; 

	gci_dcam_camera->_black_level = black_level;

	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, black_level);

	return DCamSetFeature(gci_dcam_camera, DCAM_IDFEATURE_BRIGHTNESS, (double)RoundRealToNearestInteger(black_level));
}


int  gci_dcam_camera_get_blacklevel(GciCamera* camera, CameraChannel c, double *val)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if(gci_dcam_camera == NULL)
		return CAMERA_ERROR;
	
	*val =gci_dcam_camera->_black_level; 
	
	return CAMERA_SUCCESS;
}

int gci_dcam_camera_set_sensitivity(GciCamera* camera, CameraChannel c, double sensitivity)
{
	double gain = 0.0;
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
		
	if ((sensitivity < 0) || (sensitivity > 255))
		return CAMERA_ERROR; 

	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_SENSITIVITY))
		return CAMERA_SUCCESS;

	// Sensitivity can not be greater than 0 if the normal gain < safe preset
	// specified in the config file.
	// Otherwise the EM Camera can be damaged
	gci_camera_get_gain(camera, CAMERA_CHANNEL1, &gain);

	if(sensitivity > 0 && gain < (double) gci_dcam_camera->_gain_safe_preset_for_sensitivity) {

		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING,
			"User setting sensitivity while gain < safe threshold - setting sensitivity to 0.0");  

		gci_dcam_camera->_sensitivity = 0.0;
		
		if(camera->_extra_ui_panel != -1)
			SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SENSITIVITY, 0.0);

		return CAMERA_SUCCESS; 
	}	

	gci_dcam_camera->_sensitivity = sensitivity;

	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_SENSITIVITY, sensitivity);

	return DCamSetFeature(gci_dcam_camera, DCAM_IDFEATURE_SENSITIVITY, (double)RoundRealToNearestInteger(sensitivity));
}


int  gci_dcam_camera_get_sensitivity(GciCamera* camera, CameraChannel c, double *val)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;

	if(gci_dcam_camera == NULL)
		return CAMERA_ERROR;
	
	*val =gci_dcam_camera->_sensitivity; 
	
	return CAMERA_SUCCESS;
}

// DCAM_IDPROP_DARKCALIB_SAMPLES is not defined in older sdk's
#ifndef DCAM_IDPROP_DARKCALIB_SAMPLES
#define DCAM_IDPROP_DARKCALIB_SAMPLES 0
#endif

int gci_dcam_camera_acquire_subtract_reference(GciCamera* camera)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	int event = DCAM_EVENT_CYCLEEND;

	SetWaitCursor(1);

	// Set dark calib samples	
	if (!dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_DARKCALIB_SAMPLES, 1)){
		char buffer[256], string_buffer[256];
		int error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		SetWaitCursor(0);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_setpropertyvalue (DCAM_IDPROP_DARKCALIB_SAMPLES) failed: %s", string_buffer);
		return CAMERA_ERROR;
	}

	// Set dark calib mode	
	if (!dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_CAPTUREMODE, DCAMPROP_CAPTUREMODE__DARKCALIB)){
		char buffer[256], string_buffer[256];
		int error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		SetWaitCursor(0);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_setpropertyvalue (DCAM_IDPROP_CAPTUREMODE) failed: %s", string_buffer);
		return CAMERA_ERROR;
	}

	// acquire the image
	if (gci_dcam_capture(gci_dcam_camera) == CAMERA_ERROR) {   // dcam_capture and wait for busy
		// try to set normal mode
		gci_dcam_idle(gci_dcam_camera);
		gci_dcam_wait_for_ready(gci_dcam_camera);
		dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_CAPTUREMODE, DCAMPROP_CAPTUREMODE__NORMAL);
		SetWaitCursor(0);
		return CAMERA_ERROR;
	}
	
	if (!dcam_wait(gci_dcam_camera->_hCam, &event, 60000, NULL) ) {   
		int error;
		char buffer[256] = "";

		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		gci_dcam_error_code_to_string(error, buffer);  

		// try to set normal mode
		gci_dcam_idle(gci_dcam_camera);
		gci_dcam_wait_for_ready(gci_dcam_camera);
		dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_CAPTUREMODE, DCAMPROP_CAPTUREMODE__NORMAL);
		SetWaitCursor(0);
		return CAMERA_ERROR;
	}

	if (gci_dcam_idle(gci_dcam_camera)){
		// try to set normal mode
		gci_dcam_wait_for_ready(gci_dcam_camera);
		dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_CAPTUREMODE, DCAMPROP_CAPTUREMODE__NORMAL);
		SetWaitCursor(0);
		return CAMERA_ERROR;
	}

	if (gci_dcam_wait_for_ready(gci_dcam_camera)){
		// try to set normal mode
		dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_CAPTUREMODE, DCAMPROP_CAPTUREMODE__NORMAL);
		SetWaitCursor(0);
		return CAMERA_ERROR;
	}

	// Set normal mode
	if (!dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_CAPTUREMODE, DCAMPROP_CAPTUREMODE__NORMAL)){
		char buffer[256], string_buffer[256];
		int error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_setpropertyvalue (DCAM_IDPROP_CAPTUREMODE) failed: %s", string_buffer);
		SetWaitCursor(0);
		return CAMERA_ERROR;
	}

	// Save buffer to flash memory 0
	if (!dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_STORESUBTRACTIMAGETOMEMORY, 1)){
		char buffer[256], string_buffer[256];
		int error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_setpropertyvalue (DCAM_IDPROP_STORESUBTRACTIMAGETOMEMORY) failed: %s", string_buffer);
		SetWaitCursor(0);
		return CAMERA_ERROR;
	}

	SetWaitCursor(0);

	return CAMERA_SUCCESS;
}

int gci_dcam_camera_enable_subtract(GciCamera* camera, int enable)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	BOOL ret;

	if (enable) {
		ret = dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_SUBTRACT, DCAMPROP_MODE__ON );
	}
	else {
		ret = dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_SUBTRACT, DCAMPROP_MODE__OFF );
	}

	if (!ret){
		char buffer[256], string_buffer[256];
		int error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_setpropertyvalue (DCAM_IDPROP_SUBTRACT) failed: %s", string_buffer);
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int gci_dcam_camera_setoffset(GciCamera* camera, int offset)
{
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
	BOOL ret;
	const int min=0, max=10000; // from dcam Property List document

	if (offset<min) offset=min;
	if (offset>max) offset=max;

	ret = dcam_setpropertyvalue( gci_dcam_camera->_hCam, DCAM_IDPROP_SUBTRACTOFFSET, offset);

	if (!ret){
		char buffer[256], string_buffer[256];
		int error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
		gci_dcam_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_setpropertyvalue (DCAM_IDPROP_SUBTRACTOFFSET) failed: %s", string_buffer);
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

void dcamcon_show_dcamerr( HDCAM hdcam, const char* apiname, const char* fmt, ...  )
{
	char	buf[ 256 ];
	long	err = 0;

	memset( buf, 0, sizeof( buf ) );

	// get error information
	err = dcam_getlasterror( hdcam, buf, sizeof( buf ) );
	printf( "failure: %s returns 0x%08X\n", apiname, err );
	if( buf[ 0 ] )	printf( "%s\n", buf );

	if( fmt != NULL )
	{
		va_list	arg;
		va_start(arg,fmt);
		vprintf( fmt, arg );
		va_end(arg);
	}
}

BOOL dcamcon_enable_emgain_protect( HDCAM hdcam )
{
	double	value;
	if( ! dcam_getpropertyvalue( hdcam, DCAM_IDPROP_EMGAINWARNING_STATUS, &value ) )
	{
		dcamcon_show_dcamerr( hdcam, "dcam_getpropertyvalue( DCAM_IDPROP_EMGAINWARNING_STATUS )", "" );
		return FALSE;
	}

	if( ! dcam_setpropertyvalue( hdcam, DCAM_IDPROP_EMGAINWARNING_ALARM, DCAMPROP_MODE__ON ) )
		dcamcon_show_dcamerr( hdcam, "dcam_setpropertyvalue( DCAM_IDPROP_EMGAINWARNING_ALARAM )", "does not support DCAMPROP_MODE__ON\n" );

	if( ! dcam_setpropertyvalue( hdcam, DCAM_IDPROP_EMGAINPROTECT_MODE, DCAMPROP_MODE__ON ) )
		dcamcon_show_dcamerr( hdcam, "dcam_setpropertyvalue( DCAM_IDPROP_EMGAINPROTECT_MODE )", "does not support DCAMPROP_MODE__ON\n" );

//	you can use default DCAM_IDPROP_EMGAINWARNING_LEVEL as strongest protection.
//	you can use default DCAM_IDPROP_EMGAINPROTECT_FRAMESAFTER as strongest protection.

	return TRUE;
}


static int gci_dcam_camera_set_ccd_mode(GciCamera* camera, CCDMode mode)
{
	int pnl;
	double start_time;
	GciDCamCamera *gci_dcam_camera = (GciDCamCamera *) camera;
		
	if ((mode != CCD_MODE_NORMAL) && (mode != CCD_MODE_EM))
		return CAMERA_ERROR; 

	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_CCD_MODE))
		return CAMERA_SUCCESS;

	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_CCDMODE, mode);

	// Must be in snap mode to do this
	gci_camera_set_snap_mode(camera);

	pnl = GCIDialogNoButtons (0, "Changing Mode", IDI_INFORMATION, "Changing CCD Mode. Please Wait");
	start_time = Timer();
	

	if( ! dcamcon_enable_emgain_protect( gci_dcam_camera->_hCam ) )
		{
			printf( "This camera does not support EM GAIN PROTECT.\n" );
		}


	if(mode == CCD_MODE_EM) {

		dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_CCDMODE, DCAMPROP_CCDMODE__EMCCD );         	
	}
	else {

		dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_CCDMODE, DCAMPROP_CCDMODE__NORMALCCD);    
	}

	// Must wait three seconds
	while(Timer() - start_time < 3.0) {
		Delay(0.2);
		ProcessSystemEvents();
	}

	
	 /*
	if(gci_camera_feature_enabled(camera, CAMERA_FEATURE_EMGAINPROTECT)) {
		// turn on the protection of the ImagEM v2 cameras

		// CURRENTLY DOES NOT WORK
		char buffer[255] = "", string_buffer[255] = "";

		GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;
		BOOL error;
        		
		error = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_EMGAINPROTECT_MODE, DCAMPROP_MODE__ON);
		
		error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
	
		gci_dcam_error_code_to_string(error, string_buffer);  
	
		send_error_text(camera, "dcam_setpropertyvalue failed: %s", string_buffer);

		
		
		
		error = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_EMGAINWARNING_ALARM, DCAMPROP_MODE__ON);         
	
			error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
	
		gci_dcam_error_code_to_string(error, string_buffer);  
	
		send_error_text(camera, "dcam_setpropertyvalue failed: %s", string_buffer);

		
		error = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_EMGAINWARNING_LEVEL, 3); 



			error = dcam_getlasterror(gci_dcam_camera->_hCam, buffer, 255);
	
		gci_dcam_error_code_to_string(error, string_buffer);  
	
		send_error_text(camera, "dcam_setpropertyvalue failed: %s", string_buffer);


		error = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_EMGAINPROTECT_AFTERFRAMES, 1); 	         
	}
*/

	DiscardPanel(pnl);

	return CAMERA_SUCCESS;
}

static void  OnRoiChanged (IcsViewerWindow *window, const Point p1, const Point p2, void* data)
{
	GciCamera *camera = (GciCamera *) data;
	
	SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, p1.x);
	SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, p1.y);
	SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, p2.x - p1.x);
	SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, p2.y - p1.y);		
	
}

static int gci_dcam_camera_initialise (GciCamera* camera)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;
	
	int menu_bar;
	int i;
	char text[20] = "";   
	
	if(camera == NULL)
		return CAMERA_ERROR;
	
	gci_camera_set_extra_panel_uir(camera, "gci_dcam_camera_ui.uir", EXTRA_PNL); 

	gci_camera_set_exposure_range(camera, 1, 5000, "ms", VAL_INTEGER);       
	
	// Set blacklevel range
	SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, ATTR_MIN_VALUE, camera->_min_bl);
	SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, ATTR_MAX_VALUE, camera->_max_bl);
	SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, ATTR_DFLT_VALUE, camera->_min_bl); 	
	SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, camera->_min_bl); 	
	
	// Sub Window Max Size
	SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, ATTR_MIN_VALUE, 16);
	SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, ATTR_MAX_VALUE, camera->_max_width);
	SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, ATTR_MIN_VALUE, 16);
	SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, ATTR_MAX_VALUE, camera->_max_height);
	
	GCI_ImagingWindow_SetRoiChangedHandler(camera->_camera_window, OnRoiChanged, camera);   
		
	menu_bar = GetPanelMenuBar (camera->_extra_ui_panel);

	if ( InstallMenuCallback (menu_bar, DCAM_MENU_OPTIONS_PROP, DCAM_Camera_onPropertyMenuClicked, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, DCAM_Camera_onDataMode, gci_dcam_camera) < 0)
		return CAMERA_ERROR;											 
  	
  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_LIGHTMODE, DCAM_Camera_onLightMode, gci_dcam_camera) < 0)
		return CAMERA_ERROR;												
  	
  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_QUIT, DCAM_Camera_onExtrasQuit, gci_dcam_camera) < 0)
		return CAMERA_ERROR;
  	
  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, DCAM_Camera_onBlackLevel, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SENSITIVITY, DCAM_Camera_onSensitivty, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_CCDMODE, DCAM_Camera_onCCDMode, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_ENABLESUBTRACT, DCAM_Camera_onEnableSubtract, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_ACQUIRESUBTRACTREF, DCAM_Camera_onAcquireSubtractRef, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBTRACTOFFSET, DCAM_Camera_onSetSubtractOffset, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_BINNING, DCAM_Camera_onBinning, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_TIMER, DCAM_Camera_TimerTick, gci_dcam_camera) < 0)
		return CAMERA_ERROR;
	
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_PHOTONMODE, DCAM_Camera_SetPhotonMode, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SW_APPLY, DCAM_onSubwindowApplyClicked, gci_dcam_camera) < 0)
		return CAMERA_ERROR;
	
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, DCAM_Camera_onSetSizePosition, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, DCAM_Camera_onSetSizePosition, gci_dcam_camera) < 0)
		return CAMERA_ERROR;
 
    if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, DCAM_Camera_onSetSizePosition, gci_dcam_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, DCAM_Camera_onSetSizePosition, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, DCAM_Camera_onSetSizePosition, gci_dcam_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_PRSETWINDOW, DCAM_Camera_onPresetSubWindow, gci_dcam_camera) < 0)
		return CAMERA_ERROR;	
		
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, DCAM_Camera_onAutoCenter, gci_dcam_camera) < 0)
		return CAMERA_ERROR;	
		 
//	if(dcam_camera_has_properties(camera) == 0) {
		SetMenuBarAttribute(menu_bar, DCAM_MENU_OPTIONS_PROP, ATTR_DIMMED, 1);
//	}

	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_BINNING))
	{
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_BINNING, ATTR_DIMMED, 1);
		camera->_cops.set_binning_mode = NULL;
	}
	
	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_BLACKLEVEL))
	{
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, ATTR_DIMMED, 1);
		camera->_cops.set_blacklevel = NULL;
	}

	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_SENSITIVITY))
	{
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SENSITIVITY, ATTR_DIMMED, 1);
		camera->_cops.set_sensitivity = NULL;
	}
	
	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_GAIN))
	{
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_DIMMED, 1);
		camera->_cops.set_gain = NULL;
	}
	
	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_DATAMODE))
	{
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, ATTR_DIMMED, 1);
		camera->_cops.set_datamode = NULL;
	}
	
	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_LIGHTMODE))
	{
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_LIGHTMODE, ATTR_DIMMED, 1);
		camera->_cops.set_lightmode = NULL;
	}
	
	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_PHOTON_MODE))
	{
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_PHOTONMODE, ATTR_DIMMED, 1);
	}
	
	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_CCD_MODE))
	{
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_CCDMODE, ATTR_DIMMED, 1);
	}

	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_TEMP)) {
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_DEGS_C, 0.0);
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_DEGS_C, ATTR_DIMMED, 1);
	}

	if(!gci_camera_feature_enabled (camera, CAMERA_FEATURE_SUBTRACT)) {
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_ENABLESUBTRACT, ATTR_DIMMED, 1);
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_ACQUIRESUBTRACTREF, ATTR_DIMMED, 1);
		SetCtrlAttribute(camera->_extra_ui_panel, EXTRA_PNL_SUBTRACTOFFSET, ATTR_DIMMED, 1);
	}

	for(i=0; i < camera->number_of_datamodes; i++) {
		
		sprintf (text, "%d bit", (int) camera->supported_datamodes[i]);
		InsertListItem (camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, -1, text, (int) camera->supported_datamodes[i]);			
	}
	
	for(i=0; i < camera->number_of_binning_modes; i++) {
		
		if(camera->supported_binning_modes[i] ==1)
			sprintf (text, "None");  	
		else
			sprintf (text, "by %d", (int) camera->supported_binning_modes[i]);
		
		InsertListItem (camera->_extra_ui_panel, EXTRA_PNL_BINNING, -1, text, (int) camera->supported_binning_modes[i]);			
	}


//	DCAM_IDPROP_EMGAINWARNING_STATUS			= 0x00200260,	/* R/O, mode,	"EM GAIN WARNING STATUS"*/	/* *EMGAINPROTECT* */
//	DCAM_IDPROP_EMGAINWARNING_LEVEL				= 0x00200270,	/* R/W, long,	"EM GAIN WARNING LEVEL"	*/	/* *EMGAINPROTECT* */
///	DCAM_IDPROP_EMGAINWARNING_ALARM				= 0x00200280,	/* R/W, mode,	"EM GAIN WARNING ALARM"	*/	/* *EMGAINPROTECT* */
//	DCAM_IDPROP_EMGAINPROTECT_MODE				= 0x00200290,	/* R/W, mode,	"EM GAIN PROTECT MODE"	*/	/* *EMGAINPROTECT* */
//	DCAM_IDPROP_EMGAINPROTECT_AFTERFRAMES	

	if(gci_camera_feature_enabled(camera, CAMERA_FEATURE_EMGAINPROTECT)) {
		// turn on the protection of the ImagEM v2 cameras

		// CURRENTLY DOES NOT WORK

		GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;
		BOOL error;

		

		error = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_EMGAINWARNING_LEVEL, 3); 
		error = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_EMGAINPROTECT_AFTERFRAMES, 1); 
		error = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_EMGAINWARNING_ALARM, DCAMPROP_MODE__ON);         
		error = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_EMGAINPROTECT_MODE, DCAMPROP_MODE__ON);         
	}

	if(gci_camera_feature_enabled(camera, CAMERA_FEATURE_SENSITIVITY)) {
		
		double min, max, gain_range;
		double gain_safe_preset_for_sensitivity_precentage;
		double max_safe_sensitivity_percentage, sensitivity_range;

		// set the safe ranges of gain and sensitivity, default is no protection
		if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(camera), "GainSafePresetForSensitivityPercentage", &gain_safe_preset_for_sensitivity_precentage)<0)
			gain_safe_preset_for_sensitivity_precentage = 0.0;   
		if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(camera), "MaxSafeSensitivityPercentage", &max_safe_sensitivity_percentage)<0)
			max_safe_sensitivity_percentage = 100.0; 

		gci_camera_get_gain_range(camera, &min, &max);

		gain_range = max - min;
		gci_dcam_camera->_gain_safe_preset_for_sensitivity = (gain_range * gain_safe_preset_for_sensitivity_precentage / 100.0) +  min;
	
		gci_camera_get_sensitivity_range(camera, &min, &max);
		sensitivity_range = max - min;
		gci_dcam_camera->_max_safe_sensitivity = (sensitivity_range * max_safe_sensitivity_percentage / 100.0) +  min;
	}

	gci_camera_set_subwindow(camera, SW_FULL_SQUARE);

	return CAMERA_SUCCESS;
}


static int gci_dcam_save_settings (GciCamera* camera, const char *filepath, const char *mode)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;

	if(gci_dcam_camera == NULL)
		return CAMERA_ERROR;

	if(DCamCameraSaveIniCameraSettings(gci_dcam_camera, filepath, mode) == CAMERA_ERROR) {
	
		send_error_text(camera, "Failed to save dcam settings for device %s.", UIMODULE_GET_DESCRIPTION(camera));
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


static int gci_dcam_load_settings (GciCamera* camera, const char *filepath) 
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;

	if(gci_dcam_camera == NULL)
		return CAMERA_ERROR;

	if( DCamCameraLoadIniCameraSettings(gci_dcam_camera, filepath) == CAMERA_ERROR) {
	
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


static int gci_dcam_get_colour_type (GciCamera* camera)
{
	return MONO_TYPE;
}


int gci_dcam_camera_destroy(GciCamera* camera)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;

	gci_camera_power_down(camera);

	--number_of_gci_dcam_cameras;

	if(gci_dcam_camera->saved_settings != NULL) {
	
		free(gci_dcam_camera->saved_settings);
		gci_dcam_camera->saved_settings = NULL;	
	}
  	
	ui_module_restore_cvi_wnd_proc(gci_dcam_camera->prop_panel_id);

  	return CAMERA_SUCCESS;
}


double  gci_dcam_camera_get_temperature(GciCamera* camera)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;  
	double temperature = 0.0;
	int supported = 0;
	
	dcam_getpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_SENSORTEMPERATURE, &temperature);      
	
	return temperature;
}

int  gci_dcam_camera_set_photon_mode(GciCamera* camera, int mode)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera; 

	int ret = dcam_setpropertyvalue(gci_dcam_camera->_hCam, DCAM_IDPROP_PHOTONIMAGINGMODE, mode );         
	
	return CAMERA_SUCCESS;  
}

static int gci_dcam_on_error(GciCamera* camera)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera; 

	if( gci_dcam_free_camera_resources(gci_dcam_camera) == CAMERA_ERROR) 
		return CAMERA_ERROR; 
	
	if (gci_dcam_precapture(gci_dcam_camera, ccCapture_Sequence) == CAMERA_ERROR) 
		return CAMERA_ERROR;
			
	// gci_dcam_set_to_ready_mode contains the slow gci_dcam_allocframe function.
	if (gci_dcam_set_to_ready_mode(gci_dcam_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;

	return CAMERA_SUCCESS;
}

int gci_dcam_attempt_recovery (GciCamera* camera)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera; 

	gci_dcam_idle(gci_dcam_camera);  // in case in busy state
	gci_dcam_freeframe(gci_dcam_camera); // in case in ready state

	gci_camera_power_down (camera); 
	
	if (gci_camera_power_up (camera)==CAMERA_ERROR)
		return CAMERA_ERROR;

	if (gci_camera_initialise(camera)==CAMERA_ERROR)
		return CAMERA_ERROR;

	if (gci_camera_snap_image(camera)==CAMERA_ERROR)
		return CAMERA_ERROR;

	return CAMERA_SUCCESS;
}


void gci_dcam_camera_setup_vtable(GciCamera* camera) 
{
	memset(&(camera->_cops), 0, sizeof(struct camera_operations));
	
	CAMERA_VTABLE_PTR(camera).initialise = gci_dcam_camera_initialise;
	CAMERA_VTABLE_PTR(camera).on_error = gci_dcam_on_error;
	CAMERA_VTABLE_PTR(camera).power_up = gci_dcam_camera_power_up;
	CAMERA_VTABLE_PTR(camera).power_down = gci_dcam_camera_power_down;
	CAMERA_VTABLE_PTR(camera).set_exposure_time = gci_dcam_camera_set_exposure_time;
	CAMERA_VTABLE_PTR(camera).set_gain = gci_dcam_camera_set_gain; 
	CAMERA_VTABLE_PTR(camera).get_gain = gci_dcam_camera_get_gain;
	CAMERA_VTABLE_PTR(camera).set_live_mode = gci_dcam_camera_set_live_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_mode = gci_dcam_camera_set_snap_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_sequence_mode = gci_dcam_camera_set_snap_sequence_mode;
	CAMERA_VTABLE_PTR(camera).get_image = gci_dcam_camera_get_image; 
	CAMERA_VTABLE_PTR(camera).set_size_position = gci_dcam_camera_set_size_position;
	CAMERA_VTABLE_PTR(camera).get_size_position = gci_dcam_camera_get_size_position;
	CAMERA_VTABLE_PTR(camera).get_info =	gci_dcam_camera_get_info;
	CAMERA_VTABLE_PTR(camera).save_settings = gci_dcam_save_settings;
	CAMERA_VTABLE_PTR(camera).load_settings = gci_dcam_load_settings;
	CAMERA_VTABLE_PTR(camera).set_datamode = gci_dcam_camera_set_data_mode;
	CAMERA_VTABLE_PTR(camera).get_highest_datamode = gci_dcam_camera_get_highest_data_mode;
	CAMERA_VTABLE_PTR(camera).get_lowest_datamode = gci_dcam_camera_get_lowest_data_mode;
	CAMERA_VTABLE_PTR(camera).set_lightmode = gci_dcam_camera_set_light_mode;
	CAMERA_VTABLE_PTR(camera).get_lightmode = gci_dcam_camera_get_light_mode;
	CAMERA_VTABLE_PTR(camera).get_colour_type = gci_dcam_get_colour_type;
	CAMERA_VTABLE_PTR(camera).set_default_settings = gci_dcam_camera_set_default_settings;
	CAMERA_VTABLE_PTR(camera).save_settings_as_default = gci_dcam_camera_save_settings_as_default;
	CAMERA_VTABLE_PTR(camera).save_state = gci_dcam_camera_save_state;
	CAMERA_VTABLE_PTR(camera).restore_state = gci_dcam_camera_restore_state;
	CAMERA_VTABLE_PTR(camera).destroy = gci_dcam_camera_destroy;
	CAMERA_VTABLE_PTR(camera).set_binning_mode = gci_dcam_camera_set_binning_mode;
	CAMERA_VTABLE_PTR(camera).get_binning_mode = gci_dcam_camera_get_binning_mode;
	CAMERA_VTABLE_PTR(camera).supports_binning = gci_dcam_supports_binning;
	CAMERA_VTABLE_PTR(camera).get_highest_binning_mode = gci_dcam_camera_get_highest_binning_mode;
	CAMERA_VTABLE_PTR(camera).set_blacklevel = gci_dcam_camera_set_blacklevel;
	CAMERA_VTABLE_PTR(camera).get_blacklevel = gci_dcam_camera_get_blacklevel;
	CAMERA_VTABLE_PTR(camera).set_sensitivity = gci_dcam_camera_set_sensitivity;
	CAMERA_VTABLE_PTR(camera).get_sensitivity = gci_dcam_camera_get_sensitivity;
	CAMERA_VTABLE_PTR(camera).set_ccd_mode = gci_dcam_camera_set_ccd_mode;
	CAMERA_VTABLE_PTR(camera).set_trigger_mode = gci_dcam_camera_set_trigger_mode;
	CAMERA_VTABLE_PTR(camera).fire_internal_trigger = gci_dcam_camera_fire_internal_trigger;
	CAMERA_VTABLE_PTR(camera).set_subwindow = gci_dcam_set_preset_subwindow;
	CAMERA_VTABLE_PTR(camera).attempt_recovery = gci_dcam_attempt_recovery;
}

void gci_dcam_constructor(GciCamera *camera, HINSTANCE hInstance, const char *name, const char *description)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) camera;
	
	gci_camera_constructor (camera, name, description);
	
	gci_dcam_camera->_ghInstance = hInstance; 
	gci_dcam_camera->_index = number_of_gci_dcam_cameras++;
	gci_dcam_camera->_hCam = NULL;
	camera->_data_mode = BPP14;
	camera->_bits = 12;
	gci_dcam_camera->_binning = NO_BINNING;
	gci_dcam_camera->_trigger_mode = CAMERA_INTERNAL_TRIG;
	gci_dcam_camera->_black_level = 255.0;	  // 255 = 0%
	gci_dcam_camera->_light_mode = 1;

	gci_camera_set_sensitivity_range(camera, 0.0, 100.0);
	
	gci_dcam_camera->saved_settings = (GciDCamCamera*) malloc (sizeof(GciDCamCamera)); 
    strcpy(gci_dcam_camera->default_settings_file_path, "DCamDefaultSettings.cam");

	if(gci_dcam_camera->saved_settings == NULL) {
		return;	
	}
    
    gci_dcam_camera_setup_vtable(camera);
}

// Generic Dcam Camera
GciCamera* gci_dcam_camera_new(HINSTANCE hInstance, const char *name, const char *description, int max_width, int max_height)
{
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) malloc(sizeof(GciDCamCamera));
	GciCamera *camera = NULL;

	memset(gci_dcam_camera, 0, sizeof(GciDCamCamera));

	gci_dcam_constructor(camera, hInstance, name, description);

	camera = (GciCamera*) gci_dcam_camera;
	
	gci_camera_set_min_size(camera, 32, 8);
	gci_camera_set_max_size(camera, 500, 500);

	return camera;
}


GciCamera* gci_orca_camera_new(HINSTANCE hInstance, const char *name, const char *description)
{
	GciCamera *camera = NULL;
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) malloc(sizeof(GciDCamCamera));

	memset(gci_dcam_camera, 0, sizeof(GciDCamCamera));

	camera = (GciCamera*) gci_dcam_camera;

	gci_dcam_constructor(camera, hInstance, name, description);

	gci_camera_set_allowed_bitmode (camera, BPP8); 
	gci_camera_set_allowed_bitmode (camera, BPP12); 
	
	gci_camera_set_allowed_binningmode (camera, BINNING2X2);                    
	gci_camera_set_allowed_binningmode (camera, BINNING4X4);                     
	gci_camera_set_allowed_binningmode (camera, BINNING8X8);                     
		
	gci_camera_set_min_size(camera, 32, 8);
	gci_camera_set_max_size(camera, 1344, 1024);
	
	gci_camera_set_gain_range(camera, 0.0, 255.0, VAL_UNSIGNED_INTEGER);
	gci_camera_set_gain_range_text(camera, "(x1)", "(x10)");
	
	gci_camera_set_blacklevel_range(camera, 205.0, 255.0);
	
	gci_camera_set_feature (camera, CAMERA_FEATURE_BLACKLEVEL);      	
	gci_camera_set_feature (camera, CAMERA_FEATURE_DATAMODE);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_BINNING);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_GAIN);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_LIGHTMODE); 

	return camera;
}

GciCamera* gci_C8484_camera_new(HINSTANCE hInstance, const char *name, const char *description)
{
	GciCamera *camera = NULL;
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) malloc(sizeof(GciDCamCamera));

	memset(gci_dcam_camera, 0, sizeof(GciDCamCamera));

	camera = (GciCamera*) gci_dcam_camera;

	gci_dcam_constructor(camera, hInstance, name, description);

	gci_camera_set_allowed_bitmode (camera, BPP8); 
	gci_camera_set_allowed_bitmode (camera, BPP12); 
	
	gci_camera_set_allowed_binningmode (camera, BINNING2X2);                    
	gci_camera_set_allowed_binningmode (camera, BINNING4X4);                     
	gci_camera_set_allowed_binningmode (camera, BINNING8X8);                     
		
	gci_camera_set_min_size(camera, 32, 8);
	gci_camera_set_max_size(camera, 1344, 1024);
	
	gci_camera_set_gain_range(camera, 0.0, 1.0, VAL_UNSIGNED_INTEGER);
	gci_camera_set_gain_range_text(camera, "(x1)", "(x5)");
	
	gci_camera_set_feature (camera, CAMERA_FEATURE_DATAMODE);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_BINNING);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_GAIN);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_LIGHTMODE); 

	return camera;
}

// EM Camera
GciCamera* gci_C9100_13_camera_new(HINSTANCE hInstance, const char *name, const char *description)
{
	GciCamera *camera = NULL;
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) malloc(sizeof(GciDCamCamera));

	memset(gci_dcam_camera, 0, sizeof(GciDCamCamera));

	camera = (GciCamera*) gci_dcam_camera;

	gci_dcam_constructor(camera, hInstance, name, description);

	gci_camera_set_blacklevel_range(camera, 0.0, 255.0);

//	gci_camera_set_allowed_bitmode (camera, BPP14);   // was 14 at some time (v1 firmware?)
	gci_camera_set_allowed_bitmode (camera, BPP16); 
	
	gci_camera_set_allowed_binningmode (camera, BINNING2X2);                     
	gci_camera_set_allowed_binningmode (camera, BINNING4X4);                     
	
	gci_camera_set_min_size(camera, 32, 32);
	gci_camera_set_max_size(camera, 512, 512);

	gci_camera_set_gain_range(camera, 1.0, 5.0, VAL_UNSIGNED_INTEGER);
	gci_camera_set_gain_range_text(camera, "(x1)", "(x5)");

	gci_camera_set_sensitivity_range(camera, 0.0, 255.0);

	gci_camera_set_feature (camera, CAMERA_FEATURE_SENSITIVITY);
	gci_camera_set_feature (camera, CAMERA_FEATURE_DATAMODE);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_BINNING);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_GAIN);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_TEMP);
	gci_camera_set_feature (camera, CAMERA_FEATURE_PHOTON_MODE); 
	gci_camera_set_feature (camera, CAMERA_FEATURE_CCD_MODE);
	gci_camera_set_feature (camera, CAMERA_FEATURE_EMGAINPROTECT);

	return camera;	
}

// CMOS ORCA-Flash 2.8
GciCamera* gci_C11440_10C_camera_new(HINSTANCE hInstance, const char *name, const char *description)
{
	GciCamera *camera = NULL;
	GciDCamCamera* gci_dcam_camera = (GciDCamCamera*) malloc(sizeof(GciDCamCamera));

	memset(gci_dcam_camera, 0, sizeof(GciDCamCamera));

	camera = (GciCamera*) gci_dcam_camera;

	gci_dcam_constructor(camera, hInstance, name, description);

//	gci_camera_set_blacklevel_range(camera, 0.0, 255.0);

	gci_camera_set_allowed_bitmode (camera, BPP12); 
	
	gci_camera_set_allowed_binningmode (camera, BINNING2X2);                     
	
	gci_camera_set_min_size(camera, 2, 8);
	gci_camera_set_max_size(camera, 1920, 1440);

	gci_camera_set_gain_range(camera, 1.0, 255.0, VAL_UNSIGNED_INTEGER);
	gci_camera_set_gain_range_text(camera, "(x1)", "(x8)");

	gci_camera_set_feature (camera, CAMERA_FEATURE_DATAMODE);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_BINNING);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_GAIN);     
	gci_camera_set_feature (camera, CAMERA_FEATURE_SUBTRACT);    

	gci_dcam_camera->_require_free_resources = 1;  // This camera requires resuourses to be freed when setting the trigger mode

	return camera;	
}




