#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowMenuPlugin parent;
	
	int panel_id;
	int colour;
	int line_width;
	int active;
	unsigned char grid_x_size;
	unsigned char grid_y_size;
	
} GridPlugin;


ImageWindowPlugin* grid_plugin_constructor(IcsViewerWindow *window);

