#include "gci_camera.h"
#include "gci_jvc_camera.h"
#include "gci_utils.h"

#include "icsviewer_window.h"

#include "config.h"
#include "profile.h"

static void onCameraClose(GciCamera* camera, void *data)
{
	QuitUserInterface(0);	
}

static int error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	MessagePopup("Error", error_string);
	
	return UIMODULE_ERROR_NONE;
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	char path[GCI_MAX_PATHNAME_LEN];
    GciCamera *camera = NULL;
	
  	if (InitCVIRTE (hInstance, 0, 0) == 0)
  		return -1;    /* out of memory */
	
	if( (camera = gci_jvc_camera_new("Jvc", "Jvc Camera")) == NULL)
		return -1;

	ui_module_set_error_handler(UIMODULE_CAST(camera), error_handler, NULL);    
	
	GetPrivateDataFolder("Microscope Data", path);
	gci_camera_set_data_dir(camera, path); 

	gci_camera_set_microns_per_pixel(camera, 1.0); 
	
	if( gci_camera_signal_close_handler_connect (camera, onCameraClose, NULL) == CAMERA_ERROR) {
		return NULL;
	}

	gci_camera_power_up(camera);  
	gci_camera_initialise(camera);
	
	gci_camera_display_main_ui(camera); 
	
	gci_camera_set_live_mode(camera);
	gci_camera_activate_live_display(camera);
	gci_camera_show_window(camera);

	RunUserInterface();
	
	gci_camera_destroy(camera);      
	
	PROFILE_PRINT();        
	
  	return 0;
}
