#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowMenuPlugin parent;
	
} HistogramEqualisationPlugin;


ImageWindowPlugin* histogram_equalisation_plugin_constructor(IcsViewerWindow *window);
