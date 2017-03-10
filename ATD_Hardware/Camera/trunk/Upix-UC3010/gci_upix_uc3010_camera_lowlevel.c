#include "windows.h"

#include "gci_upix_uc3010_camera.h"
#include "gci_upix_uc3010_camera_ui.h"
#include "gci_upix_uc3010_camera_lowlevel.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Utilities.h"

#include "ThreadDebug.h"
#include <utility.h>

#include "profile.h"

static int camera_number = 1;
static Upix3010Camera *current_camera = NULL;  // Global for the callback: SnapThreadCallback
static HWND hidden_hwnd = 0;

#define UPIX_BUG_WORK_AROUND

static int flag_lock_count = 0;
static int image_lock_count = 0;

static const char* const Errors[] = 
{
        "Internal Error",
        "Camera Not Found",
        "Out of Memory",
        "Hardware IO Error",
        "Invalid Parameter",
        "Out of Bound Parameter",
        "Error Creating File",
        "Invalid File Format"
};

static void upix_3010_get_error_string(int error, char *buffer)
{
	if(error > 0) {
		strcpy(buffer, "Operation Success");
		return;
	}

	if(error <= 0 && error >= -7) {
		strcpy(buffer, Errors[-error]);
		return;
	}

	strcpy(buffer, "Unknown Error");
}

static void upix_3010_display_error_string(Upix3010Camera* camera, const char *custom_error_str, int error)
{
	char error_str[500] = "";

	upix_3010_get_error_string(error, error_str);

	printf("%s\n", error_str);

	//send_error_text(camera, error_str);
	logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s - %s", custom_error_str, error_str);  
	
}

#pragma optimize("f", on) 

void upix_camera_get_flag_lock(Upix3010Camera* upix_camera)
{
	GciCamera* camera = (GciCamera*) upix_camera;

	#ifdef VERBOSE_DEBUG
		printf("Getting Upix Flag Lock %d\n", GetCurrentThreadId());
	#endif

	if(GciCmtGetLock(upix_camera->upix_flag_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock_flag Failed");
	}

	flag_lock_count++;

	#ifdef VERBOSE_DEBUG
		printf("Got Upix Flag Lock %d %d\n", GetCurrentThreadId(), flag_lock_count);
	#endif
}

void upix_camera_release_flag_lock(Upix3010Camera* upix_camera)
{
	GciCamera* camera = (GciCamera*) upix_camera;

	#ifdef VERBOSE_DEBUG
		printf("Releasing Upix Flag Lock %d\n", GetCurrentThreadId());
	#endif

	if(GciCmtReleaseLock(upix_camera->upix_flag_lock) < 0) {
		send_error_text(camera, "GciCmtReleaseLock_flag Failed");
	}

	flag_lock_count--;

	#ifdef VERBOSE_DEBUG
		printf("Released Upix Flag Lock %d %d\n", GetCurrentThreadId(), flag_lock_count);
	#endif
}

void upix_camera_get_lock(Upix3010Camera* upix_camera)
{
	GciCamera* camera = (GciCamera*) upix_camera;

	#ifdef VERBOSE_DEBUG
		printf("upix_camera_get_lock before %d\n", GetCurrentThreadId());
	#endif

	if(GciCmtGetLock(upix_camera->upix_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}

	image_lock_count++;

	#ifdef VERBOSE_DEBUG
		printf("upix_camera_get_lock after %d %d\n", GetCurrentThreadId(), image_lock_count);
	#endif
}

void upix_camera_release_lock(Upix3010Camera* upix_camera)
{
	GciCamera* camera = (GciCamera*) upix_camera;

	#ifdef VERBOSE_DEBUG
		printf("upix_camera_release_lock before %d\n", GetCurrentThreadId());
	#endif

	if(GciCmtReleaseLock(upix_camera->upix_lock) < 0) {
		send_error_text(camera, "GciCmtReleaseLock Failed");
	}

	image_lock_count--;

	#ifdef VERBOSE_DEBUG
		printf("upix_camera_release_lock after %d %d\n", GetCurrentThreadId(), image_lock_count);
	#endif
}

void ErrorString(LPTSTR buffer) 
{ 
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) buffer,
        0, NULL );
}

static int wait_for_image(Upix3010Camera* upix_camera)
{
	GciCamera* camera = (GciCamera*) upix_camera;

	double start_time = Timer();
	double timeout = (camera->_exposure_time / 1000.0) + 2.0; 
	int image_callback_completed=0;

	printf("waiting for image\n");

	upix_camera_get_flag_lock(upix_camera);
	image_callback_completed = upix_camera->image_callback_completed;
	upix_camera_release_flag_lock(upix_camera);

	while(image_callback_completed == 0 &&
		((Timer() - start_time) < timeout)) {
	
		ProcessSystemEvents();
		Delay(0.02);

		upix_camera_get_flag_lock(upix_camera);
		image_callback_completed = upix_camera->image_callback_completed;
		upix_camera_release_flag_lock(upix_camera);

	}

	#ifdef VERBOSE_DEBUG
	if(upix_camera->image_callback_completed == 0) {
		printf("Timed out! current_camera->prevent_callback %d\n", upix_camera->prevent_callback);
		return -1;
	}
	else
		printf("waited complete - Waited for %f\n", (Timer() - start_time));
	#endif

	return 0;
}

/*==============================================================
Function:	SnapThreadCallback
Parameters:   *pBuffer - Current Image buffer. Data:BYTE,Format:RGB24,Length:ImageWidth*ImageHeight*3

Remark: For real time live video data processing.
* Example below shows drawing a line on the live video
--------------------------------------------------------------*/
static int CALLBACK SnapThreadCallback(BYTE *pBuffer)
{
	int width, height;
	FIBITMAP *tmp1, *tmp2;

	PROFILE_START("SnapThreadCallback");

	#ifdef VERBOSE_DEBUG
		printf("SnapThreadCallback 1 %d\n", GetCurrentThreadId());
	#endif

	upix_camera_get_flag_lock(current_camera);

	// Don't call this callback again if the image_callback_completed member has not been reset by get_image.
	if(current_camera->image_callback_completed == 1) {
		PROFILE_STOP("SnapThreadCallback");
		upix_camera_release_flag_lock(current_camera);
		return FALSE;
	}

	if(current_camera->prevent_callback) {
		PROFILE_STOP("SnapThreadCallback");
		upix_camera_release_flag_lock(current_camera);
		return FALSE;
	}

	upix_camera_release_flag_lock(current_camera);

	if(CameraGetImageSize(&width, &height)!= STATUS_OK) {
		current_camera->callback_thread_running = 0;
		PROFILE_STOP("SnapThreadCallback");
		return FALSE;
	}

	if(pBuffer == NULL) {
		current_camera->callback_thread_running = 0;
		PROFILE_STOP("SnapThreadCallback");
		return FALSE;
	}

	tmp1 = FIA_LoadColourFIBFromArrayData (pBuffer, 24, width, height, 0, 0, COLOUR_ORDER_RGB);

	// The uc3010 camera returns an image with a one pixel black border so we remove it here.
	tmp2 = FIA_FastCopy(tmp1, 1, 1,
		FreeImage_GetWidth(tmp1) - 1, FreeImage_GetHeight(tmp1) - 1);
	
	if(tmp1 != NULL) {
		FreeImage_Unload(tmp1);
		tmp1=NULL;
	}

	#ifdef VERBOSE_DEBUG
	printf("threaded_callback_image_count %d  get_image_count %d\n", current_camera->threaded_callback_image_count, current_camera->get_image_count);
	#endif

	if(tmp2 != NULL) {

		current_camera->threaded_callback_image_count++;

		upix_camera_get_lock(current_camera);

		if(current_camera->fib != NULL) {
			FreeImage_Unload(current_camera->fib);
			current_camera->fib = NULL;
		}

		current_camera->fib = FreeImage_Clone(tmp2);

		current_camera->image_callback_completed = 1;

		upix_camera_release_lock(current_camera);

		#ifdef VERBOSE_DEBUG
		printf("UPIX_CAMERA_IMAGE_LOAD Finished\n");
		#endif

		if(tmp2 != NULL) {
			FreeImage_Unload(tmp2);
			tmp2=NULL;
		}
	}

	PROFILE_STOP("SnapThreadCallback");

	return TRUE;
}

FIBITMAP* get_image(Upix3010Camera* upix_camera)
{
	int err;
	FIBITMAP *fib = NULL;
	BYTE snap_mode = 0;
	GciCamera* camera = (GciCamera*) upix_camera;
	static double start_time;

#if 0
	static int fake_failure_flag = 0;
	fake_failure_flag++;
	if (fake_failure_flag > 20){  // fail every certain number of times
		if (fake_failure_flag > 27)  // fail for a number of times
			fake_failure_flag = 0;
		Delay(2.0);
		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s", "upix fake failure");
		ProcessSystemEvents();
		PROFILE_STOP("get_image");
		return NULL;
	}
#endif

	PROFILE_START("get_image");

	#ifdef VERBOSE_DEBUG
		printf("get_image: before upix_camera_get_flag_lock\n");
	#endif

	upix_camera_get_flag_lock(upix_camera);

	#ifdef VERBOSE_DEBUG
		printf("get_image: after upix_camera_get_flag_lock\n");
	#endif

	upix_camera->image_callback_completed = 0;
	upix_camera->prevent_callback = 0;

	#ifdef VERBOSE_DEBUG
		printf("get_image: before upix_camera_release_flag_lock\n");
	#endif

	upix_camera_release_flag_lock(upix_camera);

	#ifdef VERBOSE_DEBUG
		printf("get_image: after upix_camera_release_flag_lock\n");

	#endif
	if(upix_camera->run_mode == UPIX_RUNMODE_PLAY_SNAP) {
		
		//  Currently PreCapture triggers the shutter which sends a pulse to the camera, this must occur after dcam_capture
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "PreCapture", GCI_VOID_POINTER, camera);
	
		#ifdef VERBOSE_DEBUG
		printf("SoftTriggerStart: Time from last call %f\n", (Timer() - start_time));
		#endif

		start_time = Timer();

		if((err = CameraSoftTriggerStart()) != STATUS_OK) {
			
			upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

			// We need this to stop getting errors.
			//camera_play(upix_camera, 1);
			//Delay(3);
			//camera_play(upix_camera, 0);

			PROFILE_STOP("get_image");

			return NULL;
		}
	}

	if(wait_for_image(upix_camera) < 0) {

		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s", "upix wait_for_image timed out.");  
		ProcessSystemEvents();

		return NULL;
	}

	if(upix_camera->image_callback_completed == 0) {
		// We have no new image the bloody dll is not calling the callback again
		//camera_play(upix_camera, 1);
		//wait_for_image(upix_camera);
		//camera_play(upix_camera, 0);

		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s", "upix callback failed to complete");  
		ProcessSystemEvents();

		PROFILE_STOP("get_image");

		return NULL;
	}

	// Need to retur a copy as upix_camera->fib can be deleted from the windows message
	// While icsviewer is using it.
	upix_camera_get_lock(current_camera);
	fib = FreeImage_Clone(upix_camera->fib);
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "PostCapture", GCI_VOID_POINTER, camera);
	upix_camera_release_lock(current_camera);

	PROFILE_STOP("get_image");

	return fib;
}

#pragma optimize("f", off) 

int upix_get_serial_number(Upix3010Camera* upix_camera, BYTE *serial_number)
{
	Upix3010Camera* camera = (Upix3010Camera*) upix_camera;
	int err;
	// Serial number is 10 bytes
	BYTE serialNumber[10] = "";

	// Get serial number
	if((err = CameraReadSerial(serial_number)) != STATUS_OK) {
		upix_3010_display_error_string(camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int get_real_exposure_from_upix_exposure(Upix3010Camera *upix_camera, int upix_exposure, double *real_exposure)
{
	Upix3010Camera* camera = (Upix3010Camera*) upix_camera;
	int err;
	unsigned int rowTime;
	
	if((err = CameraGetRowTime(&rowTime)) != STATUS_OK) {
		upix_3010_display_error_string(camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	} //us
	
	*real_exposure = (double)((float)upix_exposure*rowTime/1000.0);

	*real_exposure = (double)RoundRealToNearestInteger(*real_exposure*10.0)/10.0;  // Round to 2dp

	return CAMERA_SUCCESS;
}

int get_upix_exposure_from_real_exposure(Upix3010Camera *upix_camera, double real_exposure, int *upix_exposure)
{
	Upix3010Camera* camera = (Upix3010Camera*) upix_camera;
	int err;
	unsigned int rowTime;
	
	if((err = CameraGetRowTime(&rowTime)) != STATUS_OK) {
		upix_3010_display_error_string(camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}  //us

	*upix_exposure = (int) ((real_exposure * 1000.0) / rowTime);

	return CAMERA_SUCCESS;
}

int get_exposure(Upix3010Camera *upix_camera, double *exposure)
{
	int err, upix_exposure;
	double real_exposure;

	Upix3010Camera* camera = (Upix3010Camera*) upix_camera;

	if ((err = CameraGetExposureTime(&upix_exposure)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	if(get_real_exposure_from_upix_exposure(upix_camera, upix_exposure, &real_exposure) == CAMERA_ERROR)
		return CAMERA_ERROR;

	*exposure = real_exposure;

	return CAMERA_SUCCESS;
}

int set_exposure(Upix3010Camera *upix_camera, double exposure)
{
	int err=0, returned_exp, upix_exposure;
	unsigned short e;

	Upix3010Camera* camera = (Upix3010Camera*) upix_camera;

	if(get_upix_exposure_from_real_exposure(upix_camera, exposure, &upix_exposure) == CAMERA_ERROR)
		return CAMERA_ERROR;

	if ((CameraGetMaxExposureTime(&e)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}
	
	if(upix_exposure < 1)
		upix_exposure = 1;

	if(upix_exposure > e)
		upix_exposure = e;

	if ((err = CameraSetExposureTime(upix_exposure)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	if ((err = CameraGetExposureTime(&returned_exp)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	if(get_real_exposure_from_upix_exposure(upix_camera, returned_exp, &exposure) == CAMERA_ERROR)
		return CAMERA_ERROR;

	return CAMERA_SUCCESS;
}

int set_colour_enhancement(Upix3010Camera *upix_camera, int status)
{
	int err;

	Upix3010Camera* camera = (Upix3010Camera*) upix_camera;

	if ((err = CameraSetColorEnhancement(status)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int set_colour_gain(Upix3010Camera *upix_camera, int red_gain, int green_gain, int blue_gain)
{
	int err;

	GciCamera* camera = (GciCamera*) upix_camera;

	red_gain = max(20, min(red_gain, 200));
	green_gain = max(20, min(green_gain, 200));
	blue_gain = max(20, min(blue_gain, 200));

	if ((err = CameraSetGain(red_gain, green_gain, blue_gain)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_RED_GAIN, red_gain);
	SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_GREEN_GAIN, green_gain);
	SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLUE_GAIN, blue_gain);

	return CAMERA_SUCCESS;
}

int get_colour_gain(Upix3010Camera *upix_camera, int *red_gain, int *green_gain, int *blue_gain)
{
	int err;

//	GciCamera* camera = (Upix3010Camera*) upix_camera;

	if ((err = CameraGetGain(red_gain, green_gain, blue_gain)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int set_one_time_auto_white_balance(Upix3010Camera *upix_camera)
{
	int err, was_live;

	GciCamera* camera = (GciCamera*) upix_camera;

	was_live = gci_camera_is_live_mode(camera);

	if ((err = CameraSetAWBState(1)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	if (was_live)
		gci_camera_set_live_mode(camera);

	return CAMERA_SUCCESS;
}

int set_gain(Upix3010Camera *upix_camera, int gain)
{
	int err;

	Upix3010Camera* camera = (Upix3010Camera*) upix_camera;

	gain = max(1, min(gain, 80));

	if ((err = CameraSetAnalogGain(gain)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int get_gain(Upix3010Camera *upix_camera, double *gain)
{
	int err;
	unsigned short upix_gain;

	Upix3010Camera* camera = (Upix3010Camera*) upix_camera;

	if ((err = CameraGetAnalogGain(&upix_gain)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	*gain = upix_gain;

	return CAMERA_SUCCESS;
}

int upix_hardware_initialise (Upix3010Camera* upix_camera)
{
	if (camera_play(upix_camera, 1) == CAMERA_ERROR){ //set live mode to start with, seems more stable, maybe because of broken dll - first snap does not work ever
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int upix_initialise (Upix3010Camera* upix_camera)
{
	int err;
	unsigned short max_exposure = 0;
	BYTE num, serialNumber[10] = "";
	GciCamera* camera = (GciCamera*) upix_camera;
	
	CameraGetMultiCameraNumber(&num);

	// It would seem like you have to pass hwnd here 
	// Regardless of whether you just want IMAGEMODE_CALLBACK_ONLY or you
	// get crashes.
	if ((err = CameraInit(SnapThreadCallback,
									IMAGEMODE_CALLBACK_ONLY,
									upix_camera->resolution,
									upix_camera->window_hwnd,
									NULL,
									upix_camera->camera_number)) != STATUS_OK )
	{
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);

		return CAMERA_ERROR;
	}

	//set_snap_mode(camera);
	upix_camera->initialised = 1;

	// Does not seem to work for the uc3010 returns garbage
	// Get serial number
	//if((err = upix_get_serial_number(upix_camera, serialNumber)) == CAMERA_ERROR) {
	//	upix_3010_display_error_string(camera, __FUNCTION__, err);
	//	return CAMERA_ERROR;
	//}

	// Binning or not always
	if((err = CameraSetSubsampleMode(SUBSAMPLE_MODE_SKIP)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	// Gamma=1 always
	if((err = CameraSetGamma(100)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	// Disable AE
	if((err = CameraSetAeState(0)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	// Contrast = 0 always
	if((err = CameraSetContrast(0)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	// Disable Colour Enhancement
	if((err = CameraSetColorEnhancement(0)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	// Light Frequency
	if((err = CameraSetLightFrquency(LIGHT_FREQUENCY_50HZ)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	// Strobe off
	if((err = CameraSetStrobeMode(STROBE_MODE_OFF)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	if((err = CameraSetFrameSpeed(FRAME_SPEED_NORMAL)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	// Global for the image callback
	current_camera = upix_camera;

	return CAMERA_SUCCESS;
}

int upix_uninitialise (Upix3010Camera* upix_camera)
{
	if (camera_stop(upix_camera) == CAMERA_ERROR){ //set snap mode to start with
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int upix_set_sensor_mode(Upix3010Camera* upix_camera, DS_FRAME_SPEED speed)
{
	if(CameraSetFrameSpeed(speed) != STATUS_OK)
		return CAMERA_ERROR;

	return CAMERA_SUCCESS;
}

int upix_get_sensor_mode(Upix3010Camera* upix_camera, DS_FRAME_SPEED* speed)
{
	if(CameraGetFrameSpeed((BYTE *)speed) != STATUS_OK)
		return CAMERA_ERROR;

	if(*speed)
		*speed = FRAME_SPEED_HIGH;
	else
		*speed = FRAME_SPEED_NORMAL;

	return CAMERA_SUCCESS;
}

int upix_set_resolution (Upix3010Camera* upix_camera, DS_RESOLUTION resolution)
{
	GciCamera* camera = (GciCamera*) upix_camera;
	int was_live=0;

	#ifdef VERBOSE_DEBUG
	printf("Set Resolution\n");
	#endif

	upix_camera->prevent_callback = 1;

	wait_for_image(upix_camera);

	was_live = gci_camera_is_live_mode(camera);

	// Check sensible values are passed
	if (resolution < R2048_1536)
		resolution = R2048_1536;
	
	if (resolution > R512_384)
		resolution = R512_384;

	// Does uninit
	if(camera_stop (upix_camera) == CAMERA_ERROR) {
		upix_camera->prevent_callback = 0;
		return CAMERA_ERROR;
	}

	upix_camera->resolution = resolution;

	if(upix_hardware_initialise (upix_camera) == CAMERA_ERROR) {
		upix_camera->prevent_callback = 0;	
		return CAMERA_ERROR;
	}

	if(was_live)
		gci_camera_set_live_mode(camera);

	upix_camera->prevent_callback = 0;

	return CAMERA_SUCCESS;
}

int camera_play(Upix3010Camera* upix_camera, int live)
{	
	// Start the camera in live mode (continuation) or ready it in another triggered mode
	GciCamera *camera = (GciCamera *) upix_camera; 
	int err = 1;
	BYTE current_snap_mode = 255;
	DS_SNAP_MODE SnapMode;

	#ifdef VERBOSE_DEBUG
	printf("camera_play - start\n");
	#endif

	if (live)
		SnapMode = SNAP_MODE_CONTINUATION;
	else {
		if (camera->_trigger_mode == CAMERA_EXTERNAL_TRIG)
			SnapMode = SNAP_MODE_EXTERNAL_TRIGGER;
		else 
			SnapMode = SNAP_MODE_SOFT_TRIGGER;
	}

	if (upix_initialise(upix_camera) == CAMERA_ERROR){
		return CAMERA_ERROR;
	}

	if (SnapMode != SNAP_MODE_CONTINUATION)
	{
		if((err = CameraSetAeState(FALSE)) != STATUS_OK) {
			upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
			return CAMERA_ERROR;
		}
	}
		
	if((err = CameraGetSnapMode(&current_snap_mode)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}

	if(current_snap_mode == SNAP_MODE_CONTINUATION) {
		Delay(0.1);
	}		

	if((err = CameraPlay(SnapMode)) != STATUS_OK) {
		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		return CAMERA_ERROR;
	}
	
	if(live)
		upix_camera->run_mode = UPIX_RUNMODE_PLAY_LIVE;
	else
		upix_camera->run_mode = UPIX_RUNMODE_PLAY_SNAP;

	return CAMERA_SUCCESS;
}

int camera_stop(Upix3010Camera* upix_camera)
{
	GciCamera* camera = (GciCamera*) upix_camera;

	int err = 1;
	BYTE snap_mode = 0;

	#ifdef VERBOSE_DEBUG
		printf("camera_stop: before upix_camera_get_flag_lock\n");
	#endif

	upix_camera_get_flag_lock(upix_camera);
	upix_camera->prevent_callback = 1;
	upix_camera_release_flag_lock(upix_camera);

	#ifdef VERBOSE_DEBUG
		printf("camera_stop: after upix_camera_get_flag_lock\n");
	#endif

	if((err = CameraStop()) != STATUS_OK) {

		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		upix_camera->prevent_callback = 0;

		return CAMERA_ERROR;
	}	
	
	if((err = CameraUnInit()) != STATUS_OK) {

		upix_3010_display_error_string(upix_camera, __FUNCTION__, err);
		upix_camera->prevent_callback = 0;
		
		return CAMERA_ERROR;
	}	

	upix_camera->prevent_callback = 0;

	upix_camera->run_mode = UPIX_RUNMODE_STOP;

	return CAMERA_SUCCESS;
}

static void upix_camera_get_image_lock(Upix3010Camera* upix_camera)
{
	GciCamera* camera = (GciCamera*) upix_camera;

	if(GciCmtGetLock(upix_camera->get_image_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}
}

static void upix_camera_release_image_lock(Upix3010Camera* upix_camera)
{
	GciCamera* camera = (GciCamera*) upix_camera;

	if(GciCmtReleaseLock(upix_camera->get_image_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}
}

int start_live(Upix3010Camera* upix_camera)
{
	if (camera_stop(upix_camera) == CAMERA_ERROR){
		return CAMERA_ERROR;
	}

	if (camera_play(upix_camera, 1) == CAMERA_ERROR){
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}