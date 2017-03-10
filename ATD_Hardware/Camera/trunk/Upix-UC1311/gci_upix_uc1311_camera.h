#ifndef __GCI_UPIX_1311_CAMERA__
#define __GCI_UPIX_1311_CAMERA__

#include "camera\gci_camera.h"
#include "UC1311API.h"

typedef struct _Upix1311Camera Upix1311Camera;

struct _Upix1311Camera
{
	GciCamera camera;

	HWND window_hwnd;
	BYTE camera_number;
	int upix_lock;
	int image_callback_completed;
	int initialised;
	int camera_stopped;
	int prevent_callback;

	int exposure;
	int gain;
};

GciCamera* gci_upix_uc1311_camera_new(const char *name, const char* description);
int get_exposure(Upix1311Camera *upix_camera, double *exposure);
int set_exposure(Upix1311Camera *upix_camera, int exposure);
int set_gain(Upix1311Camera *upix_camera, int gain);
int get_gain(Upix1311Camera *upix_camera, double *gain);
int upix_initialise (Upix1311Camera* upix_camera);
int upix_uninitialise (Upix1311Camera* camera);
int camera_play(Upix1311Camera* camera, DS_SNAP_MODE SnapMode);
int camera_stop(Upix1311Camera* camera);
int snap_image (Upix1311Camera* camera);
int start_live(Upix1311Camera* camera);
int set_power(Upix1311Camera* camera, int power);
int initialise (Upix1311Camera* camera);

#endif
