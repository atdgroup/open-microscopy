#include "icsviewer_window.h"
#include "icsviewer_uir.h"
#include "fft_plugin.h"
#include "ImageViewer.h"
#include "gci_utils.h"

#include "string_utils.h"

#include "FreeImageAlgorithms_HBitmap.h"
#include "FreeImageAlgorithms_Utilities.h" 
#include "FreeImageAlgorithms_IO.h" 
#include "FreeImageAlgorithms_Arithmetic.h"   
#include "FreeImageAlgorithms_Palettes.h" 

static void DestroyFftPanel(FFTPlugin *fftplugin)
{
	if(fftplugin->hbitmap != NULL) {
		DeleteObject(fftplugin->hbitmap);
		fftplugin->hbitmap = NULL;		
	}
		
	ReleaseDC((HWND) fftplugin->handle, fftplugin->canvas_hdc);
	
	DiscardPanel(fftplugin->panel_id);
	
	fftplugin->activated = 0;
}	 

static int CVICALLBACK OnPanelOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FFTPlugin *fftplugin = (FFTPlugin *) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			DestroyFftPanel(fftplugin); 
	
			break;
		}
	}
		
	return 0;
}

static void CreateFftPanel(FFTPlugin *fftplugin)
{
	int top, left, width, height;
	
	fftplugin->panel_id = LoadPanel(0, uir_file_path, FFT_PNL);
	
	if ( InstallCtrlCallback (fftplugin->panel_id, FFT_PNL_OK_BUTTON, OnPanelOk, fftplugin) < 0) {
		return;
	}

	GetCtrlAttribute(fftplugin->panel_id, FFT_PNL_DEC, ATTR_WIDTH, &width);
	GetCtrlAttribute(fftplugin->panel_id, FFT_PNL_DEC, ATTR_HEIGHT, &height); 
	GetCtrlAttribute(fftplugin->panel_id, FFT_PNL_DEC, ATTR_LEFT, &left);
	GetCtrlAttribute(fftplugin->panel_id, FFT_PNL_DEC, ATTR_TOP, &top);   

	GetPanelAttribute (fftplugin->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &(fftplugin->handle));
 
	/* Add canvas */
 	fftplugin->canvas = ImageViewer_Create((HWND) fftplugin->handle, left, top, width, height);

	SetWindowPos(fftplugin->canvas, NULL, left, top,
		width, height, SWP_NOZORDER | SWP_NOACTIVATE);
	
	fftplugin->canvas_hdc = GetDC(fftplugin->canvas);   
	
 	ImageViewer_SetInteractMode(fftplugin->canvas, NO_INTERACT_MODE);	
	ImageViewer_EnableZoomToFit(fftplugin->canvas);
	
	ImageViewer_SetBackgroundColour(fftplugin->canvas, RGB(0,0,255));
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	FFTPlugin *fftplugin = (FFTPlugin *) callbackData; 
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData;
	IcsViewerWindow *window = plugin->window;
	
	FIBITMAP *fftdib, *real_dib, *standard_dib, *log_dib;
	
	if(fftplugin->activated == 1)
		return;
	
	fftdib = FIA_FFT(window->panel_dib);
		
	real_dib = FIA_ConvertComplexImageToAbsoluteValued(fftdib);
	
	log_dib = FIA_Log(real_dib);
	
	standard_dib = FreeImage_ConvertToStandardType(log_dib, 1);
	
	FreeImage_Unload(fftdib);   
	FreeImage_Unload(real_dib);	
	FreeImage_Unload(log_dib);
	
	CreateFftPanel(fftplugin);

	assert(fftplugin->hbitmap == NULL);
	
	FIA_SetRainBowPalette(standard_dib);
	
	fftplugin->hbitmap = FIA_GetDibSection(standard_dib, fftplugin->canvas_hdc, 0, 0,
						 	FreeImage_GetWidth(standard_dib), FreeImage_GetHeight(standard_dib));

	FreeImage_Unload(standard_dib);
	
	ImageViewer_SetImage(fftplugin->canvas, fftplugin->hbitmap);
	
	DisplayPanel(fftplugin->panel_id);
	
	fftplugin->activated = 1;
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	if(plugin->window->live_mode)
		return 0;

	ACTIVATE_FOR_ALL_IMAGES(plugin);
	
	return 1;
}


static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	FFTPlugin *fftplugin = (FFTPlugin *) plugin;     
}

ImageWindowPlugin* fft_plugin_constructor(IcsViewerWindow *window) 
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "FFTPlugin", sizeof(FFTPlugin)); 
	FFTPlugin *fftplugin = (FFTPlugin *) plugin;   
	
	Plugin_AddMenuItem(plugin, "Processing//Fourier Transform",
		0, on_menu_clicked, fftplugin);
    	
    fftplugin->activated = 0;
    fftplugin->hbitmap = NULL;
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;  
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
