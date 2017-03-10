#include "mosaic.h"
#include "toolbox.h"
#include <utility.h>

//#include "EpiLamp.h"
//#include "microscope.h"
#include "optical_calibration.h"
//#include "optical_shutter.h"
#include "imaging.h"   
#include "ImagingFacade.h"

#include "GL_CVIRegistry.h"

#include "FreeImageAlgorithms_IO.h"
//#include "FreeImageAlgorithms_Ics.h"
//#include "FreeImageAlgorithms_MetaData.h"

#include "string_utils.h"

////////////////////////////////////////////////////////////////////////////
//RJL June 2006
//GCI Microscopy development. 
//Mosaic image module - adapted from Microfocus version by GP
////////////////////////////////////////////////////////////////////////////

static void save_mosaic_metadata(GCIWindow *window, char *ics_filepath,  char *extension, void* callback)
{
	char temp[50], author[20], date[20], dimensions[30], extents[30];
	char mode[30], microscope_name[30], light_source[30];
	char objective_string[30], cube_string[30]="empty", camera_model[64];
	char obj_number[10], cube_exc_string[15]="0", cube_emm_string[15]="0", exposure_string[15];
	char gain_string[10], black_level_string[10], binning_string[10], light_mode_string[10];
	int error, author_length, image_width, image_height, fluo_mode, objective_number;
	int cube, cube_emm, cube_exc, binning, light_mode;
	double umPerPix, gain, exposure, black_level, total_width, total_height, extents_x, extents_y;
	ICS *ics;  
	
	GciCamera* camera;
	MosaicWindow *mosaic_window = (MosaicWindow *) callback;
	
	FIBITMAP *fib = GCI_ImagingWindow_GetDisplayedFIB(window);
	
	image_width = FreeImage_GetWidth(fib);
	image_height = FreeImage_GetHeight(fib);
	
    // Retrieve the name of the user that it currently logged in
    GetCurrentUser (author, 100, &author_length);
  
    english_date(temp);
    sprintf(date, "%s %s", TimeStr(), temp);
    
	sprintf(dimensions, "%d %d", image_width, image_height);

	camera = GCI_Imaging_GetCamera();
	umPerPix = gci_camera_get_true_microns_per_pixel(camera);
	
	total_width = mosaic_window->number_of_cols * mosaic_window->tile_width;
	extents_x = total_width - ((mosaic_window->number_of_cols - 1) * mosaic_window->x_overlap);
	
	total_height = mosaic_window->number_of_rows * mosaic_window->tile_height;
	extents_y = total_height - ((mosaic_window->number_of_rows - 1) * mosaic_window->y_overlap);
	
	sprintf(extents, "%.3e %.3e", extents_x * mosaic_window->um_per_pixel * 1e-6, extents_y * mosaic_window->um_per_pixel * 1e-6);  

	fluo_mode = GCI_ImagingGetOperationMode();

	if (fluo_mode == BRIGHT_FIELD) {
		sprintf(mode, "%s", "Brightfield");
		strcpy(light_source, "Argon");
	}
	else if ((fluo_mode == FLUORESCENCE) || (fluo_mode == FLUOR_NO_SHUTTER)) {
		sprintf(mode, "%s", "Fluorescence");
		strcpy(light_source, "Hg arc");
	}
	else if (fluo_mode == PHASE_CONTRAST) {
		sprintf(mode, "%s", "PhaseContrast");
		strcpy(light_source, "Tungsten");
	}
										
	GCI_ImagingMicroscopeGetName(microscope_name);
	
	GCI_ImagingGetObjective(&objective_number);
	str_itoa(objective_number, obj_number);

	GCI_ImagingGetObjectiveString(objective_string);

	cube = GCI_ImagingGetFluorCube();
	if (cube != IMAGING_ERROR) {	
		GCI_ImagingGetFluorCubeName(cube, cube_string);

		if (strcmp(cube_string, "empty")) {				  //not an empty position
			GCI_ImagingGetFluorCubeExc(cube, &cube_exc);
			str_itoa(cube_exc, cube_exc_string); 

			GCI_ImagingGetFluorCubeEmm(cube, &cube_emm);
			str_itoa(cube_emm, cube_emm_string);
		}
	}
	
	gci_camera_get_description(camera, camera_model); 
	
	exposure = gci_camera_get_exposure_time(camera);
	ftoa(exposure, exposure_string);

	gain = gci_camera_get_gain(camera);
	ftoa(gain, gain_string);
	
	black_level = gci_camera_get_blacklevel(camera);
	ftoa(black_level, black_level_string);
	
	binning = gci_camera_get_binning_mode(camera);
	str_itoa(binning, binning_string);

	light_mode = gci_camera_get_light_mode(camera);
	str_itoa(light_mode, light_mode_string);
	
	IcsOpen(&ics, ics_filepath, "rw");         
	
	error = FreeImageIcs_SetIcsHistoryKeyValueStrings(ics,
											 "version", "2",
											 "institution", "GCI",
											 "author", author,
											 "created on", date,
											 "type", mode,
											 "dimensions", dimensions,
											 "extents", extents,
											 "labels", "x y",
											 "units", "m m",
											 "microscope_ID", microscope_name,
											 "mode", mode,
											 "light_source", light_source,
											 "objective", obj_number,
											 "objective desc", objective_string,
											 "cube", cube_string,
											 "cube exc nm", cube_exc_string,
											 "cube emm nm", cube_emm_string,
											 "camera", camera_model,
											 "exposure", exposure_string,
											 "gain", gain_string,
											 "black_lev", black_level_string,
											 "binning", binning_string,
											 "light_mode", light_mode_string,
											 NULL);

	IcsClose (ics);   

}


static void mosaic_read_or_write_panel_registry_settings(MosaicWindow *mosaic_window, int write)
{
	int visible;

	// load or save panel positions
	
	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {
	
		GetPanelAttribute (mosaic_window->mosaic_window_id, ATTR_VISIBLE, &visible);
	
		if(!visible)
			return;
	
		SetPanelAttribute (mosaic_window->mosaic_window_id, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	}
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, "software\\GCI\\Microscope\\MosaicWindow\\",  "top", mosaic_window->mosaic_window_id, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, "software\\GCI\\Microscope\\MosaicWindow\\",  "left", mosaic_window->mosaic_window_id, ATTR_LEFT);
}


static void mosaic_window_close_handler(GCIWindow *window, void *callback_data)
{
	MosaicWindow *mosaic_window = (MosaicWindow *) callback_data;
	
	mosaic_read_or_write_panel_registry_settings(mosaic_window, 1);
	
	GCI_ImagingWindow_Hide(window);
}


static void mosaic_window_create(MosaicWindow *mosaic_window, IPIPixelType pixelType)
{
	double h, w;
	int w1;

	IPI_Create(&(mosaic_window->mosaic_image), pixelType, 0);   

	w = mosaic_window->tile_width * mosaic_window->number_of_cols;
	h = mosaic_window->tile_height * mosaic_window->number_of_rows;
	
	w -= (int) (((mosaic_window->number_of_cols - 1) * mosaic_window->x_overlap) + 0.5);
	h -= (int) (((mosaic_window->number_of_rows - 1) * mosaic_window->y_overlap) + 0.5);

	w1 = (int) (w / 8 + 0.5); // Width must be a multiple of 8
	w = w1 * 8;
	
	IPI_SetImageSize (mosaic_window->mosaic_image, w, h);
	IPI_DrawRect (mosaic_window->mosaic_image, mosaic_window->mosaic_image, IPI_FULL_RECT, IPI_DRAW_PAINT, 0.0);
}


MosaicWindow* mosaic_window_new(int left, int top, int width, int height)
{
	MosaicWindow *mosaic_window = (MosaicWindow *) malloc (sizeof(MosaicWindow));

	width -= 70;	//correct width for lack of tool bar
	
	mosaic_window->window = GCI_ImagingWindow_Create("Mosaic Image", left, top, width, height, 0, 3);
	
	mosaic_window->mosaic_window_id = GCI_ImagingWindow_GetPanelID(mosaic_window->window);
	
	GCI_ImagingWindow_SetSaveHandler(mosaic_window->window, save_mosaic_metadata, mosaic_window); 
	
	GetScreenSize (&(mosaic_window->screen_height), &(mosaic_window->screen_width));
	
	GCI_ImagingWindow_SetResizeFitStyle(mosaic_window->window, 1); 
		
	GCI_ImagingWindow_HideToolBar(mosaic_window->window); 
	//GCI_ImagingWindow_HidePaletteBar(mosaic_window->window);
		
	GCI_ImagingWindow_SetCloseEventHandler(mosaic_window->window, mosaic_window_close_handler, mosaic_window );

	mosaic_read_or_write_panel_registry_settings(mosaic_window, 0);

	mosaic_window->header_filepath = (char *) malloc (sizeof(char) * MAX_PATHNAME_LEN); 
	
	mosaic_window->composite_image_created = 0;
	
	mosaic_window->mosaic_image = 0;
	
	return mosaic_window;
}


void mosaic_window_set_pixel_type(MosaicWindow *mosaic_window, IPIPixelType pixelType)
{
	mosaic_window->pixeltype = pixelType;
}


void mosaic_window_set_row_and_col_size(MosaicWindow *mosaic_window, int number_of_cols, int number_of_rows)
{
	mosaic_window->number_of_cols = number_of_cols;
	mosaic_window->number_of_rows = number_of_rows;
}


void mosaic_window_set_overlap(MosaicWindow *mosaic_window, double x_percentage_overlap, double y_percentage_overlap)
{
	mosaic_window->x_percentage_overlap = x_percentage_overlap;
	mosaic_window->y_percentage_overlap = y_percentage_overlap;
}


void mosaic_window_destroy(MosaicWindow *mosaic_window)
{
	if(mosaic_window->header_filepath != NULL)
		free(mosaic_window->header_filepath);
	
	if (mosaic_window->mosaic_image > 0) {
		IPI_Dispose(mosaic_window->mosaic_image);
		mosaic_window->mosaic_image = 0;
	}
	
	mosaic_read_or_write_panel_registry_settings(mosaic_window, 1);
	
	GCI_ImagingWindow_Close(mosaic_window->window);
	mosaic_window->window = NULL;

  	free(mosaic_window);
  	mosaic_window = NULL;
}

void mosaic_window_hide(MosaicWindow *mosaic_window)
{
	if (mosaic_window == NULL) return;
	if (mosaic_window->window == NULL) return;
	
	GCI_ImagingWindow_Hide(mosaic_window->window);
}

static int display_ipi_image(GCIWindow* window, IPIImageRef image, char *title)
{
	FIBITMAP *dib=NULL;

	if (image == NULL) return -1;

	dib = GCI_FreeImage_IPIImageRefToFIB(image);
	if (dib == NULL) return -1;

	GCI_ImagingWindow_LoadImage(window, dib);  
	
	if(title != NULL)
		GCI_ImagingWindow_SetWindowTitle( window, title);

	return 0;
}


void mosaic_window_show(MosaicWindow *mosaic_window)
{
	GCI_ImagingWindow_Show(mosaic_window->window); 
}


void mosaic_window_update(MosaicWindow *mosaic_window)
{
	display_ipi_image(mosaic_window->window, mosaic_window->mosaic_image, "Mosaiac"); 
}


void mosaic_window_clear(MosaicWindow *mosaic_window)
{
	mosaic_window->composite_image_created = 0;
	
	if(mosaic_window->mosaic_image != 0)
		IPI_Dispose(mosaic_window->mosaic_image);
}


void mosaic_window_add_image(MosaicWindow *mosaic_window, IPIImageRef image, int col, int row)
{
	GciCamera* camera;
	IPIImageInfo tile_info;
	IPIImageRef tile=0;
	int top, left;
	double image_aspect_ratio, umPerPix;
	
	IPI_GetImageInfo (image, &tile_info);
	
	// We must caculate the aspect ratio of the image if this is the first image added.
	if(mosaic_window->composite_image_created == 0) {
	
		// Determine the size of the tiles
		// Create destination image making it fit as closely as possible our default screen
		image_aspect_ratio = (double)tile_info.width / tile_info.height;
		
		mosaic_window->tile_width  = (int) (mosaic_window->screen_width / mosaic_window->number_of_cols + 0.5);
		mosaic_window->tile_height = (int) (mosaic_window->tile_width / image_aspect_ratio + 0.5);
	
		if (mosaic_window->tile_height * mosaic_window->number_of_rows > mosaic_window->screen_height) {
	
			mosaic_window->tile_height = (int) (mosaic_window->screen_height / mosaic_window->number_of_rows + 0.5);
			mosaic_window->tile_width = (int) (mosaic_window->tile_height * image_aspect_ratio + 0.5);
		}

		camera = GCI_Imaging_GetCamera();
		umPerPix = gci_camera_get_true_microns_per_pixel(camera);
		mosaic_window->um_per_pixel = umPerPix*tile_info.width/mosaic_window->tile_width;
		
		mosaic_window->x_overlap = (double) mosaic_window->tile_width * mosaic_window->x_percentage_overlap / 100.0;
		mosaic_window->y_overlap = (double) mosaic_window->tile_height * mosaic_window->y_percentage_overlap / 100.0;
	
		mosaic_window_create(mosaic_window, tile_info.pixelType);
		mosaic_window->composite_image_created = 1;
	}

	IPI_Create(&tile, tile_info.pixelType, 0);
	
	// resizes the image to the new image tile
	IPI_Resample (image, tile, mosaic_window->tile_width, mosaic_window->tile_height, 0, IPI_FULL_RECT);
	
	left = (int) (col * ((double)mosaic_window->tile_width - mosaic_window->x_overlap) + 0.5);
	top = (int) (row *  ((double)mosaic_window->tile_height - mosaic_window->y_overlap) + 0.5);
	
	IPI_ImageToImage (tile, mosaic_window->mosaic_image, left, top);
																					
	IPI_Dispose(tile);
}
