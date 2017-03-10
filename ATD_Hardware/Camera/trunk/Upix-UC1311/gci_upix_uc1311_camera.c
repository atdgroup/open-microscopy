#include "gci_upix_uc1311_camera.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "FreeImageAlgorithms_IO.h"

#include "ThreadDebug.h"
#include <utility.h>

/* Function pointers used as virtual functions */
static struct camera_operations dummy_camera_ops;

static int gci_upix_uc1311_camera_power_up(GciCamera* camera)
{
  	return CAMERA_SUCCESS;
}

static int gci_upix_uc1311_camera_power_down(GciCamera* camera)
{
	return CAMERA_SUCCESS;
}

static int  gci_upix_uc1311_camera_set_exposure_time(GciCamera* camera, double exposure)
{
	Upix1311Camera *upix_camera = (Upix1311Camera *) camera;

	set_exposure(upix_camera, exposure);

	return CAMERA_SUCCESS;
}


static int  gci_upix_uc1311_camera_set_gain(GciCamera* camera, CameraChannel channel, double gain)
{
	Upix1311Camera *upix_camera = (Upix1311Camera *) camera;

	if(gain < 1.0)
		gain = 1.0;

	set_gain(upix_camera, (int)gain);

	return CAMERA_SUCCESS;
}

static int  gci_upix_uc1311_camera_get_gain(GciCamera* camera, CameraChannel channel, double *gain)
{
	Upix1311Camera *upix_camera = (Upix1311Camera *) camera;

	get_gain(upix_camera, gain);
	
	return CAMERA_SUCCESS;
}

static int  gci_upix_uc1311_camera_set_live_mode(GciCamera* camera)
{
	Upix1311Camera *upix_camera = (Upix1311Camera *) camera;

	start_live(upix_camera);

	return CAMERA_SUCCESS;
}


static int  gci_upix_uc1311_camera_set_snap_mode(GciCamera* camera)
{
	Upix1311Camera *upix_camera = (Upix1311Camera *) camera;

	camera_stop(upix_camera);
	camera_play(camera, SNAP_MODE_SOFT_TRIGGER);

	return CAMERA_SUCCESS;
}


static int  gci_upix_uc1311_camera_set_snap_sequence_mode(GciCamera* camera)
{
	Upix1311Camera *upix_camera = (Upix1311Camera *) camera;

	camera_stop(upix_camera);
	camera_play(camera, SNAP_MODE_SOFT_TRIGGER);

	return CAMERA_SUCCESS;
}


static FIBITMAP*  gci_upix_uc1311_camera_get_image(GciCamera* camera, const Rect *rect)
{
	return get_image(camera);
}


static int  gci_upix_uc1311_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = 320;
	*height = 240;

	return CAMERA_SUCCESS;
}


static int  gci_upix_uc1311_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = 1280;
	*height = 1024;

	return CAMERA_SUCCESS;
}


static int gci_upix_uc1311_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus, char *camera_id, char *camera_version, char *driver_version)
{
	if(vendor != NULL)
		strcpy(vendor, "Vineon");
	
	if(model != NULL)
		strcpy(model,"UPix 1311");
	
	if(camera_id != NULL) {
		BYTE serial_number[10];
		char serial_txt[100] = "";
		upix_get_serial_number(camera, &serial_number);
		sprintf(serial_txt, "%d", serial_number);
		strcpy(camera_id, serial_txt);
	}
	
	if(camera_version)
		strcpy(camera_version,"1311");
	
	return CAMERA_SUCCESS;
}


static int gci_upix_uc1311_camera_set_trigger_mode(GciCamera* camera, TriggerMode trig_mode)
{
	if(trig_mode == CAMERA_EXTERNAL_TRIG)
		camera_play(camera, SNAP_MODE_EXTERNAL_TRIGGER);
	else 
		camera_play(camera, SNAP_MODE_SOFT_TRIGGER);

	return CAMERA_SUCCESS;
}
	
static int gci_upix_uc1311_camera_set_data_mode(GciCamera* camera, DataMode data_mode)
{
	return CAMERA_SUCCESS;
}


int gci_upix_uc1311_camera_destroy(GciCamera* camera)
{
	Upix1311Camera *upix_camera = (Upix1311Camera *) camera;

	if(camera == NULL)
		return CAMERA_ERROR;

  	camera_stop (camera);
  	
	CmtDiscardLock(upix_camera->upix_lock);

  	return CAMERA_SUCCESS;
}


int gci_upix_uc1311_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
												 unsigned int *width, unsigned int *height, unsigned char *auto_centre)
{
	FIBITMAP* dib = gci_upix_uc1311_camera_get_image(camera, NULL);

	*left = 0;
	*top = 0;
	*width = FreeImage_GetWidth(dib);
	*height = FreeImage_GetHeight(dib);
	*auto_centre = 0;

	FreeImage_Unload(dib);

	return CAMERA_SUCCESS;
}

static int Upix1311GetIniSectionString(Upix1311Camera* upix_camera, char *buffer)
{
	GciCamera *camera = (GciCamera*) upix_camera;    
	double sensitivity; 
	double gain;
	DataMode dm;
	
	memset(buffer, 0, 1);   
	
	gci_camera_get_gain(camera, CAMERA_CHANNEL1, &gain);
	
	gci_camera_get_sensitivity(camera, CAMERA_CHANNEL1, &sensitivity);  

	// Write the global section
	sprintf(buffer, "Exposure=%f\nGain=%f\nSensitivity=%f\n\n",
					camera->_exposure_time, gain, sensitivity);
	
	str_change_char(buffer, '\n', '\0'); 
	
	return CAMERA_SUCCESS;  
}

static int gci_upix_uc1311_save_settings (GciCamera* camera, const char *filepath, const char *mode)
{
	Upix1311Camera* upix_camera = (Upix1311Camera*) camera;  
	char buffer[2000] = "";
	FILE *fp = NULL;
	
	if(camera == NULL)
		return CAMERA_ERROR;

	Upix1311GetIniSectionString(camera, buffer);
	
	if(!WritePrivateProfileSection(UIMODULE_GET_NAME(camera), buffer, filepath)) {
		send_error_text(camera, "Failed to save dcam settings for device %s.", UIMODULE_GET_DESCRIPTION(camera));
		return CAMERA_ERROR;  
	}
		
	return CAMERA_SUCCESS;
}

static void loadFromIniDictionary(Upix1311Camera* upix_camera, dictionary* ini)
{
	char buffer[64];
	int tmp;
	double d_tmp;
	GciCamera *camera = (GciCamera*) upix_camera;     
	
	sprintf (buffer, "%s:Exposure", UIMODULE_GET_NAME(camera));
	d_tmp = iniparser_getdouble(ini, buffer, -1.0);
	if (d_tmp>0.0)
		gci_camera_set_exposure_time(camera, d_tmp);   

	sprintf (buffer, "%s:Gain", UIMODULE_GET_NAME(camera));
	d_tmp = iniparser_getdouble(ini, buffer, -1.0);
	if (d_tmp>=0.0)
		gci_camera_set_gain(camera, CAMERA_CHANNEL1, d_tmp);
	
	sprintf (buffer, "%s:Sensitivity", UIMODULE_GET_NAME(camera));
	d_tmp = iniparser_getdouble(ini, buffer, -1);
	if (d_tmp>=0)
		gci_camera_set_sensitivity(camera, CAMERA_CHANNEL1, d_tmp);     
}

static int gci_upix_uc1311_load_settings (GciCamera* camera, const char *filepath) 
{
	dictionary* ini = iniparser_load  (filepath);    
	
	if(camera == NULL)
		return CAMERA_ERROR;
	
	loadFromIniDictionary(camera, ini);
		
	iniparser_freedict(ini);

	return CAMERA_SUCCESS;
}

static int gci_upix_uc1311_camera_save_state(GciCamera* camera, CameraState *state)
{
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
	
	return CAMERA_SUCCESS;
}

static int gci_upix_uc1311_camera_restore_state(GciCamera* camera, CameraState *state)
{
	gci_camera_set_snap_mode(camera);
	gci_camera_set_data_mode(camera, state->data_mode);
	gci_camera_set_exposure_time(camera, state->exposure_time);
	gci_camera_set_gain(camera, CAMERA_ALL_CHANNELS, state->gain1);

	if(state->live) {
		gci_camera_set_live_mode(camera);
		gci_camera_activate_live_display(camera);
	}
		
	return CAMERA_SUCCESS;
}

static int gci_upix_uc1311_camera_set_default_settings (GciCamera* camera)
{
	gci_camera_set_data_mode(camera, BPP12);

	return CAMERA_SUCCESS; 
}

static int gci_upix_uc1311_camera_get_highest_datamode (GciCamera* camera, DataMode *data_mode)  
{
	*data_mode = BPP12;
	
	return CAMERA_SUCCESS; 
}

static int gci_upix_uc1311_camera_get_lowest_datamode (GciCamera* camera, DataMode *data_mode)  
{
	*data_mode = BPP8;
	
	return CAMERA_SUCCESS; 
}

static double gci_upix_uc1311_get_colour_type(GciCamera* camera)
{
	return 1.0;
}

static int gci_dcam_get_colour_type (GciCamera* camera)
{
	return MONO_TYPE;
}

static int gci_upix_uc1311_camera_initialise (GciCamera* camera)
{
	Upix1311Camera* upix_camera = (Upix1311Camera*) camera; 
	double gain, exposure;

	upix_initialise (upix_camera);

	gci_camera_set_exposure_range(camera, 1, 232, "ms", VAL_INTEGER);   

	get_exposure(upix_camera, &exposure);
	gci_camera_set_exposure_time(camera, exposure);   

	gci_camera_set_gain_range(camera, 1.0, 50.0, VAL_UNSIGNED_INTEGER);
	gci_camera_set_gain_range_text(camera, "(1)", "(50)");

	gci_camera_get_gain(camera, CAMERA_ALL_CHANNELS, &gain);
	gci_camera_set_gain(camera, CAMERA_ALL_CHANNELS, gain);

	return CAMERA_SUCCESS;
}

GciCamera* gci_upix_uc1311_camera_new(const char *name, const char* description)
{
	Upix1311Camera* upix_camera = (Upix1311Camera*) malloc(sizeof(Upix1311Camera));
	GciCamera *camera = (GciCamera*) upix_camera;

	memset(upix_camera, 0, sizeof(Upix1311Camera));

	gci_camera_constructor(camera, name, description);

	GciCmtNewLock ("UPixCamera", 0, &(upix_camera->upix_lock));

	upix_camera->initialised = 0;
	upix_camera->camera_stopped = 1;
	upix_camera->camera_number = 1; //camera_number++;
	upix_camera->image_callback_completed = 0;

	//ui_module_set_window_proc(UIMODULE_CAST(camera), camera->_main_ui_panel, (LONG_PTR) CymapWndProc);   
	
	//GetPanelAttribute (camera->_main_ui_panel, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	//upix_camera->window_hwnd = (HWND) window_handle;

	CAMERA_VTABLE_PTR(camera).initialise = gci_upix_uc1311_camera_initialise;
	CAMERA_VTABLE_PTR(camera).set_default_settings = gci_upix_uc1311_camera_set_default_settings;
	CAMERA_VTABLE_PTR(camera).destroy = gci_upix_uc1311_camera_destroy;
	CAMERA_VTABLE_PTR(camera).power_up = gci_upix_uc1311_camera_power_up;
	CAMERA_VTABLE_PTR(camera).power_down = gci_upix_uc1311_camera_power_down;
	CAMERA_VTABLE_PTR(camera).set_exposure_time = gci_upix_uc1311_camera_set_exposure_time;
	CAMERA_VTABLE_PTR(camera).set_gain = gci_upix_uc1311_camera_set_gain; 
	CAMERA_VTABLE_PTR(camera).get_gain = gci_upix_uc1311_camera_get_gain;
	CAMERA_VTABLE_PTR(camera).set_live_mode = gci_upix_uc1311_camera_set_live_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_mode = gci_upix_uc1311_camera_set_snap_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_sequence_mode = gci_upix_uc1311_camera_set_snap_sequence_mode;
	CAMERA_VTABLE_PTR(camera).get_colour_type = gci_upix_uc1311_get_colour_type;
	CAMERA_VTABLE_PTR(camera).get_image = gci_upix_uc1311_camera_get_image; 
	CAMERA_VTABLE_PTR(camera).get_size_position = gci_upix_uc1311_get_size_position;
	CAMERA_VTABLE_PTR(camera).get_info =	gci_upix_uc1311_camera_get_info;
	CAMERA_VTABLE_PTR(camera).set_datamode =	gci_upix_uc1311_camera_set_data_mode;
	CAMERA_VTABLE_PTR(camera).set_trigger_mode =	gci_upix_uc1311_camera_set_trigger_mode; 
	CAMERA_VTABLE_PTR(camera).save_settings = gci_upix_uc1311_save_settings;
	CAMERA_VTABLE_PTR(camera).load_settings = gci_upix_uc1311_load_settings;
	CAMERA_VTABLE_PTR(camera).save_state = gci_upix_uc1311_camera_save_state;
	CAMERA_VTABLE_PTR(camera).restore_state = gci_upix_uc1311_camera_restore_state;
	//CAMERA_VTABLE_PTR(camera).get_fps = gci_upix_uc1311_camera_get_fps;   
	CAMERA_VTABLE_PTR(camera).get_highest_datamode = gci_upix_uc1311_camera_get_highest_datamode;
	CAMERA_VTABLE_PTR(camera).get_lowest_datamode = gci_upix_uc1311_camera_get_lowest_datamode;
	
	return camera;
}



