#include "icsviewer_private.h" 
#include "ImageViewer_Drawing.h"
#include "crosshair_tool.h"

#include "string_utils.h"


static void draw_cross(CrossHairTool *crosshair_tool, POINT point)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) crosshair_tool;
	
	ImageViewer_DrawCross(plugin->window->canvas_window, point, crosshair_tool->crosshair_width, crosshair_tool->colour);  
}


static void on_buffer_paint(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	CrossHairTool *crosshair_tool = (CrossHairTool *) plugin;
	Tool *tool = (Tool *) plugin;
	POINT canvas_point;
	
	if(IsToolActive(tool) || IsToolLocked(tool)) {
	
		if(!crosshair_tool->draw)
			return;
		
		ImageViewer_TranslateImagePointToWindowPoint(plugin->window->canvas_window, crosshair_tool->image_point, &canvas_point);

		draw_cross(crosshair_tool, canvas_point);
	}
}


int crosshair_tool_get_image_pos(Tool *tool, IcsViewerWindow *window, int *x, int *y)
{
	CrossHairTool *crosshair_tool = (CrossHairTool *) tool;
	
	if(window->panel_dib == NULL)
		return GCI_IMAGING_ERROR;
	
	if(!crosshair_tool->draw) {
	
		*x = -1;
		*y = -1;
		
		return GCI_IMAGING_ERROR;    	
	}
	
	*x = crosshair_tool->image_point.x;
	*y = crosshair_tool->image_point.y; 
	
	
	return GCI_IMAGING_SUCCESS; 
}


int crosshair_tool_get_viewer_pos(Tool *tool, IcsViewerWindow *window, int *x, int *y)
{
	CrossHairTool *crosshair_tool = (CrossHairTool *) tool;
	
	*x = crosshair_tool->canvas_point.x;
	*y = crosshair_tool->canvas_point.y; 
	
	return GCI_IMAGING_SUCCESS; 
}

void set_crosshair_viewer_point(CrossHairTool *crosshair_tool, POINT point)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) crosshair_tool; 
	
	crosshair_tool->canvas_point = point;
	
	ImageViewer_TranslateWindowPointToImagePoint(plugin->window->canvas_window,
		crosshair_tool->canvas_point, &(crosshair_tool->image_point));
	
	crosshair_tool->draw = 1;   
	
	ImageViewer_Redraw(plugin->window->canvas_window);
	
	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
}


void set_crosshair_image_point(CrossHairTool *crosshair_tool, POINT point)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) crosshair_tool;        

	crosshair_tool->image_point = point;

	ImageViewer_TranslateImagePointToWindowPoint(plugin->window->canvas_window,
		crosshair_tool->image_point, &(crosshair_tool->canvas_point));
	
	crosshair_tool->draw = 1;   
	
	ImageViewer_Redraw(plugin->window->canvas_window);
	
	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
}


/*
void GCI_ImagingWindow_SetCrossHairImagePoint (IcsViewerWindow *window, POINT pt)  
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) window->crosshair_tool;   
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) crosshair_tool;   

	if(!IsToolActive((Tool*) plugin))
		return;
	
	if(pt.x < 0 || pt.x > FreeImage_GetWidth(plugin->window->panel_dib))
		return;

	if(pt.y < 0 || pt.x > FreeImage_GetHeight(plugin->window->panel_dib))
		return;

	crosshair_tool->image_point = pt;
	
	ImageViewer_TranslateImagePointToWindowPoint(plugin->window->canvas_window,
		crosshair_tool->image_point, &(crosshair_tool->canvas_point));

	#ifdef _CVI_DEBUG_
	
	Debug(plugin->window, "Window Point (%d,%d), Image Point (%d,%d)", data2.point.x, data2.point.y,
		crosshair_tool->image_point.x, crosshair_tool->image_point.y);
	
	#endif
	
	// If the client has specified a function to call when a profile is drawn use that instead.
	if(!GCI_Signal_IsConnected(UIMODULE_SIGNAL_TABLE(plugin->window), "Crosshair")) {

		crosshair_tool->draw = 1;
	
		ImageViewer_Redraw(plugin->window->canvas_window);
	
		InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "Crosshair", GCI_VOID_POINTER,
		plugin->window, GCI_POINT, crosshair_tool->image_point, GCI_POINT, crosshair_tool->canvas_point);
}


void GCI_ImagingWindow_SetCrossHairWindowPoint (IcsViewerWindow *window, POINT pt)  
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) window->crosshair_tool;   
	CrossHairTool *crosshair_tool = (CrossHairTool *) plugin; 
	
	if(!IsToolActive((Tool*) plugin))
		return;
	
	if(!ImageViewer_IsPointWithinDisplayedImageRect(PLUGIN_CANVAS(plugin), data2.point))
		return;
	
	crosshair_tool->canvas_point = data2.point;
	ImageViewer_TranslateWindowPointToImagePoint(PLUGIN_CANVAS(plugin), data2.point, &(crosshair_tool->image_point));        
	
	#ifdef _CVI_DEBUG_
	
	Debug(plugin->window, "Window Point (%d,%d), Image Point (%d,%d)", data2.point.x, data2.point.y,
		crosshair_tool->image_point.x, crosshair_tool->image_point.y);
	
	#endif
	
	// If the client has specified a function to call when a profile is drawn use that instead.
	if(!GCI_Signal_IsConnected(UIMODULE_SIGNAL_TABLE(plugin->window), "Crosshair")) {

		crosshair_tool->draw = 1;
	
		ImageViewer_Redraw(plugin->window->canvas_window);
	
		InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "Crosshair", GCI_VOID_POINTER,
		plugin->window, GCI_POINT, crosshair_tool->image_point, GCI_POINT, crosshair_tool->canvas_point);
}
*/

static void on_mouse_down (ImageWindowPlugin *plugin, EventData data1, EventData data2)  
{
	CrossHairTool *crosshair_tool = (CrossHairTool *) plugin; 
	
	if(!IsToolActive((Tool*) plugin))
		return;
	
	if(!ImageViewer_IsPointWithinDisplayedImageRect(PLUGIN_CANVAS(plugin), data2.point))
		return;
	
	crosshair_tool->canvas_point = data2.point;
	ImageViewer_TranslateWindowPointToImagePoint(PLUGIN_CANVAS(plugin), data2.point, &(crosshair_tool->image_point));        
	
	#ifdef _CVI_DEBUG_
	
	Debug(plugin->window, "Window Point (%d,%d), Image Point (%d,%d)", data2.point.x, data2.point.y,
		crosshair_tool->image_point.x, crosshair_tool->image_point.y);
	
	#endif
	
	// If the client has specified a function to call when a profile is drawn use that instead.
	//if(!GCI_Signal_IsConnected(UIMODULE_SIGNAL_TABLE(plugin->window), "Crosshair")) {

		crosshair_tool->draw = 1;
	
		ImageViewer_Redraw(plugin->window->canvas_window);
	
		InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	//}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "Crosshair", GCI_VOID_POINTER,
		plugin->window, GCI_POINT, crosshair_tool->image_point, GCI_POINT, crosshair_tool->canvas_point);
}

static void on_binning_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	CrossHairTool *crosshair_tool = (CrossHairTool *) plugin;
	
	int prev_bin = data1.binning;
	int binning = data2.binning;
	
	// If crosshair is visible we want to modify it even if it's not active
	if(!crosshair_tool->draw)
		return;
	
	crosshair_tool->canvas_point.x *= prev_bin;
	crosshair_tool->canvas_point.x /= binning;
	crosshair_tool->canvas_point.y *= prev_bin;
	crosshair_tool->canvas_point.y /= binning;
	crosshair_tool->image_point.x *= prev_bin;
	crosshair_tool->image_point.x /= binning;
	crosshair_tool->image_point.y *= prev_bin; 
	crosshair_tool->image_point.y /= binning; 
	
	crosshair_tool->draw = 1;
	
	ImageViewer_Redraw(plugin->window->canvas_window);
	
	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "Crosshair", GCI_VOID_POINTER,
		plugin->window, GCI_POINT, crosshair_tool->image_point, GCI_POINT, crosshair_tool->canvas_point);
}


static void on_tool_enabled(Tool *tool, int main_ctrl_changed, int active, int locked, void *callback_data)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) tool;
	
	// We dont want the canvas panning
	plugin->window->cursor_type = CURSOR_NORMAL; //CURSOR_CROSS;  
	ImageViewer_SetInteractMode(PLUGIN_CANVAS(plugin), NO_INTERACT_MODE);  
	
	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));       
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
}


static void on_disk_file_loaded (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
    Tool *tool = (Tool *) plugin;

    if(IsToolActive(tool))
	  GCI_ImagingWindow_DisableCrossHair(plugin->window);
}

static int on_validate_plugin(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}


ImageWindowPlugin* crosshair_tool_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewToolPluginType(window, "CrossHairTool",
		sizeof(CrossHairTool), "Cross Hair", "crosshair_icon.bmp", 1, on_tool_enabled, NULL);
	
	CrossHairTool* crosshair_tool = (CrossHairTool*) plugin; 
	
	
	crosshair_tool->crosshair_width = 16;    
	crosshair_tool->colour = RGB(0,255,0);
	crosshair_tool->line_width = 1;
	crosshair_tool->draw = 0;
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_buffer_paint) = on_buffer_paint;
	PLUGIN_VTABLE(plugin, on_mouse_down) = on_mouse_down;  
	PLUGIN_VTABLE(plugin, on_binning_changed) = on_binning_changed;    
	PLUGIN_VTABLE(plugin, on_disk_file_loaded) = on_disk_file_loaded; 
	
	return plugin;
}
