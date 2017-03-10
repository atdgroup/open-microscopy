#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowMenuPlugin parent;
	
} ScreenShotPlugin;


ImageWindowPlugin* screenshot_plugin_constructor(IcsViewerWindow *window);
