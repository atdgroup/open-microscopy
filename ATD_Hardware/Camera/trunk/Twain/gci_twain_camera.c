#include "gci_camera.h" 
#include "gci_twain_camera.h"
#include "gci_twain_camera_ui.h" 
#include "string_utils.h"
#include "gci_utils.h"
#include "FreeImageAlgorithms_IO.h"

#include "TWAIN.h"
#include "EZTWAIN.H"  

#include <userint.h>
#include "toolbox.h"

#include "FreeImageAlgorithms.h" 
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_HBitmap.h"

#include "icsviewer_uir.h"
#include "string_utils.h"


static HWND hwnd;   

/*

static ListType sourceList;

int openDSM (void)
{
// must open DSM with correct hwnd or messages from DS will not be recieved by our window procedure
	//hMainWnd = (HWND)GetCVIWindowHandle ();
	hMainWnd = (HWND)GetCVIWindowHandleForCurrThread ();
	
	
	if (!TWAIN_OpenSourceManager((HWND)hMainWnd)) 
	{
   		TWAIN_ErrorBox("Unable to open Source Manager");
   		return(-1);
   	}

 // 	if ((panelHandle = LoadPanel (0, "TwainDriver_UI.uir", PANEL_2)) < 0)
//	return (-1);

   	return (0);
}
*/		

/*
int getTwainSourceList (void)
{
	TW_IDENTITY source;
	int error=0;
	
	if (openDSM()<0) {error=-1; goto Error;}
	
	if (sourceList!=0)
	{
		ListDispose (sourceList);
		sourceList = 0;
	}
	
	sourceList = ListCreate (sizeof(TW_IDENTITY));
	if (sourceList==0) {error=-2; goto Error;}

	if (TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &source))
		ListInsertItem (sourceList, (void *)&source, 1);
	else
		{error=-3; goto Error;}

	while (TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &source))
		ListInsertItem (sourceList, (void *)&source, END_OF_LIST);

Error:
	return(error);
}
*/

/*
int openTwainSource (char *nameStr)
{
	TW_IDENTITY source;
	int n, i;

	if (getTwainSourceList()<0)
	{
   		TWAIN_ErrorBox("Unable to get Source list");
   		return(-2);
   	}
   	
	n = ListNumItems (sourceList);
   	for (i=1; i<=n; i++)
   	{
		ListGetItem (sourceList, (void *)&source, i);
		if (strcmp (source.ProductName, nameStr)==0) break;
   	}
   	
   	if (i<=n) // We have a match
   	{
		if (!TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &source)) return(-4);
   	}
   	else return(-3);
 
	return(0);
}
*/

/*
void closeTwain (void)
{
	TWAIN_DisableSource();
	TWAIN_CloseSource();
	TWAIN_CloseSourceManager(hMainWnd);
	TWAIN_UnloadSourceManager();
}
*/

/*
int selectTwainSource (void)
{
	return (!TWAIN_SelectImageSource(hMainWnd));
}
*/

/*
int openTwainDefaultSource (void)
{
	if (openDSM()<0)
		return(-1);
	
	return (!TWAIN_OpenDefaultSource ());
}
*/

static FIBITMAP* GCI_TwainAcquireFIB (IcsViewerWindow *window, int useTwainDeviceUI)
{
    HANDLE hdib;
    FIBITMAP *fib;
	int bpp, width, height;
	static opened_source = 0;	
	
	if(opened_source == 0) {
		
		//hwnd = (HWND)GetCVIWindowHandleForCurrThread ();
	
	    // Display TWAIN Select Source dialog
    	TWAIN_SelectImageSource(window->canvas_window);
		TWAIN_SetHideUI(FALSE);      
		TWAIN_OpenDefaultSource();
		
		TWAIN_NegotiateXferCount(1);  
		//TWAIN_EnableSource(hwnd); 
		opened_source = 1;
	}
	
    // If you can't get a Window handle, use NULL:
    //hdib = TWAIN_AcquireNative(NULL, TWAIN_GRAY | TWAIN_RGB);
	hdib = TWAIN_AcquireNative(window->canvas_window, TWAIN_ANYTYPE);    
	//hdib = TWAIN_WaitForNativeXfer(window->canvas_window);
	
    if (hdib) {

		LPBITMAPINFOHEADER pbi = (LPBITMAPINFOHEADER) GlobalLock(hdib);
  
		width = TWAIN_DibWidth(hdib);
		height = TWAIN_DibHeight(hdib); 
		bpp = TWAIN_DibDepth(hdib);
			
		fib = FreeImage_Allocate(width, height, bpp, 0, 0, 0);    
			
		memcpy(FreeImage_GetBits(fib), (const void*) DibBits(pbi), pbi->biSizeImage);     
	
		GlobalUnlock(hdib);    
			
        // <your image processing here>
        TWAIN_FreeNative(hdib);
			  
		return fib;    
    }
	
	return NULL;
}


static open_twain_source_like(GciCamera* camera, const char *name)
{
	TW_IDENTITY source;  
	
	if (TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &source)) {
		if(strstr(source.ProductName, name) != NULL) {
			if (!TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &source))
				return CAMERA_ERROR;
			
			if(!TWAIN_EnableSource(hwnd))
				return CAMERA_ERROR; 	
			
			return CAMERA_SUCCESS;
		}
		
		while (TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &source))  {
			if(strstr(source.ProductName, name) != 0) {
				if (!TWAIN_Mgr(DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &source))
					return CAMERA_ERROR; 
				
				if(!TWAIN_EnableSource(hwnd))
					return CAMERA_ERROR; 	
				
				return CAMERA_SUCCESS;
			}
		}
	}
	
	return CAMERA_ERROR;
}

static int gci_twain_camera_power_up(GciCamera* camera)
{
	/*
	TW_IDENTITY source; 
	
	// Must open DSM with correct hwnd or messages from DS will not be recieved by our window procedure
	// hMainWnd = (HWND)GetCVIWindowHandle ();
	hwnd = (HWND)GetCVIWindowHandleForCurrThread ();
	
//	if (!TWAIN_OpenSourceManager(hwnd)) 
//	{
 //  		TWAIN_ErrorBox("Unable to open Source Manager");
 //  		return CAMERA_ERROR;
 //  	}

	    // Display TWAIN Select Source dialog
    TWAIN_SelectImageSource(hwnd);
	TWAIN_OpenDefaultSource();
	TWAIN_SetHideUI(TRUE);  
	TWAIN_NegotiateXferCount(1);  
	TWAIN_EnableSource(hwnd);    
	*/
	
//	if(open_twain_source_like(camera, "KY-TWAIN") == CAMERA_ERROR)
//		return CAMERA_ERROR;
		

	//
	//TWAIN_SetHideUI(FALSE); 
	   

  	return CAMERA_SUCCESS;
}


static int gci_twain_camera_power_down(GciCamera* camera)
{
	TWAIN_DisableSource();
	TWAIN_CloseSource();
	TWAIN_CloseSourceManager((HWND)GetCVIWindowHandleForCurrThread ());
	TWAIN_UnloadSourceManager();
	
	return CAMERA_SUCCESS;
}

static int  gci_twain_camera_set_exposure_time(GciCamera* camera, double exposure)
{
	//printf("Setting exposure time for twain camera to %f\n", exposure);

	return CAMERA_SUCCESS;
}


static int  gci_twain_camera_set_gain(GciCamera* camera, CameraChannel channel, double gain)
{
	//printf("Setting gain for twain camera to %f\n", gain);

	return CAMERA_SUCCESS;
}

static int  gci_twain_camera_get_gain(GciCamera* camera, CameraChannel channel, double *gain)
{
	*gain = 2.0;

	return CAMERA_SUCCESS;
}

static int  gci_twain_camera_set_live_mode(GciCamera* camera)
{
	//printf("Setting live mode for twain camera to live mode.\n");

	return CAMERA_SUCCESS;
}


static int  gci_twain_camera_set_snap_mode(GciCamera* camera)
{
	//printf("Setting live mode for twain camera to snap mode.\n");

	return CAMERA_SUCCESS;
}


static int  gci_twain_camera_set_snap_sequence_mode(GciCamera* camera)
{
	//printf("Setting live mode for twain camera to snap sequence mode.\n");

	return CAMERA_SUCCESS;
}


static FIBITMAP*  gci_twain_camera_get_image(GciCamera* camera, const Rect *rect)
{
	return GCI_TwainAcquireFIB (camera->_camera_window, 1) ;
}


static FIBITMAP *  gci_twain_camera_get_image_average(GciCamera* camera, int frames)
{
	//printf("Returning image average for twain camera\n");

	return NULL;
}


static int  gci_twain_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = 100;
	*height = 100;

	//printf("returning camera min size\n");

	return CAMERA_SUCCESS;
}


static int  gci_twain_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = 1024;
	*height = 1200;

	//printf("returning camera max size\n");

	return CAMERA_SUCCESS;
}


static int gci_twain_camera_get_info(GciCamera* camera, char *vendor, char *model, char *bus, char *camera_id, char *camera_version, char *driver_version)
{
	strcpy(vendor, "twain Vendor");
	strcpy(model,"twain Model");
	strcpy(bus,"twain Bus");
	strcpy(camera_id,"twain camera id");
	strcpy(camera_version,"1.03");
	strcpy(driver_version,"2.78");
	
	return CAMERA_SUCCESS;
}


static int gci_twain_camera_set_trigger_mode(GciCamera* camera, TriggerMode trig_mode)
{
	//printf("Set data mode for twain camera\n");
	
	return CAMERA_SUCCESS;
}
	
static int gci_twain_camera_set_data_mode(GciCamera* camera, DataMode data_mode)
{
	//printf("Set data mode for twain camera\n");
	
	return CAMERA_SUCCESS;
}


int gci_twain_camera_destroy(GciCamera* camera)
{
	if(camera == NULL)
		return CAMERA_ERROR;

  	//printf("Destroying twain camera\n");
  	
  	return CAMERA_SUCCESS;
}


int gci_twain_get_size_position(GciCamera* camera, unsigned int *left, unsigned int *top,
												 unsigned int *width, unsigned int *height, unsigned char *auto_centre)
{
	*left = 0;
	*top = 0;
	*width = 1344;
	*height = 1024;
	*auto_centre = 0;

	return CAMERA_SUCCESS;
}

static int gci_twain_save_settings (GciCamera* camera, const char *filepath, const char *mode)
{
	return CAMERA_SUCCESS;
}

static int gci_twain_load_settings (GciCamera* camera, const char *filepath) 
{
	return CAMERA_SUCCESS;
}

static int gci_twain_camera_save_state(GciCamera* camera, CameraState *state)
{
	return CAMERA_SUCCESS;
}

static int gci_twain_camera_restore_state(GciCamera* camera, CameraState *state)
{
	return CAMERA_SUCCESS;
}

static int gci_twain_camera_set_default_settings (GciCamera* camera)
{
	return CAMERA_SUCCESS; 
}

static int gci_twain_camera_get_highest_datamode (GciCamera* camera, DataMode *data_mode)  
{
	*data_mode = BPP12;
	
	return CAMERA_SUCCESS; 
}

static double gci_twain_camera_get_fps(GciCamera* camera)
{
	return 1.0;
}


int gci_twain_initialise(GciCamera *camera)
{

	camera->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(camera), "gci_twain_camera_ui.uir", TCAM_PNL, 1);  
	
//		ui_module_set_window_proc(UIMODULE_CAST(camera), camera->_main_ui_panel, (LONG_PTR) CameraWndProc);	
		
	ui_module_set_main_panel_title (UIMODULE_CAST(camera));
//		SetCtrlVal(camera->_main_ui_panel, CAMERA_PNL_CAMERA_LABEL, UIMODULE_GET_NAME(camera)); 

	#ifdef THREADED_CAM_AQ
	
	camera->_timer = NewAsyncTimer (0.05, -1, 0, camera_on_live_timer_tick, camera);
	SetAsyncTimerAttribute (camera->_timer, ASYNC_ATTR_ENABLED, 0);  
	
	#else
	
	camera->_timer = NewCtrl(camera->_main_ui_panel, CTRL_TIMER, "", 0, 0); 
	SetCtrlAttribute(camera->_main_ui_panel, camera->_timer, ATTR_INTERVAL, 0.05);
	SetCtrlAttribute(camera->_main_ui_panel, camera->_timer, ATTR_ENABLED, 0);
	
//		if ( InstallCtrlCallback (camera->_main_ui_panel, camera->_timer, camera_on_live_timer_tick, camera) < 0)
//			return CAMERA_ERROR;
	
	#endif
	
	
	if ( InstallCtrlCallback (camera->_main_ui_panel, TCAM_PNL_CAPTURE, TwainCameraOnCapturePressed, camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (camera->_main_ui_panel, TCAM_PNL_CLOSE, TwainCameraOnClosePressed, camera) < 0)
		return CAMERA_ERROR;
		
	return CAMERA_SUCCESS;   
}


GciCamera* gci_twain_camera_new(void)
{
	GciCamera* camera = gci_camera_new();

	camera->_default_ui = 0;
	
	gci_camera_set_description(camera, "twain Camera");
	gci_camera_set_name(camera, "twain Camera");
	gci_camera_set_min_size(camera, 32, 8);
	gci_camera_set_max_size(camera, 1344, 1024);
	
	CAMERA_VTABLE_PTR(camera).initialise = gci_twain_initialise;
	CAMERA_VTABLE_PTR(camera).set_default_settings = gci_twain_camera_set_default_settings;
	CAMERA_VTABLE_PTR(camera).destroy = gci_twain_camera_destroy;
	CAMERA_VTABLE_PTR(camera).power_up = gci_twain_camera_power_up;
	CAMERA_VTABLE_PTR(camera).power_down = gci_twain_camera_power_down;
	CAMERA_VTABLE_PTR(camera).set_exposure_time = gci_twain_camera_set_exposure_time;
	CAMERA_VTABLE_PTR(camera).set_gain = gci_twain_camera_set_gain; 
	CAMERA_VTABLE_PTR(camera).get_gain = gci_twain_camera_get_gain;
	CAMERA_VTABLE_PTR(camera).set_live_mode = gci_twain_camera_set_live_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_mode = gci_twain_camera_set_snap_mode;
	CAMERA_VTABLE_PTR(camera).set_snap_sequence_mode = gci_twain_camera_set_snap_sequence_mode;
	CAMERA_VTABLE_PTR(camera).get_image = gci_twain_camera_get_image; 
	CAMERA_VTABLE_PTR(camera).get_image_average = gci_twain_camera_get_image_average; 
	CAMERA_VTABLE_PTR(camera).get_size_position = gci_twain_get_size_position;
	CAMERA_VTABLE_PTR(camera).get_info =	gci_twain_camera_get_info;
	CAMERA_VTABLE_PTR(camera).set_datamode =	gci_twain_camera_set_data_mode;
	CAMERA_VTABLE_PTR(camera).set_trigger_mode =	gci_twain_camera_set_trigger_mode; 
	CAMERA_VTABLE_PTR(camera).save_settings = gci_twain_save_settings;
	CAMERA_VTABLE_PTR(camera).load_settings = gci_twain_load_settings;
	CAMERA_VTABLE_PTR(camera).save_state = gci_twain_camera_save_state;
	CAMERA_VTABLE_PTR(camera).restore_state = gci_twain_camera_restore_state;
	CAMERA_VTABLE_PTR(camera).set_default_settings = gci_twain_camera_set_default_settings;
	CAMERA_VTABLE_PTR(camera).get_fps = gci_twain_camera_get_fps;   
	
	return camera;
}



