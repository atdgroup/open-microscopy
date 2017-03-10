#include "HardWareTypes.h"

#include "camera\gci_camera.h"
#include "gci_ui_module.h"
#include "uir_files\gci_camera_ui.h"
#include "gci_camera_callbacks.h" 
#include "string_utils.h"
#include "gci_utils.h"
#include "status.h"

#include "toolbox.h"

#include "icsviewer_window.h"
#include "icsviewer_signals.h"
#include "FreeImage.h"
#include "FreeImageAlgorithms_Arithmetic.h"
#include "FreeImageAlgorithms_Utilities.h" 
#include "GL_CVIRegistry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <userint.h>
#include <ansi_c.h> 
#include <utility.h> 

#include "profile.h"
#include "asynctmr.h" 

#include "ThreadDebug.h"

#define CAMERA_BASE_MSG (WM_USER+30) 
#define GRABBED_IMAGE_MSG (CAMERA_BASE_MSG+1) 

#ifdef THREAD_DEBUGGING
static int thread_locks[10000];
#endif

int send_error_text (GciCamera* camera, char fmt[], ...)
{
	char message[512];
	static int called_cleanup=0;
	va_list ap;
	va_start(ap, fmt);
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(camera), message);  
	
 	ui_module_send_valist_error(UIMODULE_CAST(camera), "Camera Error", fmt, ap);
	
	va_end(ap);
	
	// the problem here is that gci_camera_perform_error_cleanup can itself cause an error and send us into an endless loop
	// don't let it call more than once	
	if (!called_cleanup) {
		called_cleanup = 1;
		gci_camera_perform_error_cleanup(camera);
	}

	called_cleanup = 0; // allow this cleanup again

	return CAMERA_SUCCESS;
}


/* This is only here until freeimage adds it */
static FIBITMAP*
GCI_FreeImage_ConvertTo_FIT_INT16(FIBITMAP* dib)
{
	long number_of_pixels, i;
	int width, height, bpp;
	FREE_IMAGE_TYPE type;
	FIBITMAP *out_dib;
	
	if(dib == NULL)
		return NULL;
	
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	out_dib = FreeImage_AllocateT(FIT_INT16, width, height, 16, 0, 0, 0);

	bpp = FreeImage_GetBPP(dib);
	type = FreeImage_GetImageType(dib);
	
	if( type == FIT_COMPLEX)
		return NULL;

	if( !FIA_IsGreyScale(dib) )
		return NULL;

	number_of_pixels = width * height;

	if(number_of_pixels == 0)
		return NULL;

	switch(bpp)
	{
		case 8:
		{
		
			unsigned char * dib_ptr = (unsigned char *) FreeImage_GetBits(dib);
			short * out_dib_ptr = (short *) FreeImage_GetBits(out_dib); 
			
			for(i=0; i < number_of_pixels; i++) {
			
				*out_dib_ptr++ = (short) *dib_ptr++;
			}
		}
		
	
		case 16:
		{
			if(type == FIT_UINT16) {
			
				unsigned short * dib_ptr = (unsigned short *) FreeImage_GetBits(dib);
				short * out_dib_ptr = (short *) FreeImage_GetBits(out_dib); 
			
				for(i=0; i < number_of_pixels; i++) {
			
					*out_dib_ptr++ = (short) *dib_ptr++;
				}
			}
			else
				return FreeImage_Clone(dib);
		}
		
		case 32:
		{
		
			if(type == FIT_FLOAT) {
			
				float * dib_ptr = (float *) FreeImage_GetBits(dib);
				short * out_dib_ptr = (short *) FreeImage_GetBits(out_dib); 
			
				for(i=0; i < number_of_pixels; i++) {
			
					*out_dib_ptr++ = (short) *dib_ptr++;
				}
			}
		}

		case 64:
		{
		
			if(type == FIT_DOUBLE) {
			
				double * dib_ptr = (double *) FreeImage_GetBits(dib);
				short * out_dib_ptr = (short *) FreeImage_GetBits(out_dib); 
			
				for(i=0; i < number_of_pixels; i++) {
			
					*out_dib_ptr++ = (short) *dib_ptr++;
				}
			}
		}
	}

	return out_dib;
}

static FIBITMAP*
GCI_FreeImage_ConvertTo_FIT_UINT16(FIBITMAP* dib)
{
	long number_of_pixels, i;
	int width, height, bpp;
	FREE_IMAGE_TYPE type;
	FIBITMAP *out_dib;
	
	if(dib == NULL)
		return NULL;
	
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
	out_dib = FreeImage_AllocateT(FIT_UINT16, width, height, 16, 0, 0, 0);

	bpp = FreeImage_GetBPP(dib);
	type = FreeImage_GetImageType(dib);
	
	if( type == FIT_COMPLEX)
		return NULL;

	if( !FIA_IsGreyScale(dib) )
		return NULL;

	number_of_pixels = width * height;

	if(number_of_pixels == 0)
		return NULL;

	switch(bpp)
	{
		case 8:
		{
		
			unsigned char * dib_ptr = (unsigned char *) FreeImage_GetBits(dib);
			unsigned short * out_dib_ptr = (unsigned short *) FreeImage_GetBits(out_dib); 
			
			for(i=0; i < number_of_pixels; i++) {
			
				*out_dib_ptr++ = (unsigned short) *dib_ptr++;
			}
		}
		
	
		case 16:
		{
			if(type == FIT_INT16) {
			
				short * dib_ptr = (short *) FreeImage_GetBits(dib);
				unsigned short * out_dib_ptr = (unsigned short *) FreeImage_GetBits(out_dib); 
			
				for(i=0; i < number_of_pixels; i++) {
			
					*out_dib_ptr++ = (unsigned short) *dib_ptr++;
				}
			}
			else
				return FreeImage_Clone(dib);
		}
		
		case 32:
		{
		
			if(type == FIT_FLOAT) {
			
				float * dib_ptr = (float *) FreeImage_GetBits(dib);
				unsigned short * out_dib_ptr = (unsigned short *) FreeImage_GetBits(out_dib); 
			
				for(i=0; i < number_of_pixels; i++) {
			
					*out_dib_ptr++ = (unsigned short) *dib_ptr++;
				}
			}
		}

		case 64:
		{
		
			if(type == FIT_DOUBLE) {
			
				double * dib_ptr = (double *) FreeImage_GetBits(dib);
				unsigned short * out_dib_ptr = (unsigned short *) FreeImage_GetBits(out_dib); 
			
				for(i=0; i < number_of_pixels; i++) {
			
					*out_dib_ptr++ = (unsigned short) *dib_ptr++;
				}
			}
		}
	}

	return out_dib;
}


static int error_handler (const char *title, const char *error_string, void *callback_data)
{
	GciCamera *camera = (GciCamera *) callback_data;     
	
	logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, error_string);   

	gci_camera_set_snap_mode(camera);

	return UIMODULE_ERROR_NONE;
}

static void gci_camera_read_or_write_main_panel_registry_settings(GciCamera *camera, int write)
{
	ui_module_read_or_write_registry_settings(UIMODULE_CAST(camera), write);    
}


static void gci_camera_read_or_write_extra_panel_registry_settings(GciCamera *camera, int write)
{
	ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(camera), camera->_extra_ui_panel, write) ;    
}


static void gci_camera_read_or_write_camera_imagewindow_registry_settings(GciCamera* camera, int panel_id, int write)
{
	char buffer[500], name[UIMODULE_NAME_LEN];
	int visible, remote_session = 0, maximised=0;

	if(panel_id == -1)
		return;

	remote_session = GetSystemMetrics(SM_REMOTESESSION);

	if(remote_session)
		return;

	// load or save panel positions
	
	// Check if the window is maximised.
	// If it is dont try to set size and position
	GetPanelAttribute(panel_id, ATTR_WINDOW_ZOOM, &maximised);

	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {
		GetPanelAttribute (panel_id, ATTR_VISIBLE, &visible);
		if(!visible)
			return;

		if(maximised == VAL_MINIMIZE)
			SetPanelAttribute (panel_id, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	}
	
	// Check if the window is maximised.
	// If it is dont try to set size and position
	GetPanelAttribute(panel_id, ATTR_WINDOW_ZOOM, &maximised);

	if(maximised == VAL_MAXIMIZE)
		return;

	gci_camera_get_name(camera, name);
	
	sprintf(buffer, "software\\GCI\\Microscope\\Cameras\\%s\\ImageWindow\\", name);

	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "top", panel_id, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "left", panel_id, ATTR_LEFT);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "width", panel_id, ATTR_WIDTH); 
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "height", panel_id, ATTR_HEIGHT); 
}

//////////////////////////////////////////////////////////////////////////////////

static void CameraImageWindowCloseEventHandler( GCIWindow *window, void *callback_data )
{
	int id;

	GciCamera *camera = (GciCamera *) callback_data;
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "CameraImageWindowClose", GCI_VOID_POINTER, camera, GCI_VOID_POINTER, camera->_camera_window); 
	
	if( (id = GCI_ImagingWindow_GetPanelID(window)) != -1) {
	
		gci_camera_read_or_write_camera_imagewindow_registry_settings(camera, id, 1);
	}
	
	GCI_ImagingWindow_Hide(camera->_camera_window);
	
	if(camera != NULL)
		gci_camera_set_snap_mode(camera);
}


static void CameraImageWindowResizedorMovedEventHandler( GCIWindow *window, void *callback_data )
{
	int id;
	GciCamera *camera = (GciCamera *) callback_data;

	if( (id = GCI_ImagingWindow_GetPanelID(window)) != -1) {
	
		gci_camera_read_or_write_camera_imagewindow_registry_settings(camera, id, 1);
	}
}


static int SetupImageWindow(GciCamera* camera)
{
	if(camera->_camera_window == NULL) {
	
		// Create a Imaging Window that cannot be closed
		if( (camera->_camera_window = GCI_ImagingWindow_CreateAdvanced2((UIMODULE_GET_NAME(camera)), (UIMODULE_GET_DESCRIPTION(camera)), 0, 0, 0, 0, 1, 0, 0)) == NULL ) {
			camera->_camera_window = NULL;
			return CAMERA_ERROR;
		}
			
		GCI_ImagingWindow_Initialise(camera->_camera_window);
		
		GCI_ImagingWindow_SignalOnlyOnCloseOrExit(camera->_camera_window);
		
		GCI_ImagingWindow_SetLiveStatus(camera->_camera_window, 1);  
		
		GCI_ImagingWindow_SetCloseEventHandler( camera->_camera_window, CameraImageWindowCloseEventHandler, camera);
		
		GCI_ImagingWindow_SetResizedorMovedHandler( camera->_camera_window, CameraImageWindowResizedorMovedEventHandler, camera); 
	}
	
	GCI_ImagingWindow_SetWindowTitle( camera->_camera_window, UIMODULE_GET_DESCRIPTION(camera));
	
	GCI_ImagingWindow_SetResizeFitStyle(camera->_camera_window);
	
	return CAMERA_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////

int CVICALLBACK camera_on_live_timer_thread(void *callback)
{
	GciCamera *camera = (GciCamera*) callback;
	static int count = 0;
	double fps;
	static double time = 0.0;
	const int period = 5;
	
	while(1) {	

		if(camera == NULL || camera->_exit_thread) {
			camera->_live_thread_suspend = 0;
			camera->_live_thread_exited = 1;
			return -1;
		}

		//printf("In Live thread\n");

		if(camera->_live_thread_suspend == 1) {
			
			camera->_live_thread_suspended = 1;

			//printf("Thread suspended\n");

			// If suspend thread succeeds then this thread will hang here.
			SuspendThread (GetCurrentThread ());

			camera->_live_thread_suspended = 0;
			camera->_live_thread_suspend = 0;
		
			//printf("Thread resumed\n");

			continue;
		}
	

		if(!gci_camera_is_live_mode(camera) || !camera->_powered_up) {
			continue;
		}
	
		if(camera->_camera_window == NULL) {
			continue;
		}

		if(camera->_live_thread_suspend == 1)
			continue;

//		gci_camera_get_lock(camera);

		camera->grabbed_dib = gci_camera_get_image(camera, NULL); 
			
		if(camera->grabbed_dib == NULL) {
//			gci_camera_release_lock(camera);
			continue;
		}

		SendMessageTimeout(camera->window_hwnd, GRABBED_IMAGE_MSG, 0, 0, SMTO_ABORTIFHUNG, 1000, NULL);  
  
		if(camera->grabbed_dib != NULL)
			FreeImage_Unload(camera->grabbed_dib);
		
		camera->grabbed_dib = NULL;
	
//		gci_camera_release_lock(camera);

		count++;
		
		if((count % period) == 0) {
			if(!camera->_fps_supported) {
				fps = (double)period/((double)clock()/(double)CLOCKS_PER_SEC-time);  
				time=(double)clock()/(double)CLOCKS_PER_SEC;  

				camera->_fps = fps;
			}
			ProcessDrawEvents();
		}

	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////

static int VOID_CAMERA_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (GciCamera *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (GciCamera *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int VOID_CAMERA_PTR_WINDOW_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (GciCamera *, GCIWindow*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (GciCamera *) args[0].void_ptr_data, (GCIWindow *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int VOID_CAMERA_PTR_IMAGE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (GciCamera *, FIBITMAP**, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (GciCamera *) args[0].void_ptr_data, (FIBITMAP **) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

//////////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK CameraWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	GciCamera *camera = (GciCamera *) data;
	
	switch(message) {
			
    	case GRABBED_IMAGE_MSG:
    	{
			double rate = 0.0;
			static int count = 0;
	
			if(camera->_live_thread_suspend == 1)
				return 0;

			if(camera->grabbed_dib == NULL)
				return 0;
			
			if(camera->_prevent_getting_images == 1)
				return 0;

			gci_camera_display_image(camera, camera->grabbed_dib, NULL);  
	
			if(count == 0) {
				rate = gci_camera_get_frame_rate(camera); 
				GCI_ImagingWindow_SetFramesPerSecondIndicator(camera->_camera_window, rate);
				SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_LIVE_FPS, rate);   
			}
				
			count++;
			
			if(count > 5)
				count = 0;
			
			return 0;
    	}


      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) camera->uimodule_func_ptr,
							hwnd, message, wParam, lParam);
}


int gci_camera_feature_enabled (GciCamera* camera, CameraFeature feature)
{
	// No features set assume the camera has all features.
	if(camera->features_set == 0)
		return 1;
	
	if(camera->features[feature] == 1)
		return 1;
	
	return 0;
}

int gci_camera_set_feature (GciCamera* camera, CameraFeature feature)
{
	camera->features_set = 1;
	
	camera->features[feature] = 1;
	
	return CAMERA_SUCCESS;
}

static int gci_camera_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	GciCamera* camera = (GciCamera*) device;

	return gci_camera_save_settings(camera, filepath, mode);
}

static int gci_camera_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	GciCamera* camera = (GciCamera*) device;

	return gci_camera_load_settings(camera, filepath);
}

void gci_camera_get_lock(GciCamera* camera)
{
	//printf("gci_camera_get_lock: ThreadID: %d\n", CmtGetCurrentThreadID());

	if(GciCmtGetLock(camera->_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}
}

void gci_camera_release_lock(GciCamera* camera)
{
	//printf("gci_camera_release_lock: ThreadID: %d\n", CmtGetCurrentThreadID());

	if(GciCmtReleaseLock(camera->_lock) < 0) {
		send_error_text(camera, "GciCmtGetLock Failed");
	}
}

static int gci_camera_setup_device_for_calibration (OpticalCalibrationDevice* device, PROFILE_HANDLER handler, void *data)
{
	GciCamera* camera = (GciCamera*) device;

	gci_camera_display_main_ui(camera);
	gci_camera_set_binning_mode(camera, NO_BINNING);
	gci_camera_set_live_mode(camera); 
	gci_camera_activate_live_display(camera);

	GCI_ImagingWindow_SetResizeFitStyle(camera->_camera_window);
	GCI_ImagingWindow_EnableLineTool(camera->_camera_window);
	GCI_ImagingWindow_DisableProfile(camera->_camera_window);

	camera->optical_calibration_device_profile_handler_id = GCI_ImagingWindow_SetProfileHandler(camera->_camera_window, handler, data);

	GCI_ImagingWindow_Show(camera->_camera_window);

	return OPTICAL_CALIBRATION_DEVICE_SUCCESS;
}

void gci_camera_bring_to_front(GciCamera* camera)
{
	SetWindowPos(GCI_ImagingWindow_GetWindowHandle(camera->_camera_window), HWND_TOP, 0, 0,
				0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

static int gci_camera_deinitilise_device_for_calibration (OpticalCalibrationDevice* device)
{
	GciCamera* camera = (GciCamera*) device;

	if(camera->_camera_window != NULL && camera->optical_calibration_device_profile_handler_id > 0) {
		GCI_ImagingWindow_DisconnectProfileHandler(camera->_camera_window, camera->optical_calibration_device_profile_handler_id);                  
		camera->optical_calibration_device_profile_handler_id = -1;     
		GCI_ImagingWindow_DisableLineTool(camera->_camera_window);
		GCI_ImagingWindow_EnableProfile(camera->_camera_window);
	}

	return CAMERA_SUCCESS;
}

static IcsViewerWindow* gci_camera_get_calibration_window (OpticalCalibrationDevice* device)
{
	GciCamera* camera = (GciCamera*) device;

	return camera->_camera_window;
}

int gci_camera_constructor (GciCamera* camera, const char *name, const char* description)
{
	#ifdef THREAD_DEBUGGING
	memset(thread_locks, 0, 10000);
	#endif
	
	optical_calibration_device_constructor(OPTICAL_CALIBRATION_DEVICE_CAST(camera), name);
	ui_module_set_description(UIMODULE_CAST(camera), description);

	camera->grabbed_dib = NULL;
	camera->_default_ui = 1;
	camera->features_set = 0;
	camera->number_of_datamodes = 0;     
	camera->number_of_binning_modes = 0;   
	camera->optical_calibration_device_profile_handler_id = -1;   

    camera->last_subwindow_rect.left = 0;
    camera->last_subwindow_rect.top = 0;
    camera->last_subwindow_rect.width = 0;
    camera->last_subwindow_rect.height = 0;
    
	memset(camera->features, 0, sizeof(unsigned int) * CAMERA_FEATURE_COUNT);
	memset(camera->supported_datamodes, 0, sizeof(DataMode) * 10);
	memset(camera->supported_binning_modes, 0, sizeof(BinningMode) * 10); 
		
	memset(camera->_min_gain_text, 0, sizeof(char) * 20);
	memset(camera->_max_gain_text, 0, sizeof(char) * 20);          
	
	camera->_horz_flip = 0;
	camera->_vert_flip = 0;
	camera->_rotation_angle = 0.0;
	camera->_average_frames = 0;
	camera->_colour_type = MONO_TYPE;
	camera->_powered_up = 0;
	camera->_main_ui_panel = -1;
	camera->_extra_ui_panel = -1;
	camera->_aquire_mode = 0;
	camera->_bits = 8;
	camera->_trigger_mode = CAMERA_NO_TRIG;
	camera->_bad_frame_count = 0;
	camera->_initialised = 0;
	camera->_has_two_channels = 0;
	camera->_gain_data_type_precision = 0;
	camera->_camera_window = NULL;
	camera->_live_thread_suspend = 1;
	camera->_live_thread_suspended = 1;
	camera->_exit_thread = 0;
	camera->_live_thread_exited = 0;
	camera->_prevent_getting_images = 0;
	camera->_no_display = 0;

	camera->_metadata = NULL;

	memset(&(camera->_cops), 0, sizeof(struct camera_operations));
	
	memset(camera->_data_dir, 0, 500);
		
	GciCmtNewLock ("GciCamera", 0, &(camera->_lock));
	GciCmtNewLock ("GciCamera-GrabbedImageLock", 0, &(camera->_grabbed_image_lock));

	gci_camera_deactivate_grab_timer(camera);

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "CameraShow", VOID_CAMERA_PTR_MARSHALLER);    
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "CameraExposureChanged", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "CameraGainChanged", VOID_CAMERA_PTR_MARSHALLER);  
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "BitModeChanged", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "BinningModeChanged", VOID_CAMERA_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "EnterLiveMode", VOID_CAMERA_PTR_MARSHALLER);  
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "ExitLiveMode", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "EnterSnapMode", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "ExitSnapMode", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "EnterSnapSequenceMode", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "ExitSnapSequenceMode", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "EnterGetImage", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "ExitGetImage", VOID_CAMERA_PTR_IMAGE_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "PreCapture", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "PostCapture", VOID_CAMERA_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "TriggerNow", VOID_CAMERA_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "PreDisplayImage", VOID_CAMERA_PTR_WINDOW_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "PostDisplayImage", VOID_CAMERA_PTR_WINDOW_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "CameraImageWindowClose", VOID_CAMERA_PTR_WINDOW_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(camera), "Close", VOID_CAMERA_PTR_MARSHALLER);

	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(camera), hardware_save_state_to_file) = gci_camera_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(camera), hardware_load_state_from_file) = gci_camera_hardware_load_state_from_file; 

	OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(OPTICAL_CALIBRATION_DEVICE_CAST(camera), setup_device_for_calibration) = gci_camera_setup_device_for_calibration;
	OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(OPTICAL_CALIBRATION_DEVICE_CAST(camera), get_calibration_window) = gci_camera_get_calibration_window;
	OPTICAL_CALIBRATION_DEVICE_VTABLE_PTR(OPTICAL_CALIBRATION_DEVICE_CAST(camera), deinitilise_device_for_calibration) = gci_camera_deinitilise_device_for_calibration;

	gci_camera_set_allowed_binningmode (camera, NO_BINNING); 
	
  	return CAMERA_SUCCESS;
}


int  gci_camera_initialise(GciCamera *camera)
{ 
	int window_handle;

	if(camera->_default_ui) {
		
		camera->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(camera), "gci_camera_ui.uir", CAMERA_PNL, 1);  
	
		GetPanelAttribute (camera->_main_ui_panel, ATTR_SYSTEM_WINDOW_HANDLE, &(window_handle)); 
		camera->window_hwnd = (HWND) window_handle; 

		camera->uimodule_func_ptr = ui_module_set_window_proc(UIMODULE_CAST(camera), camera->_main_ui_panel, (LONG_PTR) CameraWndProc);	
		
		ui_module_set_main_panel_title (UIMODULE_CAST(camera));
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_CAMERA_LABEL, UIMODULE_GET_NAME(camera)); 
	
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_SNAP, Camera_onSnap, camera) < 0)
			return CAMERA_ERROR;
  	
  		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_LIVE, Camera_onLive, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, Camera_onExposure, camera) < 0)
			return CAMERA_ERROR;
	
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_AUTO_EXPOSURE, Camera_onAutoExposure, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_AVERAGE, Camera_onAverage, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_AVERAGENUMBER, Camera_onAverageFramesChanged, camera) < 0)
			return CAMERA_ERROR;	
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_MORE, Camera_onExtras, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_QUIT, Camera_onQuit, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_GAIN, Camera_onGain, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_INFO, Camera_onInfo, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_SAVE, Camera_onSaveSettings, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_LOAD, Camera_onLoadSettings, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_DEFAULTS, Camera_onSetDefaults, camera) < 0)
			return CAMERA_ERROR;
  
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_REINIT, Camera_onReinit, camera) < 0)
			return CAMERA_ERROR;
		
		if ( InstallCtrlCallback (camera->_main_ui_panel, CAMERA_PNL_SAVE_IMAGES, Camera_onSaveImages, camera) < 0)
			return CAMERA_ERROR;

		// Set min max gain on ui panels
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_MIN_VALUE, camera->_min_gain);
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_MAX_VALUE, camera->_max_gain);
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_DFLT_VALUE, camera->_min_gain); 
	
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN_TEXT_MIN, camera->_min_gain_text);
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN_TEXT_MAX, camera->_max_gain_text);
	
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_PRECISION, camera->_gain_data_type_precision);      
	}

	if(camera->_camera_window == NULL) {
	
		if( SetupImageWindow(camera) == CAMERA_ERROR) {
	
    		send_error_text(camera,"Can not create image window for device %s\n", UIMODULE_GET_DESCRIPTION(camera));
    		return CAMERA_ERROR;
  		}
	}
	
	if(camera->_cops.initialise != NULL) {
    
    	if( (*camera->_cops.initialise)(camera) == CAMERA_ERROR ) {
	
			logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "Init operation failed for camera");   
			return CAMERA_ERROR;
  		}
  	}

	camera->_initialised = 1; 
	
	// Set any default settings for the camera.
	gci_camera_set_default_settings(camera);

	if(camera->_cops.get_fps == NULL) {
		// FPS not support so we calculate it ourselves.
		camera->_fps_supported = 0;
	}
	else
		camera->_fps_supported = 1;	
	
	camera->_live_thread_suspend = 1;
	camera->_live_thread_suspended = 1;
	camera->_exit_thread = 0;
	camera->_live_thread_exited = 0;
	camera->_prevent_getting_images = 0;

	CmtScheduleThreadPoolFunction (gci_thread_pool(), camera_on_live_timer_thread, camera, &(camera->_live_thread));
		
	return CAMERA_SUCCESS;        
}

int gci_camera_is_initialised(GciCamera *camera)
{
	return camera->_initialised;
}

int  gci_camera_dual_channel_enabled(GciCamera* camera)
{
	if(camera->_cops.is_dual_channel_mode == NULL)
    	return 0;

	return (*camera->_cops.is_dual_channel_mode)(camera);
}

GciCamera* gci_camera_new()
{
	GciCamera* camera = (GciCamera*)malloc(sizeof(GciCamera));
	
	gci_camera_constructor (camera, "Unknown Camera", "Unknown Camera");
	
	return camera;
}

//////////////////////////////////////////////////////////////////////////////////

int gci_camera_signal_close_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "Close", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for Close signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int  gci_camera_signal_on_show_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "CameraShow", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for CameraShow signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;	
}

int  gci_camera_signal_on_show_handler_is_connected (GciCamera* camera)
{
	return GCI_Signal_IsConnected(UIMODULE_SIGNAL_TABLE(camera), "CameraShow");
}

int  gci_camera_signal_on_exposure_changed_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "CameraExposureChanged", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for CameraExposureChanged signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;		
}


int  gci_camera_signal_on_gain_changed_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "CameraGainChanged", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for CameraGainChanged signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;		
}

int gci_camera_signal_datamode_change_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "BitModeChanged", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for BitModeChanged signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int gci_camera_signal_binning_change_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "BinningModeChanged", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for BinningModeChanged signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int gci_camera_signal_enter_live_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "EnterLiveMode", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for EnterLiveMode signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int gci_camera_signal_exit_live_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "ExitLiveMode", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for ExitLiveMode signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int gci_camera_signal_enter_snap_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "EnterSnapMode", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for EnterSnapMode signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int gci_camera_signal_exit_snap_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "ExitSnapMode", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for ExitSnapMode signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int gci_camera_signal_enter_snap_sequence_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "EnterSnapSequenceMode", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for EnterSnapSequenceMode signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int gci_camera_signal_exit_snap_sequence_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "ExitSnapSequenceMode", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for ExitSnapSequenceMode signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int  gci_camera_signal_enter_get_image_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	int id = 0;

	id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "EnterGetImage", handler, callback_data);

	if(id == SIGNAL_ERROR)
		send_error_text(camera, "Can not connect signal handler for EnterGetImage signal");

	return id;
}


int  gci_camera_signal_exit_get_image_handler_connect  (GciCamera* camera, CAMERA_IMAGE_EVENT_HANDLER handler, void *callback_data)
{
	int id = 0;

	id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "ExitGetImage", handler, callback_data);

	if(id == SIGNAL_ERROR)
		send_error_text(camera, "Can not connect signal handler for ExitGetImage signal");

	return id;
}


int  gci_camera_signal_enter_get_image_handler_disconnect (GciCamera* camera, int id)
{
	if( GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(camera), "EnterGetImage", id) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not disconnect signal handler for EnterGetImage signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int  gci_camera_signal_exit_get_image_handler_disconnect  (GciCamera* camera, int id)
{
	if( GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(camera), "ExitGetImage", id) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not disconnect signal handler for ExitGetImage signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int  gci_camera_signal_post_capture_handler_disconnect  (GciCamera* camera, int id)
{
	if( GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(camera), "PostCapture", id) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not disconnect signal handler for PostCapture signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int  gci_camera_signal_pre_capture_handler_connect (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "PreCapture", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for PreCapture signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int  gci_camera_signal_post_capture_handler_connect  (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	int id = 0;

	id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "PostCapture", handler, callback_data);

	if(id == SIGNAL_ERROR)
		send_error_text(camera, "Can not connect signal handler for PostCapture signal");

	return id;
}

int  gci_camera_signal_trigger_now_handler_connect  (GciCamera* camera, CAMERA_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "TriggerNow", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for TriggerNow signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int  gci_camera_signal_image_pre_display_handler_connect (GciCamera* camera, CAMERA_WINDOW_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "PreDisplayImage", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for PreDisplayImage signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int  gci_camera_signal_image_post_display_handler_connect (GciCamera* camera, CAMERA_WINDOW_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "PostDisplayImage", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for PostDisplayImage signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int  gci_camera_signal_image_window_close_handler_connect (GciCamera* camera, CAMERA_WINDOW_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(camera), "CameraImageWindowClose", handler, callback_data) == SIGNAL_ERROR) {
	
		send_error_text(camera, "Can not connect signal handler for CameraImageWindowClose signal");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

void gci_camera_set_extra_panel_uir(GciCamera* camera, const char *uir_filename, int panel)
{
	camera->_extra_ui_panel = ui_module_add_panel(UIMODULE_CAST(camera), uir_filename, panel, 0); 
	
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_MORE, ATTR_DIMMED, 0);
}

//////////////////////////////////////////////////////////////////////////////////

void gci_camera_set_rotation(GciCamera* camera, double angle)
{
	camera->_rotation_angle = angle;
}

void gci_camera_set_horizontal_flip(GciCamera* camera, int flip)
{
	camera->_horz_flip = flip;
}

void gci_camera_set_vertical_flip(GciCamera* camera, int flip)
{
	camera->_vert_flip = flip;
}

static void gci_camera_kill_thread(GciCamera* camera)
{
	double start_time = Timer();
	camera->_exit_thread = 1;
	
	while(camera->_live_thread_exited == 0 && (Timer() - start_time) < 5.0) {
		//printf("Wait for thread to exit\n");
		Delay(0.1);
		ProcessSystemEvents();
	}

	if((Timer() - start_time) > 5.0) {
		logger_log(UIMODULE_LOGGER(camera), LOGGER_ERROR, "%s thread failed to exit. Are we deadlocked ?", UIMODULE_GET_DESCRIPTION(camera));   
	}

	//CmtWaitForThreadPoolFunctionCompletion(gci_thread_pool(), camera->_live_thread, 0);
}

void gci_camera_prevent_getting_images(GciCamera* camera)
{
	camera->_prevent_getting_images = 1;
}

int gci_camera_destroy(GciCamera* camera)
{
	int id;
  	
	// Resume thread so it can be killed.
	gci_camera_prevent_getting_images(camera);
	gci_camera_activate_grab_timer(camera);
	gci_camera_kill_thread(camera);

	//printf("Before Lock\n");

	gci_camera_get_lock(camera);

	//printf("After Lock\n");

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "Close", GCI_VOID_POINTER, camera);
  	
	gci_camera_deactivate_grab_timer(camera);
	
	if(camera->_cops.destroy == NULL) {
    
    	send_error_text(camera, "Destroy operation not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera));

    	return CAMERA_ERROR;
  	}

	if( (*camera->_cops.destroy)(camera) == CAMERA_ERROR ) {
	
		send_error_text(camera, "Destroy operation failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera));

		gci_camera_release_lock(camera);

		return CAMERA_ERROR;
	}

	/* Destroy imaging window */
	if(camera->_camera_window != NULL) {
		
		if( (id = GCI_ImagingWindow_GetPanelID(camera->_camera_window)) != -1) {
	
			gci_camera_read_or_write_camera_imagewindow_registry_settings(camera, id, 1);
		}
		
		GCI_ImagingWindow_DestroyWindow(camera->_camera_window);
		camera->_camera_window = NULL;
	}

//	gci_camera_read_or_write_main_panel_registry_settings(camera, 1);
//	gci_camera_read_or_write_extra_panel_registry_settings(camera, 1);
	
//	if(camera->_extra_ui_panel != -1)
//		ui_module_destroy_panel(UIMODULE_CAST(camera), camera->_extra_ui_panel);

	optical_calibration_device_destroy (OPTICAL_CALIBRATION_DEVICE_CAST(camera));

	camera->_main_ui_panel = -1;
	camera->_extra_ui_panel = -1;
  	camera->_camera_window = NULL;
	
	gci_camera_release_lock(camera);

	CmtDiscardLock(camera->_lock);
	CmtDiscardLock(camera->_grabbed_image_lock);

	free(camera);
	
  	return CAMERA_SUCCESS;
}


int gci_camera_set_description(GciCamera* camera, const char *description)
{
	ui_module_set_description(UIMODULE_CAST(camera), description);
	
  	return CAMERA_SUCCESS;
}


int gci_camera_get_description(GciCamera* camera, char *description)
{
    strcpy(description, UIMODULE_GET_DESCRIPTION(camera));
    
    return CAMERA_SUCCESS;
}


int gci_camera_set_data_dir(GciCamera* camera, const char *dir)
{
	memset(camera->_data_dir, 0, GCI_MAX_PATHNAME_LEN);

    strcpy(camera->_data_dir, dir);
  	
	ui_module_set_data_dir(UIMODULE_CAST(camera), dir);

  	return CAMERA_SUCCESS;
}


int gci_camera_get_data_dir(GciCamera* camera, char* dir)
{
  	strcpy(dir, camera->_data_dir);
    
    return CAMERA_SUCCESS;
}


int gci_camera_set_name(GciCamera* camera, char *name)
{
  	if(ui_module_set_name(UIMODULE_CAST(camera), name) == UI_MODULE_ERROR)
    	return CAMERA_ERROR;

  	return CAMERA_SUCCESS;
}


int gci_camera_get_name(GciCamera* camera, char *name)
{
	if(ui_module_get_name(UIMODULE_CAST(camera), name) == UI_MODULE_SUCCESS)
    	return CAMERA_SUCCESS;
  
  	return CAMERA_ERROR;
}

//////////////////////////////////////////////////////////////////////////////////

int gci_camera_power_up(GciCamera* camera)
{
	if(camera->_powered_up > 0)
		return CAMERA_SUCCESS;

	if(camera->_cops.power_up == NULL) {
    	send_error_text(camera, "Power up operation not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

	if( (*camera->_cops.power_up)(camera) == CAMERA_ERROR ) {
    	return CAMERA_ERROR;
	}
	
	camera->_powered_up = 1;

  	return CAMERA_SUCCESS; 
}


int gci_camera_is_powered_up(GciCamera* camera)
{
	return camera->_powered_up;
}

int gci_camera_perform_error_cleanup(GciCamera* camera)
{
	if(camera->_cops.on_error == NULL) {
    	return CAMERA_ERROR;
  	}

	if( (*camera->_cops.on_error)(camera) == CAMERA_ERROR ) {
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}

int gci_camera_power_down(GciCamera* camera)
{
	if(camera->_powered_up == 0)
		return CAMERA_SUCCESS;

	gci_camera_kill_thread(camera);

  	if(camera->_cops.power_down == NULL) {
    
    	send_error_text(camera, "Power down operation not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );

    	return -1;
  	}

	if( (*camera->_cops.power_down)(camera) == CAMERA_ERROR ) {
	
		send_error_text(camera, "Power down operation failed for device %s\n",UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}

	camera->_powered_up = 0;

  	return CAMERA_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////

int  gci_camera_save_state(GciCamera* camera, CameraState *state)
{
	if(camera->_cops.save_state == NULL) {
	
    	send_error_text(camera,"Save state not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}
  	
	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s save state", UIMODULE_GET_DESCRIPTION(camera));   

  	if( (*camera->_cops.save_state)(camera, state)== CAMERA_ERROR) {
  	
  		send_error_text(camera, "Save state operation failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
  	}
	
  	return CAMERA_SUCCESS;
}


int  gci_camera_restore_state(GciCamera* camera, CameraState *state)
{
	if(camera->_cops.restore_state == NULL) {
	
    	send_error_text(camera,"Restore state not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}
  	
	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s restore state", UIMODULE_GET_DESCRIPTION(camera)); 

  	if( (*camera->_cops.restore_state)(camera, state)== CAMERA_ERROR) {
  	
  		send_error_text(camera, "Restore state operation failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
  	}
	
  	return CAMERA_SUCCESS;
}


double gci_camera_get_frame_rate(GciCamera* camera)
{
	if(camera->_cops.get_fps == NULL) {
		return camera->_fps;
  	}

	return (*camera->_cops.get_fps)(camera);
}


//////////////////////////////////////////////////////////////////////////////////

int gci_camera_set_exposure_time(GciCamera* camera, double exposure)
{
	//int live;
	double exposure_tmp, actual_exposure;

  	if(camera->_cops.set_exposure_time == NULL) {
    	send_error_text(camera, "Set exposure not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}
		
	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s exposure to %f", UIMODULE_GET_DESCRIPTION(camera), exposure); 

	//live = gci_camera_is_live_mode(camera);      

	if(exposure < 5.0) {
		exposure = 5.0;
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, exposure);
	}

	if( (*camera->_cops.set_exposure_time)(camera, exposure, &actual_exposure) == CAMERA_ERROR ) {
	
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		send_error_text(camera, "Failed to set exposure time");
		return CAMERA_ERROR;
	}

	camera->_exposure_time = actual_exposure;
	
	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(camera), "CameraExposureChanged", GCI_VOID_POINTER, camera);  
	
	if(camera->_main_ui_panel != -1) {
	
		GetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, &exposure_tmp);
		
		// Dont set the ui if the change came from that control.
		if(exposure_tmp != actual_exposure)
			SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, actual_exposure);
	}
	
	return CAMERA_SUCCESS;
}

double gci_camera_get_exposure_time(GciCamera* camera)
{
	if (camera == NULL)
		return 0.0;
	
	return camera->_exposure_time;
}

int gci_camera_set_blacklevel(GciCamera* camera, CameraChannel channel, double blacklevel) 
{
	if(camera->_cops.set_blacklevel == NULL)
    	return CAMERA_SUCCESS;
	
	if (channel==CAMERA_ALL_CHANNELS) 
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s blacklevel to %f", UIMODULE_GET_DESCRIPTION(camera), blacklevel);
	else
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s blacklevel channel %d to %f", UIMODULE_GET_DESCRIPTION(camera), channel, blacklevel);
	
	if ( (*camera->_cops.set_blacklevel)(camera, channel, blacklevel) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set blacklevel for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}

int gci_camera_set_sensitivity(GciCamera* camera, CameraChannel channel, double sensitivity) 
{
	if(camera->_cops.set_sensitivity == NULL)
    	return CAMERA_SUCCESS;
	
	if (channel==CAMERA_ALL_CHANNELS) 
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s sensitivity to %f", UIMODULE_GET_DESCRIPTION(camera), sensitivity);
	else
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s sensitivity channel %d to %f", UIMODULE_GET_DESCRIPTION(camera), channel, sensitivity);
	
	if ( (*camera->_cops.set_sensitivity)(camera, channel, sensitivity) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set sensitivity for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}

int gci_camera_set_ccd_mode (GciCamera* camera, CCDMode mode)
{
	if(camera->_cops.set_ccd_mode == NULL)
    	return CAMERA_SUCCESS;

	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s ccd mode to %d", UIMODULE_GET_DESCRIPTION(camera), mode);

	if ( (*camera->_cops.set_ccd_mode)(camera, mode) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set ccd mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera));
		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}

int gci_camera_get_blacklevel(GciCamera* camera, CameraChannel channel, double *bl)
{
	if(camera->_cops.get_blacklevel == NULL) 
    	return CAMERA_SUCCESS;

  	if ( (*camera->_cops.get_blacklevel)(camera, channel, bl) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to get blacklevel for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}

int gci_camera_get_sensitivity(GciCamera* camera, CameraChannel channel, double *sensitivity)
{
	if(camera->_cops.get_sensitivity == NULL) 
    	return CAMERA_SUCCESS;
 
  	if ( (*camera->_cops.get_sensitivity)(camera, channel, sensitivity) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to get sensitivity for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}

void gci_camera_set_exposure_range(GciCamera* camera, double min, double max, const char *label, int type)
{
	switch(type)
	{
		case VAL_CHAR:                       
		case VAL_INTEGER:                     
		case VAL_SHORT_INTEGER:               
		case VAL_UNSIGNED_SHORT_INTEGER:
		case VAL_UNSIGNED_INTEGER:
		case VAL_UNSIGNED_CHAR:
		{
			// Remove fractional point
			SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, ATTR_PRECISION, 0);
		}
		                    
		case VAL_FLOAT:                       
		case VAL_DOUBLE: 
		{	
			SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, ATTR_PRECISION, 2); 
			
			break;
		}
		
		case VAL_STRING:
			return;
	}

	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, ATTR_MIN_VALUE, min);
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, ATTR_MAX_VALUE, max);
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, ATTR_DFLT_VALUE, min); 
	SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, min);              
	
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_EXPOSURE, ATTR_LABEL_TEXT, label);
}


void gci_camera_set_gain_range(GciCamera* camera, double min, double max, int type)
{
	switch(type)
	{
		case VAL_CHAR:                       
		case VAL_INTEGER:                     
		case VAL_SHORT_INTEGER:               
		case VAL_UNSIGNED_SHORT_INTEGER:
		case VAL_UNSIGNED_INTEGER:
		case VAL_UNSIGNED_CHAR:
		{
			// Remove fractional point
			camera->_gain_data_type_precision = 0;
			break;
		}
		                    
		case VAL_FLOAT:                       
		case VAL_DOUBLE: 
		{	
			camera->_gain_data_type_precision = 2;    
			break;
		}
		
		case VAL_STRING:
			return;
	}
		
	camera->_min_gain = min;
	camera->_max_gain = max;

	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_MIN_VALUE, camera->_min_gain);
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_MAX_VALUE, camera->_max_gain);
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_DFLT_VALUE, camera->_min_gain); 
	
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_PRECISION, camera->_gain_data_type_precision);      
}

void gci_camera_set_gain_range_text(GciCamera* camera, char *min, char *max)
{
	strcpy(camera->_min_gain_text, min);
	strcpy(camera->_max_gain_text, max);   

	SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN_TEXT_MIN, camera->_min_gain_text);
	SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN_TEXT_MAX, camera->_max_gain_text);
}

void gci_camera_get_gain_range(GciCamera* camera, double *min, double *max)
{
	if(camera->_default_ui) {
		GetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_MIN_VALUE, min);
		GetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_MAX_VALUE, max);
	}
}

void gci_camera_set_blacklevel_range(GciCamera* camera, double min, double max)
{
	camera->_min_bl = min;
	camera->_max_bl = max;
}

void gci_camera_set_sensitivity_range(GciCamera* camera, double min, double max)
{
	camera->_min_sensitivity = min;
	camera->_max_sensitivity = max;
}

void gci_camera_get_sensitivity_range(GciCamera* camera, double *min, double *max)
{
	*min = camera->_min_sensitivity;
	*max = camera->_max_sensitivity;
}

void gci_camera_set_blacklevel_range_text(GciCamera* camera, char *min, char *max)
{
	strcpy(camera->_min_bl_text, min);
	strcpy(camera->_min_bl_text, max);   
}

void gci_camera_disable_gain_control(GciCamera* camera)
{
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_DIMMED, 1);              	
}


void gci_camera_enable_gain_control(GciCamera* camera)
{
	SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_GAIN, ATTR_DIMMED, 0);              	
}


int gci_camera_set_data_mode(GciCamera* camera, DataMode data_mode) 
{
	if(camera->_cops.set_datamode == NULL) {
    	send_error_text(camera,"Set data mode not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s data mode to %d bit", UIMODULE_GET_DESCRIPTION(camera), data_mode);

	if ( (*camera->_cops.set_datamode)(camera, data_mode) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set data mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	camera->_data_mode = data_mode;
		
	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(camera), "BitModeChanged", GCI_VOID_POINTER, camera);
	
	return CAMERA_SUCCESS;
}

int gci_camera_set_highest_data_mode(GciCamera* camera) 
{
	DataMode data_mode;

	if(camera->_cops.get_highest_datamode == NULL) {
//		send_error_text(camera,"Set highest data mode not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "%s Error: Set highest data mode not implemented", UIMODULE_GET_DESCRIPTION(camera));  // just warn of this in log
    	return CAMERA_ERROR;
  	}

	if ( (*camera->_cops.get_highest_datamode)(camera, &data_mode) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set highest data mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	return gci_camera_set_data_mode(camera, data_mode); 	
}

int gci_camera_set_lowest_data_mode(GciCamera* camera) 
{
	DataMode data_mode;

	if(camera->_cops.get_lowest_datamode == NULL) {
//    	send_error_text(camera,"Set lowest data mode not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "%s Error: Set lowest data mode not implemented", UIMODULE_GET_DESCRIPTION(camera));  // just warn of this in log
    	return CAMERA_ERROR;
  	}

	if ( (*camera->_cops.get_lowest_datamode)(camera, &data_mode) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set lowest data mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	return gci_camera_set_data_mode(camera, data_mode); 	
}

int gci_camera_get_data_mode(GciCamera* camera)
{
	return camera->_data_mode;
}

int gci_camera_set_light_mode(GciCamera* camera, LightMode light_mode) 
{
	if(camera->_cops.set_lightmode == NULL)
    	return CAMERA_SUCCESS;
	
	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s light mode to %d", UIMODULE_GET_DESCRIPTION(camera), light_mode);

	if ( (*camera->_cops.set_lightmode)(camera, light_mode) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set light mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}

int gci_camera_supports_fast_mode(GciCamera* camera) 
{
	if(camera->_cops.supports_fast_mode == NULL) {
    	return 0;
  	}

	return (*camera->_cops.supports_fast_mode)(camera);
}

int gci_camera_set_fast_mode(GciCamera* camera, int enable) 
{
	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s set_fast_mode mode to %d", UIMODULE_GET_DESCRIPTION(camera), enable);

	if(camera->_cops.set_fast_mode == NULL) {
    	return CAMERA_SUCCESS;
  	}

	camera->_prevent_getting_images = 1;

	if ( (*camera->_cops.set_fast_mode)(camera, enable) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to call gci_camera_set_fast_mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		camera->_prevent_getting_images = 0;	
		return CAMERA_ERROR;
	}
	
	camera->_prevent_getting_images = 0;	

	return CAMERA_SUCCESS;
}

int gci_camera_get_light_mode(GciCamera* camera)
{
	if(camera->_cops.get_lightmode == NULL)
    	return CAMERA_SUCCESS;

  	return (*camera->_cops.get_lightmode)(camera);
}

int gci_camera_get_colour_type(GciCamera* camera)
{
	int colour;

	if(camera->_cops.get_colour_type == NULL) {
    	send_error_text(camera,"Get colour type not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

	colour = (*camera->_cops.get_colour_type)(camera);
			
	if( colour == CAMERA_ERROR) {
	
		send_error_text(camera, "Failed to get colour type");
		return CAMERA_ERROR;
	}
	else {
	
		camera->_colour_type = colour;
	}
	
	return camera->_colour_type;
}


int gci_camera_set_gain(GciCamera* camera, CameraChannel channel, double gain)
{
	// In this fn, the Lynx camera may not allow the gain to be set to the required value because of the dual tap gains
	// so we must set the struct to hold the value before setting the camera and then use the value actually set.
	
	if(camera->_cops.set_gain == NULL) {
    	send_error_text(camera,"Set gain not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

	if (channel==CAMERA_ALL_CHANNELS) 
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s gain to %f", UIMODULE_GET_DESCRIPTION(camera), gain);
	else
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s gain channel %d to %f", UIMODULE_GET_DESCRIPTION(camera), channel, gain);
	
	if ( (*camera->_cops.set_gain)(camera, channel, gain) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set gain");
		return CAMERA_ERROR;
	}

	if(camera->_main_ui_panel != -1) {
		if(channel == CAMERA_CHANNEL1 || channel == CAMERA_ALL_CHANNELS)
			SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_GAIN, gain); 
	}
		
	camera->_gain = gain;
	
	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(camera), "CameraGainChanged", GCI_VOID_POINTER, camera);  
	
	return CAMERA_SUCCESS;
}

int gci_camera_get_gain(GciCamera* camera, CameraChannel channel, double *gain)
{
	if(camera->_cops.get_gain == NULL) {
    	send_error_text(camera,"Get gain not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}
			  
	if ( (*camera->_cops.get_gain)(camera, channel, gain) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to get gain");
		return CAMERA_ERROR;
	}
	
	camera->_gain = *gain;
	
	return CAMERA_SUCCESS;      
}

int gci_camera_attempt_recovery (GciCamera* camera)
{
	if(camera->_cops.attempt_recovery == NULL) {
    	return CAMERA_SUCCESS;
  	}

	logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "%s: Attemtping restart.", UIMODULE_GET_DESCRIPTION(camera) );
	ProcessSystemEvents();

	if ( (*camera->_cops.attempt_recovery)(camera) == CAMERA_ERROR ) {
			
		send_error_text(camera, "gci_camera_attempt_recovery failed");
		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;   
}

//////////////////////////////////////////////////////////////////////////////////

void gci_camera_activate_grab_timer(GciCamera* camera)
{
	int live_thread_status = 0;
	HANDLE live_thread_handle = NULL;

	// If the thread is not suspended then dont resume the thread again
	if(camera->_live_thread_suspended == 0)
		return;

	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), camera->_live_thread,
			ATTR_TP_FUNCTION_EXECUTION_STATUS, &live_thread_status);

	// Thread no longer executing. Must be shutdown time.
	if(live_thread_status != kCmtThreadFunctionExecuting)
		return;

	//printf("Activating live thread\n");
		
	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), camera->_live_thread,
		ATTR_TP_FUNCTION_THREAD_HANDLE, &live_thread_handle);

	if(ResumeThread (live_thread_handle) == -1) {
		GCI_MessagePopup("Error", "Resume Thread Error: %d", GetLastError());
	}
}

void gci_camera_deactivate_grab_timer(GciCamera* camera)
{
	double start_time = Timer();
	int live_thread_status = -1;

	// Already suspended so do nothing
	if(camera->_live_thread_suspended == 1)
		return;

	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), camera->_live_thread,
			ATTR_TP_FUNCTION_EXECUTION_STATUS, &live_thread_status);

	// Thread no longer executing. Must be shutdown time.
	if(live_thread_status != kCmtThreadFunctionExecuting)
		return;

	//printf("Deactivating live thread\n");
	camera->_live_thread_suspend = 1;

	while(camera->_live_thread_suspended == 0 && camera->_exit_thread == 0 && ((Timer() - start_time) < 10.0)) {
		// Maybe I should use YieldProcessor here. Instead of the ProcessSysyemEvents ?
		// Either way this is neccessary to force the current thread to pause so that
		// the other thread has access to the chip's resources.
		//printf("In While loop of deactivate\n");
		Delay(0.05);
		ProcessSystemEvents();
	}
} 


int gci_camera_set_live_mode(GciCamera* camera)
{
	int last_aquire_state = camera->_aquire_mode;

	int was_suspended = camera->_live_thread_suspended;

	// Not suspended so we stop the live thread
	gci_camera_deactivate_grab_timer(camera);

  	if(camera->_cops.set_live_mode == NULL) {
  	
    	send_error_text(camera,"Set live mode not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}
	
	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s live", UIMODULE_GET_DESCRIPTION(camera));    

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "EnterLiveMode", GCI_VOID_POINTER, camera);

  	if ( (*camera->_cops.set_live_mode)(camera) == CAMERA_ERROR ) {
  	
  		//send_error_text(camera,"Can not set live / snap mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		
		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "Can not set live / snap mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera));
		
		return CAMERA_ERROR;
	}

	// We was live before 
	// if(was_suspended == 0)
	gci_camera_activate_grab_timer(camera);

	camera->_aquire_mode = LIVE; 
	
	GCI_ImagingWindow_SetLiveStatus(camera->_camera_window, 1);
	
	if(camera->_default_ui) {
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_LIVE, ATTR_DIMMED, 1);				
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_BUSY, 1);
	}
	
	if(last_aquire_state == SNAP) {
	
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "ExitSnapMode", GCI_VOID_POINTER, camera);
	}
	else if (last_aquire_state == SNAP_SEQUENCE) {

		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "ExitSnapSequenceMode", GCI_VOID_POINTER, camera);
	}   
	

	return CAMERA_SUCCESS;
}

int gci_camera_set_snap_mode(GciCamera* camera)
{
	int last_aquire_state = camera->_aquire_mode;
	char title[50];

	gci_camera_deactivate_grab_timer(camera);

  	if(camera->_cops.set_snap_mode == NULL) {
  	
    	send_error_text(camera,"Set snap mode not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}
		
	if(camera->_main_ui_panel != -1 && camera->_default_ui) {
	
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_LIVE, ATTR_DIMMED, 0);
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_BUSY, 0); 
	}

	camera->_aquire_mode = SNAP;	
		
	GCI_ImagingWindow_SetLiveStatus(camera->_camera_window, 0);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "EnterSnapMode", GCI_VOID_POINTER, camera);
	
  	if ( (*camera->_cops.set_snap_mode)(camera) == CAMERA_ERROR ) {
  	
  		//send_error_text(camera,"Can not set snap mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "Can not set snap mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera));
		
		return CAMERA_ERROR;
	}

	// We are no longer live so reset the fps
	camera->_fps = 0.0;

	if(camera->_default_ui) {
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_LIVE, 0);				
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_BUSY, 0);
	}
	
	sprintf(title, "%s", UIMODULE_GET_DESCRIPTION(camera) );
	
	// We are no longer in live mode or snaped an image
	// For any existing window update the title to show capturing has stopped. 
	if(camera->_camera_window != NULL)
		GCI_ImagingWindow_SetWindowTitle(camera->_camera_window, title);	

	if(last_aquire_state == LIVE) {
	
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "ExitLiveMode", GCI_VOID_POINTER, camera);
	}
	else if (last_aquire_state == SNAP_SEQUENCE) {

		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "ExitSnapSequenceMode", GCI_VOID_POINTER, camera);
	} 
	
	return CAMERA_SUCCESS;
}


int gci_camera_set_snap_sequence_mode(GciCamera* camera)
{
	int last_aquire_state = camera->_aquire_mode; 
		
	gci_camera_deactivate_grab_timer(camera);

  	if(camera->_cops.set_snap_sequence_mode == NULL) {
  	
    	send_error_text(camera,"Set snap sequence mode not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}
		
	if(camera->_main_ui_panel != -1 && camera->_default_ui) {
	
		SetCtrlAttribute(camera->_main_ui_panel, CAMERA_PNL_LIVE, ATTR_DIMMED, 0);
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_BUSY, 0);
	}
		
	camera->_aquire_mode = SNAP_SEQUENCE; 
	
	GCI_ImagingWindow_SetLiveStatus(camera->_camera_window, 0);
			
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "EnterSnapSequenceMode", GCI_VOID_POINTER, camera);
			
  	if ( (*camera->_cops.set_snap_sequence_mode)(camera) == CAMERA_ERROR ) {
  	
  		send_error_text(camera,"Can not set snap sequence mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		return CAMERA_ERROR;
	}

	if(last_aquire_state == LIVE) {
	
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "ExitLiveMode", GCI_VOID_POINTER, camera);
	}
	else if (last_aquire_state == SNAP) {

		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "ExitSnapMode", GCI_VOID_POINTER, camera);
	}   
	
	return CAMERA_SUCCESS;
}


int gci_camera_activate_live_display(GciCamera* camera)
{
	if(gci_camera_is_live_mode(camera)) {
		gci_camera_activate_grab_timer(camera);     
	}
	else
		return CAMERA_ERROR;
	
	return CAMERA_SUCCESS;
}

int gci_camera_show_window(GciCamera* camera)
{
	int id = GCI_ImagingWindow_GetPanelID(camera->_camera_window); 
	
	gci_camera_read_or_write_camera_imagewindow_registry_settings(camera, id, 0);   
	
	GCI_ImagingWindow_Show(camera->_camera_window);    
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "CameraShow", GCI_VOID_POINTER, camera);  

	return CAMERA_SUCCESS;
}

int  gci_camera_snap_image(GciCamera* camera)
{
	FIBITMAP *dib = NULL;
	char title[50];
	int is_live;

	if (camera == NULL)
		return CAMERA_ERROR;

	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s snap image", UIMODULE_GET_DESCRIPTION(camera));    
	
	GCI_ImagingWindow_SetFramesPerSecondIndicator(camera->_camera_window, 0.0);   
	
	is_live = gci_camera_is_live_mode(camera);
	
	gci_camera_set_snap_mode(camera);	

	dib = gci_camera_get_image(camera, NULL); 

	if(dib == NULL)
		return CAMERA_ERROR;

	sprintf(title, "%s Snap", UIMODULE_GET_DESCRIPTION(camera) );        
	
	gci_camera_display_image(camera, dib, title);
	
	FreeImage_Unload(dib);
	
	return CAMERA_SUCCESS;
}

int  gci_camera_snap_average_image(GciCamera* camera, unsigned char average)
{
	FIBITMAP *dib = NULL;
	char title[50];
	int is_live;

	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s snap image", UIMODULE_GET_DESCRIPTION(camera));    
	
	GCI_ImagingWindow_SetFramesPerSecondIndicator(camera->_camera_window, 0.0);   
	
	is_live = gci_camera_is_live_mode(camera);
	
	gci_camera_set_snap_mode(camera);	

	dib = gci_camera_get_image_average_for_frames(camera, average); 
	
	sprintf(title, "%s Snap", UIMODULE_GET_DESCRIPTION(camera) );        
	
	gci_camera_display_image_advanced(camera, dib, title, average);
	
	FreeImage_Unload(dib);
	
	return CAMERA_SUCCESS;
}

int  gci_camera_update_live_image(GciCamera* camera)
{
	FIBITMAP *dib = NULL;
	char title[50];
	
	if (!gci_camera_is_live_mode(camera)) return CAMERA_SUCCESS;	// not in live mode, just exit
	
	dib = gci_camera_get_image(camera, NULL); 
	
	sprintf(title, "%s", UIMODULE_GET_DESCRIPTION(camera) );        
	
	gci_camera_display_image(camera, dib, title);
	
	FreeImage_Unload(dib);
	
	return CAMERA_SUCCESS;
}


int gci_camera_is_live_mode(GciCamera* camera)
{
	return (camera->_aquire_mode == LIVE) ? 1 : 0;
}


int gci_camera_is_snap_mode(GciCamera* camera)
{
	return (camera->_aquire_mode == SNAP) ? 1 : 0;
}


int gci_camera_is_snap_sequence_mode(GciCamera* camera)
{
	return (camera->_aquire_mode == SNAP_SEQUENCE) ? 1 : 0;
}


int  gci_camera_set_average_frame_number(GciCamera* camera, const int frames)
{
	camera->_average_frames = frames;
	
	return CAMERA_SUCCESS;
}

int  gci_camera_get_average_frame_number(GciCamera* camera)
{
	return camera->_average_frames;
}

static FIBITMAP * try_getting_image(GciCamera* camera, const Rect *rect)
{
	FIBITMAP *dib = NULL;

	camera->_bad_frame_count = 0;  

	do {
	
		if(camera->_prevent_getting_images)
//			goto FINISHED;
			return dib;

		if (camera->_aquire_mode != LIVE)
			logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s get image.", UIMODULE_GET_DESCRIPTION(camera));

		dib = (*camera->_cops.get_image)(camera, rect);
	
		if(dib != NULL)
			break;
		
		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "%s get image failed.", UIMODULE_GET_DESCRIPTION(camera));
	}
	while (camera->_bad_frame_count++ < 3);

	return dib;
}

FIBITMAP * gci_camera_get_image(GciCamera* camera, const Rect *rect)
{
	FIBITMAP *dib = NULL;

		
	//get_time_string(time_str);
	//printf("%s: gci_camera_get_image call gci_camera_get_lock\n", time_str);
	gci_camera_get_lock(camera);

	//#ifdef VERBOSE_DEBUG
	//printf("gci_camera_get_image started\n");
	//#endif

	if(camera->_cops.get_image == NULL) {
	
    	send_error_text(camera,"Get Image not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	goto FINISHED;
  	}

	if(camera->_prevent_getting_images)
		goto FINISHED;

	//get_time_string(time_str);
	//printf("%s: gci_camera_get_image call EnterGetImage\n", time_str);
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "EnterGetImage", GCI_VOID_POINTER, camera);

	if ((camera->_main_ui_panel >= 0) && (camera->_aquire_mode != LIVE) && camera->_default_ui)
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_BUSY, 1);


	//get_time_string(time_str);
	//printf("%s: gci_camera_get_image call try_getting_image\n", time_str);

	dib = try_getting_image (camera, rect);


	//get_time_string(time_str);
	//printf("%s: try_getting_image done.\n", time_str);

	if(dib == NULL)
	{
		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "%s get image failed three times.", UIMODULE_GET_DESCRIPTION(camera));
  		gci_camera_attempt_recovery (camera);
		gci_camera_set_snap_mode(camera);
		
		//get_time_string(time_str);
		//printf("%s: gci_camera_get_image call try_getting_image after recovery\n", time_str);

		dib = try_getting_image (camera, rect);

		if(dib == NULL)
		{
			logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "%s recovery failed.", UIMODULE_GET_DESCRIPTION(camera));
			goto FINISHED;
		}
	}

  	if(camera->_horz_flip == 1)
  		FreeImage_FlipHorizontal(dib); 
  	
	if(camera->_vert_flip == 1)    
		FreeImage_FlipVertical(dib);   
  	
	if(camera->_rotation_angle != 0.0) {

		FIBITMAP *rotated = FreeImage_Rotate(dib, camera->_rotation_angle, 0);
		FreeImage_Unload(dib);
		dib = rotated;
	}

	if(GCI_Signal_IsConnected(UIMODULE_SIGNAL_TABLE(camera), "ExitGetImage")) {
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(camera), "ExitGetImage", GCI_VOID_POINTER, camera, GCI_VOID_POINTER, &dib);
	}

	if ((camera->_main_ui_panel >= 0) && (camera->_aquire_mode != LIVE) && camera->_default_ui)
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_BUSY, 0);
	
FINISHED:

	//#ifdef VERBOSE_DEBUG
	//printf("gci_camera_get_image finished\n");
	//#endif

	//get_time_string(time_str);
	//printf("%s: gci_camera_get_image call gci_camera_release_lock\n", time_str);
	gci_camera_release_lock(camera);

  	return dib;
}

static FIBITMAP**  camera_get_images(GciCamera* camera, int number_of_images)
{
	FIBITMAP** sequence = (FIBITMAP**) malloc(sizeof(FIBITMAP*) * number_of_images);
	int i;
	
	gci_camera_set_snap_sequence_mode(camera);   
	
	for(i=0; i < number_of_images; i++) {
		
		sequence[i] = gci_camera_get_image(camera, NULL);	
	}
	
   	return sequence;
}
	
	
FIBITMAP ** gci_camera_get_images(GciCamera* camera, int number_of_images)
{
	FIBITMAP **dib;

	if(camera->_cops.get_images == NULL) {
	
    	// Get Images not implemented for device so we do it the normal way
    	return camera_get_images(camera, number_of_images);   
  	}

	if ((camera->_main_ui_panel >= 0) && (camera->_aquire_mode != LIVE) && camera->_default_ui)
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_BUSY, 1);

  	if( (dib = (*(camera->_cops.get_images))(camera, number_of_images) ) == NULL) {
  	
  		send_error_text(camera,"Can not get image for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		
  		gci_camera_set_snap_mode(camera);
  	
  		return NULL;
  	}
  		
	if ((camera->_main_ui_panel >= 0) && (camera->_aquire_mode != LIVE) && camera->_default_ui)
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_BUSY, 0);

  	return dib;
}

int gci_camera_set_no_display (GciCamera* camera, int val)
{
	camera->_no_display = val;
	return CAMERA_SUCCESS;
}

int gci_camera_display_image_advanced(GciCamera* camera, FIBITMAP *dib, char *title, int average_count)
{
//	int id;

	if(dib == NULL || camera == NULL || dib->data == NULL)
		return CAMERA_ERROR;

	// GCI_ImagingWindow_LoadFreeImageBitmap calls Process System events 
	// and the window may have been closed
	if(camera == NULL || camera->_camera_window == NULL)
		return CAMERA_ERROR;	
	
	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(camera), "PreDisplayImage", GCI_VOID_POINTER, camera, GCI_VOID_POINTER, camera->_camera_window);

	if (camera->_no_display==0)
		GCI_ImagingWindow_LoadImage(camera->_camera_window, dib);
	
	if(average_count > 0)
		camera->_last_image_average_count = average_count;
	else
		camera->_last_image_average_count = 0;

	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(camera), "PostDisplayImage", GCI_VOID_POINTER, camera, GCI_VOID_POINTER, camera->_camera_window);

	return CAMERA_SUCCESS;
}

int gci_camera_get_average_count_of_last_image_displayed(GciCamera* camera)
{
	return camera->_last_image_average_count;
}

int gci_camera_display_image(GciCamera* camera, FIBITMAP *dib, char *title)
{
	return gci_camera_display_image_advanced(camera, dib, title, 0);
}


int	gci_camera_set_microns_per_pixel(GciCamera* camera, double factor)
{
	if(!camera->_camera_window)
		return CAMERA_ERROR;	

	GCI_ImagingWindow_SetMicronsPerPixelFactor(camera->_camera_window, factor);
	
	return CAMERA_SUCCESS;
}


double gci_camera_get_microns_per_pixel(GciCamera* camera)
{
	if(!camera->_camera_window)
		return CAMERA_ERROR;	

	return GCI_ImagingWindow_GetMicronsPerPixelFactor(camera->_camera_window);
}


GCIWindow* gci_camera_get_imagewindow(GciCamera* camera)
{
	return camera->_camera_window;
}


FIBITMAP* gci_camera_get_image_average(GciCamera* camera)
{
	return gci_camera_get_image_average_for_frames(camera, gci_camera_get_average_frame_number(camera) );
}


FIBITMAP* gci_camera_get_image_average_for_frames(GciCamera* camera, int frames)
{
	int i, width, height, colour_mode;
	FIBITMAP *float_dib = NULL, *dib = NULL;
	FIBITMAP *red_channel_double_dib = NULL, *green_channel_double_dib = NULL, *blue_channel_double_dib = NULL;
	FIBITMAP *red_channel_dib = NULL, *green_channel_dib = NULL, *blue_channel_dib = NULL;

	colour_mode = gci_camera_get_colour_type(camera);
	
	gci_camera_set_live_mode(camera);
	
	// Camera in live mode - faster than the sequence above
	SetWaitCursor(1);

	// We get the size form the actual image.
	// Hack as gci_camera_get_size is incorrect for lynx camera.
	// This method is more robust.
	if((dib = gci_camera_get_image(camera, NULL)) == NULL)
		goto Error;
	
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
			
	FreeImage_Unload(dib);

	if(colour_mode == MONO_TYPE) { 
	
		float_dib = FreeImage_AllocateT(FIT_FLOAT, width, height, 32, 0, 0, 0);
	
		for (i=0; i < frames; i++) {
	
			if((dib = gci_camera_get_image(camera, NULL)) == NULL)
				goto Error;
	
			if( FIA_AddGreyLevelImages(float_dib, dib) == FIA_ERROR)	
				goto Error;
		
			FreeImage_Unload(dib);
		}
			
		// Divide by number of images. 
		if( FIA_DivideGreyLevelImageConstant(float_dib, frames) == FIA_ERROR)	
			goto Error;

/*
		if( gci_camera_get_data_mode(camera) == BPP8) {
	
			dib = FreeImage_ConvertToStandardType(float_dib, 0);
		}
		else {
			
			type = FreeImage_GetImageType(dib);
			
	 		if (type == FIT_UINT16)
				dib = GCI_FreeImage_ConvertTo_FIT_UINT16(float_dib);
			else
				dib = GCI_FreeImage_ConvertTo_FIT_INT16(float_dib);
		}
*/		
		dib = FreeImage_Clone(float_dib);
		
		FreeImage_Unload(float_dib);
	}
	else {
	
		red_channel_double_dib = FreeImage_AllocateT(FIT_FLOAT, width, height, 32, 0, 0, 0);
		green_channel_double_dib = FreeImage_AllocateT(FIT_FLOAT, width, height, 32, 0, 0, 0);
		blue_channel_double_dib = FreeImage_AllocateT(FIT_FLOAT, width, height, 32, 0, 0, 0);
	
		for (i=0; i < frames; i++) {
	
			dib = gci_camera_get_image(camera, NULL);
		
			// We have a RGB Type. We must get the individual channels and add each of those.
			red_channel_dib = FreeImage_GetChannel(dib, FICC_RED);
			green_channel_dib = FreeImage_GetChannel(dib, FICC_GREEN);
			blue_channel_dib = FreeImage_GetChannel(dib, FICC_BLUE);
			
			if( FIA_AddGreyLevelImages(red_channel_double_dib, red_channel_dib) == FIA_ERROR)
				goto Error;
			
			if( FIA_AddGreyLevelImages(green_channel_double_dib, green_channel_dib) == FIA_ERROR)
				goto Error;
			
			if( FIA_AddGreyLevelImages(blue_channel_double_dib, blue_channel_dib) == FIA_ERROR)
				goto Error;
			
			FreeImage_Unload(red_channel_dib);
			FreeImage_Unload(green_channel_dib);
			FreeImage_Unload(blue_channel_dib);
			
			FreeImage_Unload(dib);
		}	
	
		// Divide channels by number of images. 
		if( FIA_DivideGreyLevelImageConstant(red_channel_double_dib, frames) == FIA_ERROR)
			goto Error;	
		
		if( FIA_DivideGreyLevelImageConstant(green_channel_double_dib, frames) == FIA_ERROR)
			goto Error;	
		
		if( FIA_DivideGreyLevelImageConstant(blue_channel_double_dib, frames) == FIA_ERROR)
			goto Error;
	
		red_channel_dib = FreeImage_ConvertToStandardType(red_channel_double_dib, 0);
		green_channel_dib = FreeImage_ConvertToStandardType(green_channel_double_dib, 0); 
		blue_channel_dib = FreeImage_ConvertToStandardType(blue_channel_double_dib, 0); 
	
		FreeImage_Unload(red_channel_double_dib);
		FreeImage_Unload(green_channel_double_dib);
		FreeImage_Unload(blue_channel_double_dib);
	
		dib = gci_camera_get_image(camera, NULL); 
	
		FreeImage_SetChannel(dib, red_channel_dib, FICC_RED);
		FreeImage_SetChannel(dib, green_channel_dib, FICC_GREEN);
		FreeImage_SetChannel(dib, blue_channel_dib, FICC_BLUE);
		
		FreeImage_Unload(red_channel_dib);
		FreeImage_Unload(green_channel_dib);
		FreeImage_Unload(blue_channel_dib);
	}

	gci_camera_set_snap_mode(camera);
	
	SetWaitCursor(0);

	return dib;
	

Error:
	
	if(red_channel_dib != NULL)
		FreeImage_Unload(red_channel_dib);
				
	if(green_channel_dib != NULL)
		FreeImage_Unload(green_channel_dib);
					
	if(blue_channel_dib != NULL)
		FreeImage_Unload(blue_channel_dib);
				
	if(red_channel_double_dib != NULL)
		FreeImage_Unload(red_channel_double_dib);
				
	if(green_channel_double_dib != NULL)
		FreeImage_Unload(green_channel_double_dib);
					
	if(blue_channel_double_dib != NULL)
		FreeImage_Unload(blue_channel_double_dib);
			
	if(dib != NULL)
		FreeImage_Unload(dib);
			
	if(float_dib != NULL)
		FreeImage_Unload(float_dib);
			
	SetWaitCursor(0);
		
	return NULL;	
}


FIBITMAP* gci_camera_get_displayed_image(GciCamera* camera)
{
	if(camera == NULL || camera->_camera_window == NULL)
		return NULL;
		
	return GCI_ImagingWindow_GetOriginalFIB(camera->_camera_window); 
}


int gci_camera_set_default_settings(GciCamera* camera)
{
	if(camera->_cops.set_default_settings == NULL) {
	
    	send_error_text(camera,"Set default settings not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

  	if( (*camera->_cops.set_default_settings)(camera)  == CAMERA_ERROR) {
  	
  		send_error_text(camera,"Can not set default settings for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		return CAMERA_ERROR;
  	}
	
	return CAMERA_SUCCESS;
}


int gci_camera_set_subwindow(GciCamera* camera, SUBWINDOW_SIZE size)
{
	if(camera->_cops.set_subwindow == NULL) {
	
    	send_error_text(camera,"set_subwindow not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

  	if( (*camera->_cops.set_subwindow)(camera, size)  == CAMERA_ERROR) {
  	
  		send_error_text(camera,"Can not set set_subwindow for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		return CAMERA_ERROR;
  	}
	
	return CAMERA_SUCCESS;
}

int  gci_camera_save_settings_as_default(GciCamera* camera)
{
	if(camera->_cops.save_settings_as_default == NULL) {
	
    	send_error_text(camera,"Save settings as default not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

  	if( (*camera->_cops.save_settings_as_default)(camera)  == CAMERA_ERROR) {
  	
  		send_error_text(camera,"Save settings as default method failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		return CAMERA_ERROR;
  	}
	
	return CAMERA_SUCCESS;
}

int gci_camera_supports_focal_indicator_settings(GciCamera *camera)
{
	if(camera->_cops.supports_focal_indicator_settings == NULL) {
    	return 0;
  	}

  	return (*camera->_cops.supports_focal_indicator_settings)(camera);
}


FocusSettings gci_camera_get_focal_indicator_settings (GciCamera* camera)
{
	FocusSettings settings;

	if(camera->_cops.get_focal_indicator_settings == NULL) {
	
		settings.sample_type = FOCUS_SETTINGS_SAMPLE_DETAILED;
		settings.crop_type = FOCUS_SETTINGS_CROP_NONE;
		settings.resample_type = FOCUS_SETTINGS_RESAMPLE_NONE;
  	}
	else {
  		settings = (*camera->_cops.get_focal_indicator_settings)(camera);
	}

	return settings;
}

int  gci_camera_get_state_as_ini_string(GciCamera* camera, char *buffer)
{
	if(camera->_cops.get_state_as_ini_string == NULL) {
	
    	send_error_text(camera,"get_state_as_ini_string not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

  	if( (*camera->_cops.get_state_as_ini_string)(camera, buffer)  == CAMERA_ERROR) {
  	
  		send_error_text(camera,"get_state_as_ini_string method failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		return CAMERA_ERROR;
  	}
	
	return CAMERA_SUCCESS;
}

int gci_camera_save_settings(GciCamera* camera, const char *filepath, const char *mode)
{
	char name[500], buffer[300];

	if(camera->_cops.save_settings == NULL) {
	
    	send_error_text(camera,"Save settings not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

    if (FileExists(filepath, NULL))
    	SetFileAttrs (filepath, 0, -1, -1, -1);       //clear read-only

  	if( (*camera->_cops.save_settings)(camera, filepath, mode)  == CAMERA_ERROR) {
  	
  		send_error_text(camera,"Can not save settings for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		return CAMERA_ERROR;
  	}
	
	if(camera->_default_ui) {
		
		GetFilenameFromPath(filepath, name);
		sprintf(buffer, "%s", name);
	
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_FILENAME, buffer);
	}

	return CAMERA_SUCCESS;
}


int  gci_camera_load_settings(GciCamera* camera, const char *filename)
{
	char name[500], buffer[300];
	
	memset(name, 0, 1);
	
	if(filename == NULL) {
	
    	return CAMERA_ERROR;
	}

	if(camera->_cops.load_settings == NULL) {
	
    	send_error_text(camera,"Load settings not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

  	if( (*camera->_cops.load_settings)(camera, filename)  == CAMERA_ERROR) {
  	
  		return CAMERA_ERROR;
  	}
	
	if(camera->_default_ui) {
		
		GetFilenameFromPath(filename, name);
		sprintf(buffer, "%s", name);
		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_FILENAME, buffer);
	}
		
	return CAMERA_SUCCESS;
}

int  gci_camera_set_min_size(GciCamera* camera, unsigned int width, unsigned int height)
{
	camera->_min_width = width;
	camera->_min_height = height; 

	return CAMERA_SUCCESS;  
}

int  gci_camera_set_max_size(GciCamera* camera, unsigned int width, unsigned int height)
{
	camera->_max_width = width;
	camera->_max_height = height; 
	
	return CAMERA_SUCCESS;  
}

int gci_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = camera->_min_width;
	*height = camera->_min_height;
	
	return CAMERA_SUCCESS;
}


int gci_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = camera->_max_width;
	*height = camera->_max_height;
	
	return CAMERA_SUCCESS;
}


int  gci_camera_set_size_position(GciCamera* camera, unsigned int left, unsigned int top,
													 unsigned int width, unsigned int height, unsigned char auto_centre)
{
	if(camera->_cops.set_size_position == NULL) {
	
  		  send_error_text(camera,"Set size and position is not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		  return CAMERA_ERROR;
 	}
	
	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s AOI to %d %d %d %d", UIMODULE_GET_DESCRIPTION(camera), left, top, width, height);

	if( (*camera->_cops.set_size_position)(camera, left, top, width, height, auto_centre) == CAMERA_ERROR) {
	
		send_error_text(camera,"Set size and position method failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		return CAMERA_ERROR;
	}
	
	return CAMERA_SUCCESS;
}


int  gci_camera_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
													 unsigned int *width, unsigned int *height, unsigned char *auto_centre)
{
	if(camera->_cops.get_size_position == NULL) {
	
  		  send_error_text(camera,"Get size and position is not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		  return CAMERA_ERROR;
 	}

	if( (*camera->_cops.get_size_position)(camera, left, top, width, height, auto_centre) == CAMERA_ERROR) {
	
		send_error_text(camera,"Get size and position method failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		//logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "Get size and position method failed for device %s", UIMODULE_GET_DESCRIPTION(camera));

		return CAMERA_ERROR;
	}

	return CAMERA_SUCCESS;
}


int  gci_camera_get_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	int left, top;
	unsigned char auto_centre;

	gci_camera_get_size_position(camera, &left, &top, width, height, &auto_centre);
	
	return CAMERA_SUCCESS;
}


int  gci_camera_get_position(GciCamera* camera, unsigned int *left, unsigned int *top, unsigned char *auto_centre)
{
	int width, height;

	gci_camera_get_size_position(camera, left, top, &width, &height, auto_centre);
		
	return CAMERA_SUCCESS;
}


int gci_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus, char *camera_id, char *camera_version, char *driver_version)
{
	if(camera->_cops.get_info == NULL) {
    	send_error_text(camera,"Get camera info is not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

  	if ( (*camera->_cops.get_info)(camera, vendor, model, bus, camera_id, camera_version, driver_version) == CAMERA_ERROR) {
 
  		send_error_text(camera,"Can not get camera info for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
  		return CAMERA_ERROR;
  	}
  	
  	return CAMERA_SUCCESS;
}


int gci_camera_display_main_ui(GciCamera* camera)
{
	if(camera->_main_ui_panel != -1 && camera->_default_ui)
		ui_module_display_main_panel(UIMODULE_CAST(camera));
	
	return CAMERA_SUCCESS;
}


int  gci_camera_hide_main_ui(GciCamera* camera)
{
	if(camera->_main_ui_panel != -1 && camera->_default_ui) {
	
		gci_camera_read_or_write_main_panel_registry_settings(camera, 1);
		HidePanel(camera->_main_ui_panel);
	}
	
	return CAMERA_SUCCESS;
}


int  gci_camera_display_extra_ui(GciCamera* camera)
{
	if(camera->_extra_ui_panel != -1) {
	
		int visible = 0;
		
		GetPanelAttribute(camera->_extra_ui_panel, ATTR_VISIBLE, &visible);
		
		if(visible)
			return CAMERA_SUCCESS; 	
		
		ui_module_display_panel(UIMODULE_CAST(camera), camera->_extra_ui_panel);
	}
		
	return CAMERA_SUCCESS;
}


int  gci_camera_hide_extra_ui(GciCamera* camera)
{
  	if(camera->_extra_ui_panel != -1 && camera->_default_ui) {
  	
  		gci_camera_read_or_write_extra_panel_registry_settings(camera, 1);
  		
		HidePanel(camera->_extra_ui_panel);
	}
		
	return CAMERA_SUCCESS;
}


int  gci_camera_hide_ui(GciCamera* camera)
{
	if(camera->_main_ui_panel != -1 && camera->_default_ui) {
	
		gci_camera_read_or_write_main_panel_registry_settings(camera, 1);
		
		HidePanel(camera->_main_ui_panel);
	}

	if(camera->_extra_ui_panel != -1) {
  	
  		gci_camera_read_or_write_extra_panel_registry_settings(camera, 1);
  		
		HidePanel(camera->_extra_ui_panel);
	}
		
	gci_camera_set_snap_mode(camera);
	
	//GCI_ImagingWindow_Hide(camera->_camera_window);
		
	return CAMERA_SUCCESS;
}


int  gci_camera_disable_ui(GciCamera* camera, int disable)
{
	if (camera != NULL) {
		SetPanelAttribute (camera->_main_ui_panel, ATTR_DIMMED, disable);
		
		if(camera->_extra_ui_panel != -1) 
			SetPanelAttribute (camera->_extra_ui_panel, ATTR_DIMMED, disable);
	}
	return CAMERA_SUCCESS;
}

int  gci_camera_destroy_extra_ui(GciCamera* camera)
{
  	if(camera->_extra_ui_panel != -1) {
  	
  		gci_camera_read_or_write_extra_panel_registry_settings(camera, 1);
  		
  		DiscardPanel(camera->_extra_ui_panel);
  	}
  	
  	camera->_extra_ui_panel = -1;
  	
  	return CAMERA_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////

int gci_camera_supports_binning(GciCamera *camera)
{
	if(camera->_cops.supports_binning == NULL) {
    	return 0;
  	}

  	return (*camera->_cops.supports_binning)(camera);
}



int  gci_camera_set_binning_mode(GciCamera* camera, BinningMode binning)
{
	if(camera->_cops.set_binning_mode == NULL) 
    	return CAMERA_SUCCESS;

	logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s binning to %d by %d", UIMODULE_GET_DESCRIPTION(camera), binning, binning);  
	
  	if( (*camera->_cops.set_binning_mode)(camera, binning) == CAMERA_ERROR) {
  	
  		send_error_text(camera, "Binning mode operation failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
  	}
  	
	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(camera), "BinningModeChanged", GCI_VOID_POINTER, camera);

  	return CAMERA_SUCCESS;
}


BinningMode  gci_camera_get_binning_mode(GciCamera* camera)
{
	if(camera->_cops.get_binning_mode == NULL)
    	return NO_BINNING;

  	return (*camera->_cops.get_binning_mode)(camera);
}

int gci_camera_set_highest_binning_mode(GciCamera* camera) 
{
	BinningMode binning;

	if(camera->_cops.get_highest_binning_mode == NULL) {
//    	send_error_text(camera,"Set highest binning mode not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		logger_log(UIMODULE_LOGGER(camera), LOGGER_WARNING, "%s Error: Set highest binning mode not implemented", UIMODULE_GET_DESCRIPTION(camera));  // just warn of this in log
    	return CAMERA_SUCCESS; // This is not neccessarily a fatal error
  	}

	if ( (*camera->_cops.get_highest_binning_mode)(camera, &binning) == CAMERA_ERROR ) {
			
		send_error_text(camera, "Failed to set highest binning mode for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		return CAMERA_ERROR;
	}
	
	return gci_camera_set_binning_mode(camera, binning); 	
}

double  gci_camera_get_true_microns_per_pixel(GciCamera* camera)
{
	double microns_per_pixel;
	BinningMode binning;
	
	if(camera->_camera_window == NULL)
		return 1.0;

	//Return microns per pixel corrected for current pixel binning
	if(camera->_cops.get_binning_mode == NULL) 
		binning = NO_BINNING;
	else 
		binning = (*camera->_cops.get_binning_mode)(camera);
	
	microns_per_pixel = GCI_ImagingWindow_GetMicronsPerPixelFactor(camera->_camera_window);
  	return microns_per_pixel * binning;
}


//////////////////////////////////////////////////////////////////////////////////
int  gci_camera_set_trigger_mode(GciCamera* camera, TriggerMode trig_mode)
{
	if(camera->_cops.set_trigger_mode == NULL) {
	
    	send_error_text(camera,"Set trigger mode not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	camera->_trigger_mode = 0;
		return CAMERA_ERROR;
  	}

	if (trig_mode == CAMERA_NO_TRIG)
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s trigger mode to CAMERA_NO_TRIG", UIMODULE_GET_DESCRIPTION(camera));  
	else if (trig_mode == CAMERA_INTERNAL_TRIG)
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s trigger mode to CAMERA_INTERNAL_TRIG", UIMODULE_GET_DESCRIPTION(camera));  
	else if (trig_mode == CAMERA_EXTERNAL_TRIG)
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s trigger mode to CAMERA_EXTERNAL_TRIG", UIMODULE_GET_DESCRIPTION(camera));  
	else
		logger_log(UIMODULE_LOGGER(camera), LOGGER_INFORMATIONAL, "%s trigger mode to unknown value", UIMODULE_GET_DESCRIPTION(camera));  

  	if( (*camera->_cops.set_trigger_mode)(camera, trig_mode) == CAMERA_ERROR) {
  		send_error_text(camera, "Set trigger mode operation failed for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
		camera->_trigger_mode = -1;
		return CAMERA_ERROR;
  	}
  	
	camera->_trigger_mode = trig_mode;

  	return CAMERA_SUCCESS;
}

int  gci_camera_get_trigger_mode(GciCamera* camera, TriggerMode *trig_mode)
{
	*trig_mode = camera->_trigger_mode;
		
		return CAMERA_SUCCESS; 
}

int  gci_camera_fire_internal_trigger(GciCamera* camera)
{
	if(camera->_cops.fire_internal_trigger == NULL) {
	
    	send_error_text(camera,"Fire_internal_trigger not implemented for device %s\n", UIMODULE_GET_DESCRIPTION(camera) );
    	return CAMERA_ERROR;
  	}

  	return (*camera->_cops.fire_internal_trigger)(camera);
}


int gci_camera_set_allowed_bitmode (GciCamera* camera, DataMode mode)
{
	camera->supported_datamodes[camera->number_of_datamodes++] = mode;
	
	return CAMERA_SUCCESS;    
}


int gci_camera_set_allowed_binningmode (GciCamera* camera, BinningMode mode)
{
	camera->supported_binning_modes[camera->number_of_binning_modes++] = mode;
	
	return CAMERA_SUCCESS;    
}


int gci_camera_get_number_of_binning_modes (GciCamera* camera)
{
	return camera->number_of_binning_modes;
}

BinningMode* gci_camera_get_allowed_binning_modes (GciCamera* camera)
{
	return camera->supported_binning_modes;
}
