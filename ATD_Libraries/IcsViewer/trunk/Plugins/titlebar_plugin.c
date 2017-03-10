#include "icsviewer_window.h"
#include "icsviewer_private.h"

#include <userint.h>
#include <utility.h>
#include "ImageViewer.h"
#include "titlebar_plugin.h"
#include "string_utils.h"
#include "gci_utils.h"

#include "FreeImageAlgorithms_IO.h"

#include "FreeImageIcs_IO.h"
#include "FreeImageIcs_MetaData.h"


static void GetImageDimensionsString(IcsViewerWindow *window, char *dimensions_string)
{
	char pixel_dimensions[50], temp[50], unit[50];
	double width, height; 
	
	int image_width = FreeImage_GetWidth(window->panel_dib);
	int image_height = FreeImage_GetHeight(window->panel_dib);
	
	sprintf(pixel_dimensions, "%dx%d", image_width, image_height);

	if(window->microns_per_pixel == 1.0) {
		strcpy(dimensions_string, pixel_dimensions);
		return;	
	}
	
	width = (image_width * window->microns_per_pixel * window->binning_size);
	height = (image_height * window->microns_per_pixel * window->binning_size);  

	GCI_ImagingWindow_GetMetaDataKey(window, "units", temp, "");

	if(strcmp(temp, "") == 0)
		return;

	strcpy(unit, "microns");	 
	sprintf(dimensions_string, "%s (%.0f x %.0f %s)", pixel_dimensions, width, height, unit); 
}


static void GetImageZoomString(IcsViewerWindow *window, char *zoom_string)
{
	double zoom_factor = ImageViewer_GetZoomFactor(window->canvas_window);
	// Do a reasonable check on this factor
	if (zoom_factor>0 && zoom_factor<1.0e6)
		sprintf(zoom_string, "(%.0f%%)", zoom_factor*100.0);		 
}


static void GetDisplayFrameRateString(IcsViewerWindow *window, char *fps_string)
{
	sprintf(fps_string, "Display Fps %.2f", window->fps);    
}

static void update_titlebar(ImageWindowPlugin *plugin, IcsViewerWindow *window)
{
	char filename[300];
	char dimension_string[100] = "", zoom_string[50] = "";
	char title_string[300] = "", existing_title_string[300] = "", fps[100] = "";

	if(window->panel_dib == NULL) {

		if(strcmp(window->panel_user_title, ""))
			strcpy(window->panel_tmp_title, window->panel_user_title);
		else
			strcpy(window->panel_tmp_title, window->window_name);

		PostMessage(window->panel_window_handle, ICSVIEWER_UPDATE_TITLEBAR, 0, 0);
		return;
	}

	GetImageDimensionsString(plugin->window, dimension_string);
	GetImageZoomString(plugin->window, zoom_string);
	
	// Make empty str
	fps[0] = 0;
	
	if(window->live_mode)
		GetDisplayFrameRateString(plugin->window, fps); 
			
	if(window->filename != NULL && strlen(window->filename) < GCI_MAX_PATHNAME_LEN) {
		SplitPath (window->filename, NULL, NULL, filename);
		sprintf(title_string, "%s %s %s %s", filename, dimension_string, zoom_string, fps);     
	}
	else {
		sprintf(title_string, "%s %s %s", dimension_string, zoom_string, fps);          	
	}
		
	if( strlen(window->panel_user_title) > 0 ) {
		sprintf(window->panel_tmp_title, "%s %s", window->panel_user_title, title_string);
	}
	else {
		strcpy(window->panel_tmp_title, title_string);
	}

	GetPanelAttribute (window->panel_id, ATTR_TITLE, existing_title_string);
		
	if( strcmp(window->panel_tmp_title, existing_title_string) != 0 ) {
		strcpy(window->panel_tmp_title, window->panel_tmp_title);
		PostMessage(window->panel_window_handle, ICSVIEWER_UPDATE_TITLEBAR, 0, 0);
	}
}


static void on_image_displayed(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	update_titlebar(plugin, plugin->window); 
}

static void on_microns_per_pixel_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	update_titlebar(plugin, plugin->window); 
}


static int on_validate_plugin(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}


static void on_zoom_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	update_titlebar(plugin, plugin->window);
}


static void on_resize (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	if(!ImageViewer_IsZoomToFit (PLUGIN_CANVAS(plugin)))
		return;

	update_titlebar(plugin, plugin->window); 
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	TitlebarPlugin* title_plugin = (TitlebarPlugin*) plugin; 
}

ImageWindowPlugin* titlebar_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "TitlebarPlugin", sizeof(TitlebarPlugin)); 

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_image_displayed) = on_image_displayed;
	PLUGIN_VTABLE(plugin, on_zoom_changed) = on_zoom_changed; 
	PLUGIN_VTABLE(plugin, on_microns_per_pixel_changed) = on_microns_per_pixel_changed; 
	PLUGIN_VTABLE(plugin, on_resize) = on_resize;
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
