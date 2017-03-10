#include "icsviewer_private.h" 
#include "icsviewer_window.h"
#include "icsviewer_uir.h"
#include "binning_menu_plugin.h"
#include "ImageViewer.h"

#include "string_utils.h"

#include "gci_utils.h"

#include "FreeImageAlgorithms_Filters.h"
#include "FreeImageAlgorithms_Utilities.h"

static void get_binning_type_and_radius(BinningPlugin* binning_plugin, int menu_item_id, FIA_BINNING_TYPE *type, int *radius)
{
	if(menu_item_id == binning_plugin->square_3x3_menu_id) {
		*type = FIA_BINNING_SQUARE;
		*radius = 3;
	}
	if(menu_item_id == binning_plugin->square_5x5_menu_id) {
		*type = FIA_BINNING_SQUARE;
		*radius = 5;
	}
	if(menu_item_id == binning_plugin->square_7x7_menu_id) {
		*type = FIA_BINNING_SQUARE;
		*radius = 7;
	}
	if(menu_item_id == binning_plugin->square_9x9_menu_id) {
		*type = FIA_BINNING_SQUARE;
		*radius = 9;
	}
	if(menu_item_id == binning_plugin->square_11x11_menu_id) {
		*type = FIA_BINNING_SQUARE;
		*radius = 11;
	}
	if(menu_item_id == binning_plugin->circular_3x3_menu_id) {
		*type = FIA_BINNING_CIRCULAR;
		*radius = 3;
	}
	if(menu_item_id == binning_plugin->circular_5x5_menu_id) {
		*type = FIA_BINNING_CIRCULAR;
		*radius = 5;
	}
	if(menu_item_id == binning_plugin->circular_7x7_menu_id) {
		*type = FIA_BINNING_CIRCULAR;
		*radius = 7;
	}
	if(menu_item_id == binning_plugin->circular_9x9_menu_id) {
		*type = FIA_BINNING_CIRCULAR;
		*radius = 9;
	}
	if(menu_item_id == binning_plugin->circular_11x11_menu_id) {
		*type = FIA_BINNING_CIRCULAR;
		*radius = 11;
	}
	if(menu_item_id == binning_plugin->gaussian_3x3_menu_id) {
		*type = FIA_BINNING_GAUSSIAN;
		*radius = 3;
	}
	if(menu_item_id == binning_plugin->gaussian_5x5_menu_id) {
		*type = FIA_BINNING_GAUSSIAN;
		*radius = 5;
	}
	if(menu_item_id == binning_plugin->gaussian_7x7_menu_id) {
		*type = FIA_BINNING_GAUSSIAN;
		*radius = 7;
	}
	if(menu_item_id == binning_plugin->gaussian_9x9_menu_id) {
		*type = FIA_BINNING_GAUSSIAN;
		*radius = 9;
	}
	if(menu_item_id == binning_plugin->gaussian_11x11_menu_id) {
		*type = FIA_BINNING_GAUSSIAN;
		*radius = 11;
	}
}

static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData; 
	BinningPlugin* binning_plugin = (BinningPlugin*) plugin; 

	int radius;
	FIBITMAP *float_dib;
	FREE_IMAGE_TYPE type = FreeImage_GetImageType(plugin->window->panel_dib);
	FIA_BINNING_TYPE binning_type;

	get_binning_type_and_radius(binning_plugin, menuItem, &binning_type, &radius);

	float_dib = FIA_Binning (plugin->window->panel_dib, FIA_BINNING_GAUSSIAN, radius);
	
	if(float_dib == NULL) {
	
		GCI_MessagePopup ("Error", "FIA_Binning Failed Image Type was %d bpp = %d",
			FreeImage_GetImageType(plugin->window->panel_dib), FreeImage_GetBPP(plugin->window->panel_dib));  
			
		return;
	}
	
	GCI_ImagingWindow_LoadImage(plugin->window, float_dib);
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	if(plugin->window->live_mode) {
		Plugin_DimMenuPathItem(plugin, "Processing//Binning//Gaussian");  
		return 0;
	}
	else {
		Plugin_UnDimMenuPathItem(plugin, "Processing//Binning//Gaussian");  	
	}
	
	if(FIA_IsGreyScale(data2.dib))
		return 1;
	else
		return 0; 
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	BinningPlugin* binning_plugin = (BinningPlugin*) plugin; 
}

ImageWindowPlugin* binning_plugin_constructor(IcsViewerWindow *window) 
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "BinningPlugin", sizeof(BinningPlugin)); 
	BinningPlugin* binning_plugin = (BinningPlugin*) plugin; 

	binning_plugin->square_3x3_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Square//3x3",
		0, on_menu_clicked, plugin);
    	
    binning_plugin->square_5x5_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Square//5x5",
		0, on_menu_clicked, plugin);
   
    binning_plugin->square_7x7_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Square//7x7",
		0, on_menu_clicked, plugin);

	binning_plugin->square_9x9_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Square//9x9",
		0, on_menu_clicked, plugin);
    
	binning_plugin->square_11x11_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Square//11x11",
		0, on_menu_clicked, plugin);

	//////////////////////////////////////////////////////

	binning_plugin->circular_3x3_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Circular//3x3",
		0, on_menu_clicked, plugin);
    	
    binning_plugin->circular_5x5_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Circular//5x5",
		0, on_menu_clicked, plugin);
   
    binning_plugin->circular_7x7_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Circular//7x7",
		0, on_menu_clicked, plugin);

	binning_plugin->circular_9x9_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Circular//9x9",
		0, on_menu_clicked, plugin);
    
	binning_plugin->circular_11x11_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Circular//11x11",
		0, on_menu_clicked, plugin);

	///////////////////////////////////////
	binning_plugin->gaussian_3x3_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Gaussian//3x3",
		0, on_menu_clicked, plugin);
    	
    binning_plugin->gaussian_5x5_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Gaussian//5x5",
		0, on_menu_clicked, plugin);
   
    binning_plugin->gaussian_7x7_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Gaussian//7x7",
		0, on_menu_clicked, plugin);

	binning_plugin->gaussian_9x9_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Gaussian//9x9",
		0, on_menu_clicked, plugin);
    
	binning_plugin->gaussian_11x11_menu_id = Plugin_AddMenuItem(plugin, "Processing//Binning//Gaussian//11x11",
		0, on_menu_clicked, plugin);

    PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;   
    PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
