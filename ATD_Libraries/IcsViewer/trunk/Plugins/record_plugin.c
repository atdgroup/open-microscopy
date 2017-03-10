#include "icsviewer_window.h"
#include "icsviewer_uir.h"
#include "record_plugin.h"
#include "ImageViewer.h"
#include "gci_utils.h"
#include <utility.h>
#include "shellapi.h" 

#include "string_utils.h"

#include "FreeImageAlgorithms_HBitmap.h"
#include "FreeImageAlgorithms_Utilities.h" 
#include "FreeImageAlgorithms_IO.h" 
#include "FreeImageAlgorithms_Arithmetic.h"   
#include "FreeImageAlgorithms_Palettes.h" 

#define FRAME_DIR "C:\\Temp\\IcsViewerFrames\\"


static void Cleanup(RecordPlugin *record_plugin)
{
	char buffer[1000]; 
	
	// Erase contents of directory.
	sprintf(buffer, "%s*.jpg", FRAME_DIR);
	DeleteFile (buffer);
	
	record_plugin->recording = 1;
	record_plugin->count = 0; 
}

static void StartRecord(RecordPlugin *record_plugin)
{
	// Start writing frames as jpegs into a tmpory dir
	// We then transcode these into a video.
	int fileSize;
	
	
	if(!FileExists (FRAME_DIR, &fileSize)) {
	
		// Make directory in "directory" to hold images
		MakeDir (FRAME_DIR);
	}
		
	record_plugin->recording = 1;
	record_plugin->count = 0; 
}


static void on_image_load (ImageWindowPlugin *plugin, EventData data1, EventData data2)  
{
	int err;
	char buffer[1000];   
	
	RecordPlugin *record_plugin = (RecordPlugin *) plugin;  
	
	if(record_plugin->recording > 0)
	{
		record_plugin->count++;      
		
		// Construct filename.
		sprintf(buffer, "%s%d.jpg", FRAME_DIR, record_plugin->count);
	
		// Save Frames
		err = FIA_SaveFIBToFile (data1.dib, buffer, BIT24);

		if(err == FIA_ERROR)
		{
			MessagePopup("Error", "Cannot save to video");
			
			
			
			return;
		}
	}
}


static void ExecuteTranscoder(RecordPlugin *record_plugin)
{
	int window_handle;
	char command[1000];
	
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) record_plugin; 
	
	GetPanelAttribute (plugin->window->panel_id, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);
 
	//sprintf(command, "", latex_file, out_directory);
	
	//printf("%s\n", params);
	
	sprintf(command, "C:\\mplayer\\memcoder \"mf://%s*.jpg\" -mf fps=25 -o output.avi -ovc lavc -lavcopts  vcodec=mpeg4", 
		FRAME_DIR);   
	
	ShellExecute((HWND) window_handle, "open", command , "", NULL, SW_SHOWNORMAL);    

	
	Delay(1);
	
	//ShellExecute((HWND) window_handle, "open", pm->pdf_reader_path , params, NULL, SW_SHOWNORMAL);
}


static int CVICALLBACK OnRecordOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	RecordPlugin *record_plugin = (RecordPlugin *) callbackData; 
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			record_plugin->recording = 0;
			record_plugin->count = 0;   
			record_plugin->panel_id = 0;
			
			// Transcode the frames.
			ExecuteTranscoder(record_plugin);
	
			Cleanup(record_plugin);    
			
			DiscardPanel(record_plugin->panel_id);    
			
			break;
		}
	}
		
	return 0;
}



static void StartWdmRecord()
{
	
}

static int CVICALLBACK OnRecordClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	RecordPlugin *record_plugin = (RecordPlugin *) callbackData; 
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			record_plugin->recording = 1;
			
			StartWdmRecord();
			
			break;
		}
	}
		
	return 0;
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	RecordPlugin *record_plugin = (RecordPlugin *) callbackData;  
	
	record_plugin->panel_id = LoadPanel(0, uir_file_path, REC_PNL);
	
	if ( InstallCtrlCallback (record_plugin->panel_id, REC_PNL_OK_BUTTON, OnRecordOkClicked, record_plugin) < 0) {
		return;
	}
	
	if ( InstallCtrlCallback (record_plugin->panel_id, REC_PNL_RECORD_BUTTON, OnRecordClicked, record_plugin) < 0) {
		return;
	}
		
	// Start Recording
	record_plugin->recording = 1;   
	record_plugin->count = 0;   
		
	StartRecord(record_plugin);
		
	DisplayPanel(record_plugin->panel_id);
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}


static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	RecordPlugin *record_plugin = (RecordPlugin *) plugin; 
	
	free(record_plugin);
}

ImageWindowPlugin* record_plugin_constructor(IcsViewerWindow *window) 
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "RecordPlugin", sizeof(RecordPlugin)); 
	RecordPlugin *record_plugin = (RecordPlugin *) plugin;
	
	Plugin_AddMenuItem(plugin, "File//Record",
		NULL, on_menu_clicked, "1");
    
    record_plugin->recording = 0;
	record_plugin->count = 0; 
	
	PLUGIN_VTABLE(plugin, on_image_load) = on_image_load;              
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;    
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
