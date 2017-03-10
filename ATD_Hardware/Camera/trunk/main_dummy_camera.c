#include "gci_camera.h"
#include "gci_dummy_camera.h"
#include "gci_utils.h"

#include "icsviewer_window.h"

#include "profile.h"
#include "config.h"

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
	
	camera = gci_dummy_camera_new();
	
	if(camera != NULL)
		gci_dummy_set_dummy_file_path(TOPLEVEL_SOURCE_DIRECTORY "\\Camera\\trunk\\dummy\\test.jpg");    

	ui_module_set_error_handler(UIMODULE_CAST(camera), error_handler, NULL);    
	
	gci_camera_set_data_dir(camera, "DummySettings"); 

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
