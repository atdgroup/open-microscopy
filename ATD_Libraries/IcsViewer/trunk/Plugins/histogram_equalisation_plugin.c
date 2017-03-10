#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "histogram_equalisation_plugin.h"

#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_Utilities.h" 
#include "FreeImageAlgorithms_Statistics.h" 

#include "ImageViewer.h"

static void GCI_ImagingWindow_HistogramEquilisation (HistogramEqualisationPlugin* histogram_equalisation_plugin)
{
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) histogram_equalisation_plugin;   
	IcsViewerWindow *window = plugin->window;
	
	FIBITMAP *dib = FIA_HistEq_Random_Additions(window->panel_dib);
	
	GCI_ImagingWindow_LoadImage(window, dib);        
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	if(plugin->window->live_mode || FreeImage_GetBPP(data2.dib) != 8)
		return 0;

	return 1;   
}
								

static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	HistogramEqualisationPlugin* histogram_equalisation_plugin = (HistogramEqualisationPlugin*) callbackData;  
	
	GCI_ImagingWindow_HistogramEquilisation (histogram_equalisation_plugin);
	
	
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	HistogramEqualisationPlugin* he_plugin = (HistogramEqualisationPlugin*) plugin; 
}

ImageWindowPlugin* histogram_equalisation_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "HistogramEqualisationPlugin",
									sizeof(HistogramEqualisationPlugin));

	Plugin_AddMenuItem(plugin, "Processing//Histogram Equalisation",
		0, on_menu_clicked, plugin);

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin; 
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
