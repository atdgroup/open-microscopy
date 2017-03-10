#include "gci_camera.h"
#include "gci_dcam_camera.h"
#include "FreeImage.h"
#include "gci_utils.h"

#include "config.h"

#include <utility.h> 

static void onCameraClose(GciCamera* camera, void *data)
{
	QuitUserInterface(0);	
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	GciCamera *camera;
	
  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */

	if( (camera = gci_C9100_13_camera_new(hInstance, "C9100_13", "C9100_13 Camera")) == NULL)
		return -1;

	gci_camera_set_data_dir(camera, "C:\\Documents and Settings\\Pierce\\Desktop\\Working Area\\bin\\Microscope Data\\"); 

	gci_camera_set_microns_per_pixel(camera, 1.0); 
	
	if( gci_camera_signal_close_handler_connect (camera, onCameraClose, NULL) == CAMERA_ERROR) {
		return NULL;
	}

	gci_camera_power_up(camera);
	
	gci_camera_display_main_ui(camera); 
	
	//gci_camera_set_live_mode(camera);
	//gci_camera_activate_live_display(camera);
	gci_camera_show_window(camera);

	RunUserInterface();

	gci_camera_destroy(camera);

  	return 0;
}
