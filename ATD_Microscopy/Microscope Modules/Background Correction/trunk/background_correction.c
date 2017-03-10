#include "background_correction.h"
#include "background_correction_ui.h"

#include "camera\gci_camera.h"

#include "FreeImageIcs_IO.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Filters.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Arithmetic.h" 

#include "gci_utils.h"
#include "string_utils.h"

#include "CubeSlider.h"   
#include "lamp.h"
#include "stage\stage.h"
#include "shutter.h"
#include "microscope.h"

#include "ToolTip.h"

#include <formatio.h>
#include <userint.h>
#include <ansi_c.h>
#include <utility.h>
#include "toolbox.h"

//////////////////////////////////////////////////////////////////
// Reference images and background correction
// All Images are saved to disk as real type (floating point) ics files.
//////////////////////////////////////////////////////////////////

#define NUMBER_OF_FRAMES_FOR_AVERAGE 14
#define MAX_NUMBER_OF_CUBES 10	

struct _ref_images
{
	UIModule parent;
	
	GciCamera *camera;
	XYStage *stage;
	Shutter *shutter;
	FluoCubeManager* cube_manager;
	Lamp *lamp;
	
	Microscope *ms;
	
	int _master_camera_changed_signal_id;
	int enabled_correction;
	HWND tooltip;
	char directory_name[500];

	double	white_image_peak_intensity[MAX_NUMBER_OF_CUBES];   
	FIBITMAP *black_image;
	FIBITMAP *white_images[MAX_NUMBER_OF_CUBES];

};

static void OnMasterCameraChanged (Microscope* microscope, void *data)
{
	ref_images* ri = (ref_images*) data;

    ri->camera = MICROSCOPE_MASTER_CAMERA(ri->ms); 

	ref_images_recall_images(ri);
}

ref_images* ref_images_new(Microscope *ms)
{
	int panel_id;
	
	ref_images *ri = (ref_images *) malloc (sizeof(ref_images));		
	
	ri->black_image = NULL;
	memset(ri->white_images, 0, sizeof(FIBITMAP*) * MAX_NUMBER_OF_CUBES);

	ri->ms = ms;
  	ri->camera = microscope_get_camera(ri->ms);
    ri->stage = microscope_get_stage(ri->ms);	
	ri->shutter = microscope_get_shutter(ri->ms); 
	ri->cube_manager = microscope_get_cube_manager(ri->ms);
	ri->lamp = microscope_get_lamp(ri->ms);
	ri->enabled_correction = 0;
	
	microscope_get_data_directory(ms, ri->directory_name);   
	
	ui_module_constructor(UIMODULE_CAST(ri), "BackgroundCorrection");  
	
	panel_id = ui_module_add_panel(UIMODULE_CAST(ri), "background_correction_ui.uir", REFIM_PNL, 1);
	
	//SetCtrlAttribute(UIMODULE_MAIN_PANEL_ID(ri), REFIM_PNL_PROCESS, ATTR_DIMMED, 1);

	if ( InstallCtrlCallback (panel_id, REFIM_PNL_CLOSE, OnBackgroundCorrectionClose, ri) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (panel_id, REFIM_PNL_BLACK, OnGetBlackImage, ri) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (panel_id, REFIM_PNL_BLANK, OnGetWhiteImage, ri) < 0)
		return NULL;	
	
	//if ( InstallCtrlCallback (panel_id, REFIM_PNL_PROCESS, OnProcess, ri) < 0)
	//	return NULL;	
	
	if ( InstallCtrlCallback (panel_id, REFIM_PNL_ENABLED, OnBackgroundCorrectionEnabled, ri) < 0)
		return NULL;
	
	ri->tooltip = ToolTip_Create (panel_id);

	ToolTip_AddToolTipForCtrl(ri->tooltip, panel_id, REFIM_PNL_BLACK, "Acquire an image with no illumination for use in background correction"); 
	ToolTip_AddToolTipForCtrl(ri->tooltip, panel_id, REFIM_PNL_BLANK, "Acquire an image through a clear portion of slide for use in background correction");
	//ToolTip_AddToolTipForCtrl(ri->tooltip, panel_id, REFIM_PNL_PROCESS, "Performs background correction for the image currently shown.");      

	
	ri->_master_camera_changed_signal_id = microscope_master_camera_changed_handler_connect(ri->ms, OnMasterCameraChanged, ri);

	return ri;
}


static void ref_images_cleanup(ref_images* ri)
{
	int i;

	if(ri->black_image != NULL) {
		FreeImage_Unload(ri->black_image);
		ri->black_image = NULL;
	}
	
	for (i=0; i < MAX_NUMBER_OF_CUBES; i++) {

		if(ri->white_images[i] == NULL)
			continue;

		FreeImage_Unload(ri->white_images[i]);
		ri->white_images[i] = NULL;
	}
}

void ref_images_destroy(ref_images* ri)
{
	ref_images_cleanup(ri);

	if (ri->_master_camera_changed_signal_id >= 0)
		microscope_master_camera_changed_handler_disconnect(ri->ms, ri->_master_camera_changed_signal_id);

	ui_module_destroy(UIMODULE_CAST(ri));
	
	free(ri);
}


int ref_images_get_black_image(ref_images* ri)
{
	int cube=0, msgPnl=0;
	FIBITMAP *tmp = NULL;
	MicroscopeIlluminationMode mode;
	CameraState state;   

	if (!GCI_ConfirmPopup("", IDI_INFORMATION, "Ready to aquire new ""black"" reference?")) {
		return 0;
	}
	
	SetWaitCursor(1);
	
	ref_images_disable(ri);

	gci_camera_set_snap_mode(ri->camera); 		//stop live acquisition
	
	gci_camera_save_state(ri->camera, &state);

//	microscope_set_hi_resolution_mode (ri->ms);  //Ensure full pixel resolution           
	// choose not to use "high res" mode here as this will adjust the exposure if we are binning
	gci_camera_set_binning_mode(ri->camera, NO_BINNING);   // Acquire all refs for no binning
	
	// Set max bpp
	if(gci_camera_set_highest_data_mode(ri->camera) == CAMERA_ERROR)
		goto RI_ERROR;

	mode = microscope_get_illumination_mode (ri->ms);      
	
	 if (mode != MICROSCOPE_FLUORESCENCE) {  
		 if (ri->lamp != NULL){
		 	lamp_off(ri->lamp);
			Delay(2.0);   // wait for lamp to be really off
		}
	}
	else { 	
		
		if(ri->shutter != NULL) {
			
			if(shutter_close(ri->shutter) == SHUTTER_ERROR)
			{
				SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
				
				GCIDialog (0, "Warning", IDI_WARNING, GCI_OK_BUTTON, 
					"Failed to close Hg shutter");    
			}
		
			// Stops the shutter opening when we are acquiring the black image
			shutter_inhibit(ri->shutter, 1);  
		}
	}
	
	stage_set_joystick_off (ri->stage);
	
	//find_resource("background_correction_ui.uir", buffer); 

	msgPnl = GCIDialogNoButtons (0, "Background Correction", IDI_INFORMATION, "Acquiring black image. Please Wait");        
	
	ri->black_image = gci_camera_get_image_average_for_frames(ri->camera, NUMBER_OF_FRAMES_FOR_AVERAGE);
	
	if(mode == MICROSCOPE_FLUORESCENCE)
		if (ri->shutter != NULL)
			// turn shutter back on if necessary
			shutter_inhibit(ri->shutter, 0); 
	
	if(ri->black_image == NULL) {
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		
		//GCI_MessagePopup("Warning", "Failed to get average of %d frames from the camera", NUMBER_OF_FRAMES_FOR_AVERAGE);
		
		SetWaitCursor(0);
		GCIDialog (0, "Warning", IDI_WARNING, GCI_OK_BUTTON, 
			"Failed to get average of %d frames from the camera", NUMBER_OF_FRAMES_FOR_AVERAGE);        
	
		
		goto RI_ERROR;   
	}
	
	// gci_camera_get_image_average_for_frames() has been changed to return a float image so no need for this 
//	tmp = FIA_ConvertToGreyscaleFloatType(ri->black_image, FIT_FLOAT);
//	FreeImage_Unload(ri->black_image);
//	ri->black_image = tmp;
	
	microscope_switch_illumination_mode (ri->ms, mode, 1);

	if (mode != MICROSCOPE_FLUORESCENCE)
		if (ri->lamp != NULL)
			lamp_on(ri->lamp);

	gci_camera_restore_state(ri->camera, &state);

	gci_camera_display_image(ri->camera, ri->black_image, "Black Image");
	
	RI_ERROR:
	
	stage_set_joystick_on (ri->stage);  
	
	DiscardPanel (msgPnl);
	SetWaitCursor(0);
	
	return 0;
}

static void wait_for_user_action(ref_images* ri, const char *message)
{
	char buffer[500];
	int ctrl;
	int ret_pnl = -1, msgPnl;
	FIBITMAP *dib = NULL;

	find_resource("background_correction_ui.uir", buffer); 

	msgPnl = LoadPanel(0, buffer, MSG); 
	
	SetCtrlVal (msgPnl, MSG_TEXT, message);   
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	DisplayPanel (msgPnl);

	while(1) {

		ProcessSystemEvents();

		GetUserEvent (0, &ret_pnl, &ctrl);
		
		if (ret_pnl == msgPnl)
			break;

		#ifndef THREADED_CAM_AQ
		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(ri->camera, NULL); 			
		gci_camera_display_image(ri->camera, dib, NULL);	
		FreeImage_Unload(dib);   
		#endif
	}

	DiscardPanel(msgPnl);
}


static void save_image(ref_images* ri, FIBITMAP* dib, const char* name)
{
	int fsize, bpp;
	FREE_IMAGE_TYPE type;
	char camera_name[UIMODULE_NAME_LEN], fname[GCI_MAX_PATHNAME_LEN];
	
	memset(fname, 0, 1);
	
	strcat(fname, ri->directory_name);
	gci_camera_get_name(ri->camera, camera_name);
	strcat(fname, "\\");
	strcat(fname, camera_name);
	strcat(fname, name);

	type = FreeImage_GetImageType(dib);
	bpp = FreeImage_GetBPP(dib);
	
	//clear read-only
	if (FileExists (fname, &fsize))
		SetFileAttrs (fname, 0, -1, -1, -1);	   

	FreeImageIcs_SaveImage (dib, fname, 1);

	SetFileAttrs (fname, 1, -1, -1, -1);	   // Set read-only
}

static void save_images(ref_images* ri)
{
	char tmp[GCI_MAX_PATHNAME_LEN];
	int i;
	MicroscopeIlluminationMode mode;     
	
	if(ri->black_image == NULL) {
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		
		//GCI_MessagePopup("", "Please acquire a black image first");
		
		GCIDialog (0, "Background Correction", IDI_EXCLAMATION, GCI_OK_BUTTON, 
			"Please acquire a black image first");        

		return;
	}

	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	//GCI_MessagePopup("", "WARNING - If you change the objective or light level\nyou must acquire a new white image.");

	GCIDialog (0, "Background Correction", IDI_WARNING, GCI_OK_BUTTON, 
		"WARNING - If you change the objective or light level\nyou must acquire a new white image.");        
	
	save_image(ri, ri->black_image, "_Blackref.ics");
	
	mode = microscope_get_illumination_mode (ri->ms);  
	
	// Save the white ref image(s)
	if ( mode == MICROSCOPE_FLUORESCENCE) {
	
		for (i=1; i < MAX_NUMBER_OF_CUBES; i++) {
		
			if(ri->white_images[i] == NULL)
				continue;

			sprintf(tmp, "_Fluorwhiteref%d.ics", i);

			save_image(ri, ri->white_images[i], tmp);
		}
		
		return;
	}

	// Bright-field or phase-contrast mode - only one ref image
	if ( mode == MICROSCOPE_BRIGHT_FIELD)
		save_image(ri, ri->white_images[0], "_BrightFieldwhiteref.ics");
	else if ( mode == MICROSCOPE_PHASE_CONTRAST)
		save_image(ri, ri->white_images[0], "_PhaseContrastwhiteref.ics");

	return;		 
}

// White Image
int ref_images_get_white_image(ref_images* ri)
{
	char buffer[500];
	int cube=0, msgPnl = 0;
	double min=0.0;
	MicroscopeIlluminationMode mode;     
	FIBITMAP *tmp = NULL;
	FIABITMAP *filtered_dib = NULL;
	FREE_IMAGE_TYPE type;
	CameraState state;   
	
	if(ri->black_image == NULL) {
		
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		
		GCIDialog (0, "Background Correction", IDI_EXCLAMATION, GCI_OK_BUTTON, 
			"Please acquire a black image first");      
		
		return -1;
	}
	
	ref_images_disable(ri);

	type = FreeImage_GetImageType(ri->black_image);
	
	mode = microscope_get_illumination_mode (ri->ms);  

	if ( mode == MICROSCOPE_FLUORESCENCE) {
		if(cube_manager_get_current_cube_position(ri->cube_manager, &cube) == CUBE_MANAGER_ERROR)
			return -1;
	}
	
	gci_camera_save_state(ri->camera, &state);   // Added
	
	//microscope_set_focusing_mode(ri->ms); // Added
	
	gci_camera_set_live_mode(ri->camera);

	//if ( mode == MICROSCOPE_FLUORESCENCE)
	//	wait_for_user_action(ri, "Place uniform fluorescent\nsample on stage.");  
	//else if ( mode == MICROSCOPE_BRIGHT_FIELD)
	//	wait_for_user_action(ri, "Place uniform clean sample\non stage.");
	
	wait_for_user_action(ri, "Place sample on stage and focus.");

	stage_set_joystick_on (ri->stage);

	if ( mode == MICROSCOPE_FLUORESCENCE) {
		wait_for_user_action(ri, "Place fluorescent reference sample on stage.\n"
								 "(Same focal plane)");  
	}
	else if ( mode == MICROSCOPE_BRIGHT_FIELD)
		wait_for_user_action(ri, "Move to clean reference region.");


	stage_set_joystick_off (ri->stage);

	gci_camera_set_snap_mode(ri->camera);

	// Set max bpp
	gci_camera_set_highest_data_mode(ri->camera);   // Acquire all refs for highest data mode      
	
	//	microscope_set_hi_resolution_mode(ri->ms);	// ADDED
	// choose not to use "high res" mode here as this will adjust the exposure if we are binning
	gci_camera_set_binning_mode(ri->camera, NO_BINNING);   // Acquire all refs for no binning
	
	Delay(0.2);

	gci_camera_snap_image(ri->camera);
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	
	find_resource("background_correction_ui.uir", buffer); 

	msgPnl = LoadPanel(0, buffer, MSG); 
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
	
	if (GCI_ConfirmPopup("", IDI_INFORMATION, "OK to aquire new ""white"" reference?")) {
		
		SetWaitCursor(1);

		msgPnl = GCIDialogNoButtons (0, "Background Correction", IDI_INFORMATION, "Acquiring white average");        

		ri->white_images[cube] = gci_camera_get_image_average_for_frames(ri->camera, NUMBER_OF_FRAMES_FOR_AVERAGE);

		gci_camera_display_image(ri->camera, ri->white_images[cube], "white Reference");

		// If fluorescence apply median filter to remove toot (low light levels)
		if ( mode == MICROSCOPE_FLUORESCENCE) {

			filtered_dib = FIA_SetZeroBorder(ri->white_images[cube], 10, 10);

			tmp = FIA_MedianFilter(filtered_dib, 10, 10);

			FIA_Unload(filtered_dib);
			FreeImage_Unload(ri->white_images[cube]); 
			ri->white_images[cube] = tmp;
			
			FIA_FindMinMax(ri->white_images[cube], &min, &(ri->white_image_peak_intensity[cube]));    
		}
		else {
			
			FIA_FindMinMax(ri->white_images[0], &min, &(ri->white_image_peak_intensity[0]));    
		}

		DiscardPanel (msgPnl);      
		
		//gci_camera_display_image(ri->camera, ri->white_images[cube], "white Reference");

		save_images(ri);
	}
	
	stage_set_joystick_on (ri->stage);

	gci_camera_restore_state(ri->camera, &state);
	
//	gci_camera_snap_image(ri->camera);   
	
	SetWaitCursor(0);
	
	return 0;
}


static FIBITMAP* load_image(ref_images* ri, const char* name)
{
	char camera_name[UIMODULE_NAME_LEN], fname[1000];

	memset(fname, 0, 1);

	gci_camera_get_name(ri->camera, camera_name); 
	sprintf(fname, "%s\\%s%s", ri->directory_name, camera_name, name);

	return FreeImageIcs_LoadFIBFromIcsFilePath (fname);
}

void ref_images_recall_images(ref_images* ri)
{
	char tmp[GCI_MAX_PATHNAME_LEN];
	int i;
	double min = 0.0;
	MicroscopeIlluminationMode mode;

	// Why not load all the ref images here. Default to the brightfeild ref rather than phase contrast (unless in that mode)
	
	ref_images_cleanup(ri);

	ri->black_image = load_image(ri, "_Blackref.ics");
 
	//Load the white ref image(s)
//	if ( microscope_get_illumination_mode (ri->ms) == MICROSCOPE_FLUORESCENCE) {
		
		
//		return;
//	}

	//Bright-field or phase-contrast mode - only one ref image
//	if ( microscope_get_illumination_mode (ri->ms) == MICROSCOPE_BRIGHT_FIELD)
//		ri->white_images[0] = load_image(ri, "_BrightFieldwhiteref.ics");
//	else if ( microscope_get_illumination_mode (ri->ms) == MICROSCOPE_PHASE_CONTRAST)
//		ri->white_images[0] = load_image(ri, "_PhaseContrastwhiteref.ics");

	mode =  microscope_get_illumination_mode (ri->ms);

	if (mode == MICROSCOPE_PHASE_CONTRAST) {
		ri->white_images[0] = load_image(ri, "_PhaseContrastwhiteref.ics");
		FIA_FindMinMax(ri->white_images[0], &min, &(ri->white_image_peak_intensity[0]));   
	}
	else if(mode == MICROSCOPE_BRIGHT_FIELD) {

		ri->white_images[0] = load_image(ri, "_BrightFieldwhiteref.ics");
		FIA_FindMinMax(ri->white_images[0], &min, &(ri->white_image_peak_intensity[0]));   
	}
	else if(mode == MICROSCOPE_FLUORESCENCE) {

		for (i=1; i < MAX_NUMBER_OF_CUBES; i++) {

			sprintf(tmp, "_Fluorwhiteref%d.ics", i);

			ri->white_images[i] = load_image(ri, tmp);

			FIA_FindMinMax(ri->white_images[i], &min, &(ri->white_image_peak_intensity[i]));  
		}	
	}

	//if(ref_images_can_process(ri))
	//	SetCtrlAttribute(UIMODULE_MAIN_PANEL_ID(ri), REFIM_PNL_PROCESS, ATTR_DIMMED, 0);
}

void ref_images_enable(ref_images* ri)
{
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(ri), REFIM_PNL_ENABLED, 1);
	ri->enabled_correction = 1;	
}

void ref_images_disable(ref_images* ri)
{
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(ri), REFIM_PNL_ENABLED, 0);   
	ri->enabled_correction = 0;		
}


int ref_images_can_process(ref_images* ri)
{
	int cube=0;
		
	//Let other modules know whether background correction can take place
	if ( microscope_get_illumination_mode (ri->ms) == MICROSCOPE_FLUORESCENCE) {
	
		if(cube_manager_get_current_cube_position(ri->cube_manager, &cube) == CUBE_MANAGER_ERROR)
			return 0;
	}

	if (ri->black_image == NULL)
		return 0;
		
	if (ri->white_images[cube] == NULL)
		return 0;
	
	return 1;
}

int ref_images_should_process(ref_images* ri)
{
	if(!ri->enabled_correction)
		return 0;
		
	return ref_images_can_process(ri);
}


static void ref_images_check_dimensions(ref_images *ri, FIBITMAP* image, int cube,
		FIBITMAP** altered_black_image, FIBITMAP** altered_white_image)
{
	BinningMode binning;
	FIBITMAP *tmp = NULL;
	
	int image_width = FreeImage_GetWidth(image);
	int image_height = FreeImage_GetHeight(image);

	int black_image_width = FreeImage_GetWidth(ri->black_image);
	int black_image_height = FreeImage_GetHeight(ri->black_image);
	
	//gci_camera_get_size_position(ri->camera, &left, &top, &size_x, &size_y, &autocentre);
	
	binning = gci_camera_get_binning_mode(ri->camera);

	// Binning is 1 for no binning. It is above 1 for binning modes. It is never 0
	if ((binning != NO_BINNING) || (image_width != black_image_width) || (image_height != black_image_height)) {
/*
		if ((image_width > black_image_width)  || (image_height > black_image_height)) {
			//Stretch or shrink reference images
			*altered_black_image = FreeImage_Rescale(ri->black_image, image_width, image_height, FILTER_BOX);
			*altered_white_image = FreeImage_Rescale(ri->white_images[cube], image_width, image_height, FILTER_BOX);
		}
		else {
			
			//Cut a bit out of the reference images
			tmp = FreeImage_Copy(ri->black_image, left, top, left + size_x, top + size_y);
			FreeImage_Unload(ri->black_image);
			ri->black_image = tmp;
			
			tmp = FreeImage_Copy(ri->white_images[cube], left, top, left + size_x, top + size_y);
			FreeImage_Unload(ri->white_images[cube]);
			ri->white_images[cube] = tmp;
			
			*altered_black_image = FreeImage_Copy(ri->black_image, left, top, left + size_x, top + size_y);
			*altered_white_image = FreeImage_Copy(ri->white_images[cube], left, top, left + size_x, top + size_y); 
		}
*/

		// PB/GP 10/03/08
		// Cannot see how the above worked with binning (code above dates from at least spectral imager code)
		// Ref images are stored at full res, if we are now binning we need to resample the ref images
		// WHy would you ever crop from the middle?
		
		//Stretch or shrink reference images
		*altered_black_image = FreeImage_Rescale(ri->black_image, image_width, image_height, FILTER_BOX);
		*altered_white_image = FreeImage_Rescale(ri->white_images[cube], image_width, image_height, FILTER_BOX);
	}
	else{
		*altered_black_image = FreeImage_Clone(ri->black_image);
		*altered_white_image = FreeImage_Clone(ri->white_images[cube]);
	}
}


#pragma optimize("f", on) 
static void ref_image_perform_correction(ref_images* ri, FIBITMAP* image,
		FIBITMAP *black_image, FIBITMAP* white_image)
{
	register int x, y;
	int mode, cube=0,bpp, max_intensity, width, height, number_of_pixels;
	float *image_ptr, *black_ptr, *white_ptr, bin_factor=1.0;
	BinningMode binning;
	FREE_IMAGE_TYPE type;

	bpp = gci_camera_get_data_mode(ri->camera);
	
	if(bpp < 24) {
		// Assert that the images are of all float type. When we correct for
		// the background we convert to the desired type at the same time.
		if((type = FreeImage_GetImageType(image)) != FIT_FLOAT)
			logger_log(UIMODULE_LOGGER(ri), LOGGER_ERROR, "bg correction: image not a FIT_FLOAT");

		if((type = FreeImage_GetImageType(black_image)) != FIT_FLOAT)
			logger_log(UIMODULE_LOGGER(ri), LOGGER_ERROR, "bg correction: black_image not a FIT_FLOAT");

		if((type = FreeImage_GetImageType(white_image)) != FIT_FLOAT)
			logger_log(UIMODULE_LOGGER(ri), LOGGER_ERROR, "bg correction: white_image not a FIT_FLOAT");
	}
	else {

		if((type = FreeImage_GetImageType(image)) != FIT_BITMAP)
			logger_log(UIMODULE_LOGGER(ri), LOGGER_ERROR, "bg correction: image not a FIT_BITMAP");

		if((type = FreeImage_GetImageType(black_image)) != FIT_BITMAP)
			logger_log(UIMODULE_LOGGER(ri), LOGGER_ERROR, "bg correction: black_image not a FIT_BITMAP");

		if((type = FreeImage_GetImageType(white_image)) != FIT_BITMAP)
			logger_log(UIMODULE_LOGGER(ri), LOGGER_ERROR, "bg correction: white_image not a FIT_BITMAP");
	}

	binning = gci_camera_get_binning_mode(ri->camera);
	
	mode = microscope_get_illumination_mode (ri->ms);  
	
	if ( mode == MICROSCOPE_FLUORESCENCE) {
		if(cube_manager_get_current_cube_position(ri->cube_manager, &cube) == CUBE_MANAGER_ERROR)
			return;
		
		max_intensity = (int) (ri->white_image_peak_intensity[cube]);  
		if (bpp == BPP8)
			max_intensity /= 16;	 // as we store the max_intensity as a 12 bit and we want an 8 bit RESULT 
			// NEED TO DETERMINE THE HIGHEST BBP FOR THE CAMERA FOR THE FACTOR 16.0 
	}	
	else {

		max_intensity = (int) pow(2, bpp) - 1;
		//max_intensity = (int) (ri->white_image_peak_intensity[0]);  

		//if (bpp == BPP8)
		//	max_intensity /= 16;	// As we store the max_intensity as a 12 bit and we want an 8 bit RESULT 
	}

	// bin_factor attempts to correct for the increase in brightness if we are now binning
	// we expect this to be _real_ binning, so 2x2 binning = factor of 4 in greylevel
	if (binning > NO_BINNING)
		bin_factor = (float) pow((double)binning, 2.0);
	else
		bin_factor = 1.0;
	
	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);
	number_of_pixels = width * height;

	if (bpp == BPP8)
	{
		for(y = 0; y < height; y++) { 
		
			image_ptr = (float *) FreeImage_GetScanLine(image, y);   
			black_ptr = (float *) FreeImage_GetScanLine(black_image, y);   
			white_ptr = (float *) FreeImage_GetScanLine(white_image, y);   
	
			for(x=0; x < width; x++) {
			
				image_ptr[x] = (float) ((image_ptr[x] * 16.0 - black_ptr[x]) * max_intensity) / (white_ptr[x] * bin_factor - black_ptr[x]);
				// TODO the 16.0 in the line above may only work for a camera that is 12 and 8 bit as in this case the refs are stored in 12 bit - what if we have e.g. 16 and 8?????
				image_ptr[x] = (float) max(0.0, min(255.0, image_ptr[x]));		// Prevent overload / underload
			}
		}
	}
	else if(bpp == BPP24) {

		unsigned char *char_image_ptr, *char_black_ptr, *char_white_ptr;
		unsigned short red, green, blue;
		int image_red, image_green, image_blue;
		int black_red, black_green, black_blue;
		int white_red, white_green, white_blue;
		int line;

		int bpp = FreeImage_GetBPP(image);

		if(bpp != 24) {

			GCI_MessagePopup("Error", "Camera image is not 24bit");
			return;
		}

		bpp = FreeImage_GetBPP(black_image);

		if(bpp != 24) {

			GCI_MessagePopup("Error", "Black image is not 24bit");
			return;
		}

		bpp = FreeImage_GetBPP(white_image);

		if(bpp != 24) {

			GCI_MessagePopup("Error", "White image is not 24bit");
			return;
		}

		line = FreeImage_GetLine (image);

		for(y = 0; y < height; y++) { 
		
			char_image_ptr = (unsigned char *) FreeImage_GetScanLine(image, y);   
			char_black_ptr = (unsigned char *) FreeImage_GetScanLine(black_image, y);   
			char_white_ptr = (unsigned char *) FreeImage_GetScanLine(white_image, y);   
	
			for(x=0; x < line; x+=3) {
			
				image_red = char_image_ptr[x + FI_RGBA_RED];
				image_green = char_image_ptr[x + FI_RGBA_GREEN];
				image_blue = char_image_ptr[x + FI_RGBA_BLUE];

				black_red = char_black_ptr[x + FI_RGBA_RED];
				black_green = char_black_ptr[x + FI_RGBA_GREEN];
				black_blue = char_black_ptr[x + FI_RGBA_BLUE];

				white_red = char_white_ptr[x + FI_RGBA_RED];
				white_green = char_white_ptr[x + FI_RGBA_GREEN];
				white_blue = char_white_ptr[x + FI_RGBA_BLUE];

				red = (unsigned short)(max(0.0, (image_red - black_red)) * 250.0) / (white_red - black_red);
				green = (unsigned short)(max(0.0, (image_green - black_green)) * 250.0) / (white_green - black_green);
				blue = (unsigned short)(max(0.0, (image_blue - black_blue)) * 250.0) / (white_blue - black_blue);

				char_image_ptr[x + FI_RGBA_RED] = (unsigned char) max(0.0, min(255.0, red));
				char_image_ptr[x + FI_RGBA_GREEN] = (unsigned char) max(0.0, min(255.0, green));
				char_image_ptr[x + FI_RGBA_BLUE] = (unsigned char) max(0.0, min(255.0, blue));
			}
		}
	}
	else if(bpp == BPP12 || bpp == BPP14 || bpp == BPP16)
	{
		for(y = 0; y < height; y++) { 
		
			image_ptr = (float *) FreeImage_GetScanLine(image, y);   
			black_ptr = (float *) FreeImage_GetScanLine(black_image, y);   
			white_ptr = (float *) FreeImage_GetScanLine(white_image, y);   
	
			for(x=0; x < width; x++) {
			
				image_ptr[x] = ((image_ptr[x] - black_ptr[x]) * max_intensity) / (white_ptr[x] * bin_factor - black_ptr[x]);
		
				image_ptr[x] = (float) max(0.0, min((float)max_intensity, image_ptr[x]));		// Prevent overload / underload
			}
		}
	}
}
#pragma optimize("f", off) 


int ref_images_in_place_process_forced(ref_images* ri, FIBITMAP** image)
{
	FIBITMAP *tmp = NULL, *altered_black_image = NULL, *altered_white_image = NULL;
	FREE_IMAGE_TYPE type = FreeImage_GetImageType(*image);

	int cube=0, bpp, mode;

	bpp = gci_camera_get_data_mode(ri->camera);   

	// Compute image = (image-black image)/white image * max intensity level of white image.
	mode = microscope_get_illumination_mode (ri->ms);  

	if ( mode == MICROSCOPE_FLUORESCENCE) { 
	
		if(cube_manager_get_current_cube_position(ri->cube_manager, &cube) == CUBE_MANAGER_ERROR)
			return 1;
	}
		
	if (ri->black_image == NULL) {
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		
		GCIDialog (0, "Background Correction", IDI_EXCLAMATION, GCI_OK_BUTTON, 
			"Please acquire a black image first");      
		
		return 1;
	}
	
	if (ri->white_images[cube] == NULL) {
	
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		if ( mode == MICROSCOPE_FLUORESCENCE )  {
			
			GCIDialog (0, "Background Correction", IDI_EXCLAMATION, GCI_OK_BUTTON, 
				"Please acquire a white image for this cube first");   
		}
			
		else {
			
			GCIDialog (0, "Background Correction", IDI_EXCLAMATION, GCI_OK_BUTTON, 
				"Please acquire a white image first");   
		}
			
		return 1;
	}
	
	// If not colour - Change to float
	//if((FreeImage_GetBPP(*image) < 24) && (FreeImage_GetImageType(*image) == FIT_BITMAP)) {

	if(FIA_IsGreyScale(*image)) { 
		tmp = FIA_ConvertToGreyscaleFloatType(*image, FIT_FLOAT);
		FreeImage_Unload(*image);
		*image = tmp;
	}

	// Portion of ref images used must match input image
	ref_images_check_dimensions(ri, *image, cube, &altered_black_image, &altered_white_image);

	ref_image_perform_correction(ri, *image, altered_black_image, altered_white_image);

	if(bpp == BPP8) {     
		
		tmp = FreeImage_ConvertToType(*image, type, 0);
		FreeImage_Unload(*image);
		*image = tmp;
	}
	else if(bpp == BPP24) {
		// Do Nothing
	}
	else {
		tmp = FIA_ConvertFloatTypeToType(*image, FIT_UINT16, 0);
		FreeImage_Unload(*image);
		*image = tmp;	
	}
	
	if(altered_black_image != NULL) {
		FreeImage_Unload(altered_black_image);
		altered_black_image = NULL;
	}

	if(altered_white_image != NULL) {
		FreeImage_Unload(altered_white_image);
		altered_white_image = NULL;
	}

	return 0;
}

int ref_images_in_place_process(ref_images* ri, FIBITMAP** image)
{
	if(!ri->enabled_correction)
		return 0;

	return ref_images_in_place_process_forced(ri, image);
}

FIBITMAP* ref_images_process(ref_images* ri, FIBITMAP* image)
{
	FIBITMAP *dst = NULL, *tmp = NULL, *altered_black_image = NULL, *altered_white_image = NULL;
	FREE_IMAGE_TYPE type = FreeImage_GetImageType(image);

	int cube=0;

	dst = FreeImage_Clone(image);
	
	ref_images_in_place_process(ri, &dst);  
	
	return dst;
}

////////////////////////////////////////////////////////////////////////
int CVICALLBACK OnGetBlackImage (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ref_images* ri = (ref_images* ) callbackData;
			
			ref_images_get_black_image(ri);   
				
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnGetWhiteImage (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ref_images* ri = (ref_images* ) callbackData;
			
			ref_images_get_white_image(ri);   
				
			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnBackgroundCorrectionClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ref_images* ri = (ref_images* ) callbackData;
			
			ui_module_hide_all_panels(UIMODULE_CAST(ri)); 
				
			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnProcess (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{		
			ref_images* ri = (ref_images* ) callbackData;  

			FIBITMAP* dib = gci_camera_get_displayed_image(ri->camera);
		
			ref_images_in_place_process_forced(ri, &dib);     
			gci_camera_display_image(ri->camera, dib, "Processed Image");

			FreeImage_Unload(dib);
		
			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnBackgroundCorrectionEnabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{	
			ref_images* ri = (ref_images* ) callbackData;  
			
			GetCtrlVal(panel, control, &(ri->enabled_correction));

			break;
		}
	}
	
	return 0;
}
