#include "icsviewer_window.h"
#include "icsviewer_private.h"   
#include "icsviewer_uir.h"
#include "rotate_menu_plugin.h"
#include "ImageViewer.h"

#include "gci_utils.h"

#include "string_utils.h"

static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData; 
	
	double amount;
	float x_extents, y_extents;
	FIBITMAP *dib;
	char temp_string[500];
		
	GetMenuBarAttribute(menubar, menuItem, ATTR_ITEM_NAME, temp_string);   
	
	if(strstr(temp_string, "Anti"))
		amount = -90;  
	else
		amount = 90;  
	
	dib = FreeImage_RotateClassic(plugin->window->panel_dib, amount); 
	
	if(dib == NULL) {
	
		GCI_MessagePopup ("Error", "Rotate Failed Image Type was %d bpp = %d",
			FreeImage_GetImageType(plugin->window->panel_dib), FreeImage_GetBPP(plugin->window->panel_dib));  
			
		return;
	}
	
	GCI_ImagingWindow_LoadImage(plugin->window, dib);  
	
	GCI_ImagingWindow_GetMetaDataKey(plugin->window, "extents", temp_string, "");

	if(strcmp(temp_string, "") == 0)
		return;
		
	sscanf(temp_string, "%e %e", &x_extents, &y_extents); 	
	sprintf(temp_string, "%.2e %.2e", y_extents, x_extents);    	
	
	GCI_ImagingWindow_SetMetaDataKey(plugin->window, "extents", temp_string);
	
	sprintf(temp_string, "%d %d", Plugin_GetImageHeight(plugin), Plugin_GetImageWidth(plugin));  
	
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
	RotatePlugin* rotate_plugin = (RotatePlugin*) plugin; 
}

ImageWindowPlugin* rotate_plugin_constructor(IcsViewerWindow *window) 
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "RotatePlugin", sizeof(RotatePlugin)); 

	Plugin_AddMenuItem(plugin, "Processing//Rotate//Rotate 90 Degrees Clockwise",
		0, on_menu_clicked, plugin);
    	
    Plugin_AddMenuItem(plugin, "Processing//Rotate//Rotate 90 Degrees Anitclockwise",
		0, on_menu_clicked, plugin);
   
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;  
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
