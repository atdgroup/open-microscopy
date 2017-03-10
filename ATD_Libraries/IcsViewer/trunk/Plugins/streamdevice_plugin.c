#if defined(STANDALONE_APP) || defined(STREAM_DEVICE_PLUGIN)

#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "streamdevice_plugin.h"
#include "directshow_wrapper.h"

#include "icsviewer_uir.h"
#include "ImageViewer.h"

#include <utility.h>
#include "shellapi.h" 
#include "save_plugin.h"

#include "gci_utils.h"
#include "string_utils.h"
#include "GL_CVIRegistry.h"

#include "FreeImage.h"
#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_HBitmap.h"
#include "FreeImageAlgorithms_Utilities.h"

#include "asynctmr.h"

struct _StreamPlugin
{
	ImageWindowPlugin parent;
	DSC *dsc;					// DirectShow Capture Wrapper

	int NumberOfVideoInputs;

	HINSTANCE hDll;
	int panel_id;
	int	device_id_opened;
	int window_handle;
	LONG_PTR  		panel_original_proc_fun_ptr;
	int fast_display;
	int codecChoice;
	int device_count;
	int last_image_failed;
	int force_8bit;

	DscFilterInfo *dsc_device_filter_array;
	DscFilterInfo *dsc_codec_filter_array;
	Crossbar	  *crossbar_array;

	char fake_stream_filepath[500];
	char fname[500];

	#ifdef FAKE_DEVICE_STREAM
	int fake_stream_timer_id;
	#endif
};


LRESULT CALLBACK StreamPlugin_WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	StreamPlugin *stream_plugin = (StreamPlugin *) data;
	IcsViewerWindow *ics_window = stream_plugin->parent.window;

	switch(message) {
		
		case WM_GRAPHNOTIFY:
		{
			printf("processing graph notify\n");
			directshow_process_wndproc_event (stream_plugin->dsc);
	
			break;
		}
	}

	return CallWindowProc ((WNDPROC) stream_plugin->panel_original_proc_fun_ptr,
							(HWND) stream_plugin->window_handle, message, wParam, lParam);
}


static void on_resize (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) plugin;  
	IcsViewerWindow *ics_window = stream_plugin->parent.window;
	RECT rc;

	if(stream_plugin->dsc == NULL)
		return;

    // Make the preview video fill our window
    GetClientRect(ics_window->canvas_window, &rc);

	directshow_resize_video_window(stream_plugin->dsc, rc);
}

static int CVICALLBACK OnOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			HidePanel(stream_plugin->panel_id);    
			
			break;
		}
	}
		
	return 0;
}

static int CVICALLBACK OnRecordClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) stream_plugin; 
	IcsViewerWindow *window = (IcsViewerWindow *) plugin->window;

	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val;
			char directory[GCI_MAX_PATHNAME_LEN] = "";

			GetCtrlVal(panel, control, &val);
	
			//directshow_stop_capture(stream_plugin->dsc);

			// If toggle button down start live.
			if(val) {

				int codec_index;
				char fpath[GCI_MAX_PATHNAME_LEN] = "";
			
				if (LessLameFileSelectPopup (window->panel_id,
					GetDefaultDirectoryPath(window, directory), "*.avi",
					"*.avi", "Save Video As", VAL_OK_BUTTON, 0, 0, 1, 1, fpath) <= 0) {
					return -1;
				}

				// Set new default directory basewd on user selection.
				GetDirectoryForFile(fpath, directory);
				strncpy(window->default_directory, directory, 499);

				// Get the directory for fname and save it in the registry. As we only that directory by default next time.
				checkRegistryValueForString (1, REGKEY_HKCU, REGISTRY_SUBKEY, "DefaultDir", directory);
	
				// Get Code or compressor
				GetCtrlVal(stream_plugin->panel_id, STREAM_PNL_CODEC_RING, &codec_index);    

				GCI_ImagingWindow_SetLiveStatus(plugin->window, 1);       
			
				GCI_ImagingWindow_SetWindowTitle(plugin->window, "Stream Acquired Image - Recording"); 
	
				directshow_setup_save_capture_to_file(stream_plugin->dsc, fpath, codec_index, plugin->window->canvas_window);

				directshow_run(stream_plugin->dsc);

				GCI_ImagingWindow_DisableAllActions(plugin->window);
			}
			else {

				directshow_stop_capture(stream_plugin->dsc);

				GCI_ImagingWindow_EnableAllActions(plugin->window);
			}

			break;
		}
	}
		
	return 0;
}




static void DimCaptureControls(StreamPlugin *stream_plugin)
{
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_INPUT_CHANNEL_RING, ATTR_DIMMED, 1);
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_CODEC_RING, ATTR_DIMMED, 1); 
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, ATTR_DIMMED, 1);     
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_SNAP_BUTTON, ATTR_DIMMED, 1);     
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_RECORD_BUTTON, ATTR_DIMMED, 1); 	
}

static void UnDimCaptureControls(StreamPlugin *stream_plugin)
{
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_INPUT_CHANNEL_RING, ATTR_DIMMED, 0); 
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_CODEC_RING, ATTR_DIMMED, 0); 
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, ATTR_DIMMED, 0);     
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_SNAP_BUTTON, ATTR_DIMMED, 0);     
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_RECORD_BUTTON, ATTR_DIMMED, 0); 	
}
	
static int CVICALLBACK OnInputChannelRingChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 

	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			int val;

			GetCtrlVal(panel, control, &val);

			directshow_crossbar_set_video_input(stream_plugin->dsc, &stream_plugin->crossbar_array[val]);

			break;
		}
	}
		
	return 0;
}


static int DiscoverDevices(StreamPlugin *stream_plugin)
{
	directshow_enum_filters(stream_plugin->dsc, CLSID_VideoInputDeviceCategory, NULL, &(stream_plugin->device_count));

	if(stream_plugin->device_count == 0) {
		return -1;
	}

	if(stream_plugin->dsc_device_filter_array != NULL)
		free(stream_plugin->dsc_device_filter_array);

	stream_plugin->dsc_device_filter_array = malloc(sizeof(DscFilterInfo) * stream_plugin->device_count);

	directshow_enum_filters(stream_plugin->dsc, CLSID_VideoInputDeviceCategory,
		stream_plugin->dsc_device_filter_array, &(stream_plugin->device_count));

	return stream_plugin->device_count;
}

static int stream_device_reopen_device(StreamPlugin *stream_plugin)
{
	return directshow_reopen_device(stream_plugin->dsc);
}
static int stream_device_open_device(StreamPlugin *stream_plugin, int device)
{
	// Here we close the device and the reopen the select one.
	int menu_bar, number_of_crossbar_dialogs;

	if(stream_plugin->device_id_opened == device)
		return 0;

	directshow_close_device(stream_plugin->dsc);

	if(directshow_open_device(stream_plugin->dsc, device) != S_OK) {
		goto Error;
	}

	stream_plugin->device_id_opened = device;

	directshow_get_capture_filter_property_page_count(stream_plugin->dsc, &number_of_crossbar_dialogs);

	menu_bar = GetPanelMenuBar (stream_plugin->panel_id);	

	if (number_of_crossbar_dialogs < 1) {
		SetMenuBarAttribute (menu_bar, MENUBAR_MENU_VID_CAP_CB, ATTR_DIMMED, 1);
		SetMenuBarAttribute (menu_bar, MENUBAR_MENU_VID_CAP_CB2, ATTR_DIMMED, 1);
	}
	else if(number_of_crossbar_dialogs < 2) {
		SetMenuBarAttribute (menu_bar, MENUBAR_MENU_VID_CAP_CB2, ATTR_DIMMED, 1);
	}

	if(directshow_setup_live_capture(stream_plugin->dsc) != S_OK)
		goto Error;

	if(directshow_run(stream_plugin->dsc) != S_OK)
		goto Error;

	return 0;

Error:
	
	GCI_MessagePopup("Error", "Failed to run device. Is the device attached ?");

	return -1;
}

static int CVICALLBACK OnLiveClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) stream_plugin;  
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val, device_id;

			GetCtrlVal(panel, control, &val);
	
			// If toggle button down start live.
			if(val) {
			
				GetCtrlVal(stream_plugin->panel_id, STREAM_PNL_DEVICE_RING, &device_id);

				stream_device_open_device(stream_plugin, device_id);

				GCI_ImagingWindow_StreamDeviceLive(plugin->window);

				//SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, ATTR_DIMMED, 1);
				//ProcessDrawEvents();
			}
			else {

				GCI_ImagingWindow_StreamDeviceStopCapture(plugin->window);
			}
			
			break;
		}
	}
		
	return 0;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceForce8bit(IcsViewerWindow *window, int force)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;
    
	if(force != 0)
		force = 1;

	stream_plugin->force_8bit = force;

	return GCI_IMAGING_SUCCESS;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_UseStreamDeviceWithNameLike(IcsViewerWindow *window, const char *name, int count)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;
	int i, device_count = 0;

	DiscoverDevices(stream_plugin);

	if(window->stream_plugin == NULL || stream_plugin->dsc == NULL)
		return GCI_IMAGING_ERROR;

	for(i=0; i < stream_plugin->device_count; i++) {

		if(strncmp(name, stream_plugin->dsc_device_filter_array[i].friendly_name, count) == 0) {
			
			stream_device_open_device(stream_plugin,  stream_plugin->dsc_device_filter_array[i].filter_id);
			return GCI_IMAGING_SUCCESS;
		}
	}

	return GCI_IMAGING_ERROR;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_UseStreamDeviceWithId(IcsViewerWindow *window, int id)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;
	int i, device_count = 0;

	DiscoverDevices(stream_plugin);

	if(window->stream_plugin == NULL || stream_plugin->dsc == NULL)
		return GCI_IMAGING_ERROR;

	for(i=0; i < stream_plugin->device_count; i++) {

		if(stream_plugin->dsc_device_filter_array[i].filter_id == id) {
			
			if(stream_device_open_device(stream_plugin,  stream_plugin->dsc_device_filter_array[i].filter_id) < 0)
				return GCI_IMAGING_ERROR;
			
			stream_plugin->dsc_device_filter_array[i].filter_id = -1;
			return GCI_IMAGING_SUCCESS;
		}
	}

	return GCI_IMAGING_ERROR;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_PrintStreamDevices(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;
	int i, device_count = 0;

	DiscoverDevices(stream_plugin);

	if(window->stream_plugin == NULL || stream_plugin->dsc == NULL)
		return GCI_IMAGING_ERROR;

	for(i=0; i < stream_plugin->device_count; i++) {

		printf("Device %s Id: %d\n", stream_plugin->dsc_device_filter_array[i].friendly_name, stream_plugin->dsc_device_filter_array[i].filter_id);
	}

	return GCI_IMAGING_ERROR;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceSnap(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;
    ImageWindowPlugin * plugin = (ImageWindowPlugin *) stream_plugin;     
    
	//SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_SNAP_BUTTON, ATTR_DIMMED, 1);
	//ProcessDrawEvents();

	GCI_ImagingWindow_EnableAllActions(plugin->window);

	directshow_snap_image(stream_plugin->dsc);

	// Turn Live button off.
	SetCtrlVal(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, 0);   
	SetCtrlVal(stream_plugin->panel_id, STREAM_PNL_RECORD_BUTTON, 0);      
			
	GCI_ImagingWindow_SetLiveStatus(plugin->window, 0);   
			
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_SNAP_BUTTON, ATTR_DIMMED, 0);
	ProcessDrawEvents();

	return GCI_IMAGING_SUCCESS;
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDevicePause(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;
    ImageWindowPlugin * plugin = (ImageWindowPlugin *) stream_plugin;     
    
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_SNAP_BUTTON, ATTR_DIMMED, 1);
	ProcessDrawEvents();

	GCI_ImagingWindow_EnableAllActions(plugin->window);

	directshow_pause(stream_plugin->dsc);

	// Turn Live button off.
	SetCtrlVal(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, 0);   
	SetCtrlVal(stream_plugin->panel_id, STREAM_PNL_RECORD_BUTTON, 0);      
			
	GCI_ImagingWindow_SetLiveStatus(plugin->window, 0);   
			
	SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_SNAP_BUTTON, ATTR_DIMMED, 0);
	ProcessDrawEvents();

	return GCI_IMAGING_SUCCESS;
}


int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceLive(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;

	SetCtrlVal(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, 1);
	ProcessDrawEvents();

	GCI_ImagingWindow_SetLiveStatus(window, 1);       
			
	GCI_ImagingWindow_SetWindowTitle(window, "Wdm Stream Acquired Image"); 
	
	if(stream_plugin->fast_display) {
		GCI_ImagingWindow_DisableAllActions(window);
		directshow_setup_fast_live_capture(stream_plugin->dsc, window->canvas_window);
	}
	else
		directshow_setup_live_capture(stream_plugin->dsc);

	directshow_run(stream_plugin->dsc);		
	
	//SetAsyncTimerAttribute (stream_plugin->fake_stream_timer_id, ASYNC_ATTR_ENABLED,  0);

	return GCI_IMAGING_SUCCESS;
}

DirectShowCapture* IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceDirectShowInterface(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;

	return stream_plugin->dsc;
}

IBaseFilter* IW_DLL_CALLCONV
GCI_ImagingWindow_GetCurrentStreamDeviceCaptureFilter(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;

	return directshow_get_capture_filter(stream_plugin->dsc);
}

IW_DLL_API int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceSetFastDisplay(IcsViewerWindow *window, int enabled)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;

	return streamdevice_plugin_set_fast_display_mode(stream_plugin, enabled);
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceStopCapture(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;

	directshow_stop_capture(stream_plugin->dsc);

	GCI_ImagingWindow_EnableAllActions(window);

	return GCI_IMAGING_SUCCESS;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceShowCapturePropertyPages(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;

	GCI_ImagingWindow_StreamDeviceStopCapture(stream_plugin->parent.window);

	if(directshow_show_capture_filter_property_pages(stream_plugin->dsc, (HWND) stream_plugin->window_handle) == S_OK)
		return GCI_IMAGING_SUCCESS;

	return GCI_IMAGING_ERROR;
}

int IW_DLL_CALLCONV
GCI_ImagingWindow_CloseStreamDevice(IcsViewerWindow *window)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;

	directshow_close_device(stream_plugin->dsc);

	return GCI_IMAGING_SUCCESS;
}

static int ShowCaptureDevices(StreamPlugin *stream_plugin)
{
	int i, device_count = 0;
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) stream_plugin;             
	
	device_count = DiscoverDevices(stream_plugin);
	
	ClearListCtrl(stream_plugin->panel_id, STREAM_PNL_DEVICE_RING);

	for(i=0; i < device_count; i++) {

		InsertListItem (stream_plugin->panel_id, STREAM_PNL_DEVICE_RING, -1,
			stream_plugin->dsc_device_filter_array[i].friendly_name, stream_plugin->dsc_device_filter_array[i].filter_id);  
	}

	if(device_count <= 0) {
		GCI_MessagePopup("Error", "No capture devices found");
		return 0;
	}
	else if(device_count == 1) {

		// Only one device so we try to open it
		stream_device_open_device(stream_plugin,  stream_plugin->dsc_device_filter_array[0].filter_id);
	}

	return device_count;
}

static int stream_device_get_device_inputs(StreamPlugin *stream_plugin)
{
	int i, number_of_crossbars = 0;

	// Update the channel input dropdown.

	ClearListCtrl(stream_plugin->panel_id, STREAM_PNL_INPUT_CHANNEL_RING);

	if(directshow_crossbar_get_video_inputs(stream_plugin->dsc, NULL, &number_of_crossbars) == S_FALSE) {
		return 0;
	}

	if(stream_plugin->crossbar_array != NULL)
		free(stream_plugin->crossbar_array);

	stream_plugin->crossbar_array = (Crossbar*) malloc(sizeof(Crossbar) * number_of_crossbars);
	
	directshow_crossbar_get_video_inputs(stream_plugin->dsc, stream_plugin->crossbar_array, &number_of_crossbars);

	for(i=0; i < number_of_crossbars; i++) {

		InsertListItem (stream_plugin->panel_id, STREAM_PNL_INPUT_CHANNEL_RING, -1,
			stream_plugin->crossbar_array[i].friendly_name, i);  
	}

	//UnDimCaptureControls(stream_plugin);

	return number_of_crossbars;
}

static void OpenDeviceFromList(StreamPlugin *stream_plugin)
{
	// Here we close the device and the reopen the select one.
	int val;

	GetCtrlVal(stream_plugin->panel_id, STREAM_PNL_DEVICE_RING, &val);          
	
	if(stream_plugin->device_id_opened == val)
		return;

	stream_device_open_device(stream_plugin, val);

	if(stream_device_get_device_inputs(stream_plugin) <= 0) {
		SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_INPUT_CHANNEL_RING, ATTR_DIMMED, 1);
	}
}


static int CVICALLBACK OnSnappedClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) stream_plugin;      

	switch (event)
	{
		case EVENT_COMMIT:
		{
			if(stream_plugin->dsc == NULL)
				break;

			if(stream_plugin->device_id_opened == -1)
				OpenDeviceFromList(stream_plugin);

			GCI_ImagingWindow_StreamDeviceSnap(plugin->window);
	
			break;
		}
	}
		
	return 0;
}

static int CVICALLBACK OnDeviceRingChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 
	
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			OpenDeviceFromList(stream_plugin);
		}
	}
		
	return 0;
}


static void CVICALLBACK OnCaptureFilterMenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{  
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 

	GCI_ImagingWindow_StreamDeviceShowCapturePropertyPages(stream_plugin->parent.window);
}

static void CVICALLBACK OnCapturePinMenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{  
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 

	GCI_ImagingWindow_StreamDeviceStopCapture(stream_plugin->parent.window);

	directshow_show_capture_filter_pin_property_pages(stream_plugin->dsc, (HWND) stream_plugin->window_handle);
}

static void CVICALLBACK OnCaptureCrossbarsMenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{  
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 

	GCI_ImagingWindow_StreamDeviceStopCapture(stream_plugin->parent.window);

	directshow_show_capture_filter_crossbar_property_pages(stream_plugin->dsc, 1, (HWND) stream_plugin->window_handle);
}

static void CVICALLBACK OnCaptureCrossbars2MenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{  
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 

	GCI_ImagingWindow_StreamDeviceStopCapture(stream_plugin->parent.window);

	directshow_show_capture_filter_crossbar_property_pages(stream_plugin->dsc, 2, (HWND) stream_plugin->window_handle);
}

		
static int CVICALLBACK OnKeyPress (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;

		case EVENT_KEYPRESS:
		{
			switch (eventData1)
			{
				case 32: // space bar
				{					 
					StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 
	                ImageWindowPlugin * plugin = (ImageWindowPlugin *) stream_plugin;      
	
					if(stream_plugin->device_id_opened == -1)
						OpenDeviceFromList(stream_plugin);

			        GCI_ImagingWindow_StreamDeviceSnap(plugin->window);

					return 1;
				}
			}
		}
	
	}

	return 0;
}

int streamdevice_plugin_set_fast_display_mode(StreamPlugin * stream_plugin, int enabled)
{
	int menuBar;
	IcsViewerWindow *ics_window = stream_plugin->parent.window;

	if(enabled != 0)
		enabled = 1;

	menuBar = GetPanelMenuBar (stream_plugin->panel_id);	

	if(enabled) {
	
		SetMenuBarAttribute (menuBar, MENUBAR_MENU_FAST_DISPLAY, ATTR_CHECKED, 1);
		stream_plugin->fast_display = 1;
	}
	else
	{
		stream_plugin->fast_display = 0;
		SetMenuBarAttribute (menuBar, MENUBAR_MENU_FAST_DISPLAY, ATTR_CHECKED, 0);
	}

	GCI_ImagingWindow_StreamDeviceStopCapture(ics_window);

	stream_device_reopen_device(stream_plugin);

	GCI_ImagingWindow_StreamDeviceLive(ics_window);

	SetCtrlVal(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, 0); 
	SetCtrlVal(stream_plugin->panel_id, STREAM_PNL_RECORD_BUTTON, 0); 

	return GCI_IMAGING_SUCCESS;
}

static void CVICALLBACK OnFastDisplayMenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{  
	int status;
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 

	GetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, &status);

	streamdevice_plugin_set_fast_display_mode(stream_plugin, !status);
}

static void OnForce8bitDisplayMenuClicked (int menuBar, int menuItem, void *callbackData,
		int panel)
{  
	int status;
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData; 
	IcsViewerWindow *ics_window = stream_plugin->parent.window;

	GetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, &status);

	SetMenuBarAttribute (menuBar, menuItem, ATTR_CHECKED, !status);

	GCI_ImagingWindow_StreamDeviceForce8bit(ics_window, !status);
}

// Stupid hack for when vidiology was return green images.
// I was stressed and went along with this suggestion.
// Not used anymore I think.
static int FIA_CheckForDodgyImage(FIBITMAP *dib)
{
	int x, y, dodgy = 0;

    BYTE *bits;

	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	// Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit) 
    int bytespp = FreeImage_GetLine (dib) / width;

	// Check 10 bottom lines
    for(y = 0; y < 10; y++)
    {
        bits = (BYTE *) FreeImage_GetScanLine (dib, y);

        for(x = 0; x < width; x++)
        {
			if(bits[FI_RGBA_GREEN] > 0 && bits[FI_RGBA_RED] == 0 
				&& bits[FI_RGBA_BLUE] == 0) {

				dodgy = 1;
			}

            // jump to next pixel
            bits += bytespp;
        }
    }

	return dodgy;
}


#ifdef FAKE_DEVICE_STREAM

int IW_DLL_CALLCONV
GCI_ImagingWindow_StreamDeviceSetFakeImagePath(IcsViewerWindow *window, const char* path)
{
	StreamPlugin *stream_plugin = (StreamPlugin *)window->stream_plugin;

	strcpy(stream_plugin->fake_stream_filepath, path);

	return GCI_IMAGING_SUCCESS;
}

int CVICALLBACK OnFakeStreamTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
		{
			int is_live;
			double start_time = 0.0;
			FIBITMAP *dib = NULL;
			StreamPlugin *stream_plugin = (StreamPlugin*) callbackData; 
			ImageWindowPlugin * plugin = (ImageWindowPlugin *) stream_plugin; 
			IcsViewerWindow *window = plugin->window;
	
			static int count = 0;
			double fps;
			static double time = 0.0;
			const int period = 5;

			count++;

			// Calculate FPS
			if((count % period) == 0) {
				fps = (double)period/((double)clock()/(double)CLOCKS_PER_SEC-time);  
				time=(double)clock()/(double)CLOCKS_PER_SEC;  

				GCI_ImagingWindow_SetFramesPerSecondIndicator(window, fps);
			}
	
			dib =  FIA_LoadFIBFromFile(stream_plugin->fake_stream_filepath);
			
			if(stream_plugin->force_8bit) {
				FIBITMAP *tmp = FreeImage_ConvertTo8Bits(dib);
				FreeImage_Unload(dib);
				dib = tmp;
			}

			if(dib == NULL)
				return S_FALSE;

			// We have a timeout version of SendMessage here as SendMessage was deadlocking for some unknown reason. 
			//SendMessage(plugin->window->panel_window_handle, ICSVIEWER_IMAGE_LOAD, 0, (LPARAM) dib);  
			SendMessageTimeout(plugin->window->panel_window_handle, ICSVIEWER_IMAGE_LOAD, 0, (LPARAM) dib, SMTO_ABORTIFHUNG, 2000, NULL);  

			FreeImage_Unload(dib);

			stream_plugin->last_image_failed = 0;

			GetCtrlVal(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, &is_live);   

			if(is_live == 0)
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "StreamDeviceImageSnapped", GCI_VOID_POINTER, plugin->window);

			break;
		}
	}
    
    return 0;
}

#endif

static HRESULT directshow_buffer_cb (DSC* dsc, double SampleTime,
            BYTE *pBuffer, long BufferLen,  VIDEOINFOHEADER *pVih, void *callback_data)
{
	double start_time = 0.0;
	FIBITMAP *dib = NULL;
	StreamPlugin *stream_plugin = (StreamPlugin*) callback_data; 
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) stream_plugin; 
	IcsViewerWindow *window = plugin->window;
	DSW_CAPTURE_MODE mode;

	static int count = 0;
	double fps;
	static double time = 0.0;
	const int period = 5;

	count++;

	// Calculate FPS
	if((count % period) == 0) {
		fps = (double)period/((double)clock()/(double)CLOCKS_PER_SEC-time);  
		time=(double)clock()/(double)CLOCKS_PER_SEC;  

		GCI_ImagingWindow_SetFramesPerSecondIndicator(window, fps);
	}

	switch(pVih->bmiHeader.biBitCount) {

		case 24:
		case 32:
		{
			dib = FIA_LoadColourFIBFromArrayData(pBuffer, pVih->bmiHeader.biBitCount,
				pVih->bmiHeader.biWidth, pVih->bmiHeader.biHeight, 0, 1, COLOUR_ORDER_RGB);

			if(stream_plugin->force_8bit) {
				FIBITMAP *tmp = FreeImage_ConvertTo8Bits(dib);
				FreeImage_Unload(dib);
				dib = tmp;
			}

			break;
		}

		case 16:
		{
			dib = FIA_LoadGreyScaleFIBFromArrayData(
				pBuffer, pVih->bmiHeader.biBitCount, pVih->bmiHeader.biWidth,
				pVih->bmiHeader.biHeight, FIT_UINT16, 0, 0);

			break;
		}

		case 8:
		{
			dib = FIA_LoadGreyScaleFIBFromArrayData(
				pBuffer, pVih->bmiHeader.biBitCount, pVih->bmiHeader.biWidth,
				pVih->bmiHeader.biHeight, FIT_BITMAP, 0, 0);

			break;
		}

		default:
			return S_FALSE;
	}

	if(dib == NULL)
		return S_FALSE;

	// We have a timeout version of SendMessage here as SendMessage was deadlocking for some unknown reason. 
	SendMessageTimeout(plugin->window->panel_window_handle, ICSVIEWER_IMAGE_LOAD, 0, (LPARAM) dib, SMTO_ABORTIFHUNG, 2000, NULL);  
	//GCI_ImagingWindow_LoadImage(plugin->window, dib);

	FreeImage_Unload(dib);

	stream_plugin->last_image_failed = 0;

	mode = directshow_get_mode(stream_plugin->dsc);

	if(mode != DSW_CAPTURE_MODE_LIVE)
		GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "StreamDeviceImageSnapped", GCI_VOID_POINTER, plugin->window);


	//#ifdef VERBOSE_DEBUG 
	//printf("StreamDeviceImageSnapped for ics viewer %s\n", UIMODULE_GET_NAME(plugin->window));
	//#endif

	return S_OK;
}

static void LoadWdmDevicePanel(StreamPlugin *stream_plugin)
{
	int menu_bar;

	if(stream_plugin->panel_id > 0)	  // Already created
		return;
	
	stream_plugin->panel_id = LoadPanel(0, uir_file_path, STREAM_PNL);       

	GetPanelAttribute (stream_plugin->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &(stream_plugin->window_handle));
 
	/* Store the original windows procedure function pointer */
	stream_plugin->panel_original_proc_fun_ptr = GetWindowLongPtr ((HWND) stream_plugin->window_handle, GWL_WNDPROC);

	/* Store the window structure with the window for use in WndProc */
	SetWindowLongPtr ((HWND) stream_plugin->window_handle, GWLP_USERDATA, (LONG_PTR)stream_plugin);
	
	/* Set the new Wnd Proc to be called */	
	SetWindowLongPtr ((HWND) stream_plugin->window_handle, GWL_WNDPROC, (LONG_PTR)StreamPlugin_WndProc);

	menu_bar = GetPanelMenuBar (stream_plugin->panel_id);	
	
	if ( InstallMenuCallback (menu_bar, MENUBAR_MENU_VID_CAP_FILTER, OnCaptureFilterMenuClicked, stream_plugin) < 0)
		return;

	if ( InstallMenuCallback (menu_bar, MENUBAR_MENU_VID_CAP_PIN, OnCapturePinMenuClicked, stream_plugin) < 0)
		return;

	if ( InstallMenuCallback (menu_bar, MENUBAR_MENU_VID_CAP_CB, OnCaptureCrossbarsMenuClicked, stream_plugin) < 0)
		return;

	if ( InstallMenuCallback (menu_bar, MENUBAR_MENU_VID_CAP_CB2, OnCaptureCrossbars2MenuClicked, stream_plugin) < 0)
		return;

	if ( InstallMenuCallback (menu_bar, MENUBAR_MENU_FAST_DISPLAY, OnFastDisplayMenuClicked, stream_plugin) < 0)
		return;

	if ( InstallMenuCallback (menu_bar, MENUBAR_MENU_FORCE_8BIT, OnForce8bitDisplayMenuClicked, stream_plugin) < 0)
		return;

	if ( InstallCtrlCallback (stream_plugin->panel_id, STREAM_PNL_OK_BUTTON, OnOkClicked, stream_plugin) < 0) {
		return;
	}
	
	if ( InstallCtrlCallback (stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, OnLiveClicked, stream_plugin) < 0) {
		return;
	}
	
	if ( InstallCtrlCallback (stream_plugin->panel_id, STREAM_PNL_SNAP_BUTTON, OnSnappedClicked, stream_plugin) < 0) {
		return;
	}
	
	if ( InstallCtrlCallback (stream_plugin->panel_id, STREAM_PNL_RECORD_BUTTON, OnRecordClicked, stream_plugin) < 0) {
		return;
	}
		
	if ( InstallCtrlCallback (stream_plugin->panel_id, STREAM_PNL_INPUT_CHANNEL_RING, OnInputChannelRingChanged, stream_plugin) < 0) {
		return;
	}
	
	if ( InstallCtrlCallback (stream_plugin->panel_id, STREAM_PNL_DEVICE_RING, OnDeviceRingChanged, stream_plugin) < 0) {
		return;
	}	

    if ( InstallPanelCallback (stream_plugin->panel_id, OnKeyPress, stream_plugin) < 0) {
		return;
	}
		
	stream_plugin->dsc = directshow_wrapper_create((HWND) stream_plugin->window_handle, directshow_buffer_cb, stream_plugin);
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) callbackData;     
	IcsViewerWindow *ics_window = stream_plugin->parent.window;
	int i, codec_count, device_count = 0;
	
	DimCaptureControls(stream_plugin); 

	ShowCaptureDevices(stream_plugin);
		
	UnDimCaptureControls(stream_plugin);

	// Build Codec list
	// Get Codec Choice
	// First add no codec choice
	InsertListItem (stream_plugin->panel_id, STREAM_PNL_CODEC_RING, 0, "No Codec", 0);  

	directshow_enum_filters(stream_plugin->dsc, CLSID_VideoCompressorCategory, NULL, &codec_count);

	if(codec_count <= 0) {
		SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_CODEC_RING, ATTR_DIMMED, 1); 
	}

	if(stream_plugin->dsc_codec_filter_array != NULL)
		free(stream_plugin->dsc_codec_filter_array);

	stream_plugin->dsc_codec_filter_array = malloc(sizeof(DscFilterInfo) * codec_count);
	
	directshow_enum_filters(stream_plugin->dsc, CLSID_VideoCompressorCategory, stream_plugin->dsc_codec_filter_array, &codec_count);

	for(i=0; i < codec_count; i++) {

		InsertListItem (stream_plugin->panel_id, STREAM_PNL_CODEC_RING, -1,
			stream_plugin->dsc_codec_filter_array[i].friendly_name, stream_plugin->dsc_codec_filter_array[i].filter_id);  
	}

	if(stream_device_get_device_inputs(stream_plugin) <= 0) {
			SetCtrlAttribute(stream_plugin->panel_id, STREAM_PNL_INPUT_CHANNEL_RING, ATTR_DIMMED, 1);
	}

	DisplayPanel(stream_plugin->panel_id);  
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}

static void on_disk_file_loaded (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) plugin;     
	
	// Not active
	if(stream_plugin->panel_id <= 0)
		return;
	
	// Turn Live button off.
	SetCtrlVal(stream_plugin->panel_id, STREAM_PNL_LIVE_BUTTON, 0);   
	
	directshow_stop_capture(stream_plugin->dsc);              	
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	StreamPlugin *stream_plugin = (StreamPlugin *) plugin;        
	
	if(stream_plugin->dsc_device_filter_array != NULL)
		free(stream_plugin->dsc_device_filter_array);

	if(stream_plugin->dsc_codec_filter_array != NULL)
		free(stream_plugin->dsc_codec_filter_array);

	if(stream_plugin->dsc != NULL) {
		directshow_wrapper_destroy(stream_plugin->dsc);
		stream_plugin->dsc = NULL;
	}

	printf("directshow_wrapper_destroy %p\n", stream_plugin->dsc);

	/* Restore the original windows procedure function pointers */
	SetWindowLongPtr ((HWND) stream_plugin->window_handle, GWL_WNDPROC, stream_plugin->panel_original_proc_fun_ptr);
}

ImageWindowPlugin* streamdevice_plugin_constructor(IcsViewerWindow *window) 
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "StreamPlugin", sizeof(StreamPlugin)); 
	StreamPlugin *stream_plugin = (StreamPlugin *) plugin;
	
	Plugin_AddMenuItem(plugin, "File//Stream Devices",
		0, on_menu_clicked, stream_plugin);
    
	Plugin_UnDimMenuPathItem(plugin, "File//Stream Devices");    
		
	DimCaptureControls(stream_plugin);   

	stream_plugin->force_8bit = 0;
	stream_plugin->device_id_opened = -1;
	stream_plugin->dsc = NULL;
	stream_plugin->fast_display = 0;
	stream_plugin->panel_id = -1;   
	stream_plugin->dsc_device_filter_array = NULL;
	stream_plugin->dsc_codec_filter_array = NULL;
	stream_plugin->crossbar_array = NULL;
	stream_plugin->last_image_failed = 0;

	#ifdef FAKE_DEVICE_STREAM
	stream_plugin->fake_stream_timer_id = NewAsyncTimer (0.1, -1, 0, OnFakeStreamTimerTick, stream_plugin);
	SetAsyncTimerName(stream_plugin->fake_stream_timer_id, "FakeStreamTimer");
	SetAsyncTimerAttribute (stream_plugin->fake_stream_timer_id, ASYNC_ATTR_ENABLED,  0);
	#endif

	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;    
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin;
	PLUGIN_VTABLE(plugin, on_disk_file_loaded) = on_disk_file_loaded;
	PLUGIN_VTABLE(plugin, on_resize) = on_resize;  
	
	if(stream_plugin->panel_id <= 0) {
		LoadWdmDevicePanel(stream_plugin);      
	}

	return plugin;
}

#endif // #ifdef STANDALONE_APP
