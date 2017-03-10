#include "icsviewer_plugin.h" 
#include "ImageViewer.h"

#include "FreeImageIcs_MetaData.h" 

typedef struct
{
	ImageWindowMenuPlugin parent;
	
	//FreeImageIcsPointer fip;    
	int info_panel;
	HWND window_handle;
	LONG_PTR original_wnd_proc;

	// Function to actually set meta data in the list control 
	APP_PROVIDED_METADATA_CALLBACK app_provided_metadata;
	
	void *app_provided_callback_data;
	
} MetaDataPlugin;


ImageWindowPlugin* metadata_plugin_constructor(IcsViewerWindow *window);

void ConvertToEditableTreeCells (int panel, int tree, int string);
