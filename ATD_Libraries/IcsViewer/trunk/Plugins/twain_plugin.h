#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowPlugin parent;
	
	int select_menu_item_id;
	int acquire_menu_item_id;
	
} TwainPlugin;


ImageWindowPlugin* twain_plugin_constructor(IcsViewerWindow *window);
