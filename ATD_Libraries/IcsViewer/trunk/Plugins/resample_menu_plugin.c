#include "icsviewer_window.h"
#include "icsviewer_private.h" 
#include "icsviewer_uir.h"
#include "resample_menu_plugin.h"
#include "ImageViewer.h"

#include "string_utils.h"

#include "gci_utils.h"

static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData; 
	
	int width, height, factor;
	FIBITMAP *dib;
	char temp_string[500];   
	double x_extents, y_extents;    

	GetMenuBarAttribute(menubar, menuItem, ATTR_ITEM_NAME, temp_string);
	
	factor = atoi(temp_string);        
	
	width = (int) (FreeImage_GetWidth(plugin->window->panel_dib)	* (factor / 100.0));
	height = (int) (FreeImage_GetHeight(plugin->window->panel_dib) * (factor / 100.0)); 
	
	dib = FreeImage_Rescale(plugin->window->panel_dib, width, height, FILTER_BSPLINE);
	
	if(dib == NULL) {
	
		GCI_MessagePopup ("Error", "Rescale Failed Image Type was %d bpp = %d",
			FreeImage_GetImageType(plugin->window->panel_dib), FreeImage_GetBPP(plugin->window->panel_dib));  
			
		return;
	}
	
	GCI_ImagingWindow_LoadImage(plugin->window, dib);   
	
	x_extents = Plugin_GetMicronsPerPixel(plugin) * width;  
	y_extents = Plugin_GetMicronsPerPixel(plugin) * height;  
	
	sprintf(temp_string, "%.2e %.2e", x_extents, y_extents);    	
	
	GCI_ImagingWindow_SetMetaDataKey(plugin->window, "extents", temp_string);

	sprintf(temp_string, "%d %d", width, height);   
	
	GCI_ImagingWindow_SetMetaDataKey(plugin->window, "dimensions", temp_string);
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	if(plugin->window->live_mode) {
		Plugin_DimMenuPathItem(plugin, "Processing");  
		return 0;
	}
	else {
		Plugin_UnDimMenuPathItem(plugin, "Processing");  	
	}
	
	if(!IsStandardTypeImage(data2.dib))
		return 0;
	else
		return 1; 
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ResamplePlugin* resample_plugin = (ResamplePlugin*) plugin; 
}

ImageWindowPlugin* resample_plugin_constructor(IcsViewerWindow *window) 
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "ResamplePlugin", sizeof(ResamplePlugin)); 

	Plugin_AddMenuItem(plugin, "Processing//Resample//25%",
		0, on_menu_clicked, plugin);
    	
    Plugin_AddMenuItem(plugin, "Processing//Resample//50%",
		0, on_menu_clicked, plugin);
   
    Plugin_AddMenuItem(plugin, "Processing//Resample//75%",
		0, on_menu_clicked, plugin);
    
    PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;   
    PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
