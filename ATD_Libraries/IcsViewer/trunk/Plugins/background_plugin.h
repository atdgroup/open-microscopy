#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowMenuPlugin parent;
	
	int panel_id;
	int colour;
	int active;
	
} BackgroundPlugin;


ImageWindowPlugin* background_plugin_constructor(IcsViewerWindow *window);

