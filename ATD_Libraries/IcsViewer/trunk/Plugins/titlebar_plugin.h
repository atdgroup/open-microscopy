#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowPlugin parent;
	
} TitlebarPlugin;


ImageWindowPlugin* titlebar_plugin_constructor(IcsViewerWindow *window);

