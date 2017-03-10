#ifndef __GCI_UPIX_3010_CAMERA__
#define __GCI_UPIX_3010_CAMERA__

#include "camera\gci_camera.h"
#include "UC3010API.h"

typedef struct _Upix3010Camera Upix3010Camera;

//parameter
typedef enum _UPIX_RUNMODE
{
		UPIX_RUNMODE_STOP=0,
		UPIX_RUNMODE_PAUSE,
		UPIX_RUNMODE_PLAY_SNAP,
		UPIX_RUNMODE_PLAY_LIVE

}UPIX_RUNMODE;

struct _Upix3010Camera
{
	GciCamera camera;

	HWND window_hwnd;
	BYTE camera_number;
	int hidden_panel;
	volatile int upix_lock;
	volatile int upix_flag_lock;
	int get_image_lock;	
	int image_callback_completed;
	int initialised;
	int camera_stopped;
	int prevent_callback;
	int callback_thread_running;
	unsigned long threaded_callback_image_count;
	unsigned long get_image_count;

	FIBITMAP	*fib;
	DS_RESOLUTION resolution;
	UPIX_RUNMODE run_mode;

	char default_settings_file_path[500];
};

GciCamera* gci_upix_uc3010_camera_new(const char *name, const char* description);
int get_exposure(Upix3010Camera *upix_camera, double *exposure);
int set_exposure(Upix3010Camera *upix_camera, double exposure);
int set_gain(Upix3010Camera *upix_camera, int gain);
int get_gain(Upix3010Camera *upix_camera, double *gain);
int upix_hardware_initialise (Upix3010Camera* upix_camera);
int upix_uninitialise (Upix3010Camera* camera);
int upix_set_sensor_mode(Upix3010Camera* upix_camera, DS_FRAME_SPEED speed);
int upix_get_sensor_mode(Upix3010Camera* upix_camera, DS_FRAME_SPEED* speed);
int upix_set_resolution (Upix3010Camera* upix_camera, DS_RESOLUTION resolution);
int camera_play(Upix3010Camera* camera, int live);
int camera_stop(Upix3010Camera* camera);
int snap_image (Upix3010Camera* camera);
int start_live(Upix3010Camera* camera);
int set_power(Upix3010Camera* camera, int power);
//int initialise (Upix3010Camera* camera);
int set_colour_enhancement(Upix3010Camera *upix_camera, int status);
int set_colour_gain(Upix3010Camera *upix_camera, int red_gain, int green_gain, int blue_gain);
int get_colour_gain(Upix3010Camera *upix_camera, int *red_gain, int *green_gain, int *blue_gain);
int set_one_time_auto_white_balance(Upix3010Camera *upix_camera);

int CVICALLBACK UPix3010OnExtraPanelClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK UPix3010OnResolutionChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK UPix3010OnColourChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK UPix3010OnOnOnePushWhiteBalancePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK UPix3010OnColourEnhancementChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK UPix3010OnOnSensorModeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

#endif
