#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowPlugin parent;
	
} RotatePlugin;


ImageWindowPlugin* rotate_plugin_constructor(IcsViewerWindow *window);
