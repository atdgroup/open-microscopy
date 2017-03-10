#include "icsviewer_plugin.h" 

typedef struct
{
	ImageWindowMenuPlugin parent;
	
	APP_PROVIDED_SAVE_CALLBACK app_provided_callback;
	void *app_provided_callback_data;

} SavePlugin;


ImageWindowPlugin* save_plugin_constructor(IcsViewerWindow *window);

