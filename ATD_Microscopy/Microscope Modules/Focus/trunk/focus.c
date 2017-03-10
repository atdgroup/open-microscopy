#include "HardWareTypes.h"      

#include "camera\gci_camera.h"
#include "focus.h"
#include "gci_utils.h"
#include "focus_ui.h"

#include "FreeImageAlgorithms_FFT.h"
#include "FreeImageAlgorithms_Drawing.h"
#include "FreeImageAlgorithms_Palettes.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Arithmetic.h"

#include "BasicWin32Window.h"

#include "microscope.h"
#include "profile.h"

#include <analysis.h>
#include <toolbox.h>

#define REDUCE 8

static void PositionFocusWindow(focus* foc)
{
	int viewer_width, focus_window_width, left, handle;
	RECT rect;
	HWND hwnd = GCI_ImagingWindow_GetWindowHandle(CAMERA_WINDOW(foc->camera));
	
	GetWindowRect(hwnd, &rect);
	viewer_width = rect.right - rect.left;
	GetPanelAttribute(foc->_panel_id, ATTR_WIDTH, &focus_window_width);

	left = (viewer_width - focus_window_width) / 2;
	SetPanelAttribute(foc->_panel_id, ATTR_LEFT, rect.left + left);
	SetPanelAttribute(foc->_panel_id, ATTR_TOP, rect.bottom + 5);

	// Always bring to top with the image window
	GetPanelAttribute(foc->_panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &handle);
	BringWindowToTop((HWND)handle);
}

static void OnWindowResizing (IcsViewerWindow *window, int left, int top, int right, int bottom, void* data)
{
	focus* foc = (focus*) data;

	PositionFocusWindow(foc);
}

static void OnWindowMinimised (IcsViewerWindow *window, void* data)
{
	focus* foc = (focus*) data;

	// We don't minimise the attached panel will just hide it then we don't get a button
	// hovering above the startbar
	SetPanelAttribute(foc->_panel_id, ATTR_VISIBLE, 0);
}

static void OnWindowRestored (IcsViewerWindow *window, void* data)
{
	focus* foc = (focus*) data;

	SetPanelAttribute(foc->_panel_id, ATTR_VISIBLE, 1);
}

static void AttachCameraWindowResizingEvents(focus *foc)
{
	if(foc->camera == NULL)
		return;

	if(foc->_resizing_handler_id != 0)
		GCI_ImagingWindow_DisconnectWindowResizingHandler(CAMERA_WINDOW(foc->camera), foc->_resizing_handler_id);

	if(foc->_moving_handler_id != 0)
		GCI_ImagingWindow_DisconnectWindowMovingHandler(CAMERA_WINDOW(foc->camera), foc->_moving_handler_id);

	if(foc->_minimise_handler_id != 0)
		GCI_ImagingWindow_DisconnectWindowMinimisedHandler(CAMERA_WINDOW(foc->camera), foc->_minimise_handler_id);

	if(foc->_restored_handler_id != 0)
		GCI_ImagingWindow_DisconnectWindowRestoredHandler(CAMERA_WINDOW(foc->camera), foc->_restored_handler_id);

	foc->_resizing_handler_id = GCI_ImagingWindow_SetWindowResizingHandler(CAMERA_WINDOW(foc->camera), OnWindowResizing, foc);

	foc->_moving_handler_id = GCI_ImagingWindow_SetWindowMovingHandler(CAMERA_WINDOW(foc->camera),
		OnWindowResizing, foc);

	foc->_minimise_handler_id = GCI_ImagingWindow_SetWindowMinimisedHandler(CAMERA_WINDOW(foc->camera),
		OnWindowMinimised, foc);

	foc->_restored_handler_id = GCI_ImagingWindow_SetWindowRestoredHandler(CAMERA_WINDOW(foc->camera), OnWindowRestored, foc);
}

void focus_set_camera (focus* foc, GciCamera *camera)
{
    foc->camera = camera; 

	// Hide the focus panel on the existing window.
	// This is just as it looks nicer when it gets moved to the new camera window.
	//focus_hide_all_panels(foc);

	AttachCameraWindowResizingEvents(foc);
	PositionFocusWindow(foc);
}

focus* focus_new(GciCamera *camera)
{
	Microscope *ms = microscope_get_microscope();
	focus *foc = (focus *) malloc (sizeof(focus));		
	
	foc->camera = camera;
	foc->_focus_on = 0;  
	foc->full_mask = NULL;
	foc->half_mask = NULL;
	
	ui_module_constructor(UIMODULE_CAST(foc), "Focus");  
	
	foc->_panel_id = ui_module_add_panel(UIMODULE_CAST(foc), "focus_ui.uir", FOCUSPNL, 1);   
	
	if ( InstallCtrlCallback (foc->_panel_id, FOCUSPNL_FOCUS_ON_OFF, cbFocusOnOff, foc) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (foc->_panel_id, FOCUSPNL_MAX, cbFocusMax, foc) < 0)
		return NULL;
					 
	if ( InstallCtrlCallback (foc->_panel_id, FOCUSPNL_CROP, cbFocusCustom, foc) < 0)
		return NULL;

	if ( InstallCtrlCallback (foc->_panel_id, FOCUSPNL_RESAMPLE, cbFocusCustom, foc) < 0)
		return NULL;
	
	AttachCameraWindowResizingEvents(foc);

	// Default to on
	focus_set_on(foc); 
	
	return foc;
}

void focus_show_all_panels(focus* foc)
{
	PositionFocusWindow(foc);
	ui_module_display_main_panel_without_registry(UIMODULE_CAST(foc)); 
}

void focus_hide_all_panels(focus* foc)
{
	ui_module_hide_all_panels(UIMODULE_CAST(foc));   
}

void focus_destroy(focus* foc)
{
	if(foc->full_mask != NULL) {
		FreeImage_Unload(foc->full_mask);
		foc->full_mask = NULL;
	}
		
	if(foc->half_mask != NULL) {     
		FreeImage_Unload(foc->half_mask);
		foc->half_mask = NULL;	
	}	
	
	ui_module_destroy(UIMODULE_CAST(foc));
	
	free(foc);
}

static double fft_focus_measure(focus* foc, FIBITMAP* src)
{   
	double fullSum, highfSum;
	FIBITMAP* complex_fft=NULL, *fft=NULL, *image=NULL;
	
	FREE_IMAGE_TYPE type;
	int bpp;
	
	PROFILE_START("fft_focus_measure");
	
	PROFILE_START("FreeImageAlgorithms_FFT");  
	
	if(src == NULL) {
		logger_log(UIMODULE_LOGGER(foc), LOGGER_ERROR, "image passed in to fft_focus_measure is NULL");
		return 0.0;
	}

	type = FreeImage_GetImageType(src);
	bpp = FreeImage_GetBPP(src);
	
	if(type == FIT_BITMAP && bpp > 16) 
	{
		// We have a colour image. We will do the FFT on one channel only
//		image = FreeImage_GetChannel(src, FICC_RED);
		// Choose a 'luminance' image
		image = FreeImage_ConvertTo8Bits(src);
	}
	else 
	{
		image = FreeImage_Clone(src);	
	}
	
	complex_fft = FIA_FFT(image);
	
	if(complex_fft == NULL) {
		logger_log(UIMODULE_LOGGER(foc), LOGGER_ERROR, "FFT image calculated in fft_focus_measure is NULL");
		return 0.0;
	}

//	FreeImage_Unload(image);
//	image = NULL;
	
	PROFILE_STOP("FreeImageAlgorithms_FFT");
	
	PROFILE_START("FreeImageAlgorithms_ConvertComplexImageToAbsoluteValuedSquared"); 
	
//	fft = FIA_ConvertComplexImageToAbsoluteValuedSquared(complex_fft);
	fft = FIA_ConvertComplexImageToAbsoluteValued(complex_fft);  // PRB Dec 2010, must not leave the AbsValue squared as this does not preserve the total energy in the image
	
	if(fft == NULL) {
		logger_log(UIMODULE_LOGGER(foc), LOGGER_ERROR, "Absolute valued image calculated in fft_focus_measure is NULL");
		return 0.0;
	}

	FIA_InPlaceShiftImageEdgeToCenter(&fft);   

//	BasicWin32Window("fft", 100, 100, 300, 300, fft);  

	PROFILE_STOP("FreeImageAlgorithms_ConvertComplexImageToAbsoluteValuedSquared"); 
	
	FreeImage_Unload(complex_fft);
	
	PROFILE_START("FreeImageAlgorithms_SumOfAllPixels");  
	
	if(foc->full_mask == NULL) {
		logger_log(UIMODULE_LOGGER(foc), LOGGER_ERROR, "Full mask image calculated in fft_focus_measure is NULL");
		return 0.0;
	}

	if(foc->half_mask == NULL) {
		logger_log(UIMODULE_LOGGER(foc), LOGGER_ERROR, "Half mask image calculated in fft_focus_measure is NULL");
		return 0.0;
	}

//	FIA_SumOfAllPixels(fft, foc->full_mask, &fullSum);  // integrate over full mask  
	FIA_SumOfAllPixels(image, foc->full_mask, &fullSum);  // integrate image over full mask - more reliable than over fft (does kissfft not preserve total energy?)
	FIA_SumOfAllPixels(fft, foc->half_mask, &highfSum);   // integrate over high freq mask

	FreeImage_Unload(image);
	image = NULL;

	PROFILE_STOP("FreeImageAlgorithms_SumOfAllPixels");  
	
	FreeImage_Unload(fft);

	PROFILE_STOP("fft_focus_measure");  
	
//	return ((double)(100000.0*highfSum/fullSum));
	return ((double)(1000.0*highfSum/fullSum));
//	return ((double)(highfSum/fullSum/1e3));
}


static void make_masks(focus* foc, FIBITMAP* image)
{
	int rad=16;	  //8 is good for x10 need to try other objectives
	int radH, radV;
	double hfLimit = 1.0;  // PRB 14.9.00, take hf from the fft to this fraction of the width and height (NB the image may have been resampled). 
 	double lfLimit = 0.03;  // PRB Dec 2010, take lf from the fft to this fraction of the width and height (NB the image may have been resampled). 
	FIARECT half_rect;
	
	int width = FreeImage_GetWidth(image);
	int height = FreeImage_GetHeight(image);
	
	int left = (int) ((1-hfLimit) * width/2);
	int top = (int) ((1-hfLimit) * height/2);
	
	FIARECT full_rect = MakeFIARect (left, top, (int) (left + hfLimit*width - 1), 
												(int) (top - 1 + hfLimit*height));

	PROFILE_START("make_masks");   
	
	if(foc->full_mask != NULL) {
		FreeImage_Unload(foc->full_mask);
		foc->full_mask = NULL;
	}
		
	if(foc->half_mask != NULL) {     
		FreeImage_Unload(foc->half_mask);
		foc->half_mask = NULL;	
	}	
	
	radH = (int) (lfLimit * width);
	radV = (int) (lfLimit * height);

	left = width/2-radH;
	top = height/2-radV;
	
	half_rect = MakeFIARect (left, top, left + 2*radH, top + 2*radV);

	foc->full_mask = FreeImage_Allocate(width, height, 8, 0, 0, 0);
	foc->half_mask = FreeImage_Allocate(width, height, 8, 0, 0, 0);
	
	FIA_DrawSolidGreyscaleEllipse (foc->full_mask, full_rect, 1, 0);
	FIA_DrawSolidGreyscaleEllipse (foc->half_mask, full_rect, 1, 0); 

//	FIA_DrawSolidGreyscaleRect (foc->full_mask, full_rect, 1);
//	FIA_DrawSolidGreyscaleRect (foc->half_mask, full_rect, 1);

	FIA_DrawSolidGreyscaleEllipse (foc->half_mask, half_rect, 0, 0); 

//	BasicWin32Window("full mask", 100, 100, 300, 300, foc->full_mask);  
//	BasicWin32Window("half mask", 100, 100, 300, 300, foc->half_mask);  

	PROFILE_STOP("make_masks"); 
}

void focus_get_image_focus(focus* foc, FIBITMAP *dib, double* fval) 
{
	int width, height, reduce = 1, crop=1, sample, left, top, right, bottom;
	BinningMode binning;
	FIBITMAP *rescaled_dib = dib, *cropped_dib = dib, *tmp=NULL, *tmp2=NULL;
	
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	
//	while (height/reduce > 300)
//		reduce *= 2;

	PROFILE_START("focus_get_image_focus");  
	
	if(gci_camera_supports_focal_indicator_settings(foc->camera))
	{
		FocusSettings settings; 
		// Get the prefered focus settings from the camera
		settings = gci_camera_get_focal_indicator_settings(foc->camera);

		SetCtrlVal(foc->_panel_id, FOCUSPNL_SAMPLE, settings.sample_type);
		SetCtrlVal(foc->_panel_id, FOCUSPNL_CROP, settings.crop_type);
		SetCtrlVal(foc->_panel_id, FOCUSPNL_RESAMPLE, settings.resample_type);
	}
	else {

		GetCtrlVal(foc->_panel_id, FOCUSPNL_SAMPLE, &sample);

		if (sample>0)  // not custom
		{
			binning = gci_camera_get_binning_mode (foc->camera);
			switch (binning)
			{
				case NO_BINNING:
			
					if (sample==1)  //cells
					{
						SetCtrlVal(foc->_panel_id, FOCUSPNL_CROP, 1);
						SetCtrlVal(foc->_panel_id, FOCUSPNL_RESAMPLE, 4);
					}
					else if (sample==2)  // tissue
					{
						SetCtrlVal(foc->_panel_id, FOCUSPNL_CROP, 2);
						SetCtrlVal(foc->_panel_id, FOCUSPNL_RESAMPLE, 2);
					}
					
					break;

				case BINNING2X2:
			
					if (sample==1)  //cells
					{
						SetCtrlVal(foc->_panel_id, FOCUSPNL_CROP, 1);
						SetCtrlVal(foc->_panel_id, FOCUSPNL_RESAMPLE, 2);
					}
					else if (sample==2)  // tissue
					{
						SetCtrlVal(foc->_panel_id, FOCUSPNL_CROP, 2);
						SetCtrlVal(foc->_panel_id, FOCUSPNL_RESAMPLE, 1);
					}
					
					break;

				default:		   // any higher binning
			
					if (sample==1)  //cells
					{
						SetCtrlVal(foc->_panel_id, FOCUSPNL_CROP, 1);
						SetCtrlVal(foc->_panel_id, FOCUSPNL_RESAMPLE, 1);
					}
					else if (sample==2)  // tissue
					{
						SetCtrlVal(foc->_panel_id, FOCUSPNL_CROP, 1);
						SetCtrlVal(foc->_panel_id, FOCUSPNL_RESAMPLE, 1);
					}

					break;
				
			}
		}	
	}

	GetCtrlVal(foc->_panel_id, FOCUSPNL_CROP, &crop);
	GetCtrlVal(foc->_panel_id, FOCUSPNL_RESAMPLE, &reduce);
	
	PROFILE_START("focus_get_image_focus - crop");  
	
	if (crop>1)
	{
		left   = width/2  - width/2/crop;
		right  = width/2  + width/2/crop;
		top    = height/2 - height/2/crop;
		bottom = height/2 + height/2/crop;
		
		cropped_dib = FreeImage_Copy(dib, left, top, right, bottom);
		
		width = FreeImage_GetWidth(cropped_dib);
		height = FreeImage_GetHeight(cropped_dib);
		
		rescaled_dib = cropped_dib;
	}
	
	PROFILE_STOP("focus_get_image_focus - crop");   
	
	PROFILE_START("focus_get_image_focus - reduce");   
	
	if (reduce>1)
	{
		//rescaled_dib = FreeImage_Rescale(cropped_dib, width/reduce, height/reduce, FILTER_BOX);
		
		if(reduce == 2) {
			rescaled_dib = FIA_RescaleToHalf(cropped_dib);
		}
		else if(reduce == 4) {
			tmp = FIA_RescaleToHalf(cropped_dib);
			rescaled_dib = FIA_RescaleToHalf(tmp);
			FreeImage_Unload(tmp);
		}
		else if(reduce == 8) {
			tmp = FIA_RescaleToHalf(cropped_dib);
			tmp2 = FIA_RescaleToHalf(tmp);
			rescaled_dib = FIA_RescaleToHalf(tmp2);      
			FreeImage_Unload(tmp);
			FreeImage_Unload(tmp2); 
		}
		
		width = FreeImage_GetWidth(rescaled_dib);
		height = FreeImage_GetHeight(rescaled_dib);
	}
	
	PROFILE_STOP("focus_get_image_focus - reduce");    
	
	// Size of image to update has changed so we create new masks
	if(width != foc->prev_rescaled_width || height != foc->prev_rescaled_height) {
		make_masks(foc, rescaled_dib); 
	}
	
	foc->prev_rescaled_width = width;
	foc->prev_rescaled_height = height;
	
	*fval = fft_focus_measure(foc, rescaled_dib);
	
	// We have cropped the image so unload the cropped image.
	if(crop>1) {
		FreeImage_Unload(cropped_dib);
		cropped_dib = 0;
	}
	
	// We have rescaled the image so unload the rescaled image.
	if(reduce>1) {								   // ******************
		FreeImage_Unload(rescaled_dib);
		rescaled_dib = 0;
	}
	
	PROFILE_STOP("focus_get_image_focus"); 
}


static void set_scale_max(focus* foc, double f)
{
	int idx;
	int pnl = foc->_panel_id;
	double max;
	
	//Autorange upwards only. BV prefers this
	GetCtrlVal(pnl, FOCUSPNL_MAX, &max);
	
	if (max < f) {
		if (max < 50000.0) {
			GetCtrlAttribute (pnl, FOCUSPNL_MAX, ATTR_CTRL_INDEX, &idx);
			SetCtrlAttribute (pnl, FOCUSPNL_MAX, ATTR_CTRL_INDEX, idx+1);
			GetCtrlVal(pnl, FOCUSPNL_MAX, &max);
			SetCtrlAttribute (pnl, FOCUSPNL_FOCUS, ATTR_MAX_VALUE, max);
		}
	}
}


void focus_set_on(focus* foc)
{
	foc->_focus_on = 1;   
	
    SetCtrlVal(foc->_panel_id, FOCUSPNL_FOCUS_ON_OFF, 1); 
}

void focus_set_off(focus* foc)
{
	foc->_focus_on = 0;
	
	SetCtrlVal(foc->_panel_id, FOCUSPNL_FOCUS_ON_OFF, 0);
	SetCtrlVal(foc->_panel_id, FOCUSPNL_FOCUS, 0.0);   
}

int focus_is_on(focus* foc)
{
	return foc->_focus_on;
}

void focus_update_focus_ui(focus* foc, FIBITMAP *dib, unsigned char average_number_of_frames)
{
	double avf = 0.0;
	static unsigned char idx=0;
	static double focus_vals[255]={0};
	
	if(dib == NULL) {
		logger_log(UIMODULE_LOGGER(foc), LOGGER_ERROR, "image passed in to focus_update_focus_ui is NULL");
		return;
	}
	
	// If switch not on quit
	if (!focus_is_on(foc))
		return;
	
	PROFILE_START("focus_update_focus_ui"); 
	
	if(average_number_of_frames <= 0)
		average_number_of_frames = 1;
	
	focus_get_image_focus(foc, dib, &focus_vals[idx]);  
	
	if(average_number_of_frames > 1) {
		
		// We are calculating the average focus for previous frames.
		Mean (focus_vals, average_number_of_frames + 1, &avf);
		
		if (idx >= average_number_of_frames)
			idx = 0;
		
		idx++;	
	}
	else {
		
		avf = focus_vals[0];
	}
		
	SetCtrlVal(foc->_panel_id, FOCUSPNL_FOCUS, avf);   
	
	set_scale_max(foc, avf);
	
	PROFILE_STOP("focus_update_focus_ui"); 
}


//////////////////////////////////////////////////////////////////////

int CVICALLBACK cbFocusOnOff (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			int f;
			focus *foc = FOCUS_MODULE_CAST(callbackData);  
			
			GetCtrlVal(foc->_panel_id, FOCUSPNL_FOCUS_ON_OFF, &f);
			
			if (f == 0)
				focus_set_off(foc); 
			else
				focus_set_on(foc);   
			
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK cbFocusMax (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			double max;
			
			focus *foc = FOCUS_MODULE_CAST(callbackData);
			
			GetCtrlVal(foc->_panel_id, FOCUSPNL_MAX, &max);
			SetCtrlAttribute (foc->_panel_id, FOCUSPNL_FOCUS, ATTR_MAX_VALUE, max);
			
			break;
		}
	}
		
	return 0;
}

int CVICALLBACK cbFocusCustom (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			focus *foc = FOCUS_MODULE_CAST(callbackData);

			SetCtrlVal(foc->_panel_id, FOCUSPNL_SAMPLE, 0);
			break;
		}
	}
		
	return 0;
}

