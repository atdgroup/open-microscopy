#include "gci_camera.h"
#include "gci_dummy_camera.h"
#include "gci_dcam_camera.h"
//#include "gci_pixellink_camera.h"
#include "gci_upix_uc3010_camera.h"

#include "gci_utils.h"

#include "icsviewer_window.h"

#include "asynctmr.h"
#include "config.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "rs232.h"
#include "profile.h"

static void onCameraClose(GciCamera* camera, void *data)
{
	gci_camera_set_snap_mode(camera);

	// Lets suspend all aync callbacks.
	SuspendAsyncTimerCallbacks ();
	SuspendTimerCallbacks ();

	QuitUserInterface(0);	
}

static int error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	MessagePopup("Error", error_string);
	
	return UIMODULE_ERROR_NONE;
}

//int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
 //                      LPSTR lpszCmdLine, int nCmdShow)

int main()
{
	char path[GCI_MAX_PATHNAME_LEN];
    GciCamera *camera = NULL;
	int err, stress=0;

	#ifdef VERBOSE_DEBUG 

	HANDLE handle_out, handle_in;
	FILE* hf_out, *hf_in;
	int hCrt;

	AllocConsole();
	
    handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
    hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;

	//PROFILE_SET_OUTPUT_STREAM(hf_out);

	#endif

  	//if (InitCVIRTE (hInstance, 0, 0) == 0)
  	//	return -1;    /* out of memory */
	
	/*
	ShowStandaloneDeviceController(&index, &stress,
		"Dummy", "Orca", "C9100_13", "C8484", "Pixelink-PL-A630", NULL);

	if(index == 0) {

		if( (camera = gci_dummy_camera_new("DummyCamera", "Dummy Camera")) == NULL)
			return -1;

		//gci_dummy_set_dummy_file_path(TOPLEVEL_SOURCE_DIRECTORY "\\Camera\\trunk\\dummy\\dummy_image.bmp");    
	}
	else if(index == 1) {

		if( (camera = gci_orca_camera_new(hInstance, "Orca", "Orca Camera")) == NULL)
			return -1;
	}
	else if(index == 2) {

//		if( (camera = gci_C9100_13_camera_new(hInstance, "C9100_13Camera1", "C9100_13 Camera")) == NULL)
//			return -1;
	}
	else if(index == 3) {

//		if( (camera = gci_C8484_camera_new(hInstance, "C8484", "C8484 Camera")) == NULL)
//			return -1;
	}
	else if(index == 4) {

	//	if( (camera = gci_pl_camera_new("Pixelink-PL-A630", "Pixelink PL-A630")) == NULL)
	//		return -1;
	}
	else {
		MessagePopup("Error", "Device does not exist");
		return 0;
	}
	*/

	if ((err = OpenComConfig (5, "COM5", 19200, 0, 8, 1, 164, -1 )) < 0) {
        return -1;
    }
    
	//SetComTime (5, 3.0);  // timeout=3sec
        
	//FlushInQ(5);
        


	camera = gci_upix_uc3010_camera_new("Upix3010", "Colour Camera");

	ui_module_set_error_handler(UIMODULE_CAST(camera), error_handler, NULL);    
	
	GetPrivateDataFolder("Microscope Data", path);
	gci_camera_set_data_dir(camera, path); 

	gci_camera_set_microns_per_pixel(camera, 1.0); 
	
	if( gci_camera_signal_close_handler_connect (camera, onCameraClose, NULL) == CAMERA_ERROR) {
		return -1;
	}

	gci_camera_power_up(camera);  
	gci_camera_initialise(camera);
//	gci_camera_set_exposure_time(camera, 40.0);

	gci_camera_display_main_ui(camera); 
	//gci_camera_set_horizontal_flip(camera, 1);
	//gci_camera_set_vertical_flip(camera, 1);

	gci_camera_set_snap_mode(camera);
	//gci_camera_set_live_mode(camera);
	//gci_camera_activate_live_display(camera);
	gci_camera_show_window(camera);

	RunUserInterface();
	
	gci_camera_destroy(camera);      
	
	#ifndef DONT_PROFILE
	PROFILE_PRINT();        
	while(1);
	#endif

  	return 0;
}
