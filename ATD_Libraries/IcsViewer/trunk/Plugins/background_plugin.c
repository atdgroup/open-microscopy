#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "background_plugin.h"
#include "ImageViewer_Drawing.h"
#include "icsviewer_uir.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "GL_CVIRegistry.h"

static int CVICALLBACK onBackgroundPanelOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	BackgroundPlugin *bg_plugin = (BackgroundPlugin *) callbackData;
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) bg_plugin;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			GetCtrlVal (bg_plugin->panel_id, BG_PNL_COLOUR_NUMERIC, &(bg_plugin->colour) );

			ics_viewer_registry_save_panel_size_position(plugin->window, bg_plugin->panel_id);

			ImageViewer_SetBackgroundColour(plugin->window->canvas_window, CviColourToColorRef(bg_plugin->colour));     
			
			DiscardPanel(bg_plugin->panel_id);
			bg_plugin->panel_id = 0;
			
			InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);

			break;
		}
	}
		
	return 0;
}


static int CVICALLBACK onBackgroundPanelColourChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	BackgroundPlugin *bg_plugin = (BackgroundPlugin *) callbackData; 
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) bg_plugin;   
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			GetCtrlVal (bg_plugin->panel_id, BG_PNL_COLOUR_NUMERIC, &(bg_plugin->colour) );
			
			ImageViewer_SetBackgroundColour(plugin->window->canvas_window, CviColourToColorRef(bg_plugin->colour));     
			
			InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE); 
			
			break;
		}
		
	return 0;
}


static void bg_show_dialog(ImageWindowPlugin *plugin, IcsViewerWindow *window) 
{
	BackgroundPlugin *bg_plugin = (BackgroundPlugin *) plugin;
	
	if(bg_plugin->panel_id <= 0) {
	
		bg_plugin->panel_id = LoadPanel(0, uir_file_path, BG_PNL);  

		ics_viewer_set_panel_to_top_left_of_window(plugin->window, bg_plugin->panel_id);

		/* Setup the ok call back */
		if ( InstallCtrlCallback (bg_plugin->panel_id, BG_PNL_OK_BUTTON, onBackgroundPanelOk, bg_plugin) < 0) {
			return;
		}
		
		/* Setup the colour numeric call back */
		if ( InstallCtrlCallback (bg_plugin->panel_id, BG_PNL_COLOUR_NUMERIC, onBackgroundPanelColourChanged, bg_plugin) < 0) {
			return;
		}
			
		SetCtrlAttribute(bg_plugin->panel_id, BG_PNL_COLOUR_NUMERIC, ATTR_DFLT_VALUE, bg_plugin->colour);
		SetCtrlVal(bg_plugin->panel_id, BG_PNL_COLOUR_NUMERIC, bg_plugin->colour); 
	}
	
	DisplayPanel(bg_plugin->panel_id);
	
	bg_plugin->active = 1;  
	
	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));        
	
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	BackgroundPlugin *bg_plugin = (BackgroundPlugin *) callbackData;
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) bg_plugin;

	bg_show_dialog(plugin, plugin->window);

	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
	
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);     
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}


static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	BackgroundPlugin *bg_plugin = (BackgroundPlugin *) plugin;     
}

ImageWindowPlugin* background_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "BackgroundPlugin", sizeof(BackgroundPlugin));

	BackgroundPlugin *bg_plugin = (BackgroundPlugin *) plugin;

	bg_plugin->panel_id = 0;
	bg_plugin->colour = 0;   // Black default colour  
	
	Plugin_AddMenuItem(plugin, "Options//Background Colour",
		VAL_MENUKEY_MODIFIER | 'B', on_menu_clicked, plugin);
	
	Plugin_UnDimMenuPathItem(plugin, "Options//Background Colour");      
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
