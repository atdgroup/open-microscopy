#include <userint.h>
#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "save_plugin.h"
#include <utility.h>
#include "icsviewer_uir.h"
#include "gci_utils.h"
#include "string_utils.h"
#include "GL_CVIRegistry.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Utilities.h" 
#include "FreeImageIcs_MetaData.h"


static int Is16bitGreyscale(SavePlugin *save_plugin)
{
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) save_plugin;  
	
	if(!FIA_IsGreyScale(plugin->window->panel_dib))
		return 0;
	
	if(FreeImage_GetImageType(plugin->window->panel_dib) != FIT_INT16 &&
	   FreeImage_GetImageType(plugin->window->panel_dib) != FIT_UINT16)
		return 0;
	
	return 1;
}

static void SaveImage(SavePlugin *save_plugin, const char *filepath) 
{
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) save_plugin;
	FREEIMAGE_ALGORITHMS_SAVE_BITDEPTH bit_depth = BIT24;
	 
	char file_ext[500], filename[GCI_MAX_PATHNAME_LEN];

	plugin->window->prevent_image_load = 1;

	/* Get filename */
	SplitPath (filepath, NULL, NULL, filename);
	get_file_extension(filename, file_ext); 	
	
	if( strcmp(file_ext, ".ics") == 0 ) {
		
		if(FreeImageIcs_SaveImage (plugin->window->panel_dib, filepath, 1) == FIA_ERROR)
			GCI_MessagePopup("Error", "Error Saving File");
			
		SaveMetaDataToImage(plugin->window, filepath);         
	}
	else {
		
		RGBQUAD *quad;

		GCI_MessagePopup("Warning", "Saving to formats other than ics will result in a loss of any metadata.");
		

		quad = FreeImage_GetPalette(plugin->window->panel_dib);

		if(FIA_SimpleSaveFIBToFile (plugin->window->panel_dib, filepath) == FIA_ERROR)
			GCI_MessagePopup("Error", "Error Saving File");
	}

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "Save", GCI_VOID_POINTER, plugin->window,
			GCI_STRING, filepath, GCI_STRING, file_ext);

	plugin->window->prevent_image_load = 0;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_SaveImage(IcsViewerWindow *window, const char *filepath)
{
	SaveImage((SavePlugin*) window->save_plugin, filepath); 
	
	return 0;
}

static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	char fname[GCI_MAX_PATHNAME_LEN] = "";
	char directory[GCI_MAX_PATHNAME_LEN] = "";
	char global_default_directory[GCI_MAX_PATHNAME_LEN] = "";

	char *default_extensions = "*.ics;*.jpg;*.png;*.tif;*.bmp;*.exr;*.cr2;*.crw,*.gif;*.pfm;*.pgm;*.ppm;*.psd;*.ico";

	int fsize = 0;

	SavePlugin *save_plugin = (SavePlugin *) callbackData;
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData; 
	
	// Perform client defined action instead.
	if(save_plugin->app_provided_callback != NULL) {
		
		save_plugin->app_provided_callback(plugin->window, save_plugin->app_provided_callback_data);		
		return;
	}

	GetDefaultDirectoryPath(plugin->window, global_default_directory);
	
	if (LessLameFileSelectPopup (plugin->window->panel_id, global_default_directory,
		"*.ics", default_extensions, "Save Image As", VAL_OK_BUTTON, 0, 0, 1, 1, fname) <= 0) {
		return;
	}

	// Set new default directory based on user selection.
	GetDirectoryForFile(fname, directory);
	strncpy(plugin->window->default_directory, directory, GCI_MAX_PATHNAME_LEN-1);
	// If we are using the global default dir, store it there too
	if(strlen(global_default_directory) > 1) {
		GCI_ImagingWindow_SetDefaultDirectoryPathForAllWindows(directory);
	}

	// Get the directory for fname and save it in the registry. As we only that directory by default next time.
	checkRegistryValueForString (1, REGKEY_HKCU, REGISTRY_SUBKEY, "DefaultDir", directory);
	
	if(FileExists(fname, &fsize)) {
		if(GCI_ConfirmPopup("Warning", IDI_WARNING, "File already exists.\nDo you wish to overwrite ?") == 0)
			return;
	}

	SaveImage(save_plugin, fname);
}

static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	Plugin_UnDimMenuPathItem(plugin, "File//Save As");
	
	return 1;
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	SavePlugin* save_plugin = (SavePlugin*) plugin; 
}

ImageWindowPlugin* save_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "SavePlugin", sizeof(SavePlugin));
	SavePlugin *save_plugin = (SavePlugin *) plugin;

	save_plugin->app_provided_callback = NULL;

	Plugin_AddMenuItem(plugin, "File//Save As",
		VAL_MENUKEY_MODIFIER | 'A', on_menu_clicked, plugin);

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin; 
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
