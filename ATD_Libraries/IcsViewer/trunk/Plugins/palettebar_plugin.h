#include "icsviewer_tools.h" 

typedef struct
{
	ImageWindowPlugin parent;  
	
	int canvas_id;
	int	max_intensity_label;
	int min_intensity_label;

	int number_of_frames;
	
} PaletteBarPlugin;


ImageWindowPlugin* palettebar_plugin_constructor(IcsViewerWindow *window);

void SetMaximumValueOnPaletteBar(PaletteBarPlugin* palettebar_plugin, double value);

void SetMinimumValueOnPaletteBar(PaletteBarPlugin* palettebar_plugin, double value);
