#include "icsviewer_window.h"
#include "line_tool.h"

#include "string_utils.h"

#include <userint.h>
#include "toolbox.h"
#include <utility.h>

#include "gci_utils.h"

#include "ImageViewer_Drawing.h" 

#include "string_utils.h"

#include "icsviewer_uir.h"

#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_Utilities.h"


static void on_mouse_down (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	LineTool *line_tool = (LineTool *) plugin;   
	POINT pt;

	if(!IsToolActive((Tool*) plugin))
		return;
	
	line_tool->draw = 0;     
	
	pt = ImageViewer_GetClosetPosOnZoomedImageFromCursorPos(PLUGIN_CANVAS(plugin));  
	
	if(!ImageViewer_IsPointWithinDisplayedImageRect(PLUGIN_CANVAS(plugin), pt))
		return;
	
	line_tool->mouse_button_is_down = 1;
	
	line_tool->canvas_p1 = pt;
	
	ImageViewer_TranslateWindowPointToImagePoint(PLUGIN_CANVAS(plugin), pt, &(line_tool->image_p1)); 
}


static void on_mouse_up (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	int x_diff, y_diff;
	POINT p1, p2;
	EventData eventdata1 = NewEventData();
	EventData eventdata2 = NewEventData();
	
	LineTool *line_tool = (LineTool *) plugin; 

	if(!IsToolActive((Tool*) plugin))
		return;
		
	line_tool->draw = 0;
	
	if(!line_tool->mouse_button_is_down)
		return;
	
	line_tool->mouse_button_is_down = 0;   
	
	line_tool->canvas_p2 = ImageViewer_GetClosetPosOnZoomedImageFromCursorPos(PLUGIN_CANVAS(plugin));     
	
	if(!ImageViewer_IsPointWithinDisplayedImageRect(PLUGIN_CANVAS(plugin), line_tool->canvas_p2))
		return;

	x_diff = abs(line_tool->canvas_p2.x - line_tool->canvas_p1.x);
	y_diff = abs(line_tool->canvas_p2.y - line_tool->canvas_p1.y);
	
	// If the profile line drawn is less than 5 in both direction we don't profile
	if(x_diff < 10 && y_diff < 10) {
		
		ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
		InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
	
		return;
	}
	
	ImageViewer_TranslateWindowPointToImagePoint(PLUGIN_CANVAS(plugin), line_tool->canvas_p2,
		&(line_tool->image_p2)); 
	
	printf("canvas up: %d image up: %d\n", line_tool->canvas_p2.y, line_tool->image_p2.y);

	line_tool->draw = 1; 
	
	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));    
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
	
	ImageViewer_TranslateImagePointToFreeImagePoint(PLUGIN_CANVAS(plugin), line_tool->image_p1, &p1);
	ImageViewer_TranslateImagePointToFreeImagePoint(PLUGIN_CANVAS(plugin), line_tool->image_p2, &p2);
	
	eventdata1.point = p1;
	eventdata2.point = p2;  
  
	SEND_EVENT(plugin->window, on_line_tool_drawn, eventdata1, eventdata2, "on_line_tool_drawn")  
}


static void on_mouse_move(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	LineTool *line_tool = (LineTool *) plugin;  
		
	if(line_tool->mouse_button_is_down == 0)
		return;
	
	// If the mouse cursor is outside the window the project back to the closet point on the canvas.
	line_tool->canvas_p2 = data2.point;
	
	ImageViewer_TranslateWindowPointToImagePoint(PLUGIN_CANVAS(plugin), line_tool->canvas_p2, &(line_tool->image_p2)); 
	
	line_tool->draw = 1;      
			
	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
	
	return;
}


static int on_validate_plugin(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}


static void adjustPoints (ImageWindowPlugin *plugin, POINT *image_p1, POINT *image_p2)
{
	LineTool *line_tool = (LineTool *) plugin;
	int width, height;
	
	width = FreeImage_GetWidth(plugin->window->panel_dib);
	height = FreeImage_GetHeight(plugin->window->panel_dib);
	
	if (width != line_tool->current_image_size.x)
	{
		if (line_tool->current_image_size.x > 0) 
		{
			(*image_p1).x = RoundRealToNearestInteger((double)(*image_p1).x * (double)width / (double)line_tool->current_image_size.x);
			(*image_p2).x = RoundRealToNearestInteger((double)(*image_p2).x * (double)width / (double)line_tool->current_image_size.x);
		}
		line_tool->current_image_size.x = width;
	}
	
	if (height != line_tool->current_image_size.y)
	{
		if (line_tool->current_image_size.y > 0) 
		{
			(*image_p1).y = RoundRealToNearestInteger((double)(*image_p1).y * (double)height / (double)line_tool->current_image_size.y);
			(*image_p2).y = RoundRealToNearestInteger((double)(*image_p2).y * (double)height / (double)line_tool->current_image_size.y);
		}
		line_tool->current_image_size.y = height;
	}
}

static void on_buffer_paint(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	LineTool *line_tool = (LineTool *) plugin;
	Tool *tool =  (Tool *) plugin; 
	POINT p1, p2, pt3, pt4, tmp_pt3, tmp_pt4;
	
	if(IsToolActive(tool) || IsToolLocked(tool)) {
	
		if(!line_tool->draw)
			return;
		
		adjustPoints (plugin, &(line_tool->image_p1), &(line_tool->image_p2)); 
		
		ImageViewer_TranslateImagePointToWindowPoint(PLUGIN_CANVAS(plugin), line_tool->image_p1, &p1);   
		ImageViewer_TranslateImagePointToWindowPoint(PLUGIN_CANVAS(plugin), line_tool->image_p2, &p2);

		pt3 = line_tool->image_p1;
		pt4 = line_tool->image_p2;
		
		if(pt3.x < pt4.x)
			pt4.x += 1;
		else if (pt3.x > pt4.x)
			pt3.x += 1;

		if(pt3.y < pt4.y)
			pt4.y += 1;
		else if (pt3.y > pt4.y)
			pt3.y += 1;

		ImageViewer_TranslateImagePointToWindowPoint(PLUGIN_CANVAS(plugin), pt3, &tmp_pt3);
		ImageViewer_TranslateImagePointToWindowPoint(PLUGIN_CANVAS(plugin), pt4, &tmp_pt4);
	
		ImageViewer_DrawLine(PLUGIN_CANVAS(plugin), tmp_pt3, tmp_pt4,
			1, MakeColor(255,0,0), PS_SOLID, NULL);
	}
}


static void on_tool_status_change(Tool *tool, int main_ctrl_changed, int active, int locked, void *callback_data)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) tool;
	
	// We dont want the canvas panning
	if(active) {
	
		// Main ctrl changed not lock. We will reset the line tool
		if(main_ctrl_changed) {
			
			LineTool *line_tool = (LineTool *) tool; 
	
			line_tool->mouse_button_is_down = 0;
			line_tool->image_p1.x = 0;
			line_tool->image_p1.y = 0;
			line_tool->image_p2.x = 0;
			line_tool->image_p2.y = 0;
		}
		
		plugin->window->cursor_type = CURSOR_CROSS;
		ImageViewer_SetInteractMode(PLUGIN_CANVAS(plugin), NO_INTERACT_MODE);  
	
	}
		
	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));       
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
}


ImageWindowPlugin* line_tool_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewToolPluginType(window, "LineTool",
		sizeof(LineTool), "Profile", "profile_icon.bmp", 1, on_tool_status_change, NULL);
	
	LineTool *line_tool = (LineTool *) plugin; 
	
	line_tool->colour = RGB(255,255,0);  // yellow
	line_tool->panel_id = 0;
	line_tool->mouse_button_is_down = 0;
	line_tool->image_p1.x = 0;
	line_tool->image_p1.y = 0;
	line_tool->image_p2.x = 0;
	line_tool->image_p2.y = 0;
	
	line_tool->draw = 0;
	
	line_tool->current_image_size.x = 0;
	line_tool->current_image_size.y = 0;
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_buffer_paint) = on_buffer_paint;
	PLUGIN_VTABLE(plugin, on_mouse_down) = on_mouse_down;  
	PLUGIN_VTABLE(plugin, on_mouse_up) = on_mouse_up;  
	PLUGIN_VTABLE(plugin, on_mouse_move) = on_mouse_move;  

	return plugin;
}
