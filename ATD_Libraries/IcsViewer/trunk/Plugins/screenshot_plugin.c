#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "ImageViewer.h"
#include "icsviewer_uir.h"
#include "ImageViewer_Drawing.h"
#include "screenshot_plugin.h"
#include "gci_utils.h"
#include "GL_CVIRegistry.h"

#include "string_utils.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_HBitmap.h"
#include "FreeImageAlgorithms_Utilities.h"

#include "FreeImageIcs_IO.h" 

static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	Plugin_UnDimMenuPathItem(plugin, "File//Screen Shot");  
	
	return 1;
}

static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData; 
	
	HBITMAP bitmap;
	HDC hdc, viewer_hdc;
	FIBITMAP* dib;
	int width, bpp;
	
	RECT client_rect;
	
	char fname[GCI_MAX_PATHNAME_LEN] = "";
	char *default_extensions = "*.ics;*.jpg;*.png;*.tif;*.bmp;";
	char file_ext[10];

	static char directory[GCI_MAX_PATHNAME_LEN] = "";
	
	if (LessLameFileSelectPopup (plugin->window->panel_id, GetDefaultDirectoryPath(plugin->window, directory),
		"*.jpg", default_extensions, "Save Image As", VAL_OK_BUTTON, 0, 0, 1, 1, fname) <= 0) {
		return;
	}

	// Set new default directory basewd on user selection.
	GetDirectoryForFile(fname, directory);
	strncpy(plugin->window->default_directory, directory, 499);

	// Get the directory for fname and save it in the registry. As we only that directory by default next time.
	checkRegistryValueForString (1, REGKEY_HKCU, REGISTRY_SUBKEY, "DefaultDir", directory);
	
	GetClientRect(plugin->window->canvas_window, &client_rect);

	assert(plugin->window->canvas_window != NULL);
	
	viewer_hdc = ImageViewer_GetBufferHdc(plugin->window->canvas_window);
	
	assert(viewer_hdc != NULL); 
	
	hdc = CreateCompatibleDC(viewer_hdc);
	
	assert(hdc != NULL);
	
	bitmap = CreateCompatibleBitmap (viewer_hdc,
		client_rect.right - client_rect.left, client_rect.bottom - client_rect.top);
	
	// Associate the new DC with the bitmap for drawing 
	SelectObject( hdc, bitmap );
	
	BitBlt (hdc, 0, 0,
			client_rect.right - client_rect.left,
			client_rect.bottom - client_rect.top,
			viewer_hdc,
			0, 0, SRCCOPY);
	
	dib = FIA_HBitmapToFIB(hdc, bitmap);
	
	width = FreeImage_GetWidth(dib);	
	bpp = FreeImage_GetBPP(dib);
	
	get_file_extension(fname, file_ext); 	
	
	if( strcmp(file_ext, ".ics") == 0 ) {
	
		if(FreeImageIcs_SaveImage (plugin->window->panel_dib, fname, 0) == FIA_ERROR)
			GCI_MessagePopup("Error", "Error Saving File");	
	}
	else {
	
		if(FIA_SaveFIBToFile(dib, fname, BIT24) == FIA_ERROR)
			GCI_MessagePopup("Error", "Error Saving File");
	}

	ReleaseDC(plugin->window->canvas_window, hdc);
	DeleteObject(bitmap);
	DeleteDC (hdc);
	FreeImage_Unload(dib);	
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ScreenShotPlugin* screenshot_plugin = (ScreenShotPlugin*) plugin; 
}

ImageWindowPlugin* screenshot_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "ScreenShotPlugin", sizeof(ScreenShotPlugin));

	Plugin_AddMenuItem(plugin, "File//Screen Shot",
		VAL_MENUKEY_MODIFIER | 'K', on_menu_clicked, plugin);

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin; 
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
