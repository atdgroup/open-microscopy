#include "icsviewer_plugin.h" 

typedef struct _CounterPlugin CounterPlugin;

typedef struct _CounterPoint
{
	POINT pt;
	double intensity;
	unsigned char red;
	unsigned char green;  
	unsigned char blue;  
	
	
} CounterPoint;

typedef struct
{
	CounterPlugin *counter_plugin;
	
	int colour;
	int name_id;
	int id;
	int subpanel_id;
	
	ListType points; 

} Counter;

struct _CounterPlugin
{
	ImageWindowMenuPlugin parent;
	
	int panel_id;
	int	top;
	int	active;
	
	int active_counter_id;
	
	ListType counters;
};


ImageWindowPlugin* counter_plugin_constructor(IcsViewerWindow *window);
