#include "realtime_overview.h"
#include "icsviewer_signals.h"
	   
#include "optical_calibration.h"
#include "string_utils.h"
#include "FreeImageIcs_IO.h"
#include "FreeImageAlgorithms_IO.h" 
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_LinearScale.h"

#include "microscope.h"

#include "mosaic.h"

#include <cvirte.h>
#include <userint.h>
#include <toolbox.h>
#include <formatio.h>
#include <utility.h>
 
static void ImageWindowCloseEventHandler( GCIWindow *win, void* callback_data )
{
	realtime_overview* rto = (realtime_overview*) callback_data;

	realtime_overview_deactivate(rto);
	realtime_overview_hide(rto);

	return;
}

static void OnMasterCameraChanged (Microscope* microscope, void *data)
{
	realtime_overview* rto = (realtime_overview*) data;

    rto->camera = MICROSCOPE_MASTER_CAMERA(rto->ms); 

	realtime_overview_deactivate(rto);

	// Recall init
	realtime_overview_init(rto);

	realtime_overview_activate(rto);
}

/*
static void StageWaitforStopMovingCompleted (XYStage* stage, void *data)
{
	realtime_overview* rto = (realtime_overview*) data;

	// only have the signal called once per click
	stage_signal_wait_for_stop_completion_handler_disconnect (stage, rto->stage_wait_for_stop_completion_id);
  
	if (!gci_camera_is_live_mode(rto->camera))
		gci_camera_snap_image(rto->camera);
	
	//Moves disable the joystick
	stage_set_joystick_on(stage); 
}
*/

static void OnMosaicClicked (GCIWindow *window, const Point image_point, const Point viewer_point, void* data )
{
	int pixelx, pixely, width, height; //, tile_width, tile_height, field_of_view_x, field_of_view_y;
	double startx, starty, microns_per_pixel = 1.0;
	double stage_x, stage_y, roi_width, roi_height;
	Microscope *ms;
	realtime_overview* rto = (realtime_overview*) data;
	XYStage *stage;       
	
	//region_of_interest_get_region(rto->roi, &startx, &starty, &roi_width, &roi_height);
	startx =		rto->region.left;
	starty =		rto->region.top; 
	roi_width =		rto->region.width; 
	roi_height =	rto->region.height;

	ms = microscope_get_microscope();
	stage = microscope_get_stage(ms);       

	//microns_per_pixel = gci_camera_get_true_microns_per_pixel(rto->camera);  

	if(image_point.x < 0 || image_point.y < 0)
		return;
	
	//Start represents middle of top left tile and y is "upside down"
	pixelx = image_point.x;// - rto->mosaic_window->tile_width/2;
	pixely = image_point.y;// + rto->mosaic_window->tile_height/2;
	
	width = FreeImage_GetWidth(rto->mosaic_window->mosaic_image);
	height = FreeImage_GetHeight(rto->mosaic_window->mosaic_image);

	//pixely = height - pixely;

	stage_x = startx + (((float) pixelx / width) * roi_width);
	stage_y = starty + (((float) pixely / height) * roi_height);      

	stage_set_joystick_off(rto->ms->_stage);

	stage_goto_xy_position (stage, stage_x, stage_y);

	if (!gci_camera_is_live_mode(rto->camera))
		gci_camera_snap_image(rto->camera);
	
	//Moves disable the joystick
	stage_set_joystick_on(stage); 
}


int realtime_overview_destroy(realtime_overview* rto)
{
	realtime_overview_deactivate(rto);

	if(rto->mosaic_window != NULL) {
		mosaic_window_destroy(rto->mosaic_window);  
		rto->mosaic_window = NULL;
	}

	if (rto->master_camera_changed_callback_id >= 0)
		microscope_master_camera_changed_handler_disconnect(rto->ms, rto->master_camera_changed_callback_id);

	ui_module_destroy(UIMODULE_CAST(rto));
	
	free(rto);

	return 0;	
}


void realtime_overview_hide(realtime_overview* rto)
{
	ui_module_hide_main_panel(UIMODULE_CAST(rto));
	ui_module_hide_panel(UIMODULE_CAST(rto), rto->option_panel_id);
	mosaic_window_hide(rto->mosaic_window);
}

static void setup_mosaic(realtime_overview* rto)
{
	mosaic_window_set_title(rto->mosaic_window, "Realtime overview mosaic");

	mosaic_window_setup(rto->mosaic_window, rto->type, rto->bpp, rto->region);
	mosaic_window_set_microns_per_pixel(rto->mosaic_window, rto->true_microns_per_pixel); 
	mosaic_window_clear(rto->mosaic_window);   
	mosaic_window_update(rto->mosaic_window);	
}

static void RectSizeChange(Rect *rect, Point pt, double min_factor)
{
	double factor;

	if(min_factor < 0.0)
		return; 

	if (pt.x < rect->left) {   // expand to left
		factor = (double)(rect->left-pt.x)/(double)(rect->width);
		if (factor < min_factor) factor = min_factor;
		rect->left  -= (int)(factor*(double)rect->width);
		rect->width += (int)(factor*(double)rect->width);
	}
	else if (pt.x > rect->left+rect->width) {   // expand to right
		factor = (double)(pt.x-(rect->left+rect->width))/(double)(rect->width);
		if (factor < min_factor) factor = min_factor;
		rect->width += (int)(factor*(double)rect->width);
	}

	if (pt.y < rect->top) {   // expand up
		factor = (double)(rect->top-pt.y)/(double)(rect->height);
		if (factor < min_factor) factor = min_factor;
		rect->top  -= (int)(factor*(double)rect->height);
		rect->height += (int)(factor*(double)rect->height);
	}
	else if (pt.y > rect->top+rect->height) { // expand down
		factor = (double)(pt.y-(rect->top+rect->height))/(double)(rect->height);
		if (factor < min_factor) factor = min_factor;
		rect->height += (int)(factor*(double)rect->height);
	}

/*  // symetrical expansion

	center.x = rect->left + (rect->width / 2);
	center.y = rect->top + (rect->height / 2);

	rect->width *= factor;
	rect->left = center.x - (rect->width / 2);

	rect->height *= factor;
	rect->top = center.y - (rect->height / 2);
*/
}

static int realtime_overview_is_outside_cache_area(realtime_overview* rto, double last_x, double last_y,  double x, double y)
{	// check that x or y have moved significantly fromt the last point to avoid replotting the same area
	if(x < (last_x - rto->cache_size_x) || x > (last_x + rto->cache_size_x))
		return 1;

	if(y < (last_y - rto->cache_size_y) || y > (last_y + rto->cache_size_y))
		return 1;

	return 0;
}


static int CVICALLBACK OnTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			double x, y;
			Point pt, region_pt;

			realtime_overview* rto = (realtime_overview*) callbackData;
			// Not thread safe. So we make sure it is called from a normal timer.
		
			// if mouse down dont update
			if ( GetAsyncKeyState(VK_LBUTTON) & 0x8000 ) {
				return 0;
			}

			if(rto->activate < 1)
				return 0;

			// safety checks
			if (rto == NULL) return 0;
			if (rto->mosaic_window == NULL) return 0;
			if (rto->mosaic_window->window == NULL) return 0;
			if (rto->stage == NULL) return 0;

//			mosaic_window_update(rto->mosaic_window);
		
			// additional check since after destroy (occuring during last call) have had some timer ticks remaining
			if(rto->activate < 1)
				return 0;

			if(stage_get_xy_position(rto->stage, &x, &y) == STAGE_ERROR)
				return 0;

			region_pt.x = (int) x;
			region_pt.y = (int) y;

			if (region_pt.x < rto->region.left)
				region_pt.x = rto->region.left;
			else if (region_pt.x > rto->region.left+rto->region.width)
				region_pt.x = rto->region.left+rto->region.width;

			if (region_pt.y < rto->region.top)
				region_pt.y = rto->region.top;
			else if (region_pt.y > rto->region.top+rto->region.height)
				region_pt.y = rto->region.top+rto->region.height;

			pt = mosaic_window_translate_region_point_to_image_point(rto->mosaic_window, region_pt);

			GCI_ImagingWindow_PlaceCrossHairAtImagePoint(rto->mosaic_window->window, pt);

			break;
		}
	}
	
	return 0;
}


// The client has ask for an image and this is called just before the image is captured.
static void OnCameraEnterGetImage (GciCamera* camera, void *data)
{
	realtime_overview* rto = (realtime_overview*) data;

	if(rto->activate == 0)
		return;

	// Get the stage position. 
	if(stage_get_xy_position(rto->stage, &(rto->stage_x_before_image), &(rto->stage_y_before_image)) == STAGE_ERROR) {
		rto->got_pre_capture_stage_position = 0;
	}

	rto->got_pre_capture_stage_position = 1;
}

// This is called right after the image has been acquired (It has not been transfered to a FIBITMAP yet) 
static void OnCameraPostCapture (GciCamera* camera, void *data)
{
	realtime_overview* rto = (realtime_overview*) data;
	double x, y, w=rto->stage_weight;

	if(rto->activate == 0)
		return;

	// Get the stage position. 
	if(stage_get_xy_position(rto->stage, &x, &y) == STAGE_ERROR) {
		rto->got_post_capture_stage_position = 0;
		return;
	}

	rto->got_post_capture_stage_position = 1;

	// Get the average stage position from before and after the image was acquired
//	rto->stage_x = (x + rto->stage_x_before_image) / 2.0;
//	rto->stage_y = (y + rto->stage_y_before_image) / 2.0;
	// Get the weighted average stage position from before and after the image was acquired
	rto->stage_x = w*x + (1.0-w)*rto->stage_x_before_image;
	rto->stage_y = w*y + (1.0-w)*rto->stage_y_before_image;
}

// put image in overview
static void addImageToOverview (realtime_overview* rto, FIBITMAP* dib)
{
	int ret, tile_width, tile_height, field_of_view_x, field_of_view_y;
	double microns_per_pixel = 1.0, min, max;
	FIBITMAP *tmp = NULL;
	Point pt;

	microns_per_pixel = gci_camera_get_true_microns_per_pixel(rto->camera);  

	tile_width = FreeImage_GetWidth(dib);
	tile_height = FreeImage_GetHeight(dib);
	
	field_of_view_x = (int) (tile_width * microns_per_pixel);
	field_of_view_y = (int) (tile_height * microns_per_pixel);

	pt.x = (int) rto->stage_x - field_of_view_x/2;
	pt.y = (int) rto->stage_y - field_of_view_y/2;

	// If the stage is outside our current roi then we need to increate the size of our
	// roi. Maybe + 10 % each time ?
	if(!RectContainsPoint (rto->region, pt)) {
		int enabled;
		
		// stop the timer whilst doing this
		GetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, &enabled);
		SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, 0);

		RectSizeChange(&(rto->region), pt, 0.2);
		mosaic_window_change_region(rto->mosaic_window, rto->region);
		SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, enabled);
	}

	if(FIA_IsGreyScale(dib))
		tmp = FIA_LinearScaleToStandardType(dib, 0.0, rto->image_max, &min, &max);
	else
		tmp = FreeImage_Clone(dib);

	ret = mosaic_window_add_image(rto->mosaic_window, tmp, pt.x, pt.y);

	if(ret == MOSAIC_ERROR_PASTE_TILE_FAIL) {
		logger_log(UIMODULE_LOGGER(rto), LOGGER_ERROR, "Failed to paste tile to mosaic");
		return;
	}

	FreeImage_Unload(tmp);
}

// This function is called whenever an image is grabbed.
static void OnCameraGetImage (GciCamera* camera, FIBITMAP** dib, void *data)
{
	static double last_x = 1.0e10, last_y = 1.0e10;  // init to some point far away

	realtime_overview* rto = (realtime_overview*) data;

	if(rto->activate == 0)
		return;

	// Not initialised yet ?
	if(rto->fov_x == 0.0 || rto->fov_y == 0.0) {		
		return;
	}

	if(rto->got_pre_capture_stage_position == 0 || rto->got_post_capture_stage_position == 0)
		return;

	//printf("stage x stage y (%.3f,%.3f)\n", rto->stage_x, rto->stage_y);

	if(!realtime_overview_is_outside_cache_area(rto, last_x, last_y, rto->stage_x, rto->stage_y)) {
		last_x = rto->stage_x, last_y = rto->stage_y;
		return;
	}

	addImageToOverview (rto, *dib);

	last_x = rto->stage_x, last_y = rto->stage_y;

	if(SendMessageTimeout(rto->window_hwnd, REALTIME_OVERVIEW_MOSAIC_UPDATE, 0, 0,
		SMTO_ABORTIFHUNG, 1000, NULL) <= 0) {
		return ;
	}
}

void realtime_overview_activate(realtime_overview* rto)
{
	if(rto->activate == 1)
		return;

	rto->cam_enter_callback_id = gci_camera_signal_enter_get_image_handler_connect (rto->camera, OnCameraEnterGetImage, rto);
	rto->cam_exit_callback_id = gci_camera_signal_exit_get_image_handler_connect(rto->camera, OnCameraGetImage, rto);
	rto->cam_post_capture_callback_id = gci_camera_signal_post_capture_handler_connect (rto->camera, OnCameraPostCapture, rto);

	rto->activate = 1;
	SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, 1);

	// Set the activate toggle button to reflect we are no longer active.
	SetCtrlVal(rto->option_panel_id, rto->enable_button_id, 1);

	if(!rto->mosaic_click_callback_id)
		rto->mosaic_click_callback_id = GCI_ImagingWindow_SetMouseUpHandler (rto->mosaic_window->window, OnMosaicClicked , rto);    
}

void realtime_overview_activate_only_stage(realtime_overview* rto)
{
	if(rto->activate == 2)
		return;

	gci_camera_signal_enter_get_image_handler_disconnect (rto->camera, rto->cam_enter_callback_id);
	gci_camera_signal_exit_get_image_handler_disconnect  (rto->camera, rto->cam_exit_callback_id);
	gci_camera_signal_post_capture_handler_disconnect  (rto->camera, rto->cam_post_capture_callback_id);

	rto->activate = 2;
	SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, 1);

	// Set the activate toggle button to reflect we are no longer active.
	SetCtrlVal(rto->option_panel_id, rto->enable_button_id, 0);

	if(!rto->mosaic_click_callback_id)
		rto->mosaic_click_callback_id = GCI_ImagingWindow_SetMouseUpHandler (rto->mosaic_window->window, OnMosaicClicked , rto);    
}

// On deactivate stop the timer and do nother on camera signals
void realtime_overview_deactivate(realtime_overview* rto)
{
	SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, 0);
	ProcessSystemEvents();

	if(rto->activate == 0)
		return;

	rto->activate = 0;
            
	if(rto->mosaic_click_callback_id){
		GCI_ImagingWindow_DisconnectMouseUpHandler(rto->mosaic_window->window, rto->mosaic_click_callback_id);
		rto->mosaic_click_callback_id = 0;
	}

	gci_camera_signal_enter_get_image_handler_disconnect (rto->camera, rto->cam_enter_callback_id);
	gci_camera_signal_exit_get_image_handler_disconnect  (rto->camera, rto->cam_exit_callback_id);
	gci_camera_signal_post_capture_handler_disconnect  (rto->camera, rto->cam_post_capture_callback_id);

	// Set the activate toggle button to reflect we are no longer active.
	SetCtrlVal(rto->option_panel_id, rto->enable_button_id, 0);

	ui_module_hide_main_panel(UIMODULE_CAST(rto));
}

static void AdjustOptionPanel (realtime_overview* rto, int left, int top, int right, int bottom)
{
	int handle;

	SetPanelAttribute(rto->option_panel_id, ATTR_LEFT, right + 10);
	SetPanelAttribute(rto->option_panel_id, ATTR_TOP, top + 50);

	GetPanelAttribute(rto->option_panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &handle);
	BringWindowToTop((HWND)handle);
}

void realtime_overview_display(realtime_overview* rto)
{
	HWND window_handle;
	RECT window_rect;

	SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, 1);

	ui_module_display_main_panel(UIMODULE_CAST(rto));
	mosaic_window_show(rto->mosaic_window);

	GCI_ImagingWindow_HidePaletteBar(rto->mosaic_window->window);

	window_handle = GCI_ImagingWindow_GetWindowHandle(rto->mosaic_window->window);
	GetWindowRect(window_handle, &window_rect);

	AdjustOptionPanel (rto, window_rect.left, window_rect.top, window_rect.right, window_rect.bottom);
	DisplayPanel(rto->option_panel_id);
}


int realtime_overview_init(realtime_overview* rto)
{
	FIBITMAP *dib = NULL;
	FIBITMAP *tmp = NULL;
	optical_calibration* cal;

	// get an image from the camera to start
	dib = GCI_ImagingWindow_GetOriginalFIB(gci_camera_get_imagewindow(rto->camera)); 

	if(dib == NULL) {
		// The window may not have an image so we snap one and try again
		gci_camera_snap_image(rto->camera);
		dib = GCI_ImagingWindow_GetOriginalFIB(gci_camera_get_imagewindow(rto->camera)); 
	}

	if(FreeImage_GetBPP(dib) >= 24 && FreeImage_GetImageType(dib) == FIT_BITMAP)
		rto->bpp = 24;
	else
		rto->bpp = 8;

	rto->type = FIT_BITMAP;

	// Get the fov
	gci_camera_get_size(rto->camera, &(rto->image_width), &(rto->image_height));
	cal = microscope_get_optical_calibration(rto->ms);   
	rto->true_microns_per_pixel = gci_camera_get_true_microns_per_pixel(rto->camera);           

	rto->fov_x = rto->true_microns_per_pixel * rto->image_width;   
	rto->fov_y = rto->true_microns_per_pixel * rto->image_height; 

	// Cache size is 2 % of image
	rto->cache_size_x = rto->fov_x * 0.02;
	rto->cache_size_y = rto->fov_y * 0.02;

	// init an area of the stage based on current position
	stage_get_xy_position(rto->stage, &rto->stage_x, &rto->stage_y);
	rto->region.left = (int)rto->stage_x-2500;
	rto->region.top = (int)rto->stage_y-2500;
	rto->region.width = 5000;
	rto->region.height = 5000;

	setup_mosaic(rto);

	GCI_ImagingWindow_ActivateCrossHairTool(rto->mosaic_window->window);

	// Put the current image on the mosaic at start
	addImageToOverview (rto, dib);

	if(SendMessageTimeout(rto->window_hwnd, REALTIME_OVERVIEW_MOSAIC_UPDATE, 0, 0,
		SMTO_ABORTIFHUNG, 1000, NULL) <= 0) {

		FreeImage_Unload(dib);
		return REALTIME_OVERVIEW_ERROR;
	}

	FreeImage_Unload(dib);

	return REALTIME_OVERVIEW_SUCCESS;
}

void realtime_overview_update_microns_per_pixel(realtime_overview* rto)
{
	rto->true_microns_per_pixel = gci_camera_get_true_microns_per_pixel(rto->camera);  
	
	mosaic_window_set_microns_per_pixel(rto->mosaic_window,rto->true_microns_per_pixel);
}

void realtime_overview_set_no_display(realtime_overview* rto, int no_display)
{
	if(rto->activate != 1)
		return;

	if (no_display){
		SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, 0);
	}
	else{
		SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, 1);
	}
}

void realtime_overview_force_display_update(realtime_overview* rto)
{	
	if(rto->activate != 1)
		return;

	// safety checks
	if (rto == NULL) return;
	if (rto->mosaic_window == NULL) return;
	if (rto->mosaic_window->window == NULL) return;

	mosaic_window_update(rto->mosaic_window);
}

void realtime_overview_set_image_max_scale_value (realtime_overview* rto, double max)
{
	rto->image_max = max;
}

static int CVICALLBACK OnLoadClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			realtime_overview* rto = (realtime_overview*) callbackData;
		
			char directory[GCI_MAX_PATHNAME_LEN] = "";
			char fname[GCI_MAX_PATHNAME_LEN] = "";

			microscope_get_user_data_directory(rto->ms, directory);

			if (LessLameFileSelectPopup (rto->option_panel_id, directory,
				"*.ics", "*.ics", "Load Image", VAL_OK_BUTTON, 0, 0, 1, 1, fname) <= 0) {
				return -1;
			}

			if(mosaic_window_setup_from_ics_file(rto->mosaic_window, fname) < 0)
				return -1;

			// Get the new region
			rto->region = mosaic_window_get_region(rto->mosaic_window);

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnResetClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			realtime_overview* rto = (realtime_overview*) callbackData;

			setup_mosaic(rto);

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnEnabledToggled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;
			realtime_overview* rto = (realtime_overview*) callbackData;

			GetCtrlVal(panel, control, &val);

			if(val)
				realtime_overview_activate(rto);
			else
				realtime_overview_deactivate(rto);

			break;
		}
	}
	
	return 0;
}

static void OnWindowResizing (IcsViewerWindow *window, int left, int top, int right, int bottom, void* data)
{
	realtime_overview* rto = (realtime_overview*) data;

	AdjustOptionPanel (rto, left, top, right, bottom);
}

static void OnWindowShowOrHide (IcsViewerWindow *window, int show, void* data)
{
	int handle;
	RECT rect;
	HWND hwnd;

	realtime_overview* rto = (realtime_overview*) data;

	hwnd = GCI_ImagingWindow_GetWindowHandle(rto->mosaic_window->window);
	
	GetWindowRect(hwnd, &rect);

	SetPanelAttribute(rto->option_panel_id, ATTR_LEFT, rect.right + 10);
	SetPanelAttribute(rto->option_panel_id, ATTR_TOP, rect.top + 50);

	GetPanelAttribute(rto->option_panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &handle);
	BringWindowToTop((HWND)handle);
}

static void OnWindowMinimised (IcsViewerWindow *window, void* data)
{
	realtime_overview* rto = (realtime_overview*) data;

	// We don't minimise the attached panel will just hide it then we don't get a button
	// hovering above the startbar
	SetPanelAttribute(rto->option_panel_id, ATTR_VISIBLE, 0);
}

static void OnWindowRestored (IcsViewerWindow *window, void* data)
{
	realtime_overview* rto = (realtime_overview*) data;

	SetPanelAttribute(rto->option_panel_id, ATTR_VISIBLE, 1);
}

static create_realtime_overview_option_window(realtime_overview* rto)
{
	int label_id, load_button_id, reset_button_id;
	RECT rect;
	HWND hwnd;

	rto->option_panel_id = NewPanel(0, "Realtime overview options", 0, 0, 300, 80);

	SetPanelAttribute(rto->option_panel_id, ATTR_FRAME_STYLE, VAL_OUTLINED_FRAME);
	SetPanelAttribute(rto->option_panel_id, ATTR_TITLEBAR_VISIBLE, 0);
	SetPanelAttribute(rto->option_panel_id, ATTR_SIZABLE, 0);

	hwnd = GCI_ImagingWindow_GetWindowHandle(rto->mosaic_window->window);
	
	GetWindowRect(hwnd, &rect);

	SetPanelAttribute(rto->option_panel_id, ATTR_LEFT, rect.right + 10);
	SetPanelAttribute(rto->option_panel_id, ATTR_TOP, rect.top + 50);

	GCI_ImagingWindow_SetWindowResizingHandler(rto->mosaic_window->window,
		OnWindowResizing, rto);

	GCI_ImagingWindow_SetWindowMovingHandler(rto->mosaic_window->window,
		OnWindowResizing, rto);

	//GCI_ImagingWindow_SetWindowShowOrHideHandler(rto->mosaic_window->window, OnWindowShowOrHide, rto);

	//GCI_ImagingWindow_SetWindowMaximisedHandler(rto->mosaic_window->window, OnWindowMaximised, rto);

	GCI_ImagingWindow_SetWindowRestoredHandler(rto->mosaic_window->window, OnWindowRestored, rto);

	

	label_id = NewCtrl(rto->option_panel_id, CTRL_TEXT_MSG, "", 10, 10);
	SetCtrlAttribute(rto->option_panel_id, label_id, ATTR_TEXT_BOLD, 1);
	SetCtrlVal(rto->option_panel_id, label_id, "Options");
	
	load_button_id = NewCtrl(rto->option_panel_id, CTRL_SQUARE_COMMAND_BUTTON, "Load", 40, 5);
	SetCtrlAttribute(rto->option_panel_id, load_button_id, ATTR_WIDTH, 70);
	InstallCtrlCallback(rto->option_panel_id, load_button_id, OnLoadClicked, rto);

	reset_button_id = NewCtrl(rto->option_panel_id, CTRL_SQUARE_COMMAND_BUTTON, "Reset", 80, 5);
	SetCtrlAttribute(rto->option_panel_id, reset_button_id, ATTR_WIDTH, 70);
	InstallCtrlCallback(rto->option_panel_id, reset_button_id, OnResetClicked, rto);

	rto->enable_button_id = NewCtrl(rto->option_panel_id, CTRL_SQUARE_TEXT_BUTTON, "", 120, 5);
	SetCtrlAttribute(rto->option_panel_id, rto->enable_button_id, ATTR_WIDTH, 70);
	SetCtrlAttribute(rto->option_panel_id, rto->enable_button_id, ATTR_ON_TEXT, "Disable");
	SetCtrlAttribute(rto->option_panel_id, rto->enable_button_id, ATTR_OFF_TEXT, "Enable");
	SetCtrlVal(rto->option_panel_id, rto->enable_button_id, 1);
	InstallCtrlCallback(rto->option_panel_id, rto->enable_button_id, OnEnabledToggled, rto);
}

static int RealtimeOverView_UpdateMosaic(realtime_overview* rto)
{
	double x, y;
	Point pt, region_pt;

	// Not thread safe. So we make sure it is called from a normal timer.

	// if mouse down dont update
	if ( GetAsyncKeyState(VK_LBUTTON) & 0x8000 ) {
		return 0;
	}

	if(rto->activate < 1)
		return 0;

	// safety checks
	if (rto == NULL) return 0;
	if (rto->mosaic_window == NULL) return 0;
	if (rto->mosaic_window->window == NULL) return 0;
	if (rto->stage == NULL) return 0;

	mosaic_window_update(rto->mosaic_window);

	// additional check since after destroy (occuring during last call) have had some timer ticks remaining
	if(rto->activate < 1)
		return 0;

	if(stage_get_xy_position(rto->stage, &x, &y) == STAGE_ERROR)
		return 0;

	region_pt.x = (int) x;
	region_pt.y = (int) y;

	if (region_pt.x < rto->region.left)
		region_pt.x = rto->region.left;
	else if (region_pt.x > rto->region.left+rto->region.width)
		region_pt.x = rto->region.left+rto->region.width;

	if (region_pt.y < rto->region.top)
		region_pt.y = rto->region.top;
	else if (region_pt.y > rto->region.top+rto->region.height)
		region_pt.y = rto->region.top+rto->region.height;

	pt = mosaic_window_translate_region_point_to_image_point(rto->mosaic_window, region_pt);

	GCI_ImagingWindow_PlaceCrossHairAtImagePoint(rto->mosaic_window->window, pt);

	return 0;
}

static LRESULT CALLBACK RealtimeOverviewWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	realtime_overview* rto = (realtime_overview*) data;

	switch(message) {
			
		case REALTIME_OVERVIEW_MOSAIC_UPDATE:
		{
			RealtimeOverView_UpdateMosaic(rto);
			return 0;
		}

      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) ui_module_get_original_wndproc_ptr(UIMODULE_CAST(rto)),
							hwnd, message, wParam, lParam);
}

void realtime_overview_set_close_handler(realtime_overview* rto, void (*close_handler) (IcsViewerWindow *window, void *data), void *callback_data)
{
	GCI_ImagingWindow_SetCloseEventHandler(rto->mosaic_window->window,
		close_handler, callback_data);
}

realtime_overview* realtime_overview_new(void)
{
	int window_handle;
	realtime_overview* rto = (realtime_overview*) malloc(sizeof(realtime_overview));

	memset(rto, 0, sizeof(realtime_overview));

	ui_module_constructor(UIMODULE_CAST(rto), "Realtime Overview");
  	
	rto->mosaic_click_callback_id = 0;

	rto->activate = 0;
	rto->ms = microscope_get_microscope();
	rto->camera = microscope_get_camera(rto->ms);
	rto->stage = microscope_get_stage(rto->ms);

	rto->master_camera_changed_callback_id = microscope_master_camera_changed_handler_connect(rto->ms, OnMasterCameraChanged, rto);

	rto->stage_weight = 0.5;

	rto->mosaic_window = mosaic_window_new("RealtimeOverview", 100, 100, 500, 500);

	create_realtime_overview_option_window(rto);

	// Hidden Panel solely for a non async timer.
	rto->panel_id = NewPanel(0, "", 0, 0, 100, 100);
	rto->timer = NewCtrl(rto->panel_id, CTRL_TIMER, "", 0, 0);

	GetPanelAttribute (rto->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &(window_handle)); 
	rto->window_hwnd = (HWND) window_handle; 

	ui_module_set_window_proc(UIMODULE_CAST(rto), rto->panel_id, (LONG_PTR) RealtimeOverviewWndProc);   

	SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_ENABLED, 0);
	InstallCtrlCallback (rto->panel_id, rto->timer, OnTimerTick, rto);		
	SetCtrlAttribute(rto->panel_id, rto->timer, ATTR_INTERVAL, 0.5);  

	return rto;	
}  
