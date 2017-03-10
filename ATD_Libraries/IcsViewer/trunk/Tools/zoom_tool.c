#include "icsviewer_window.h"
#include "zoom_tool.h"
#include "ImageViewer.h"

#include "string_utils.h"

typedef enum {ZOOM_SIZE_FIT, ZOOM_SIZE_FREE, ZOOM_SIZE_20, ZOOM_SIZE_50, ZOOM_SIZE_200, ZOOM_SIZE_400} ZOOM_SIZE;


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	ZoomTool *zoom_tool = (ZoomTool *) callbackData;
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) callbackData;    
	
	if(menuItem == zoom_tool->resize_fit_item_id)
		GCI_ImagingWindow_SetResizeFitStyle(plugin->window); 
		
	if(menuItem == zoom_tool->resize_free_item_id)
		GCI_ImagingWindow_SetZoomFactor(plugin->window, 1.0); 
		
	if(menuItem == zoom_tool->zoom20_item_id)
		GCI_ImagingWindow_SetZoomFactor(plugin->window, 0.2); 
		
	if(menuItem == zoom_tool->zoom50_item_id)
		GCI_ImagingWindow_SetZoomFactor(plugin->window, 0.5); 
			
	if(menuItem == zoom_tool->zoom200_item_id)
		GCI_ImagingWindow_SetZoomFactor(plugin->window, 2.0); 
		
	if(menuItem == zoom_tool->zoom400_item_id)
	    GCI_ImagingWindow_SetZoomFactor(plugin->window, 4.0); 
	    
	if(menuItem == zoom_tool->zoom800_item_id)
	    GCI_ImagingWindow_SetZoomFactor(plugin->window, 8.0); 

	Plugin_UncheckMenuItems(plugin);
	SetMenuBarAttribute ( menubar, menuItem, ATTR_CHECKED, 1);    

	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));        
	
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
}


static void on_tool_status_change(Tool *tool, int main_ctrl_changed, int active, int locked, void *callback_data)
{
	ImageWindowPlugin *plugin =  (ImageWindowPlugin *) tool;
	
	if(active) {
		plugin->window->cursor_type = CURSOR_ZOOM;  
		ImageViewer_SetInteractMode(plugin->window->canvas_window, ZOOM_MODE);  
		PostMessage(plugin->window->canvas_window, WM_SETCURSOR,
			(WPARAM)plugin->window->canvas_window, MAKELPARAM(HTCLIENT, 0));  
	}
		
	/*
	else {
		
		// We are turning the zoom button off.
		// If the counter panel is visible then return to crosshar mode
		CounterPlugin *count_plugin = (CounterPlugin *) plugin->window->counter_plugin;
		
		if(count_plugin->active)
		{
			GCI_ImagingWindow_EnableCrossHair(plugin->window);   	
		}
	}
	*/
	
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ACTIVATE_FOR_ALL_IMAGES(plugin);
	
	return 1;
}


static void on_zoom_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	Plugin_UncheckMenuItems(plugin);   
}


ImageWindowPlugin* zoom_tool_constructor(IcsViewerWindow *window) 
{
	ImageWindowPlugin* plugin = Plugin_NewToolPluginType(window, "ZoomTool",
		sizeof(ZoomTool), "Zoom", "zoom_icon.bmp", 0, on_tool_status_change, NULL);
	
	ZoomTool *zoom_tool = (ZoomTool *) plugin;
	
	zoom_tool->resize_fit_item_id = Plugin_AddMenuItem(plugin, "Zoom//Keep Image Sized To Window",
		0, on_menu_clicked, zoom_tool);
	
	zoom_tool->zoom20_item_id = Plugin_AddMenuItem(plugin, "Zoom//20%",
		0, on_menu_clicked, zoom_tool);
    	
    zoom_tool->zoom50_item_id = Plugin_AddMenuItem(plugin, "Zoom//50%",
		0, on_menu_clicked, zoom_tool);
    	
    zoom_tool->resize_free_item_id = Plugin_AddMenuItem(plugin, "Zoom//Original size (100%)",
		0, on_menu_clicked, zoom_tool);
    	
    zoom_tool->zoom200_item_id = Plugin_AddMenuItem(plugin, "Zoom//200%",
		0, on_menu_clicked, zoom_tool);
    	
    zoom_tool->zoom400_item_id = Plugin_AddMenuItem(plugin, "Zoom//400%",
		0, on_menu_clicked, zoom_tool);
		
	zoom_tool->zoom800_item_id = Plugin_AddMenuItem(plugin, "Zoom//800%",
		0, on_menu_clicked, zoom_tool);
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin; 
	PLUGIN_VTABLE(plugin, on_zoom_changed) = on_zoom_changed;  
	
	Plugin_CheckMenuPathItem(plugin, "Zoom//Keep Image Sized To Window");

	return plugin;
}
