#include "icsviewer_plugin.h"

typedef struct
{
	ImageWindowMenuPlugin parent;
	
	int 			panel_id;
	
	double 			last_min_cursor_pos;
	double 			last_max_cursor_pos; 
	
	double 			min_scale_value;
	double			max_scale_value; 
	
	double 			last_min_scale_value;
	double			last_max_scale_value; 
	
	FIBITMAP *tmp_dib;
	
} LinearScalePlugin;


ImageWindowPlugin* linear_scale_plugin_constructor(IcsViewerWindow *window);

