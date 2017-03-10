#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowPlugin parent;
	
} ResamplePlugin;


ImageWindowPlugin* resample_plugin_constructor(IcsViewerWindow *window);
