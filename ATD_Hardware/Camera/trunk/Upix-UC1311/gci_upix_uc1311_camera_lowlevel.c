#include "windows.h"

#include "gci_upix_uc1311_camera.h"
#include "FreeImageAlgorithms_IO.h"

#include "ThreadDebug.h"
#include <utility.h>

static int camera_number = 1;
static Upix1311Camera *current_camera = NULL;
static FIBITMAP *fib = NULL;

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

static void upix_1311_get_error_string(int error, char *buffer)
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

static void upix_1311_display_error_string(Upix1311Camera* camera, int error)
{
	char error_str[500] = "";

	upix_1311_get_error_string(error, error_str);

	send_error_text(camera, error_str);
}

void upix_camera_get_lock(Upix1311Camera* camera)
{
	if(GciCmtGetLock(camera->upix_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}
}

void upix_camera_release_lock(Upix1311Camera* camera)
{
	if(GciCmtReleaseLock(camera->upix_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}
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
	
	printf("SnapThreadCallback Started\n");

	if(current_camera->prevent_callback)
		return FALSE;

	upix_camera_get_lock(current_camera);

	if(CameraGetImageSize(&width, &height)!= STATUS_OK) {
		current_camera->image_callback_completed = 1;

		upix_camera_release_lock(current_camera);
		
		return FALSE;
	}

	if(fib != NULL)
		FreeImage_Unload(fib);

	fib = FIA_LoadColourFIBFromArrayData (pBuffer, 24, width, height, 0, 0, COLOUR_ORDER_RGB);

	current_camera->image_callback_completed = 1;

	upix_camera_release_lock(current_camera);

	printf("SnapThreadCallback Finished\n");

	return TRUE;
}

static void wait_for_image(Upix1311Camera* camera)
{
	double start_time = Timer();

	while(current_camera->image_callback_completed == 0 && ((Timer() - start_time) < 2.0)) {
		ProcessSystemEvents();
		Delay(0.1);
	}

	if(current_camera->image_callback_completed)
		printf("Got Image\n");

	return;
}

int upix_get_serial_number(Upix1311Camera* upix_camera, BYTE *serial_number)
{
	// Serial number is 10 bytes
	BYTE serialNumber[10] = "";

	// We need to be initialised
	if(upix_camera->initialised == 0)
		return CAMERA_ERROR;

	// Get serial number
	CameraReadSerial(serial_number);

	return CAMERA_SUCCESS;
}

int get_real_exposure_from_upix_exposure(Upix1311Camera *upix_camera, int upix_exposure, int *real_exposure)
{
	int err, exp; 
	unsigned int rawTime;
	
	CameraGetRowTime(&rawTime); //us
	
	*real_exposure = (int)((float)upix_exposure*rawTime/1000.0);

	return CAMERA_SUCCESS;
}

int get_upix_exposure_from_real_exposure(Upix1311Camera *upix_camera, int real_exposure, int *upix_exposure)
{
	int err, exp; 
	unsigned int rawTime;
	
	CameraGetRowTime(&rawTime); //us

	*upix_exposure = (real_exposure * 1000.0) / rawTime;

	return CAMERA_SUCCESS;
}

int get_exposure(Upix1311Camera *upix_camera, double *exposure)
{
	int err, upix_exposure, real_exposure;

	Upix1311Camera* camera = (Upix1311Camera*) upix_camera;

	get_upix_exposure_from_real_exposure(upix_camera, exposure, &upix_exposure);

	if ((err = CameraGetExposureTime(&upix_exposure)) != STATUS_OK )
	{
		upix_1311_display_error_string(upix_camera, err);

		return CAMERA_ERROR;
	}

	get_real_exposure_from_upix_exposure(upix_camera, upix_exposure, &real_exposure);

	*exposure = real_exposure;

	return CAMERA_SUCCESS;
}

int set_exposure(Upix1311Camera *upix_camera, int exposure)
{
	int err, returned_exp, upix_exposure, max_exposure;
	unsigned short e;

	Upix1311Camera* camera = (Upix1311Camera*) upix_camera;

	get_upix_exposure_from_real_exposure(upix_camera, exposure, &upix_exposure);

	CameraGetMaxExposureTime(&e);
	
	if(upix_exposure > e)
		upix_exposure = e;

	if ((err = CameraSetExposureTime(upix_exposure)) != STATUS_OK )
	{
		upix_1311_display_error_string(upix_camera, err);

		return CAMERA_ERROR;
	}

	if ((err = CameraGetExposureTime(&returned_exp)) != STATUS_OK )
	{
		upix_1311_display_error_string(upix_camera, err);

		return CAMERA_ERROR;
	}

	upix_camera->exposure = exposure;

	return CAMERA_SUCCESS;
}

int set_gain(Upix1311Camera *upix_camera, int gain)
{
	int err;

	Upix1311Camera* camera = (Upix1311Camera*) upix_camera;

	if(gain < 1)
		gain = 1;

	if ((err = CameraSetAnalogGain(gain)) != STATUS_OK )
	{
		upix_1311_display_error_string(upix_camera, err);

		return CAMERA_ERROR;
	}

	upix_camera->gain = gain;

	return CAMERA_SUCCESS;
}

int get_gain(Upix1311Camera *upix_camera, double *gain)
{
	int err;
	unsigned short upix_gain;

	Upix1311Camera* camera = (Upix1311Camera*) upix_camera;

	if ((err = CameraGetAnalogGain(&upix_gain)) != STATUS_OK )
	{
		upix_1311_display_error_string(upix_camera, err);

		return CAMERA_ERROR;
	}

	*gain = upix_gain;

	return CAMERA_SUCCESS;
}

int upix_initialise (Upix1311Camera* upix_camera)
{
	int err, val;
	unsigned short max_exposure = 0;
	BYTE num, serialNumber[10] = "";
	GciCamera* camera = (GciCamera*) upix_camera;
	HWND hwnd = GCI_ImagingWindow_GetImageViewHandle(camera->_camera_window);

	if(upix_camera->initialised == 1)
		return CAMERA_SUCCESS;

	CameraGetMultiCameraNumber(&num);

	// It would seem like you have to pass hwnd here 
	// Regardless of whether you just want IMAGEMODE_CALLBACK_ONLY or you
	// get crashes.
	if ((err = CameraInit(SnapThreadCallback,
									IMAGEMODE_CALLBACK_ONLY,
									R1280_1024,
									hwnd, 
									NULL,
									upix_camera->camera_number)) != STATUS_OK )
	{
		upix_1311_display_error_string(camera, err);

		return CAMERA_ERROR;
	}
	
	//set_snap_mode(camera);
	upix_camera->initialised = 1;

	// Get serial number
	upix_get_serial_number(camera, serialNumber);

	// High sensitivity always
	CameraSetFrameSpeed(FRAME_SPEED_NORMAL);

	current_camera = upix_camera;

	return CAMERA_SUCCESS;
}

int upix_uninitialise (Upix1311Camera* camera)
{
	int err;
	Upix1311Camera* upix_camera = (Upix1311Camera*) camera;

	if(upix_camera->initialised == 0)
		return CAMERA_SUCCESS;

	if((err = CameraUnInit()) != STATUS_OK) {

		upix_1311_display_error_string(camera, err);

		return CAMERA_ERROR;
	}	

	upix_camera->initialised = 0;;
}

int camera_play(Upix1311Camera* camera, DS_SNAP_MODE SnapMode)
{
	int err = 1;
	BYTE snap_mode;

	// Already playing
	if(camera->camera_stopped == 0) {
		printf("camera_play - stop\n");
		return CAMERA_SUCCESS;
	}

	upix_initialise(camera);

	if((err = CameraPlay(SnapMode)) != STATUS_OK) {
		upix_1311_display_error_string(camera, err);
		return CAMERA_ERROR;
	}

	if (SnapMode != SNAP_MODE_CONTINUATION)
	{
		CameraSetAeState(FALSE); 
	}

	current_camera->prevent_callback = 0;

	camera->camera_stopped = 0;

	printf("camera_play - stop\n");

	return CAMERA_SUCCESS;
}

int camera_stop(Upix1311Camera* camera)
{
	int err = 1;
	BYTE snap_mode = 0;

	// Already stopped.
	if(camera->camera_stopped == 1) {
		printf("camera_stop - stop already stopped\n");
		return CAMERA_SUCCESS;
	}

	current_camera->prevent_callback = 1;
	//wait_for_image(camera);

	upix_camera_get_lock(camera);

	if((err = CameraStop()) != STATUS_OK) {

		upix_1311_display_error_string(camera, err);

		upix_camera_release_lock(camera);

		return CAMERA_ERROR;
	}	

	upix_camera_release_lock(camera);

	upix_uninitialise(camera);

	camera->camera_stopped = 1;

	return CAMERA_SUCCESS;
}


int snap_image (Upix1311Camera* camera)
{
	int err;
	BYTE snap_mode = 0;

	current_camera->image_callback_completed = 0;

	if((err = CameraGetSnapMode(&snap_mode)) != STATUS_OK) {

		upix_1311_display_error_string(camera, err);

		return CAMERA_ERROR;
	}	

	if(snap_mode == SNAP_MODE_CONTINUATION)
		camera_stop(camera);

	camera_play(camera, SNAP_MODE_SOFT_TRIGGER);

	//if((err = CameraSoftTriggerStart()) != STATUS_OK) {
	//	upix_1311_display_error_string(camera, err);
	//	return CAMERA_ERROR;
	//}

	//wait_for_image(camera);

	return CAMERA_SUCCESS;
}

FIBITMAP* get_image(Upix1311Camera* camera)
{
	int err;
	FIBITMAP *tmp = NULL;
	BYTE snap_mode = 0;

	current_camera->image_callback_completed = 0;

	if((err = CameraGetSnapMode(&snap_mode)) != STATUS_OK) {

		upix_1311_display_error_string(camera, err);

		return CAMERA_ERROR;
	}	

	if(snap_mode == SNAP_MODE_SOFT_TRIGGER) {
		if((err = CameraSoftTriggerStart()) != STATUS_OK) {
			upix_1311_display_error_string(camera, err);
			return CAMERA_ERROR;
		}
	}

	wait_for_image(camera);

	if(GciCmtGetLock(current_camera->upix_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}

	tmp = FreeImage_Clone(fib);

	if(GciCmtReleaseLock(current_camera->upix_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}

	camera->image_callback_completed = 0;

	return tmp;
}

int start_live(Upix1311Camera* camera)
{
	camera_stop(camera);
	camera_play(camera, SNAP_MODE_CONTINUATION);
}

int set_power(Upix1311Camera* camera, int power)
{
	return CAMERA_SUCCESS;
}

int initialise (Upix1311Camera* camera)
{
	int i=0;
	BYTE num;
	char name[500] = "";

	CameraGetMultiCameraNumber(&num);

	// Initialise always
	upix_initialise (camera);

	return CAMERA_SUCCESS;
}
