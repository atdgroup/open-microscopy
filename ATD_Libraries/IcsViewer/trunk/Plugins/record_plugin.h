#include "icsviewer_plugin.h" 

#include "FreeImageAlgorithms_FFT.h" 

typedef struct
{
	ImageWindowPlugin parent;
	
	int count;
	int	recording;
	int panel_id;
	int ok_button;
	
} RecordPlugin;


ImageWindowPlugin* record_plugin_constructor(IcsViewerWindow *window);
