#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowMenuPlugin parent;
	
	int panel_id;
	int	scale_len_in_microns;
	int	scale_len_in_pixels;
	int active;
	int colour; 
	Point top_left;
	Point end_point;
	
} ScaleBarPlugin;


ImageWindowPlugin* scalebar_plugin_constructor(IcsViewerWindow *window);

void set_scalebar_colour(ScaleBarPlugin *plugin, COLORREF colour);

void scalebar_show_dialog(ImageWindowPlugin *plugin, IcsViewerWindow *window);
