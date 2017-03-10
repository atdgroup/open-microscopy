#include "toolbox.h"
#include "gci_C9100-13_camera.h"
#include "gci_orca_camera_lowlevel.h"  
#include "gci_C9100-13_camera_settings.h"
#include "gci_C9100-13_camera_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "FreeImageAlgorithms_IO.h"

#include "dcamapi.h"
#include "features.h"
#include "dcamapix.h"
#include "dcamprop.h"

#include <utility.h>
#include <formatio.h>

//////////////////////////////////////////////////////////////////////////////////
// Camera module for the Hamatsu C9100-13 EM camera
// RJL Jan 2007
//////////////////////////////////////////////////////////////////////////////////

#define IM_MAX_WIDTH 512
#define IM_MAX_HEIGHT 512
#define IM_MIN_WIDTH 32
#define IM_MIN_HEIGHT 32

#define DEFAULT_SETTINGS_FILE "C9100_13_DefaultSettings.cam"

static unsigned char number_of_C9100_13_cameras = 0;

/* Function pointers used as virtual functions */
static struct camera_operations C9100_13_camera_ops;


static int OrcaCheckSubWindow (int *left, int *top, int *width, int *height)
{
	//Position must be on a 8 pixel boundary
	(*left) /= 8;
	(*left) *= 8;
	(*top) /= 8;
	(*top) *= 8;
	
	//width must be a multiple of 32
	(*width) /= 32;
	(*width) *= 32;
	
	//height must be a multiple of 8
	(*height) /= 8;
	(*height) *= 8;
	
	if (*width < IM_MIN_WIDTH)
		*width = IM_MIN_WIDTH;
		
	if (*height < IM_MIN_HEIGHT)
		*height = IM_MIN_HEIGHT;
	
	if (*left > (IM_MAX_WIDTH  - *width))
		*left = (IM_MAX_WIDTH  - *width);
		
	if (*top > (IM_MAX_HEIGHT - *height))
		*top = (IM_MAX_HEIGHT - *height);
	
	return CAMERA_SUCCESS;
}

static void C9100_13_error_code_to_string(unsigned long error, char *string)
{
	switch (error) {
	
		case 0x80000101:
		
			strcpy(string, "busy cannot process");
		
			break;
			
		case 0x80000102:
		
			strcpy(string, "abort process");
		
			break;
			
		case 0x80000103:
		
			strcpy(string, "not ready state");
		
			break;
			
		case 0x80000104:
		
			strcpy(string, "not stable state");
		
			break;
			
		case 0x80000106:
		
			strcpy(string, "timeout");
		
			break;
			
		case 0x80000107:
		
			strcpy(string, "not busy state");
		
			break;

		case 0x80000f02:
		
			strcpy(string, "feature not implemented yet");
		
			break;
	}

	return;
}


static int C9100_13_precapture(GciC9100_13Camera *C9100_13_camera, int mode)
{
	GciCamera *camera = (GciCamera *) C9100_13_camera;

	//Steps we must take prior to getting images
	//mode is ccCapture_Snap or ccCapture_Sequence

	if (!dcam_precapture( C9100_13_camera->_hCam, mode)) {
		send_error_text(camera, "Precapture routine failed");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


static int C9100_13_capture(GciC9100_13Camera *C9100_13_camera)
{
	int error;
	char buffer[255], string_buffer[255]; 
	GciCamera *camera = (GciCamera *) C9100_13_camera;

	GCI_Signal_Emit(&(camera->signal_table), "PreCapture", GCI_VOID_POINTER, C9100_13_camera);

	if (!dcam_capture(C9100_13_camera->_hCam)) {
	
		error = dcam_getlasterror(C9100_13_camera->_hCam, buffer, 255);
	
		C9100_13_error_code_to_string(error, string_buffer);  
	
        send_error_text(camera, "gci_C9100_13_camera_get_image capture failed: %s", string_buffer);
        return CAMERA_ERROR;
    }
    
    if (C9100_13_camera->_trigger_mode == CAMERA_EXTERNAL_TRIG)	//shutter triggers the acquisition
    	GCI_Signal_Emit(&(camera->signal_table), "PostCapture", GCI_VOID_POINTER, C9100_13_camera);
    
    return CAMERA_SUCCESS;
}
    

//This is only supported by the Orca unfortunately.
//It would enable us to close the shutter sooner, i.e. before data transfer from the CCD
static int C9100_13_wait_for_frame_valid(GciC9100_13Camera *C9100_13_camera)
{
	int event = DCAM_EVENT_VVALIDBEGIN, error;
	char buffer[255], string_buffer[255];

	GciCamera *camera = (GciCamera *) C9100_13_camera; 

	// Wait for acquisition complete to CCD or timeout
	if (!dcam_wait(C9100_13_camera->_hCam, &event, C9100_13_camera->_timeout, NULL) ) {
	
		error = dcam_getlasterror(C9100_13_camera->_hCam, buffer, 255);
	
		C9100_13_error_code_to_string(error, string_buffer);  
	
		send_error_text(camera, "dcam_wait failed: %s", string_buffer);
		
	    if (C9100_13_camera->_trigger_mode != CAMERA_EXTERNAL_TRIG)
	    	GCI_Signal_Emit(&(camera->signal_table), "PostCapture", GCI_VOID_POINTER, C9100_13_camera);
		return NULL;
	}
	
    if (C9100_13_camera->_trigger_mode != CAMERA_EXTERNAL_TRIG)  //want shutter to close
	    GCI_Signal_Emit(&(camera->signal_table), "PostCapture", GCI_VOID_POINTER, C9100_13_camera);

	return CAMERA_SUCCESS;
}


static int C9100_13_wait_for_frame_end(GciC9100_13Camera *C9100_13_camera)
{
	int event = DCAM_EVENT_FRAMEEND, error;
	char buffer[255], string_buffer[255];

	GciCamera *camera = (GciCamera *) C9100_13_camera; 

	// Wait for acquisition complete and data transfered or timeout
	if (!dcam_wait(C9100_13_camera->_hCam, &event, C9100_13_camera->_timeout, NULL) ) {
	
		error = dcam_getlasterror(C9100_13_camera->_hCam, buffer, 255);
	
		C9100_13_error_code_to_string(error, string_buffer);  
	
		send_error_text(camera, "dcam_wait failed: %s", string_buffer);
		
	    if (C9100_13_camera->_trigger_mode != CAMERA_EXTERNAL_TRIG)
		    GCI_Signal_Emit(&(camera->signal_table), "PostCapture", GCI_VOID_POINTER, C9100_13_camera);
		return NULL;
	}
	
    if (C9100_13_camera->_trigger_mode != CAMERA_EXTERNAL_TRIG)  //want shutter to close
	    GCI_Signal_Emit(&(camera->signal_table), "PostCapture", GCI_VOID_POINTER, C9100_13_camera);

	return CAMERA_SUCCESS;
}


static int C9100_13_idle(GciC9100_13Camera *C9100_13_camera)
{
	GciCamera *camera = (GciCamera *) C9100_13_camera; 

	if (!dcam_idle(C9100_13_camera->_hCam) ) {
		
		send_error_text(camera, "dcam_idle routine failed");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


static int C9100_13_freeframe(GciC9100_13Camera *C9100_13_camera)
{
	GciCamera *camera = (GciCamera *) C9100_13_camera; 
	
	//Free resources
	if (!dcam_freeframe(C9100_13_camera->_hCam)) {
		
		send_error_text(camera, "dcam_freeframe routine failed");
		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}


static int C9100_13_wait_for_stable(GciC9100_13Camera *C9100_13_camera)
{
	int status = DCAM_STATUS_UNSTABLE;
	double t;
	
	t = Timer();
	while (status != DCAM_STATUS_STABLE) {
		dcam_getstatus( C9100_13_camera->_hCam, &status);
		if (Timer() - t > 0.5) break;
	}
		
	return CAMERA_SUCCESS; 
}


static int C9100_13_wait_for_ready(GciC9100_13Camera *C9100_13_camera)
{
	int status = DCAM_STATUS_UNSTABLE;
	double t;
	
	t = Timer();
	while (status != DCAM_STATUS_READY) {
		dcam_getstatus( C9100_13_camera->_hCam, &status);
		if (Timer() - t > 0.5) break;
	}
		
	return CAMERA_SUCCESS; 
}


static int C9100_13_wait_for_busy(GciC9100_13Camera *C9100_13_camera)
{
	int status = DCAM_STATUS_UNSTABLE;
	double t;
	
	t = Timer();
	while (status != DCAM_STATUS_BUSY) {
		dcam_getstatus( C9100_13_camera->_hCam, &status);
		if (Timer() - t > 0.5) break;
	}
		
	return CAMERA_SUCCESS; 
}


static int C9100_13_allocframe(GciC9100_13Camera *C9100_13_camera)
{
	GciCamera *camera = (GciCamera *) C9100_13_camera;

	if( !dcam_allocframe( C9100_13_camera->_hCam, 3))
	{
		send_error_text(camera, "allocframe routine failed");
		return CAMERA_ERROR;
	}
		
	return CAMERA_SUCCESS; 
}


static int C9100_13_set_to_ready_mode(GciC9100_13Camera *C9100_13_camera)
{
	C9100_13_wait_for_stable(C9100_13_camera);
	
	// Slow operation try to only call once for live mode
	C9100_13_allocframe(C9100_13_camera);
	
	C9100_13_wait_for_ready(C9100_13_camera);

	return CAMERA_SUCCESS;
}


static int C9100_13_free_camera_resources(GciC9100_13Camera *C9100_13_camera)
{
    int status;
    GciCamera *camera = (GciCamera *) C9100_13_camera;
    
    if (!dcam_getstatus(C9100_13_camera->_hCam, &status)) {
		
		send_error_text(camera, "dcam_getstatus routine failed");
		return CAMERA_ERROR;
	}
    
    if (status == DCAM_STATUS_BUSY) {

		C9100_13_idle(C9100_13_camera);

        if (!dcam_getstatus(C9100_13_camera->_hCam, &status)) {
		
			send_error_text(camera, "dcam_getstatus routine failed");
			return CAMERA_ERROR;
		}
    }
    
    if (status == DCAM_STATUS_READY) {
        
        if (C9100_13_freeframe(C9100_13_camera) == CAMERA_ERROR)
        	return CAMERA_ERROR;	
    }
    
    while ((status != DCAM_STATUS_STABLE) && (status != DCAM_STATUS_UNSTABLE)){
		dcam_getstatus(C9100_13_camera->_hCam, &status);
    }
    
    return CAMERA_SUCCESS;
}


static int C9100_13_SetFeature(GciC9100_13Camera *C9100_13_camera, int featureID, double value)
{
	DCAM_PARAM_FEATURE FeatureValue;
	
	//Used to set gain and brightness.
	
	//Set up structure with camera commands. Cribbed from the example programs. 
	FeatureValue.hdr.cbSize = sizeof(DCAM_PARAM_FEATURE);
	FeatureValue.hdr.id = DCAM_IDPARAM_FEATURE;
	FeatureValue.hdr.iFlag = 0;
	FeatureValue.hdr.oFlag = 0;
	FeatureValue.featureid = featureID;
	FeatureValue.flags = 0;
	FeatureValue.featurevalue = (float)value;

	//Send it
	if (!dcam_extended(C9100_13_camera->_hCam, DCAM_IDMSG_SETPARAM, (LPVOID)&FeatureValue, sizeof(DCAM_PARAM_FEATURE))) 
		return CAMERA_ERROR;
	
	return CAMERA_SUCCESS;
}

static int QueryCurrentTempertureSupport(GciC9100_13Camera *C9100_13_camera, int* supported)
{
	DCAM_PARAM_FEATURE_INQ	param;
	BOOL	support = FALSE;
	unsigned long error;
	char buffer[255], string_buffer[255];
	GciCamera *camera = (GciCamera *) C9100_13_camera; 

	memset( &param, 0, sizeof( param ));
	param.hdr.cbSize = sizeof( param );
	param.hdr.id	= DCAM_IDPARAM_FEATURE_INQ;
	param.hdr.iFlag = dcamparam_featureinq_featureid | dcamparam_featureinq_capflags;

	param.featureid = DCAM_IDFEATURE_TEMPERATURE;
	if( ! dcam_extended( C9100_13_camera->_hCam, DCAM_IDMSG_GETPARAM, &param, sizeof( param )) ) {
		error = dcam_getlasterror(C9100_13_camera->_hCam, buffer, 255);
		C9100_13_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_extended failed: %s", string_buffer);
		return CAMERA_ERROR;
	}
	
	if( param.capflags & DCAM_FEATURE_FLAGS_ONOFF )
	{
		if( param.capflags & DCAM_FEATURE_FLAGS_READ_OUT )
		{
			support = TRUE;
		}
	}

	*supported = support;

	return CAMERA_SUCCESS;
}

static int C9100_13_GetFeature(GciC9100_13Camera *C9100_13_camera, int featureID, double *value)
{
	DCAM_PARAM_FEATURE FeatureValue;
	unsigned long error;
	char buffer[255], string_buffer[255];
	GciCamera *camera = (GciCamera *) C9100_13_camera; 
	
	//Used to get temperature.
	
	//Set up structure with camera commands. Cribbed from the example programs. 
	FeatureValue.hdr.cbSize = sizeof(DCAM_PARAM_FEATURE);
	FeatureValue.hdr.id = DCAM_IDPARAM_FEATURE;
	FeatureValue.hdr.iFlag = dcamparam_feature_featureid | dcamparam_feature_featurevalue;
	FeatureValue.hdr.oFlag = 0;
	FeatureValue.featureid = featureID;
	FeatureValue.flags = 0;

	//Send it
	if (!dcam_extended(C9100_13_camera->_hCam, DCAM_IDMSG_GETPARAM, (LPVOID)&FeatureValue, sizeof(DCAM_PARAM_FEATURE))) {
		error = dcam_getlasterror(C9100_13_camera->_hCam, buffer, 255);
		C9100_13_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_extended failed: %s", string_buffer);
		return CAMERA_ERROR;
	}
	
	if( ! (FeatureValue.hdr.oFlag & dcamparam_feature_featurevalue ) )
		return CAMERA_ERROR;

	*value = FeatureValue.featurevalue;
	
	return CAMERA_SUCCESS;
}


static int C9100_13_SetSubarray(GciC9100_13Camera *C9100_13_camera, int ox, int oy, int gx, int gy)
{
	DCAM_PARAM_SUBARRAY Subarray;

	// Set up structure with camera commands. Cribbed from the example programs. 
	Subarray.hdr.cbSize = sizeof(DCAM_PARAM_SUBARRAY);
	Subarray.hdr.id = DCAM_IDPARAM_SUBARRAY;
	Subarray.hdr.iFlag = 0;
	Subarray.hdr.oFlag = 0;
	Subarray.hpos = ox;
	Subarray.vpos = oy;
	Subarray.hsize = gx;
	Subarray.vsize = gy;

	if (!dcam_extended(C9100_13_camera->_hCam, DCAM_IDMSG_SETPARAM, (LPVOID)&Subarray, sizeof(DCAM_PARAM_SUBARRAY))) 
		return CAMERA_ERROR;
	
	return CAMERA_SUCCESS;
}


static int C9100_13_GetSubarray(GciC9100_13Camera *C9100_13_camera, int *ox, int *oy, int *gx, int *gy)
{
	DCAM_PARAM_SUBARRAY Subarray;

	//Set up structure with camera commands. Cribbed from the example programs. 
	Subarray.hdr.cbSize = sizeof(DCAM_PARAM_SUBARRAY);
	Subarray.hdr.id = DCAM_IDPARAM_SUBARRAY;
	Subarray.hdr.iFlag = 0;
	Subarray.hdr.oFlag = 0;

	//Get current subarray settings
	if (!dcam_extended(C9100_13_camera->_hCam, DCAM_IDMSG_GETPARAM, (LPVOID)&Subarray, sizeof(DCAM_PARAM_SUBARRAY))) 
		return CAMERA_ERROR;
	
	*ox = Subarray.hpos;
	*oy = Subarray.vpos;
	*gx = Subarray.hsize;
	*gy = Subarray.vsize;
	
	return CAMERA_SUCCESS;
}


static int C9100_13_CheckSubWindow (int *left, int *top, int *width, int *height)
{
	//Position must be on a 8 pixel boundary
	(*left) /= 8;
	(*left) *= 8;
	(*top) /= 8;
	(*top) *= 8;
	
	//width must be a multiple of 32
	(*width) /= 32;
	(*width) *= 32;
	
	//height must be a multiple of 8
	(*height) /= 8;
	(*height) *= 8;
	
	if (*width < IM_MIN_WIDTH)
		*width = IM_MIN_WIDTH;
		
	if (*height < IM_MIN_HEIGHT)
		*height = IM_MIN_HEIGHT;
	
	if (*left > (IM_MAX_WIDTH  - *width))
		*left = (IM_MAX_WIDTH  - *width);
		
	if (*top > (IM_MAX_HEIGHT - *height))
		*top = (IM_MAX_HEIGHT - *height);
	
	return CAMERA_SUCCESS;
}


int C9100_13_SetDataType(GciC9100_13Camera *C9100_13_camera, int bits)
{
	int data_type, max_value;
	
	GciCamera *camera = (GciCamera *) C9100_13_camera;
	
	//Camera must be idle with no attached buffers before changing data type
	if (C9100_13_idle(C9100_13_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
	
	if (C9100_13_free_camera_resources(C9100_13_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
	
	data_type = ccDatatype_uint16;
	max_value = 16383;
	
	if(!dcam_setdatatype(C9100_13_camera->_hCam, data_type)) {
	
		send_error_text(camera, "dcam_setdatatype routine failed");
		return CAMERA_ERROR;	
	}
	
	if(!dcam_setbitsinputlutrange(C9100_13_camera->_hCam, max_value, 0)) {
	
		send_error_text(camera, "dcam_setbitsinputlutrange routine failed");
		return CAMERA_ERROR;	
	}
	
	if(!dcam_setbitsoutputlutrange(C9100_13_camera->_hCam, max_value, 0)) {
	
		send_error_text(camera, "dcam_setbitsoutputlutrange routine failed");
		return CAMERA_ERROR;	
	}
		
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_set_default_settings (GciCamera* camera)
{
	char filepath[MAX_PATHNAME_LEN];
	
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera;

	if (FindPathForFile(DEFAULT_SETTINGS_FILE, filepath) ||
		(gci_camera_load_settings(camera, filepath) == CAMERA_ERROR )) {
	
		C9100_13_idle(C9100_13_camera);

		gci_camera_set_data_mode(camera, BPP14);
		
		gci_camera_set_exposure_time(camera, 40);
		gci_camera_set_gain(camera, CAMERA_CHANNEL1, 0);
	
		C9100_13_camera->_binning = 1;
	
		if( gci_camera_set_binning_mode(camera, C9100_13_camera->_binning) == CAMERA_ERROR)
			return CAMERA_ERROR;
	
		if( gci_camera_set_size_position(camera, 0, 0, IM_MAX_WIDTH, IM_MAX_HEIGHT, 0) == CAMERA_ERROR)  
			return CAMERA_ERROR;
	
	}
	
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_save_settings_as_default (GciCamera* camera)
{
	char path[MAX_PATHNAME_LEN];
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			
	if (ConfirmPopup ("Confirm Save as Defaults",
		"Are you sure?\n The these values will be used every time\n this application is started.") != 1) {
		
		return CAMERA_ERROR;
	}
		
	GetPrivateDataFolder("Microscope Data", path);
	strcat(path, DEFAULT_SETTINGS_FILE);
	gci_camera_save_settings(camera, path, "w");  
	
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_power_up(GciCamera* camera)
{
	int ret, init_panel;
	
	init_panel = FindAndLoadUIR(0, "gci_orca_camera_ui.uir", INIT_PNL);
      
    DisplayPanel(init_panel);
         
    ret = OrcaPowerUp(camera); 
  
    DiscardPanel(init_panel);
	
	return ret;
}


static int gci_C9100_13_camera_power_down(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	if (!dcam_close(C9100_13_camera->_hCam)) {
	
		send_error_text(camera, "Failed to close camera");
		return CAMERA_ERROR;
	}
	
	if(!dcam_uninit(C9100_13_camera->_ghInstance, NULL)) {
	
		send_error_text(camera, "dcam_uninit Failed");
		return CAMERA_ERROR;
	}
	
	camera->_powered_up = 0;
	
	#ifdef POWER_VIA_I2C

		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		
		if (ConfirmPopup("", "Do you want to switch the camera off?")) {
			
			GCI_PowerCameraOff();
		}
		
	#endif
	
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_save_state(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;
	
	C9100_13_camera->saved_settings->camera._colour_type = camera->_colour_type;
	C9100_13_camera->saved_settings->camera._data_mode = camera->_data_mode;
	C9100_13_camera->saved_settings->camera._average_frames = camera->_average_frames;
	C9100_13_camera->saved_settings->camera._exposure_time = camera->_exposure_time;
    C9100_13_camera->saved_settings->camera._gain = camera->_gain;

    C9100_13_camera->saved_settings->_binning = C9100_13_camera->_binning;   

	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_restore_state(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	if(C9100_13_camera->saved_settings == NULL) {

		send_error_text(camera, "gci_C9100_13_camera_restore_state failed (previous state not saved)");
		return CAMERA_ERROR;	
	}
	
	camera->_colour_type = C9100_13_camera->saved_settings->camera._colour_type;
	gci_camera_set_data_mode(camera, C9100_13_camera->saved_settings->camera._data_mode);
	
	gci_camera_set_average_frame_number(camera, C9100_13_camera->saved_settings->camera._average_frames);
	gci_camera_set_exposure_time(camera, C9100_13_camera->saved_settings->camera._exposure_time);
	gci_camera_set_gain(camera, CAMERA_CHANNEL1, C9100_13_camera->saved_settings->camera._gain);
	
	gci_camera_set_binning_mode(camera, C9100_13_camera->saved_settings->_binning);

	return CAMERA_SUCCESS;
}


static BinningMode  gci_C9100_13_camera_get_binning_mode(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	return C9100_13_camera->_binning;
}


static int  gci_C9100_13_camera_set_exposure_time(GciCamera* camera, double exposure)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

		// Passed Milli Seconds
	if ((exposure < 1) || (exposure > 100000))
		return CAMERA_ERROR;
	
	if (!dcam_setexposuretime(C9100_13_camera->_hCam, exposure / 1000.0))
		return CAMERA_ERROR;
	
	C9100_13_camera->_timeout = (int)(exposure) + 2000;
	
	//We have to delay before sending another command to the camera
	//The delay is proportional to the exposure with a max of 1 second
	//Is this still true
	Delay( min(1.0, (exposure * 1000.0) /3.0) ); 

	return CAMERA_SUCCESS;
}


static int  gci_C9100_13_camera_set_gain(GciCamera* camera, CameraChannel channel, double gain)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;
	
	if ((gain < 0) || (gain > 255))
		return CAMERA_ERROR;
		
	return C9100_13_SetFeature (C9100_13_camera, DCAM_IDFEATURE_SENSITIVITY, (double)RoundRealToNearestInteger(gain));
}



static int gci_C9100_13_camera_set_live_mode(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;
	
	if( C9100_13_free_camera_resources(C9100_13_camera) == CAMERA_ERROR) 
		return CAMERA_ERROR; 
	
	if (C9100_13_precapture(C9100_13_camera, ccCapture_Sequence) == CAMERA_ERROR) 
		return CAMERA_ERROR;
			
	// C9100_13_set_to_ready_mode contains the slow dcam_allocframe function.
	if (C9100_13_set_to_ready_mode(C9100_13_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;
		
	C9100_13_capture(C9100_13_camera);
									 
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_set_snap_sequence_mode(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	if( C9100_13_free_camera_resources(C9100_13_camera) == CAMERA_ERROR) 
			return CAMERA_ERROR; 

	if (C9100_13_precapture(C9100_13_camera, ccCapture_Snap) == CAMERA_ERROR)
		return CAMERA_ERROR;

	// C9100_13_set_to_ready_mode contains the slow dcam_allocframe function.
	if (C9100_13_set_to_ready_mode(C9100_13_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;

	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_set_snap_mode(GciCamera* camera)
{
	gci_C9100_13_camera_set_snap_sequence_mode(camera);
	
	return CAMERA_SUCCESS;
}

static FIBITMAP * gci_C9100_13_camera_get_image(GciCamera* camera, const Rect *rect)
{
	return OrcaGetImage(camera, rect);   
}


static int gci_C9100_13_camera_set_size_position(GciCamera* camera, unsigned int left, unsigned int top,
																unsigned int width, unsigned int height, unsigned char auto_centre)
{
	unsigned int binning, canvas_width, canvas_height;
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;
  	GciOrcaCamera *orca_camera = (GciOrcaCamera *) camera;  
	
  	  	orca_free_camera_resources(orca_camera);
    
    binning = gci_camera_get_binning_mode(camera);
    
    left /= binning;
    top  /= binning;
    width /= binning;
    height /= binning;
    
    OrcaCheckSubWindow (&left, &top, &width, &height);
    
    GetCtrlAttribute (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_CANVAS, ATTR_WIDTH, &canvas_width);
    GetCtrlAttribute (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_CANVAS, ATTR_HEIGHT, &canvas_height);
    
    if (auto_centre) {
    
    	left = (IM_MAX_WIDTH / binning - width) / 2;
		top  = (IM_MAX_HEIGHT / binning - height) / 2;
    }
    
    OrcaCheckSubWindow (&left, &top, &width, &height);
    
    if ( OrcaSetSubarray(orca_camera, left, top, width, height) == CAMERA_ERROR ) {
		
		send_error_text(camera, "Failed to set subarray");
		return CAMERA_ERROR;
    }
    
    // SubWindow vals may have been changed by GCI_OrcaSetSubarray
    if ( OrcaGetSubarray(orca_camera, &left, &top, &width, &height) == CAMERA_ERROR ) {
    
		send_error_text(camera, "Failed to get subarray");
		return CAMERA_ERROR;
    }

    SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT,    left*binning);
    SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP,     top*binning);
    SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH,   width*binning);
    SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT,  height*binning);
    
    // Draw subwindow display
    // Round up here since camera will round down to somthing acceptable to it
    left = ceil ((double)left * canvas_width  / IM_MAX_WIDTH);  
    top  = ceil ((double)top  * canvas_height / IM_MAX_HEIGHT);
    width = ceil ((double)width * canvas_width  / IM_MAX_WIDTH);
    height = ceil ((double)height * canvas_height / IM_MAX_HEIGHT);
    
    if (width==0)
    	width++;
    	
    if (height==0)
    	height++; // just so something is seen on subwindow display
    
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
																unsigned int *width, unsigned int *height, unsigned char *auto_centre)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	// SubWindow vals may have been changed by GCI_C9100_13_SetSubarray
    if ( C9100_13_GetSubarray(C9100_13_camera, left, top, width, height) == CAMERA_ERROR ) {
    
		send_error_text(camera, "Failed to get subarray");
		return CAMERA_ERROR;
    }
    
	return CAMERA_SUCCESS;
}


static void C9100_13_DragBoxCallback (Rect box, void *callback_data)
{
    int canvas_width, canvas_height;
    int left, top, cols, rows;
    GciCamera* camera = (GciCamera *) callback_data;

    SetCtrlVal (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, 0);
    SetCtrlIndex (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_PRSETWINDOW, 0);
    GetCtrlAttribute (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_CANVAS, ATTR_WIDTH, &canvas_width);
    GetCtrlAttribute (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_CANVAS, ATTR_HEIGHT, &canvas_height);

    left   = box.left   * IM_MAX_WIDTH / canvas_width;
    top    = box.top    * IM_MAX_HEIGHT/ canvas_height;
    cols   = box.width  * IM_MAX_WIDTH / canvas_width; 
    rows   = box.height * IM_MAX_HEIGHT/ canvas_height;
    
    gci_C9100_13_camera_set_size_position(camera, left, top, cols, rows, 0);
}


static int gci_C9100_13_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = IM_MIN_WIDTH;
	*height = IM_MIN_HEIGHT;

	return CAMERA_SUCCESS;
}


static int  gci_C9100_13_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = IM_MAX_WIDTH;
	*height = IM_MAX_HEIGHT;

	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus, char *camera_id, char *camera_version, char *driver_version)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	if (vendor != NULL) {
	
		if (!dcam_getmodelinfo(C9100_13_camera->_index, DCAM_IDSTR_VENDOR, vendor, 64)) {
			send_error_text(camera, "Failed to get vendor info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (model != NULL) {
	
		if (!dcam_getmodelinfo(C9100_13_camera->_index, DCAM_IDSTR_MODEL, model, 64)) {
			send_error_text(camera, "Failed to get model info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (bus != NULL) {
	
		if (!dcam_getmodelinfo(C9100_13_camera->_index, DCAM_IDSTR_BUS, bus, 64)) {
			send_error_text(camera, "Failed to get bus info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (camera_id != NULL) {
	
		if (!dcam_getmodelinfo(C9100_13_camera->_index, DCAM_IDSTR_CAMERAID, camera_id, 64)) {
			send_error_text(camera, "Failed to get camera ID info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (camera_version != NULL) {
	
		if (!dcam_getmodelinfo(C9100_13_camera->_index, DCAM_IDSTR_CAMERAVERSION, camera_version, 64)) {
			send_error_text(camera, "Failed to get camera version info.");
			return CAMERA_ERROR;
		}	
	}
	
	if (driver_version != NULL) {
	
		if (!dcam_getmodelinfo(C9100_13_camera->_index, DCAM_IDSTR_DRIVERVERSION, driver_version, 64)) {
			send_error_text(camera, "Failed to get driver version info.");
			return CAMERA_ERROR;
		}	
	}
	
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_set_data_mode(GciCamera* camera, DataMode data_mode)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	if(C9100_13_camera == NULL)
		return CAMERA_ERROR;

	if (C9100_13_free_camera_resources(C9100_13_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;

	if( C9100_13_SetDataType(C9100_13_camera, data_mode) == CAMERA_ERROR ) {
	
		send_error_text(camera, "C9100_13_SetDataType function failed");
		return CAMERA_ERROR;
	}
  
  	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, data_mode);
  
	return CAMERA_SUCCESS;
}

static int  gci_C9100_13_camera_get_data_type(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	return C9100_13_camera->_data_type;
}

int gci_C9100_13_camera_set_CAMERA_EXTERNAL_TRIG_trigger_mode(GciC9100_13Camera* C9100_13_camera) 
{
	GciCamera *camera = (GciCamera *) C9100_13_camera; 

	if(C9100_13_camera->_trigger_mode == CAMERA_EXTERNAL_TRIG)
		return CAMERA_SUCCESS;

	if (C9100_13_free_camera_resources(C9100_13_camera) == CAMERA_ERROR)
		return CAMERA_ERROR;

	if (!dcam_settriggermode(C9100_13_camera->_hCam, DCAM_TRIGMODE_EDGE)) {
	
        send_error_text(camera, "dcam_settriggermode failed");
        return CAMERA_ERROR;
    }
	
	if (!dcam_settriggerpolarity(C9100_13_camera->_hCam, DCAM_TRIGPOL_NEGATIVE)) {
	
        send_error_text(camera, "dcam_settriggerpolarity failed");
        return CAMERA_ERROR;
    }
	
	C9100_13_camera->_trigger_mode = CAMERA_EXTERNAL_TRIG;
	
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_set_internal_trigger_mode(GciC9100_13Camera* C9100_13_camera) 
{
	unsigned long error;
	char buffer[255], string_buffer[255];
	GciCamera *camera = (GciCamera *) C9100_13_camera; 

	if(C9100_13_camera->_trigger_mode == CAMERA_INTERNAL_TRIG)
		return CAMERA_SUCCESS;

	if (C9100_13_free_camera_resources(C9100_13_camera) == CAMERA_ERROR)
		return CAMERA_ERROR; 

	if (!dcam_settriggermode(C9100_13_camera->_hCam, DCAM_TRIGMODE_INTERNAL)) {
	
		error = dcam_getlasterror(C9100_13_camera->_hCam, buffer, 255);
	
		C9100_13_error_code_to_string(error, string_buffer);  
	
		send_error_text(camera, "dcam_settriggermode failed: %s", string_buffer);

        return CAMERA_ERROR;
    }
	
	C9100_13_camera->_trigger_mode = CAMERA_INTERNAL_TRIG;
	
	return CAMERA_SUCCESS;
}

static int gci_C9100_13_camera_fire_internal_trigger(GciCamera* camera) 
{
	unsigned long error;
	char buffer[255], string_buffer[255];
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	//Software trigger
	if (!dcam_firetrigger(C9100_13_camera->_hCam)) {
		error = dcam_getlasterror(C9100_13_camera->_hCam, buffer, 255);
		C9100_13_error_code_to_string(error, string_buffer);  
		send_error_text(camera, "dcam_firetrigger failed: %s", string_buffer);
        return CAMERA_ERROR;
    }
	return CAMERA_SUCCESS;
}

int gci_C9100_13_camera_is_CAMERA_EXTERNAL_TRIG_trigger_mode(GciC9100_13Camera* C9100_13_camera)
{
	return C9100_13_camera->_trigger_mode;
}

static int gci_C9100_13_camera_set_trigger_mode(GciCamera* camera, TriggerMode trig_mode)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	if (trig_mode == CAMERA_EXTERNAL_TRIG)
		return gci_C9100_13_camera_set_CAMERA_EXTERNAL_TRIG_trigger_mode(C9100_13_camera);
	
	return gci_C9100_13_camera_set_internal_trigger_mode(C9100_13_camera);
}

int gci_C9100_13_supports_binning(GciCamera* camera)
{
	return 1;
}


static int gci_C9100_13_camera_set_binning_mode(GciCamera* camera, BinningMode binning)
{
	long error;
	char error_string[255];
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	if ((binning != 1) && (binning != 2) && (binning != 4))
		return CAMERA_ERROR;
   	
   	if( C9100_13_free_camera_resources(C9100_13_camera) == CAMERA_ERROR) {

		send_error_text(camera, "C9100_13_free_camera_resources failed");
		return CAMERA_ERROR;	
	}
   
	C9100_13_camera->_binning = binning;
	
	if (!dcam_setbinning(C9100_13_camera->_hCam, binning)) {
	
		error = dcam_getlasterror(C9100_13_camera->_hCam, error_string, 255);
	
		send_error_text(camera, "Failed to set binning mode: erro: %d, %s", error, error_string);
		return CAMERA_ERROR;
	}	
	
	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BINNING, binning);
	
	GCI_ImagingWindow_SetBinningSize(camera->_camera_window, binning);  
  
	return CAMERA_SUCCESS;
}


double  gci_C9100_13_camera_get_temperature(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;
	double temperature;
	
	dcam_getpropertyvalue(C9100_13_camera->_hCam, DCAM_IDPROP_SENSORTEMPERATURE, &temperature);      
	
	return temperature;
}


int  gci_C9100_13_camera_set_photon_mode(GciCamera* camera, int mode)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;

	dcam_setpropertyvalue(C9100_13_camera->_hCam, DCAM_IDPROP_PHOTONIMAGINGMODE, mode );         
	
	return CAMERA_SUCCESS;  
}


static int VOID_C9100_13_CAMERA_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (GciC9100_13Camera *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (GciC9100_13Camera *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

/*
int  gci_camera_signal_pre_capture_handler_connect (GciC9100_13Camera* C9100_13_camera, C9100_CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	GciCamera* camera = (GciCamera*) C9100_13_camera;

	if( GCI_Signal_Connect(&(camera->signal_table), "PreCapture", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for PreCapture signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int  gci_camera_signal_post_capture_handler_connect (GciC9100_13Camera* C9100_13_camera, C9100_CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	GciCamera* camera = (GciCamera*) C9100_13_camera;

	if( GCI_Signal_Connect(&(camera->signal_table), "PostCapture", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for PostCapture signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

*/
static int gci_C9100_13_camera_initialise (GciCamera* camera)
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera;

	if(camera == NULL)
		return CAMERA_ERROR;

	gci_camera_set_description(camera, "Hamamatsu C9100-13 Camera");
	gci_camera_set_name(camera, "C9100-13 Camera");
	camera->_bits = 16;
	C9100_13_camera->_binning = 1;
	C9100_13_camera->_trigger_mode = CAMERA_INTERNAL_TRIG;
	
	gci_camera_set_extra_panel_uir(camera, "gci_C9100-13_camera_ui.uir", EXTRA_PNL); 

	gci_camera_set_exposure_range(camera, 1, 5000, "ms", VAL_INTEGER);  

  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, C9100_13_Camera_onDataMode, C9100_13_camera) < 0)
		return CAMERA_ERROR;
  	
  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_QUIT, C9100_13_Camera_onExtrasQuit, C9100_13_camera) < 0)
		return CAMERA_ERROR;
  	
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_BINNING, C9100_13_Camera_onBinning, C9100_13_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_PHOTONMODE, C9100_13_Camera_SetPhotonMode, C9100_13_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, C9100_13_Camera_onSetSizePosition, C9100_13_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, C9100_13_Camera_onSetSizePosition, C9100_13_camera) < 0)
		return CAMERA_ERROR;
 
    if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, C9100_13_Camera_onSetSizePosition, C9100_13_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, C9100_13_Camera_onSetSizePosition, C9100_13_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, C9100_13_Camera_onSetSizePosition, C9100_13_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_PRSETWINDOW, C9100_13_Camera_onPresetSubWindow, C9100_13_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_TIMER, C9100_13_Camera_TimerTick, C9100_13_camera) < 0)
		return CAMERA_ERROR;
	
	return CAMERA_SUCCESS;
}

static int gci_C9100_13_get_colour_type (GciCamera* camera)
{
	return MONO_TYPE;
}


int gci_C9100_13_camera_destroy(GciCamera* camera)
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera;

	gci_camera_power_down(camera);

	--number_of_C9100_13_cameras;

	if(C9100_13_camera->saved_settings != NULL) {
	
		free(C9100_13_camera->saved_settings);
		C9100_13_camera->saved_settings = NULL;	
	}
  	
  	return CAMERA_SUCCESS;
}

static int  gci_C9100_13_camera_get_gain(GciCamera* camera, CameraChannel channel, double *gain)
{
	*gain = camera->_gain;
	
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_save_settings (GciCamera* camera, const char *filepath, const char *mode)
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera;  

	if(C9100_13CameraSaveIniCameraSettings(C9100_13_camera, filepath, mode) == CAMERA_ERROR) {
	
		send_error_text(camera, "Failed to save orca settings for device %s.", UIMODULE_GET_DESCRIPTION(camera));
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


static int gci_C9100_13_load_settings (GciCamera* camera, const char *filepath) 
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera;  

	if( C9100_13CameraLoadIniCameraSettings(C9100_13_camera, filepath) == CAMERA_ERROR) {
	
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int gci_C9100_13_camera_set_blacklevel(GciCamera* camera, CameraChannel c, double black_level)
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera; 
	GciOrcaCamera* orca_camera = (GciOrcaCamera*) camera;  
	
	if ((black_level < 0) || (black_level > 255))
		return CAMERA_ERROR; 

	C9100_13_camera->_blacklevel = black_level;

	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, black_level);

	return OrcaSetFeature(orca_camera, DCAM_IDFEATURE_BRIGHTNESS, (double)RoundRealToNearestInteger(black_level));
}


int  gci_C9100_13_camera_get_blacklevel(GciCamera* camera, CameraChannel c, double *val)
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera;  
	GciOrcaCamera* orca_camera = (GciOrcaCamera*) camera;  
	
	*val = C9100_13_camera->_blacklevel; 
	
	return CAMERA_SUCCESS;
}

int gci_C9100_13_camera_get_highest_data_mode(GciCamera* camera, DataMode *data_mode)
{
	*data_mode = BPP14;
  
	return CAMERA_SUCCESS;
}


static int gci_C9100_13_camera_set_light_mode(GciCamera* camera, LightMode light_mode)
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera;           
	GciOrcaCamera* orca_camera = (GciOrcaCamera*) camera;    
	
	if( (light_mode != LIGHT_MODE_LO) && (light_mode != LIGHT_MODE_HI) ) {
	
		send_error_text(camera, "Light mode not valid");
		return CAMERA_ERROR;
	}
	
	if (OrcaSetFeature (orca_camera, DCAM_IDFEATURE_LIGHTMODE, (double)light_mode) == CAMERA_ERROR ) {
		send_error_text(camera, "OrcaSetLightMode function failed");
		return CAMERA_ERROR;
	}
  
	C9100_13_camera->_light_mode = light_mode;

  	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_LIGHTMODE, light_mode);
  
	return CAMERA_SUCCESS;
}


int  gci_C9100_13_camera_get_light_mode(GciCamera* camera)
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) camera; 

	return C9100_13_camera->_light_mode;
}


static double gci_C9100_13_camera_get_fps(GciCamera* camera)
{
	GciC9100_13Camera *C9100_13_camera = (GciC9100_13Camera *) camera;
	double fps;
	
	dcam_getpropertyvalue(C9100_13_camera->_hCam, DCAM_IDPROP_INTERNALFRAMERATE, &fps);      
	
	return fps;
}

GciCamera* gci_C9100_13_camera_new(HINSTANCE hInstance, const char *name, const char *description)
{
	GciC9100_13Camera* C9100_13_camera = (GciC9100_13Camera*) malloc(sizeof(GciC9100_13Camera));
	GciCamera *camera = (GciCamera*) C9100_13_camera;
	
	C9100_13_camera->_ghInstance = hInstance;
	C9100_13_camera->_index = number_of_C9100_13_cameras++;
	C9100_13_camera->_hCam = NULL;
	C9100_13_camera->saved_settings = (GciC9100_13Camera*) malloc (sizeof(GciC9100_13Camera)); 

	gci_camera_constructor (camera, name, description);      
	
	if(C9100_13_camera->saved_settings == NULL) {

		printf("%s", "C9100_13_camera->saved_settings memory allocation failed");
		return NULL;	
	}

	CAMERA_VTABLE_PTR(camera).initialise = gci_C9100_13_camera_initialise;
	CAMERA_VTABLE_PTR(camera).power_up = gci_C9100_13_camera_power_up;
	CAMERA_VTABLE_PTR(camera).power_down = gci_C9100_13_camera_power_down;
	CAMERA_VTABLE_PTR(camera).set_exposure_time = gci_C9100_13_camera_set_exposure_time;
	CAMERA_VTABLE_PTR(camera).set_gain = gci_C9100_13_camera_set_gain; 
	CAMERA_VTABLE_PTR(camera).get_gain = gci_C9100_13_camera_get_gain;
	CAMERA_VTABLE_PTR(camera).set_live_mode = gci_C9100_13_camera_set_live_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_mode = gci_C9100_13_camera_set_snap_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_sequence_mode = gci_C9100_13_camera_set_snap_sequence_mode;
	CAMERA_VTABLE_PTR(camera).get_image = gci_C9100_13_camera_get_image; 
	CAMERA_VTABLE_PTR(camera).get_min_size = gci_C9100_13_camera_get_min_size;
	CAMERA_VTABLE_PTR(camera).get_max_size = gci_C9100_13_camera_get_max_size;
	CAMERA_VTABLE_PTR(camera).get_info =	gci_C9100_13_camera_get_info;
	CAMERA_VTABLE_PTR(camera).save_settings = gci_C9100_13_save_settings;
	CAMERA_VTABLE_PTR(camera).load_settings = gci_C9100_13_load_settings;
	CAMERA_VTABLE_PTR(camera).set_datamode = gci_C9100_13_camera_set_data_mode;
	CAMERA_VTABLE_PTR(camera).get_highest_datamode = gci_C9100_13_camera_get_highest_data_mode;
	CAMERA_VTABLE_PTR(camera).set_lightmode = gci_C9100_13_camera_set_light_mode;
	CAMERA_VTABLE_PTR(camera).get_lightmode = gci_C9100_13_camera_get_light_mode;
	CAMERA_VTABLE_PTR(camera).get_colour_type = gci_C9100_13_get_colour_type;
	CAMERA_VTABLE_PTR(camera).set_default_settings = gci_C9100_13_camera_set_default_settings;
	CAMERA_VTABLE_PTR(camera).save_settings_as_default = gci_C9100_13_camera_save_settings_as_default;
	CAMERA_VTABLE_PTR(camera).save_state = gci_C9100_13_camera_save_state;
	CAMERA_VTABLE_PTR(camera).restore_state = gci_C9100_13_camera_restore_state;
	CAMERA_VTABLE_PTR(camera).set_size_position = gci_C9100_13_camera_set_size_position;
	CAMERA_VTABLE_PTR(camera).get_size_position = gci_C9100_13_camera_get_size_position;
	CAMERA_VTABLE_PTR(camera).destroy = gci_C9100_13_camera_destroy;
	CAMERA_VTABLE_PTR(camera).set_binning_mode = gci_C9100_13_camera_set_binning_mode;
	CAMERA_VTABLE_PTR(camera).get_binning_mode = gci_C9100_13_camera_get_binning_mode;
	CAMERA_VTABLE_PTR(camera).supports_binning = gci_C9100_13_supports_binning;
	CAMERA_VTABLE_PTR(camera).set_blacklevel = gci_C9100_13_camera_set_blacklevel;
	CAMERA_VTABLE_PTR(camera).get_blacklevel = gci_C9100_13_camera_get_blacklevel;
	CAMERA_VTABLE_PTR(camera).set_trigger_mode = gci_C9100_13_camera_set_trigger_mode;
	CAMERA_VTABLE_PTR(camera).fire_internal_trigger = gci_C9100_13_camera_fire_internal_trigger;
	CAMERA_VTABLE_PTR(camera).get_fps = gci_C9100_13_camera_get_fps;
	
	return camera;
}


