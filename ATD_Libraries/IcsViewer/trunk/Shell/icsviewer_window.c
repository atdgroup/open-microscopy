#include "icsviewer_window.h"
#include "icsviewer_com_utils.h"

#include "toolbox.h"

#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_HBitmap.h"
#include "FreeImageAlgorithms_Filters.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Palettes.h" 
#include "FreeImageAlgorithms_LinearScale.h"

#include "icsviewer_private.h"
#include "icsviewer_signals.h" 
#include "icsviewer_plugin.h"
#include "ImageViewer_Drawing.h"
#include "gci_utils.h" 
#include "gci_menu_utils.h" 

#include "ThreadDebug.h"

#include "roi_tool.h"
#include "zoom_tool.h"
#include "line_tool.h"
#include "crosshair_tool.h" 
#include "palettebar_plugin.h" 
#include "linearscale_plugin.h"
#include "titlebar_plugin.h"
#include "metadata_plugin.h"
#include "palette_plugin.h"
#include "screenshot_plugin.h"
#include "histogram_equalisation_plugin.h"
#include "histogram_plugin.h"
#include "grid_plugin.h"
#include "background_plugin.h"
#include "fft_plugin.h"
#include "scalebar_plugin.h"
#include "profile_plugin.h"  
#include "resample_menu_plugin.h" 
#include "binning_menu_plugin.h" 
#include "rotate_menu_plugin.h"
#include "save_plugin.h"
#include "counter_plugin.h" 
#include "record_plugin.h"

#if defined(STANDALONE_APP) || defined(STREAM_DEVICE_PLUGIN)
#include "streamdevice_plugin.h" 
#endif

#ifdef ENABLE_TWAIN
#include "twain_plugin.h"
#endif

#include "icsviewer_uir.h"

#include "string_utils.h"
#include "gci_menu_utils.h"
#include "ImageViewer.h"
#include "asynctmr.h"
#include "GL_CVIRegistry.h"

#include <formatio.h>
#include <utility.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>

#include "icsviewer_3d.h"

#define NO_OF_SB_PANELS 3
static int statusbar_coords[NO_OF_SB_PANELS] = {75, 200, -1};

// Store a default save location for all windows.
// Each window also has a specific one which is orveridden by this, if set.
static char global_default_directory[GCI_MAX_PATHNAME_LEN] = "";

FIAPOINT
GdiToFiaPoint(POINT pt)
{
	FIAPOINT fia_pt;
	
	fia_pt.x = pt.x;
	fia_pt.y = pt.y;
	
	return fia_pt;
}


void format_intensity_string(IcsViewerWindow *window, double value, char *string)
{
	FREE_IMAGE_TYPE type = FreeImage_GetImageType(window->panel_dib);

	if(type == FIT_BITMAP || type == FIT_INT16 || type == FIT_UINT16) {
			
		sprintf(string, "%d", (int) value);
		return;
	}

	if((fabs(value) < 0.01 || fabs(value) > 999) && value != 0.0)
		sprintf(string, "%.2e", value); 
	else
		sprintf(string, "%.2f", value); 	
	
}


double IW_DLL_CALLCONV
GCI_ImageWindow_GetMaxPixelValueInDisplayedImage(IcsViewerWindow *window)
{
	return window->panel_max_pixel_value;
}

void IW_DLL_CALLCONV
GCI_ImageWindow_SetLinearScale(IcsViewerWindow *window, int bitmode)
{
	//Adjust linear scale when camera bit mode is changed
 	if (!Plugin_IsMenuPathItemChecked(window->linear_scale_plugin, "Display//Linear Scale")) {
 		if (bitmode == 8) {
			((LinearScalePlugin*)(window->linear_scale_plugin))->min_scale_value = 0;
			((LinearScalePlugin*)(window->linear_scale_plugin))->max_scale_value = 255;
 		}
		else {
			((LinearScalePlugin*)(window->linear_scale_plugin))->min_scale_value = window->panel_min_pixel_value;
			((LinearScalePlugin*)(window->linear_scale_plugin))->max_scale_value = window->panel_max_pixel_value;
		}
 	}
	else {
	 	if (bitmode == 8) {
			((LinearScalePlugin*)(window->linear_scale_plugin))->min_scale_value /= 16;
			((LinearScalePlugin*)(window->linear_scale_plugin))->max_scale_value /= 16;
			((LinearScalePlugin*)(window->linear_scale_plugin))->last_min_scale_value /= 16;
			((LinearScalePlugin*)(window->linear_scale_plugin))->last_max_scale_value /= 16;
	 	}
 		else {
			((LinearScalePlugin*)(window->linear_scale_plugin))->min_scale_value *= 16;
			((LinearScalePlugin*)(window->linear_scale_plugin))->max_scale_value *= 16;
			((LinearScalePlugin*)(window->linear_scale_plugin))->last_min_scale_value *= 16;
			((LinearScalePlugin*)(window->linear_scale_plugin))->last_max_scale_value *= 16;
		}
 	}

	SetMinOnPaletteBar(window, ((LinearScalePlugin*)(window->linear_scale_plugin))->min_scale_value);
	SetMaxOnPaletteBar(window, ((LinearScalePlugin*)(window->linear_scale_plugin))->max_scale_value);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetMetaDataProviderCallback(IcsViewerWindow *window, APP_PROVIDED_METADATA_CALLBACK callback, void *callback_data)
{
	MetaDataPlugin *md_plugin = (MetaDataPlugin *)window->metadata_plugin;
	
	if(md_plugin == NULL)
		return;

	md_plugin->app_provided_metadata = callback;
	md_plugin->app_provided_callback_data = callback_data;
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetSaveFileProviderCallback(IcsViewerWindow *window, APP_PROVIDED_SAVE_CALLBACK callback, void *callback_data)
{
	SavePlugin *save_plugin = (SavePlugin *)window->save_plugin;
	
	if(save_plugin == NULL)
		return;

	save_plugin->app_provided_callback = callback;
	save_plugin->app_provided_callback_data = callback_data;
}

void IW_DLL_CALLCONV
GCI_ImageWindow_SetFalseColourWavelength(IcsViewerWindow *window, double wavelength)
{
	palette_tool_set_false_colour_palette((PalettePlugin*) window->palette_plugin, wavelength);            
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetBackgroundColour(IcsViewerWindow *window, COLORREF colour)
{
	ImageViewer_SetBackgroundColour(window->canvas_window, colour);     			
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_ActivateCrossHairTool(IcsViewerWindow *window)
{
	ActivateTool(( Tool*) window->crosshair_tool);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_PlaceCrossHair(IcsViewerWindow *window, const Point point)
{
	POINT p;
	
	p.x = point.x;
	p.y = point.y;
	set_crosshair_viewer_point((CrossHairTool*)window->crosshair_tool, p);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_PlaceCrossHairAtImagePoint(IcsViewerWindow *window, const Point point)
{
	POINT p;
	
	p.x = point.x;
	p.y = point.y;
	set_crosshair_image_point((CrossHairTool*)window->crosshair_tool, p);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_DrawText(IcsViewerWindow *window, COLORREF colour, POINT pt, char *text)
{
	ImageViewer_DrawText(window->canvas_window, CviColourToColorRef(colour), pt, text);
}			  

static void SetupWindowFromMetaData(IcsViewerWindow *window)
{
	float x_extents, y_extents;
	Ics_HistoryIterator it;
	int width;
	char temp1[100], temp2[100];
	char key[ICS_LINE_LENGTH], val[ICS_LINE_LENGTH];

	//FreeImageIcsHistoryIterator iterator;
	char buffer[ICS_LINE_LENGTH], units[ICS_LINE_LENGTH];   
	
	// Store all history strings
	GCI_ImagingWindow_EmptyMetaData(window);
	
	if(!FreeImageIcs_IsIcsFile(window->filename))
		return;
	
	if(FreeImageIcs_GetIcsHistoryStringCount(window->ics) <= 0)
		return;
		
	memset(buffer, 0, ICS_LINE_LENGTH);  
	FreeImageIcs_GetFirstIcsHistoryValueWithKey(window->ics, "binning", buffer);
	
	if(strcmp(buffer, "") == 0)
		window->binning_size = 1;  
	else
		window->binning_size = atoi(buffer);  
		
		
	memset(buffer, 0,ICS_LINE_LENGTH);
	FreeImageIcs_GetFirstIcsHistoryValueWithKey(window->ics, "extents", buffer); 
	
	if( str_empty(buffer) )
		window->microns_per_pixel = 1.0;
	
	sscanf(buffer, "%f %f", &x_extents, &y_extents); 
		
	FreeImageIcs_GetFirstIcsHistoryValueWithKey(window->ics, "units", units);
		
// Cannot use FreeImage calls here as the image may not have been loaded yet
//	if(strncmp(units, "m m", 3) == 0)
//		window->microns_per_pixel = (x_extents * 1e6) / (FreeImage_GetWidth(window->panel_dib) * window->binning_size); 
//	else if(strncmp(units, "um um", 5) == 0)
//		window->microns_per_pixel = x_extents / (FreeImage_GetWidth(window->panel_dib) * window->binning_size);
	
	FreeImageIcs_GetDimensionDetails (window->ics, 0, temp1, temp2, &width);
	if(strncmp(units, "m m", 3) == 0)
		window->microns_per_pixel = (x_extents * 1e6) / (width * window->binning_size); 
	else if(strncmp(units, "um um", 5) == 0)
		window->microns_per_pixel = x_extents / (width * window->binning_size);
	
	if(FreeImageIcs_IcsNewHistoryIterator(window->ics, &it, NULL) != IcsErr_Ok)	 
		return;
	
   	while (it.next >= 0) { 
	   
    	if(FreeImageIcs_IcsGetHistoryStringI(window->ics, &it, buffer)!= IcsErr_Ok)	   // get next string, update iterator 
			continue;     
		
		FreeImageIcs_SplitHistoryString(buffer, key, val);
 
		GCI_ImagingWindow_SetMetaDataKey(window, key, val);
   	}                                            

	return;
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetMicronsPerPixelFactor(GCIWindow *window, double factor)
{
	EventData data1 = NewEventData();
	
	if(window == NULL)
		return;

	if(factor > 0.0) {
		window->microns_per_pixel = factor;
		data1.microns_per_pixel = factor;            
		SEND_EVENT(window, on_microns_per_pixel_changed, data1, NewEventData(), "on_microns_per_pixel_changed")  
	}
}


double IW_DLL_CALLCONV
GCI_ImagingWindow_GetMicronsPerPixelFactor(GCIWindow *window)
{
	return window->microns_per_pixel;	
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetLiveStatus(IcsViewerWindow *window, int status)
{
	if(status)
		window->live_mode = 1;
	else
		window->live_mode = 0;  
	
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_GetLiveStatus(IcsViewerWindow *window)
{
	return window->live_mode;
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_GetPanelID(IcsViewerWindow *window)
{
	return window->panel_id;
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_SetWindowTitle(IcsViewerWindow *window, char *title)
{
	if (window == NULL)
		return;
	
	strcpy(window->panel_user_title, title);
	
	return;
}
 
int IW_DLL_CALLCONV
GCI_ImagingWindow_SetFastDisplay(IcsViewerWindow *window, int fast)
{
	if(fast)
		GCI_ImagingWindow_DisableAllActions(window);
	else
		GCI_ImagingWindow_EnableAllActions(window);

	GCI_ImagingWindow_SetLiveStatus(window, fast);       
	window->just_display = fast;

	return 0;
}

static void FreeImage_SetPalette(FIBITMAP *dib, RGBQUAD *palette)
{
	int i = 0;
	RGBQUAD *pal = NULL;

	if(FreeImage_GetBPP(dib) != 8)
		return;

	// Build a greyscale palette
	pal = FreeImage_GetPalette(dib);

	for (i = 0; i < 256; i++) {
		pal[i].rgbRed = palette[i].rgbRed;
		pal[i].rgbGreen = palette[i].rgbGreen;
		pal[i].rgbBlue = palette[i].rgbBlue;
	}
}

void IW_DLL_CALLCONV
ImageWindow_SetPalette(IcsViewerWindow *window) 
{
	// Set the palette of the freeimage as well as just the display
	FreeImage_SetPalette(window->panel_dib, window->current_palette);

	ImageViewer_SetPalette(window->canvas_window, window->current_palette, 256); 
	ImageViewer_Redraw(window->canvas_window);
	
	SEND_EVENT(window, on_palette_changed, NewEventData(), NewEventData(), "on_palette_changed")  
}



void IW_DLL_CALLCONV
GCI_ImagingWindow_SetBinningSize(GCIWindow *window, int size)
{
	EventData data1 = NewEventData(); 
	EventData data2 = NewEventData();
	
	if(size <= 0) 
		return;
	
	// Size has not changes. Do nothing.
	if(size == window->binning_size)
		return;
	
	data1.binning = window->binning_size;  
	
	window->binning_size = size;
		
	data2.binning = window->binning_size;  
	
	SEND_EVENT(window, on_binning_changed, data1, data2, "on_binning_changed")   
}

HWND IW_DLL_CALLCONV
GCI_ImagingWindow_GetImageViewHandle(IcsViewerWindow *window)
{
	return window->canvas_window; 
}

HWND IW_DLL_CALLCONV
GCI_ImagingWindow_GetWindowHandle(IcsViewerWindow *window)
{
	int window_handle;

	GetPanelAttribute (window->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);
 
	return (HWND) window_handle;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetZoomFactor(IcsViewerWindow *window, double zoom)
{
	ImageViewer_SetZoom(window->canvas_window, zoom); 
}



void IW_DLL_CALLCONV
GCI_ImagingWindow_SetResizeFitStyle(IcsViewerWindow *window)
{
	ImageViewer_EnableZoomToFit(window->canvas_window);
}


FIBITMAP* IW_DLL_CALLCONV
GCI_ImagingWindow_GetOriginalFIB( IcsViewerWindow *window)
{
	FIBITMAP *dib = FreeImage_Clone(window->panel_dib);

	if(FreeImage_GetBPP(window->panel_dib) == 8)
		FIA_CopyPalette(window->panel_dib, dib);

	return dib;
} 


FIBITMAP* IW_DLL_CALLCONV
GCI_ImagingWindow_GetDisplayedFIB( IcsViewerWindow *window)
{
	return window->panel_dib;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetDefaultDirectoryPathForAllWindows(const char *path)
{
	strcpy(global_default_directory, path);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetDefaultDirectoryPath(IcsViewerWindow *window, const char *path)
{
	strcpy(window->default_directory, path);
}

char*
GetDefaultDirectoryPath(IcsViewerWindow *window, char *path)
{
	if(strlen(global_default_directory) > 1) {
		strcpy(path, global_default_directory);
		return path;
	}

	if(strlen(window->default_directory) > 1) {
		strcpy(path, window->default_directory);
		return path;
	}

	path[0] = '\0';

	return path;
}

char* IW_DLL_CALLCONV
GCI_ImagingWindow_GetDefaultDirectoryPath(IcsViewerWindow *window, char *path)
{
	return GetDefaultDirectoryPath(window, path);
}

static void OnMetaDataIterate (dictionary * d, const char *key, const char *val, void *data)
{
	ICS *ics = (ICS *) data; 
	char history_string[ICS_LINE_LENGTH];

	// Labels gets save by FreeImageIcs Library
	if(strcmp(key, "labels") == 0)
		return;

	FreeImageIcs_JoinKeyValueIntoHistoryString(history_string, key, val);

	if(FreeImageIcs_IcsAddHistoryString (ics, NULL, history_string) != IcsErr_Ok) {
		GCI_MessagePopup("Error", "Error Saving History");
		return;
	}
}

void SaveMetaDataToImage(IcsViewerWindow *window, const char *filepath)
{
	int size;
	ICS *ics;
	char mode[3] = "rw";
	
	if(FileExists(filepath, &size)) {
		strncpy(mode, "rw", 3);
	}
	
	if(FreeImageIcs_IcsOpen (&ics, filepath, mode) != IcsErr_Ok) {
		GCI_MessagePopup("Error", "Error Opening Ics file for metadata write.");
		return;
	}  
	
	FreeImageIcs_IcsSetNativeScale(ics, 0, 0.0, window->microns_per_pixel, "microns");
	FreeImageIcs_IcsSetNativeScale(ics, 1, 0.0, window->microns_per_pixel, "microns");

	CmtGetLock(window->lock);

	dictionary_foreach(window->metadata, OnMetaDataIterate, ics);

	CmtReleaseLock(window->lock);

//FINISH:

	if(FreeImageIcs_IcsClose(ics) != IcsErr_Ok)
    {
		GCI_MessagePopup("Error", "Error Closing File");
		return;
	}
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_GetImageBPP(IcsViewerWindow *window)
{
	int bpp;
	FREE_IMAGE_TYPE type;
	
	if(window->panel_dib == NULL)
		return 0;

	bpp = FreeImage_GetBPP(window->panel_dib);
	
	type = FreeImage_GetImageType(window->panel_dib);
	
	switch (bpp) {
	
		case 16:
		{
			if(type == FIT_INT16) {
			
				if(window->panel_min_pixel_value >= BIT12_MIN && window->panel_max_pixel_value <= BIT12_MAX)  
					return 12;
					
				if(window->panel_min_pixel_value >= BIT14_MIN && window->panel_max_pixel_value <= BIT14_MAX)  
					return 14;
					
				return 16;
			}
			
			if(type == FIT_UINT16) {
			
				if(window->panel_max_pixel_value <= U12BIT_MAX)  
					return 12;
					
				if(window->panel_max_pixel_value <= U14BIT_MAX)  
					return 14;
					
				return 16;
			}
			
			break;
		}	
	}

	return bpp;
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_GetMaxPossibleValuesForDataType(IcsViewerWindow *window, double *min, double *max)     
{

	FREE_IMAGE_TYPE type = FreeImage_GetImageType(window->panel_dib);

	switch(type)
	{
		case FIT_BITMAP:
		{
			*min = 0;
			*max = 255;
			
			break; 
		}

		case FIT_UINT16:
		{
			*min = 0;
			*max = USHRT_MAX;
			
			break;     
		}

		case FIT_INT16:
		{
			if(window->panel_min_pixel_value >= BIT12_MIN && window->panel_max_pixel_value <= BIT12_MAX) { 
				*max = BIT12_MAX;
				*min = BIT12_MIN;
				
				break;
			}
					
			if(window->panel_min_pixel_value >= BIT14_MIN && window->panel_max_pixel_value <= BIT14_MAX) { 
				*max = BIT14_MAX;
				*min = BIT14_MIN;
				
				break;  
			}  
			
			if(window->panel_min_pixel_value >= SHRT_MIN && window->panel_max_pixel_value <= SHRT_MAX) { 
				*max = SHRT_MAX;
				*min = SHRT_MIN;
				
				break;  
			}
		}

		case FIT_FLOAT:
		{
			*min = window->panel_min_pixel_value;
			*max = window->panel_max_pixel_value;
			
			break;     
		}
	}

	return;
}


static void ValidatePlugin(IcsViewerWindow *window, FIBITMAP *dib, FIBITMAP *original_dib)
{
	int i, valid = 0;
	ImageWindowPlugin *plugin = NULL; 
	EventData data1 = NewEventData();
	EventData data2 = NewEventData();   
	
	for(i=1; i <= window->number_of_plugins; i++) { 
	
		plugin = GetPluginPtrInList(window, i); 
		
		if(dib == NULL) {
			Plugin_DimAndUncheckMenuItems (plugin);	
			continue;
		}
		
		if(plugin->vtable->on_validate_plugin == NULL) {
			Plugin_DimAndUncheckMenuItems (plugin);
		}
		else {
		
			data1.dib = dib;
			data2.dib = original_dib;
			
			valid = plugin->vtable->on_validate_plugin(plugin, data1, data2);
			
			if(valid)
				Plugin_UnDimMenuItems(plugin);
			else
				Plugin_DimAndUncheckMenuItems (plugin);	
		}
	} 
}



	
void IW_DLL_CALLCONV
GCI_ImagingWindow_SetLinearScaleLimits(IcsViewerWindow *window, double min, double max)
{
	Plugin_CheckMenuPathItem(window->linear_scale_plugin, "Display//Linear Scale");
	((LinearScalePlugin*)(window->linear_scale_plugin))->min_scale_value = min;
	((LinearScalePlugin*)(window->linear_scale_plugin))->max_scale_value = max;   	
}
	

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetFramesPerSecondIndicator(IcsViewerWindow *window, double fps)
{
	window->fps = fps;	
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DiableSoftwareBinning(IcsViewerWindow *window, FIA_BINNING_TYPE binning_type, int radius)
{
	window->binning_enabled = 0;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetSoftwareBinning(IcsViewerWindow *window, FIA_BINNING_TYPE binning_type, int radius)
{
	if(radius < 3)
		radius = 3;

	if(radius > 21)
		radius = 21;

	if(binning_type < FIA_BINNING_SQUARE)
		binning_type = FIA_BINNING_SQUARE;

	if(binning_type > FIA_BINNING_GAUSSIAN)
		binning_type = FIA_BINNING_GAUSSIAN;

	window->binning_enabled = 1;
	window->binning_type = binning_type;
	window->binning_radius = radius;
}

void 
DisplayImage(IcsViewerWindow *window, FIBITMAP *dib) 
{
	FIBITMAP *standard_dib = NULL;
	EventData data1 = NewEventData(); 
	int bpp, scale=1, linearscale_active;
	double scale_min, scale_max, new_time;
	//double seconds_passed;
	
	if(dib == NULL)
		return;
	
	window->fps_count++;  
	
	new_time = Timer();
	
	PROFILE_START("DisplayImage - Limit framerate");   

	if(window->live_mode) {
		
		// We limit the display frame rate 
		if((new_time - window->last_display_time) < MIN_TIME_PER_FRAME) {
	
			PROFILE_STOP("DisplayImage - Limit framerate");   
			return;
		}
	}
	
	PROFILE_STOP("DisplayImage - Limit framerate");   

	assert(window->canvas_window_hdc != NULL);
	
	PROFILE_START("DisplayImage - Linear Scaling");   
	
	bpp = FreeImage_GetBPP(window->panel_dib);
	linearscale_active = Plugin_IsMenuPathItemChecked(window->linear_scale_plugin, "Display//Linear Scale");

	scale = linearscale_active;
	
	if(window->linear_scale_plugin == NULL)
		scale = 0;

	// If colour don't scale
	if(!FIA_IsGreyScale(dib))
		scale = 0;
	
	// If not linearscale active and 8bit greyscale we don't scale
	if(!linearscale_active && bpp == 8)
		scale = 0;
	
	if(window->linear_scale_plugin != NULL) {
		scale_min = ((LinearScalePlugin*)(window->linear_scale_plugin))->min_scale_value;
		scale_max = ((LinearScalePlugin*)(window->linear_scale_plugin))->max_scale_value;
	}

	if(scale) {
		
		// Here with have a greyscale image >= 16 bits but scale is active
		
		standard_dib = FIA_LinearScaleToStandardType(dib, scale_min, scale_max,
			&(window->panel_min_pixel_value), &(window->panel_max_pixel_value));  	
		
	}
	else {
		
		// We are not scaling
		
		// We have a non 8bit greyscale image but we are not linearscaling so 
		// We have to convert to 8bit image using autoscale over full range.
		if(FIA_IsGreyScale(dib) && bpp > 8) {
			
			standard_dib = FIA_LinearScaleToStandardType(dib, 0, 0,
				&(window->panel_min_pixel_value), &(window->panel_max_pixel_value));  		
		}
		else if(FIA_IsGreyScale(dib) == 0 && bpp >= 48) {
			// Here we have 8bit greyscale or a colour image.
			standard_dib = FIA_Convert48BitOr64BitRGBTo24BitColour(dib); 
			window->panel_min_pixel_value = 0;
			window->panel_max_pixel_value = 255;
		}
		else {
			// Here we have 8bit greyscale or a colour image.
			PROFILE_START("DisplayImage - FreeImage_Clone");   

			standard_dib = FreeImage_Clone(dib); 

			PROFILE_STOP("DisplayImage - FreeImage_Clone");   

			window->panel_min_pixel_value = 0;
			window->panel_max_pixel_value = 255;
		}
	}
	
	if(standard_dib == NULL)
		return;

	PROFILE_STOP("DisplayImage - Linear Scaling"); 
	
	PROFILE_START("DisplayImage - ValidatePlugin");    

	ValidatePlugin(window, standard_dib, dib); 

	PROFILE_STOP("DisplayImage - ValidatePlugin");    

	PROFILE_START("DisplayImage - FIA_CopyPaletteFromRGBQUAD");    

	FIA_CopyPaletteFromRGBQUAD(standard_dib, window->current_palette);
	
	PROFILE_STOP("DisplayImage - FIA_CopyPaletteFromRGBQUAD");

	PROFILE_START("DisplayImage - FIA_GetDibSection");

	PROFILE_START("DisplayImage - CleanUp");     
	
	if(window->hbitmap != NULL) {
		DeleteObject(window->hbitmap);
		window->hbitmap = NULL;		
	}
	
	PROFILE_STOP("DisplayImage - CleanUp");   

	window->hbitmap = FIA_GetDibSection(standard_dib, window->canvas_window_hdc, 0, 0,
						FreeImage_GetWidth(dib), FreeImage_GetHeight(dib));
				
	if(window->hbitmap == NULL) {
		return;
	}

	PROFILE_STOP("DisplayImage - FIA_GetDibSection");

	PROFILE_START("ImageViewer_SetImage");
	
	ImageViewer_SetImage(window->canvas_window, window->hbitmap); 
	
	window->last_display_time = Timer();
	
	PROFILE_STOP("ImageViewer_SetImage");  
	
	data1.dib = standard_dib;
	
	PROFILE_START("on_image_displayed event");  
	
	SEND_EVENT(window, on_image_displayed, data1, NewEventData(), "on_image_displayed") 
	
	PROFILE_STOP("on_image_displayed event");       
	
	FreeImage_Unload(standard_dib); 

	ProcessDrawEvents();
}


int
GCI_ImagingWindow_LoadImageAdvanced(IcsViewerWindow *window, FIBITMAP *dib, int reset_multidimensional_data)
{
	EventData data1 = NewEventData();   
	
	if(dib == NULL)
		return 0;
		
	if(window->prevent_image_load)
		return 1;
	
	// Reset multidensional panel data.
	if(reset_multidimensional_data) {
		
		window->mutidimensional_data = 0;
		
		if(window->multidimension_panel_id > 0) {
			DiscardPanel(window->multidimension_panel_id);
			window->multidimension_panel_id = 0;
			
			// Cleanup 3d files
			if(window->importer3d != NULL) {
				manipulater3d_deconstructor(window->importer3d); 
				window->importer3d = NULL;
			}
		}
		
		SetMenuBarAttribute (window->panel_menu->menubar_id, window->panel_menu->multidimensional_menu_item_id, ATTR_DIMMED, 1);     
	}
	
	// Clean up old bitmaps 
	if(window->panel_dib != NULL) {
		FreeImage_Unload(window->panel_dib);
		window->panel_dib = NULL;
	}	

	// Just do a fast a display as possible, ie no plugins or image copies
	if(window->just_display) {
		DisplayImage(window, dib);
		return 1;
	}
	
	if(dib == NULL | dib->data == NULL)
		return 0;

	if(!FreeImage_HasPixels(dib))  // extra last minute check here
		return 0;
		
	window->panel_dib = FreeImage_Clone(dib);    

	if(window->panel_dib == NULL)
		return 0;

	if(FreeImage_GetBPP(dib) == 8)
		FIA_CopyPalette(dib, window->panel_dib);

	data1.dib = window->panel_dib;    
	
	PROFILE_START("GCI_ImagingWindow_LoadImage-on_image_load");     

	SEND_EVENT(window, on_image_load, data1, NewEventData(), "on_image_load")    
	
	PROFILE_STOP("GCI_ImagingWindow_LoadImage-on_image_load");     


	PROFILE_START("GCI_ImagingWindow_LoadImage-Displaying Image");     
	
	DisplayImage(window, window->panel_dib);
	
	PROFILE_STOP("GCI_ImagingWindow_LoadImage-Displaying Image");    

	return 1;
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_LoadImage(IcsViewerWindow *window, FIBITMAP *dib)
{
	int ret;
	
	PROFILE_START("GCI_ImagingWindow_LoadImage");
	
	ret = GCI_ImagingWindow_LoadImageAdvanced(window, dib, 1); 
	
	PROFILE_STOP("GCI_ImagingWindow_LoadImage");
	
	return ret;
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_LoadImageFile(IcsViewerWindow *window, char *image_path)
{
    char ext[10], order[50] = "", label[50] = "";
	int number_of_dims, dim_multi_dimension_panel = 0;
    FIBITMAP *temp_dib;

	strcpy(window->filename, image_path);
	
	get_file_extension(window->filename, ext); 
	
	// Update the specific window default directory with the latest
	// file location the user has chosen.
	GetDirectoryForFile(window->filename, window->default_directory);   
	
	// Turn off live 
	GCI_ImagingWindow_SetLiveStatus(window, 0);         
	
	// Tell plugins file from disk has been loaded.
	SEND_EVENT(window, on_disk_file_loaded, NewEventData(), NewEventData(), "on_image_load")       	
		
	window->mutidimensional_data = 0;

	if(strcmp(ext, ".ics") == 0) {
		
		if(FreeImageIcs_IcsOpen (&(window->ics), window->filename, "r") != IcsErr_Ok)
			return -1;
		
        PROFILE_START("GCI_ImagingWindow_LoadImage-SetupWindowFromMetaData");    
		
	    SetupWindowFromMetaData(window);    
	
	    PROFILE_STOP("GCI_ImagingWindow_LoadImage-SetupWindowFromMetaData");   

		number_of_dims = FreeImageIcs_NumberOfDimensions (window->ics);
		
		// We must check the multiple dimension ics file
		// is not just a colour image
		FreeImageIcs_GetLabelForDimension(window->ics, 2, label);

		// If multi dimensional and not colour
		if(number_of_dims > 2 && strncmp(label, "c", 1) != 0) {

			if(window->importer3d != NULL) {
				manipulater3d_deconstructor(window->importer3d); 
				free(window->importer3d);  
				window->importer3d = NULL;
			}
			
			window->importer3d = get_3d_importer_for_file(window, window->filename); 
			
			if(window->importer3d == NULL) {
				GCI_MessagePopup("Error", "3d file format not supported.");
				return -1;
			}
			
			manipulater3d_load_multidimensional_data (window->importer3d, window->filename);

			manipulater3d_show_multidimensional_dialog (window->importer3d);

			window->mutidimensional_data = 1;
			
			dim_multi_dimension_panel = 0;

			return 1;
		}
		else {
			
			dim_multi_dimension_panel = 1;

			temp_dib = FreeImageIcs_LoadFIBFromIcsFile (window->ics);   
			
			FreeImageIcs_IcsClose(window->ics);   
		}
	}
	else	
		temp_dib = FIA_LoadFIBFromFile(window->filename);  

	GCI_ImagingWindow_LoadImage(window, temp_dib); 

	temp_dib = NULL;
	
	SetMenuBarAttribute(window->panel_menu->menubar_id, window->panel_menu->multidimensional_menu_item_id, ATTR_DIMMED, dim_multi_dimension_panel);
		
	window->interpret_3_dims_as_colour = 1;

	return 1;
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_ShowToolBar(IcsViewerWindow *window)
{
	int toolbar_status;
	int width, height;
	
	GetMenuBarAttribute (window->panel_menu->menubar_id, window->panel_menu->show_toolbar_menu_item_id, ATTR_CHECKED, &toolbar_status);

	if(toolbar_status == 1)
		return;

	ShowTool((Tool *)window->crosshair_tool);

	SetMenuBarAttribute (window->panel_menu->menubar_id, window->panel_menu->show_toolbar_menu_item_id, ATTR_CHECKED, 1);
	
	GetPanelAttribute(window->panel_id, ATTR_WIDTH, &width);
	GetPanelAttribute(window->panel_id, ATTR_HEIGHT, &height);    
	
	SendMessage(window->panel_window_handle, WM_SIZE, 0, MAKELPARAM(width, height));   
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_HideToolBar(IcsViewerWindow *window)
{
	int toolbar_status;
	int width, height;

	GetMenuBarAttribute (window->panel_menu->menubar_id, window->panel_menu->show_toolbar_menu_item_id, ATTR_CHECKED, &toolbar_status);

	if(toolbar_status == 0)
		return;

	SetMenuBarAttribute (window->panel_menu->menubar_id, window->panel_menu->show_toolbar_menu_item_id, ATTR_CHECKED, 0);
	
	// Unfortunately we can hide all the toolbar controls by expanding the image display control
	// You can still see part of the crosshair label
	// So I hide it here
	HideTool((Tool *)window->crosshair_tool);

	GetPanelAttribute(window->panel_id, ATTR_WIDTH, &width);
	GetPanelAttribute(window->panel_id, ATTR_HEIGHT, &height);    
	
	SendMessage(window->panel_window_handle, WM_SIZE, 0, MAKELPARAM(width, height));   
}


static void GCI_ImagingWindow_Init(IcsViewerWindow *window)
{
	/* Setup window defaults */ 
	GCI_ImagingWindow_CreateAllSignals(window);
	
	window->cRef = 0;
	window->panel_id = 0;
	window->about_panel_id = 0;
	window->multidimension_panel_id = 0;
	window->panel_window_handle = NULL;
	window->importer3d = NULL;
	window->interpret_3_dims_as_colour = 1;
	window->metadata = NULL;

	window->ics = NULL;   
	
	window->hbitmap = NULL;
	
	window->canvas_window = NULL;
	window->prevent_image_load = 0;
	window->live_mode = 0;
	window->cropping = 0;
	window->is_moving = 0;
	
	window->current_palette = malloc(sizeof(RGBQUAD) * 256);
	FIA_GetGreyLevelPalette(window->current_palette); 
	
	window->number_of_plugins = 0;
	
	window->image_view_left = 80;
	window->image_view_top = 30;
	
	GciCmtNewLock("IcsViewerLock", 0, &(window->lock));
	
	window->panel_window_visible = 0;
	window->panel_toolbar_width = 0;
	window->panel_palettebar_status = 0; 
	window->microns_per_pixel = 1.0;

	window->panel_dib = NULL;
	window->signal_on_exit = 0;
	
	window->binning_size = 1;
	
	window->no_crosshair_tool = 0;
	window->no_roi_tool = 0;
	
	window->finished_loaded = 0; 
	
	window->tool_button_top = 60;
	
	window->cursor_type = CURSOR_NORMAL;
	
	window->last_display_time = Timer();
	window->fps_count = 0;
	
	window->mutidimensional_data = 0;
	window->last_3d_dimension1_shown = 0;
	window->last_3d_dimension2_shown = 1;      
	window->last_3d_slice_shown = 0;
	window->last_dimension_moved = 2;
	
	// Get temp dir
	if(!GetEnvironmentVariable("Temp", window->temp_dir_path, 500)) {
		
		GCI_MessagePopup("Error", "Can not create temporary directory");
	}
	
	window->stream_plugin = NULL;

	// This also creates a new metadata dictionary
	GCI_ImagingWindow_EmptyMetaData(window);
	
	window->program_plugins = ListCreate (sizeof(ImageWindowPlugin *)); 
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_Show(IcsViewerWindow *window)
{
	int panel_visible = 0, width, height, left, top;
	
	if(window == NULL)
		return;
		
	if(window->panel_id <= 0)
		return;
	
	GetPanelAttribute(window->panel_id, ATTR_VISIBLE, &panel_visible);

	if(!panel_visible) {

		DisplayPanel(window->panel_id);

		GetPanelAttribute(window->panel_id, ATTR_WIDTH, &width);
		GetPanelAttribute(window->panel_id, ATTR_HEIGHT, &height);    
	
		GetPanelAttribute(window->panel_id, ATTR_LEFT, &left);
		GetPanelAttribute(window->panel_id, ATTR_TOP, &top);  

		PostMessage(window->panel_window_handle, WM_SIZE, 0, MAKELPARAM(width, height)); 
	}
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_BringToFront(IcsViewerWindow *window)
{
	if(window == NULL)
		return;

	DisplayPanel(window->panel_id);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_Hide(IcsViewerWindow *window)
{
	int panel_visible;

	GetPanelAttribute(window->panel_id, ATTR_VISIBLE, &panel_visible);
	
	if(panel_visible == 1)
		HidePanel(window->panel_id);
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_IsVisible(IcsViewerWindow *window)
{
	int panel_visible;

	GetPanelAttribute(window->panel_id, ATTR_VISIBLE, &panel_visible);
	
	return panel_visible;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SignalOnlyOnCloseOrExit(IcsViewerWindow *window)
{
	window->signal_on_exit = 1;	
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DestroyWindow(IcsViewerWindow *window)
{
	#ifndef MICROSCOPE_MODE      
		ics_viewer_registry_save_panel_size_position(window, window->panel_id);
	#endif
		
	// DiscardPanel(window->panel_id); does not post WM_DESTROY from menu exit ?
	DestroyWindow(window->panel_window_handle);
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_Close(IcsViewerWindow *window)
{
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "Close", GCI_VOID_POINTER, window);      
	
	if(window->signal_on_exit != 1)
		GCI_ImagingWindow_DestroyWindow(window);
	
	return 1;
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_EmptyMetaData(IcsViewerWindow *window)
{
	CmtGetLock(window->lock);

	if(window->metadata != NULL) {
		dictionary_del(window->metadata);
		window->metadata = NULL;
	}

	window->metadata = dictionary_new(80);

	CmtReleaseLock(window->lock);

	return 1;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetMetaData(IcsViewerWindow *window, dictionary *metadata)
{
	CmtGetLock(window->lock);

	if(window->metadata != NULL) {
		dictionary_del(window->metadata);
		window->metadata = NULL;
	}

	window->metadata = metadata; // dictionary_clone(metadata);

	CmtReleaseLock(window->lock);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetMetaDataKey(IcsViewerWindow *window, const char *key, const char *value)
{
	CmtGetLock(window->lock);

	dictionary_set(window->metadata, key, (char*) value);

	CmtReleaseLock(window->lock);
}

char* IW_DLL_CALLCONV
GCI_ImagingWindow_GetMetaDataKey(IcsViewerWindow *window, const char *key, char *value, char *def)
{
	char *val;

	CmtGetLock(window->lock);

	val = dictionary_get(window->metadata, key, def);

	strcpy(value, val);

	CmtReleaseLock(window->lock);

	return value;
}

static void dictionary_text_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	FILE *fp = (FILE *) data;
	
	fprintf(fp, "%s\t%s\n", key, val);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataDictionaryToTextFile(GCIWindow *window, dictionary* d, const char *filename)
{
	FILE *fp;
	
	CmtGetLock(window->lock);

	if((fp = fopen(filename, "w")) == NULL)
		return;
	
	dictionary_foreach(d, dictionary_text_keyval_callback, fp);
	
	fclose(fp);
	
	CmtReleaseLock(window->lock);

	return;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataToTextFile(GCIWindow *window, const char *filename)
{
	GCI_ImagingWindow_SaveMetaDataDictionaryToTextFile(window, window->metadata, filename);
}


static void dictionary_ics_keyval_callback (dictionary * d, const char *key, const char *val, void *data)
{
	ICS *ics = (ICS *) data;
	
	FreeImageIcs_IcsAddHistoryString (ics, key, val);         	
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataDictionaryToIcsFile(GCIWindow *window, dictionary* d, ICS *ics)
{
	CmtGetLock(window->lock);

	FreeImageIcs_IcsSetNativeScale(ics, 0, 0.0, window->microns_per_pixel, "microns");
	FreeImageIcs_IcsSetNativeScale(ics, 1, 0.0, window->microns_per_pixel, "microns");

	dictionary_foreach(d, dictionary_ics_keyval_callback, ics);
	
	CmtReleaseLock(window->lock);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataToIcsFile(GCIWindow *window, ICS *ics)
{
	GCI_ImagingWindow_SaveMetaDataDictionaryToIcsFile(window, window->metadata, ics);
	return;
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataDictionaryToIcsFilePath(GCIWindow *window, dictionary* d, const char *filepath)
{
	ICS *ics=NULL;
	char ext[50] = "";

	get_file_extension(filepath, ext);

	if(strcmp(ext, ".ics"))
		return;
	
	if (FreeImageIcs_IcsOpen (&ics, filepath, "rw") == IcsErr_Ok){

		GCI_ImagingWindow_SaveMetaDataDictionaryToIcsFile(window, d, ics);

		FreeImageIcs_IcsClose (ics);   
	}
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SaveMetaDataToIcsFilePath(GCIWindow *window, const char *filepath)
{
	GCI_ImagingWindow_SaveMetaDataDictionaryToIcsFilePath(window, window->metadata, filepath);
	return;
}

int Window_Destroy(IcsViewerWindow *window)
{
	int id;
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(window), "Destroy", GCI_VOID_POINTER, window);    
	
	DestroyAllPlugins(window);
	
 	/* Restore the original windows procedure function pointers */
	SetWindowLongPtr (window->panel_window_handle, GWL_WNDPROC, window->panel_original_proc_fun_ptr);
	SetWindowLongPtr (window->canvas_window, GWL_WNDPROC, window->canvas_original_proc_fun_ptr);

	if(window->panel_dib != NULL) {
		
		FreeImage_GetBPP(window->panel_dib); 
		
		FreeImage_Unload(window->panel_dib);
		window->panel_dib = NULL;
	}
	
	GCI_SignalSystem_Destroy(UIMODULE_SIGNAL_TABLE(window));   
	
	ReleaseDC(window->canvas_window, window->canvas_window_hdc);     

	// Delete any left over hbitmap
	if(window->hbitmap != NULL) {
		DeleteObject(window->hbitmap);
		window->hbitmap = NULL;		
	}

	// Discard dimesional data panel.	
	if(window->multidimension_panel_id > 0) {
		DiscardPanel(window->multidimension_panel_id);
		window->multidimension_panel_id = 0;
	}
	
	if(window->current_palette != NULL) {
		free(window->current_palette);
		window->current_palette = NULL;
	}
		
	free(window->panel_menu);
	window->panel_menu = NULL;
	
	if(window->metadata != NULL) {
		dictionary_del(window->metadata);
		window->metadata = NULL;
	}

	ListDispose (window->program_plugins); 
	
	id = window->panel_id;
	
	DiscardPanel(window->panel_id);

	free(window);
	window = NULL;

	if(window_destroyed_handler != NULL)
		window_destroyed_handler(id, window_destroyed_callback_data);  	
	
	return GCI_IMAGING_SUCCESS;
}


static void __cdecl GCI_ImagingWindow_OnToolbarToggle(int menuBar, int menuItem, void *callbackData, int panel) 
{
	int toolbar_status;

	IcsViewerWindow *window = (IcsViewerWindow *) callbackData;
	
	GetMenuBarAttribute (window->panel_menu->menubar_id, window->panel_menu->show_toolbar_menu_item_id, ATTR_CHECKED, &toolbar_status);
	
	if(toolbar_status == 1) {
	
		GCI_ImagingWindow_HideToolBar(window);;
	}
	else {
	
		GCI_ImagingWindow_ShowToolBar(window);
	}
}


static void CVICALLBACK OnWindowExit (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	IcsViewerWindow *window = (IcsViewerWindow *) callbackData;        
	
	PostMessage(window->panel_window_handle, WM_SYSCOMMAND, (WPARAM) SC_CLOSE, 0);
}


static void CVICALLBACK onImageOpen (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	char *default_extensions = "*.ics;*.jpg;*.png;*.tif;*.bmp;*.exr;*.cr2;*.crw;*.gif;*.pfm;*.pgm;*.ppm;*.psd;*.ico";

	char fname[GCI_MAX_PATHNAME_LEN]; 
	char directory[GCI_MAX_PATHNAME_LEN] = "";
	
	IcsViewerWindow *parent_window = (IcsViewerWindow *) callbackData;
	IcsViewerWindow *window;

	#ifdef OPEN_IN_SAME_WINDOW
	
		window = (IcsViewerWindow *) callbackData;    
	
	#else
	
		if ( (window = GCI_ImagingWindow_Create("ImageViewer", "Image")) == NULL ) {
			 
			GCI_MessagePopup("Error", "Can not create window");
		
			return;
		}
		
		GCI_ImagingWindow_Initialise(window);
		
		// As this window is open from another IcsViewer window we copy over the default directory toi the new window.
		strncpy(window->default_directory, parent_window->default_directory, 499);

	#endif
	
	if (LessLameFileSelectPopup (window->panel_id, GetDefaultDirectoryPath(window, directory),
		"*", default_extensions, "Load Image", VAL_OK_BUTTON, 0, 0, 1, 1, fname) <= 0) {
		return;
	}

	// Set new default directory basewd on user selection.
	GetDirectoryForFile(fname, directory);
	strncpy(window->default_directory, directory, 499);

	// Get the directory for fname and save it in the registry. As we only that directory by default next time.
	checkRegistryValueForString (1, REGKEY_HKCU, REGISTRY_SUBKEY, "DefaultDir", directory);
	
	GCI_ImagingWindow_LoadImageFile(window, fname); 
	
	GCI_ImagingWindow_Show(window); 
}


static int CVICALLBACK OnAboutPanelClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	IcsViewerWindow *window = (IcsViewerWindow *) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			DiscardPanel(window->about_panel_id);
			window->about_panel_id = 0;
			break;
		}
	}
	
	return 0;
}


static void __cdecl GCI_ImagingWindow_OnAboutShow(int menuBar, int menuItem, void *callbackData, int panel) 
{
	IcsViewerWindow *window = (IcsViewerWindow *) callbackData; 
	FILE *fp;
	char line[200];
	char version_file_path[400];  
	
	if(window->about_panel_id == 0) {
	
		window->about_panel_id = LoadPanel(0, uir_file_path, ABOUT_PNL);    
		
		/* Setup the cancel call back */
		if ( InstallCtrlCallback (window->about_panel_id, ABOUT_PNL_CLOSE_BUTTON, OnAboutPanelClose, window) < 0)
			return;
		
		ResetTextBox (window->about_panel_id, ABOUT_PNL_TEXTBOX, "");
	
		find_resource("version.txt", version_file_path);   
		
		if((fp = fopen (version_file_path, "r")) == NULL)
			return;
		 
		while(fgets(line, 200, fp) != NULL) 
 			SetCtrlVal (window->about_panel_id, ABOUT_PNL_TEXTBOX, line);

		fclose(fp);
	}
		
	DisplayPanel(window->about_panel_id);     
}

						  
static int BuildMenuBar(IcsViewerWindow *window)
{
	int zoom_id, display_id, processing_id;
	
	window->panel_menu->menubar_id = NewMenuBar (window->panel_id);
	
	window->panel_menu->file_menu_id = NewMenu (window->panel_menu->menubar_id, "File", -1);   
	
	window->panel_menu->view_menu_id = NewMenu (window->panel_menu->menubar_id, "View", -1);   
	
	window->panel_menu->option_menu_id = NewMenu (window->panel_menu->menubar_id, "Options", -1);
	
	window->panel_menu->open_menu_item_id = NewMenuItem (window->panel_menu->menubar_id, window->panel_menu->file_menu_id, 
					 					   "Open", -1, VAL_MENUKEY_MODIFIER | 'O' ,
					 					   onImageOpen, window);

	window->panel_menu->show_toolbar_menu_item_id = NewMenuItem (window->panel_menu->menubar_id, window->panel_menu->option_menu_id, 
					 					   "Toolbar", -1, VAL_MENUKEY_MODIFIER | 'T' ,
					 					   GCI_ImagingWindow_OnToolbarToggle, window);
					 		
	SetMenuBarAttribute (window->panel_menu->menubar_id, window->panel_menu->show_toolbar_menu_item_id, ATTR_CHECKED, 1);
					 			
	display_id = NewMenu (window->panel_menu->menubar_id, "Display", -1);

	processing_id = NewMenu (window->panel_menu->menubar_id, "Processing", -1);

	zoom_id= NewMenu (window->panel_menu->menubar_id, "Zoom", -1);
					 				
	window->panel_menu->help_menu_id = NewMenu (window->panel_menu->menubar_id, "Help", -1); 
	
	window->panel_menu->about_menu_item_id = NewMenuItem (window->panel_menu->menubar_id, window->panel_menu->help_menu_id, 
					 					   "About ...", -1, 0, GCI_ImagingWindow_OnAboutShow, window);
	
	return GCI_IMAGING_SUCCESS;
}

static void SetupDefaultDirectory(IcsViewerWindow *window)
{
	int real_string_size = 0;

	// If the global default directory has been set for all windows we simply initialise to that.
	// The specific window default directory can be chnaged after this.
	// This function is only called once on window initialise.
	if(strlen(global_default_directory) > 1) {
		strncpy(window->default_directory, global_default_directory, 499);
		return;
	}

	if(RegReadString (REGKEY_HKCU, REGISTRY_SUBKEY, "DefaultDir", window->default_directory,
		499, &real_string_size) < 0) {

		str_get_path_for_my_documents(window->default_directory);
	}
}
  	
static void
GCI_ImagingWindow_SetResourceSearchPath(void)
{
  	char tmp[500];
	
	find_resource("icsviewer_uir.uir", uir_file_path);  
	
	find_resource("pin_in.bmp", tmp);       

	// Get the directory path containing "pin_in.bmp"
	GetDirectoryForFile(tmp, icon_file_path); 
}

/*
static void panel_read_or_write_registry_settings(int panel_id, int write)
{
	char buffer[500], panel_define[100], remote_session = 0;
	int visible;

	if(panel_id <= 0)
		return;

	remote_session = GetSystemMetrics(SM_REMOTESESSION);

	if(remote_session)
		return;

	// load or save panel positions
	
	// Get the panel define as a module may have more than one panel
	GetPanelAttribute(panel_id, ATTR_CONSTANT_NAME, panel_define);
	
	sprintf(buffer, "software\\GCI\\Microscope\\PanelsDetails\\%s\\%s\\", "IcsViewer", panel_define);
	
	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {

		RegistrySavePanelVisibility (REGKEY_HKCU, buffer, panel_id);     
		
		GetPanelAttribute (panel_id, ATTR_VISIBLE, &visible);   
	}

	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "top", panel_id, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "left", panel_id, ATTR_LEFT);
}
*/

static void SetupInitialSize(IcsViewerWindow *window, int left, int top, int width, int height, int override)
{
	int screen_width, screen_height;
	
	/* If client didnt suppliy width and height fit window to
	 */
	if(left == 0 && top == 0 && width == 0 && height == 0) {
		
		//panel_read_or_write_registry_settings(window->panel_id, 0);
		ics_viewer_registry_read_panel_size_position(window, window->panel_id);

		GetPanelAttribute (window->panel_id, ATTR_LEFT, &left);
		GetPanelAttribute (window->panel_id, ATTR_TOP, &top);
		GetPanelAttribute (window->panel_id, ATTR_WIDTH, &width);
		GetPanelAttribute (window->panel_id, ATTR_HEIGHT, &height);
	
		if(width >= PANEL_MIN_WIDTH && height >= PANEL_MIN_HEIGHT) // Ok assume sensible values.
			return;
	}
	
	/* panel_width and panel_height have to meet the minimum size */
	if(width < PANEL_MIN_WIDTH)
		width = PANEL_MIN_WIDTH;
		
	if(height < PANEL_MIN_HEIGHT)
		height = PANEL_MIN_HEIGHT;

	 
	/* Scale down the image if neccessary */
	// Get Screen Size
	
	if (override == 0) {
	
		GetScreenSize (&screen_height, &screen_width);
	
		if(width > (screen_width * 0.75))
			width = (int) (screen_width * 0.75);

		if(height > (screen_height * 0.75))
			height = (int) (screen_height * 0.75);
		
		// If the window is not placed within the screen assume popups should display on the second window.
		if(top > screen_height || left > screen_width)
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 2);	
	}

	SetPanelAttribute(window->panel_id, ATTR_LEFT, left);
	SetPanelAttribute(window->panel_id, ATTR_TOP, top);   
	SetPanelAttribute(window->panel_id, ATTR_WIDTH, width);
	SetPanelAttribute(window->panel_id, ATTR_HEIGHT, height);
}


void SetStatusbarText(IcsViewerWindow *window, int part, char *fmt, ...)
{
    char tmpbuf[500];
    va_list argp;

	memset(tmpbuf, 0, sizeof(tmpbuf)); 
	
    va_start(argp, fmt);
    vsprintf(tmpbuf, fmt, argp);
    va_end(argp);

    // Cannot use PostMessage, as the panel type is not set correctly
    SendMessage(window->hWndStatus, SB_SETTEXT, (WPARAM)part | 0, (LPARAM)(LPSTR)tmpbuf);
}

static void CreateStatusbar(IcsViewerWindow *window)
{
	 int ret;
	 
	 window->hWndStatus = CreateWindowEx( 

            0L,                              // no extended styles

            STATUSCLASSNAMEA,                 // status bar

            "",                              // no text 

            WS_CHILD | WS_VISIBLE,  // styles

            -100, -100, 10, 10,              // x, y, cx, cy

            window->panel_window_handle,                            // parent window

            (HMENU)100,                      // window ID

            GetModuleHandle(NULL),                           // instance

            NULL);                           // window data

      if (window->hWndStatus == NULL)
         MessageBox (NULL, "Status Bar not created!", NULL, MB_OK );
	  
	  ret = SendMessage(window->hWndStatus, SB_SETPARTS, (WPARAM)NO_OF_SB_PANELS, (LPARAM)(LPINT) statusbar_coords); 
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableAllActions(IcsViewerWindow *window)
{
	UnDimTool((Tool *)window->crosshair_tool);
	UnDimTool((Tool *)window->zoom_tool);
	UnDimTool((Tool *)window->roi_tool);
	UnDimTool((Tool *)window->line_tool);

	UnDimMenuPathItem(window->panel_menu->menubar_id, "Display"); 
	UnDimMenuPathItem(window->panel_menu->menubar_id, "Options"); 
	UnDimMenuPathItem(window->panel_menu->menubar_id, "View"); 
	UnDimMenuPathItem(window->panel_menu->menubar_id, "Processing"); 
	UnDimMenuPathItem(window->panel_menu->menubar_id, "Zoom"); 
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableAllActions(IcsViewerWindow *window)
{
	DimTool((Tool *)window->crosshair_tool);
	DimTool((Tool *)window->zoom_tool);
	DimTool((Tool *)window->roi_tool);
	DimTool((Tool *)window->line_tool);

	DimMenuPathItem(window->panel_menu->menubar_id, "Display"); 
	DimMenuPathItem(window->panel_menu->menubar_id, "Options"); 
	DimMenuPathItem(window->panel_menu->menubar_id, "View"); 
	DimMenuPathItem(window->panel_menu->menubar_id, "Processing"); 
	DimMenuPathItem(window->panel_menu->menubar_id, "Zoom"); 
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetUniqueIdentifier(IcsViewerWindow *window, const char *id)
{
    strncpy(window->_unique_id, id, 49);
    window->_unique_id_set = 1;
}

char* IW_DLL_CALLCONV ics_viewer_get_registry_subkey(IcsViewerWindow *window, char *key)
{
  if(window->_unique_id_set)
    sprintf(key, "%s%s\\", REGISTRY_SUBKEY, window->_unique_id);
  else
    sprintf(key, "%s", REGISTRY_SUBKEY);
    
  return key;
}

void ics_viewer_registry_read_string(IcsViewerWindow *window, const char *key, char *value,
  unsigned int *realStringSize)
{
  char subkey[200];
  
  ics_viewer_get_registry_subkey(window, subkey);
  
  RegReadString(REGKEY_HKCU, subkey, key, value, 500, realStringSize);
}

	
void ics_viewer_registry_save_panel_position(IcsViewerWindow *window, int panel_id)
{
  char subkey[200];
  char key[500];
  char panel_define[100] = "";
  
  if(panel_id <= 0)
	return;
		
  // Get the panel define as a module may have more than one panel
  GetPanelAttribute(panel_id, ATTR_CONSTANT_NAME, panel_define);
	
  ics_viewer_get_registry_subkey(window, subkey);
  
  sprintf(key, "%s%s", subkey, panel_define);
  RegistrySavePanelPosition (REGKEY_HKCU, key, panel_id); 
}
 
void ics_viewer_registry_save_panel_size_position(IcsViewerWindow *window, int panel_id)
{
  char subkey[200];
  char key[500];
  char panel_define[100] = "";
  int err;

  if(panel_id <= 0)
	return;
		
  // Get the panel define as a module may have more than one panel
  err = GetPanelAttribute(panel_id, ATTR_CONSTANT_NAME, panel_define);
	
  if(err < 0)
	  return;

  ics_viewer_get_registry_subkey(window, subkey);
  
  sprintf(key, "%s%s", subkey, panel_define);
  RegistrySavePanelSizePosition (REGKEY_HKCU, key, panel_id);
		
}

void ics_viewer_registry_read_panel_position(IcsViewerWindow *window, int panel_id)
{
  char subkey[200];
  char key[500];
  char panel_define[100] = "";
  char remote_session = 0;

  if(panel_id <= 0)
	return;
		
  remote_session = GetSystemMetrics(SM_REMOTESESSION);

  if(remote_session)
    return;
		
  // Get the panel define as a module may have more than one panel
  GetPanelAttribute(panel_id, ATTR_CONSTANT_NAME, panel_define);
	
  ics_viewer_get_registry_subkey(window, subkey);
  
  sprintf(key, "%s%s", subkey, panel_define);
  RegistryReadPanelPosition (REGKEY_HKCU, key, panel_id); 
}

void ics_viewer_registry_read_panel_size_position(IcsViewerWindow *window, int panel_id)
{
  char subkey[200];
  char key[500];
  char panel_define[100] = "";
  char remote_session = 0;

  if(panel_id <= 0)
	return;
		
  remote_session = GetSystemMetrics(SM_REMOTESESSION);

  if(remote_session)
    return;

  // Get the panel define as a module may have more than one panel
  GetPanelAttribute(panel_id, ATTR_CONSTANT_NAME, panel_define);
	
  ics_viewer_get_registry_subkey(window, subkey);
  
  sprintf(key, "%s%s", subkey, panel_define);
  RegistryReadPanelSizePosition (REGKEY_HKCU, key, panel_id); 
}

void ics_viewer_set_panel_to_top_left_of_window(IcsViewerWindow *window, int panel_id)
{
  int left, top;

  GetPanelAttribute(window->panel_id, ATTR_LEFT, &left);
  GetPanelAttribute(window->panel_id, ATTR_TOP, &top);

  SetPanelAttribute(panel_id, ATTR_LEFT, left + 20);
  SetPanelAttribute(panel_id, ATTR_TOP, top + 50);
}

IcsViewerWindow* IW_DLL_CALLCONV
GCI_ImagingWindow_CreateAdvanced2(const char *name, const char * title, int left, int top, int width, int height, int override, int monitor, int can_close)
{
	int window_handle, number_of_monitors;
	IcsViewerWindow *window;
	size_t size = sizeof(IcsViewerWindow);

	window = (IcsViewerWindow*) malloc(size);

	if(window == NULL)
		return NULL;

	memset(window, 0, sizeof(IcsViewerWindow));

	ui_module_constructor(UIMODULE_CAST(window), "IcsViewerWindow");

	// Use default resource paths
	GCI_ImagingWindow_SetResourceSearchPath();        
	
	window->panel_menu = (IcsViewerWindowMenu *) malloc (sizeof(IcsViewerWindowMenu));
	
	SetupDefaultDirectory(window);  
	
	GCI_ImagingWindow_Init(window);
	
	strcpy(window->window_name, title); 
	
	window->panel_id = NewPanel (0, window->window_name, VAL_AUTO_CENTER, VAL_AUTO_CENTER, 480, 600);         
		
	SetPanelAttribute(window->panel_id, ATTR_CONSTANT_NAME, "VIEW_PNL");
	GCI_ImagingWindow_SetUniqueIdentifier(window, name);

	SetupInitialSize(window, left, top, width, height, override);     
	
	GetPanelAttribute(window->panel_id, ATTR_WIDTH, &width);
	GetPanelAttribute(window->panel_id, ATTR_HEIGHT, &height);
	
	GetSystemAttribute(ATTR_NUM_MONITORS, &number_of_monitors);
	
	if(number_of_monitors > 1)
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 2);
	
	//SetPanelAttribute (window->panel_id, ATTR_CONFORM_TO_SYSTEM , 1);
	SetPanelAttribute (window->panel_id, ATTR_BACKCOLOR, MakeColor(192,192,192));

	window->can_close = can_close;
	if (!window->can_close)
		// disable close, Must do it before you get the window handle
		SetPanelAttribute (window->panel_id, ATTR_CLOSE_ITEM_VISIBLE, 0);

	GetPanelAttribute (window->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);
 
   	window->panel_window_handle = (HWND) window_handle;
	
	CreateStatusbar(window);           

 	/* Store the original windows procedure function pointer */
	window->panel_original_proc_fun_ptr = GetWindowLongPtr (window->panel_window_handle, GWL_WNDPROC);
	
	/* Store the window structure with the window for use in WndProc */
	SetWindowLongPtr (window->panel_window_handle, GWLP_USERDATA, (LONG_PTR)window);
	
    /* Set the new Wnd Proc to be called */	
	SetWindowLongPtr (window->panel_window_handle, GWL_WNDPROC, (LONG_PTR)GCI_WndProc);

	ImageViewer_Init();

    /* Add canvas */
 	window->canvas_window = ImageViewer_Create(window->panel_window_handle, 80, 30, width - 50, height - 100);
 
	if(window->canvas_window == NULL) {
		
		printf("Can not create canvas window?\n");
		return NULL;	
	}
	
 	ImageViewer_SetInteractMode(window->canvas_window, PANNING_MODE);
 
	/* Store the original windows procedure function pointer */
	window->canvas_original_proc_fun_ptr = GetWindowLongPtr (window->canvas_window, GWL_WNDPROC);
 
 	/* Store the window structure with the window for use in WndProc */
	SetWindowLongPtr (window->canvas_window, GWLP_USERDATA, (LONG_PTR)window);
 
    /* Set the new Wnd Proc to be called */	
	SetWindowLongPtr (window->canvas_window, GWL_WNDPROC, (LONG_PTR) GCI_WindowCanvasProc);

	window->canvas_window_hdc = GetDC(window->canvas_window);
	
	RegisterDropWindow(window);
	
	return window;
}

IcsViewerWindow* IW_DLL_CALLCONV
GCI_ImagingWindow_CreateAdvanced(const char *name, const char * title, int left, int top, int width, int height, int override, int monitor)
{
	return GCI_ImagingWindow_CreateAdvanced2(name, title, left, top, width, height, override, monitor, 1);  	
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_SetPosition(IcsViewerWindow* window, int left, int top, int width, int height, int override)
{
	if(window == NULL)
		return;

	SetupInitialSize(window, left, top, width, height, override);
}

IcsViewerWindow* IW_DLL_CALLCONV
GCI_ImagingWindow_Create(const char *name, const char * title)
{
	return GCI_ImagingWindow_CreateAdvanced2(name, title, 0, 0, 0, 0, 1, 0, 1);  	
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_RemoveCrosshairTool(IcsViewerWindow* window)
{
	window->no_crosshair_tool = 1;
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_RemoveRoiTool(IcsViewerWindow *window)
{
	window->no_roi_tool = 1;	
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_Initialise(IcsViewerWindow* window)
{
	BuildMenuBar(window);   

	window->panel_menu->multidimensional_menu_item_id = NewMenuItem (window->panel_menu->menubar_id, window->panel_menu->view_menu_id, 
					 					   "Mutiple Dimensions", -1, VAL_MENUKEY_MODIFIER | 'M' ,
					 					   GCI_ImagingWindow_MultipleDimensionsShow, window);

	SetMenuBarAttribute(window->panel_menu->menubar_id, window->panel_menu->multidimensional_menu_item_id, ATTR_DIMMED, 1);

	window->palette_plugin = Plugin_CreatePlugin(palette_plugin_constructor, window);      
	window->linear_scale_plugin = Plugin_CreatePlugin(linear_scale_plugin_constructor, window);
	
	Plugin_CreatePlugin(background_plugin_constructor, window); 
	Plugin_CreatePlugin(counter_plugin_constructor, window);    
	//Plugin_CreatePlugin(fft_plugin_constructor, window);  
	Plugin_CreatePlugin(grid_plugin_constructor, window); 
	Plugin_CreatePlugin(titlebar_plugin_constructor, window);
	Plugin_CreatePlugin(histogram_equalisation_plugin_constructor, window);
	Plugin_CreatePlugin(histogram_plugin_constructor, window);
	Plugin_CreatePlugin(resample_plugin_constructor, window);
	Plugin_CreatePlugin(binning_plugin_constructor, window);
	Plugin_CreatePlugin(rotate_plugin_constructor, window);
	
	window->save_plugin = Plugin_CreatePlugin(save_plugin_constructor, window);
	Plugin_CreatePlugin(screenshot_plugin_constructor, window);
	window->profile_plugin = Plugin_CreatePlugin(profile_plugin_constructor, window);
	Plugin_CreatePlugin(scalebar_plugin_constructor, window);  

	#if defined(STANDALONE_APP) || defined(STREAM_DEVICE_PLUGIN)
	window->stream_plugin = Plugin_CreatePlugin(streamdevice_plugin_constructor, window);
	#endif
	
	window->palettebar_tool = Plugin_CreatePlugin(palettebar_plugin_constructor, window); 
	
	//Plugin_CreatePlugin(record_plugin_constructor, window);       
	
	window->metadata_plugin = Plugin_CreatePlugin(metadata_plugin_constructor, window);    
	window->zoom_tool = Plugin_CreatePlugin(zoom_tool_constructor, window);
	window->line_tool = Plugin_CreatePlugin(line_tool_constructor, window);
	
	if(window->no_roi_tool == 0)  
		window->roi_tool = Plugin_CreatePlugin(roi_tool_constructor, window);
	else
		window->roi_tool = NULL; 
	
	if(window->no_crosshair_tool == 0)
		window->crosshair_tool = Plugin_CreatePlugin(crosshair_tool_constructor, window);
	else
		window->crosshair_tool = NULL;      	
		
	#ifdef ENABLE_TWAIN 
	Plugin_CreatePlugin(twain_plugin_constructor, window);
	#endif

	window->number_of_plugins = ListNumItems (window->program_plugins);	

	window->panel_menu->exit_menu_item_id = NewMenuItem (window->panel_menu->menubar_id, window->panel_menu->file_menu_id, 
					 					   "Close", -1, 0,
										   OnWindowExit, window);
	
	if (!window->can_close)
		// dim close menu item (could also just not create it - think this is better)
		SetMenuBarAttribute (window->panel_menu->menubar_id, window->panel_menu->exit_menu_item_id, ATTR_DIMMED, 1);		
}


#ifdef _CVI_DEBUG_
void Debug(IcsViewerWindow *window, const char *fmt, ...)
{
	static panel_id = 0;
	char message[2000];
	int count = 0;
	va_list ap; 
	
	if(panel_id == 0)
		panel_id = LoadPanel(0, uir_file_path, DEBUG_PNL);
	
    va_start(ap, fmt);

	vsprintf(message, fmt, ap);

	va_end(ap);

	InsertTextBoxLine (panel_id, DEBUG_PNL_TEXTBOX, -1, message);
	
	GetNumTextBoxLines (panel_id, DEBUG_PNL_TEXTBOX, &count);

	// Show last three lines
	if(count >= 3)
		count -= 3;
	
	SetCtrlAttribute(panel_id, DEBUG_PNL_TEXTBOX, ATTR_FIRST_VISIBLE_LINE, count);
	DisplayPanel(panel_id);
}
#endif
