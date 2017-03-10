#include <userint.h>
#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "roi_tool.h"
#include "ImageViewer_Drawing.h"
#include "ImageViewerUtils.h" 

#include "icsviewer_uir.h"
#include "string_utils.h"

#define HANDLE_SIZE 6
#define STATUS_BAR 2

static RECT CreateRECT(int top, int left, int width, int height)
{
	RECT rt;
	
	rt.left = left;
	rt.top = top;
	rt.right = rt.left + width;
	rt.bottom = rt.top + height;
	
	return rt;
}


static int GetWidthFromRECT(RECT *rect)
{
	return rect->right - rect->left + 1;	
}

static int GetHeightFromRECT(RECT *rect)
{
	return rect->bottom - rect->top + 1;	
}


static void NormaliseRect(RECT *rect)
{
	if(rect->right < rect->left) {
		int tmp = rect->left;
		rect->left = rect->right;
		rect->right = tmp;
	}
	
	if(rect->bottom < rect->top) {
		int tmp = rect->top;
		rect->top = rect->bottom;
		rect->bottom = tmp;
	}
}


static void CreateHandles(RoiTool *roi_tool, RECT *rect)
{
	int l, t, r, b, midv, midh, h, hb2, rect_width, rect_height;

	h   = HANDLE_SIZE;
   	hb2 = HANDLE_SIZE / 2;
	
	rect_width = rect->right - rect->left;
	rect_height = rect->bottom - rect->top;   
	
    l    = rect->left - hb2;
   	t    = rect->top  - hb2;
    r    = rect->right  - hb2;
   	b    = rect->bottom - hb2;
	midh = rect->left + (rect_width / 2) - hb2;
   	midv = rect->top + (rect_height / 2) - hb2;
      
	roi_tool->rectHandles[0].position = HANDLE_TOPLEFT;
	roi_tool->rectHandles[0].type = CURSOR_NWSE;			
	roi_tool->rectHandles[0].rect = CreateRECT (t, l, HANDLE_SIZE, HANDLE_SIZE);   
	roi_tool->rectHandles[0].cursor = VAL_SIZE_NW_SE_CURSOR;
		
	roi_tool->rectHandles[1].position = HANDLE_TOP;
	roi_tool->rectHandles[1].type = CURSOR_NS;
	roi_tool->rectHandles[1].rect   = CreateRECT (t, midh, HANDLE_SIZE, HANDLE_SIZE);
	roi_tool->rectHandles[1].cursor = VAL_SIZE_NS_CURSOR;
		
	roi_tool->rectHandles[2].position = HANDLE_TOPRIGHT;
	roi_tool->rectHandles[2].type = CURSOR_NESW; 
	roi_tool->rectHandles[2].rect = CreateRECT (t, r, HANDLE_SIZE, HANDLE_SIZE);
	roi_tool->rectHandles[2].cursor = VAL_SIZE_NE_SW_CURSOR;
		
	roi_tool->rectHandles[3].position = HANDLE_RIGHT;
	roi_tool->rectHandles[3].type = CURSOR_WE;
	roi_tool->rectHandles[3].rect = CreateRECT (midv, r, HANDLE_SIZE, HANDLE_SIZE); 
	roi_tool->rectHandles[3].cursor = VAL_SIZE_EW_CURSOR;
		
	roi_tool->rectHandles[4].position = HANDLE_RIGHTBOTTOM;
	roi_tool->rectHandles[4].type = CURSOR_NWSE;  
	roi_tool->rectHandles[4].rect = CreateRECT (b, r, HANDLE_SIZE, HANDLE_SIZE);
	roi_tool->rectHandles[4].cursor = VAL_SIZE_NW_SE_CURSOR;
		
	roi_tool->rectHandles[5].position = HANDLE_BOTTOM;
	roi_tool->rectHandles[5].type = CURSOR_NS;
	roi_tool->rectHandles[5].rect = CreateRECT (b, midh, HANDLE_SIZE, HANDLE_SIZE); 
	roi_tool->rectHandles[5].cursor = VAL_SIZE_NS_CURSOR;
		
	roi_tool->rectHandles[6].position = HANDLE_LEFTBOTTOM;
	roi_tool->rectHandles[6].type = CURSOR_NESW; 
	roi_tool->rectHandles[6].rect = CreateRECT (b, l, HANDLE_SIZE, HANDLE_SIZE);    
	roi_tool->rectHandles[6].cursor = VAL_SIZE_NE_SW_CURSOR;
		
	roi_tool->rectHandles[7].position = HANDLE_LEFT;
	roi_tool->rectHandles[7].type = CURSOR_WE;
	roi_tool->rectHandles[7].rect = CreateRECT (midv, l, HANDLE_SIZE, HANDLE_SIZE);
	roi_tool->rectHandles[7].cursor = VAL_SIZE_EW_CURSOR;
}


static int IsCanvasRectAtLeast10x10(RoiTool *roi_tool)
{
	if((roi_tool->canvas_rect.right - roi_tool->canvas_rect.left) <	10)
		return 0;
	
	if((roi_tool->canvas_rect.bottom - roi_tool->canvas_rect.top) <	10)
		return 0;
	
	return 1;
}


static int HaveWeSelectedAHandle(RoiTool *roi_tool, POINT down_point)
{
	int i;
	ImageWindowPlugin* plugin =  (ImageWindowPlugin*) roi_tool;
	
	if(roi_tool->draw_rect == 0)
		return 0;
	
	// Rectangle too small
	if(!IsCanvasRectAtLeast10x10(roi_tool))
		return 0;
	
	for(i=0; i < 8; i++)
	{
		RectHandle handle = roi_tool->rectHandles[i];
		
		if(PtInRect (&(handle.rect), down_point))
		{
			roi_tool->selectedHandle = handle.position;
		
			plugin->window->cursor_type = handle.type;
			
			PostMessage(plugin->window->canvas_window, WM_SETCURSOR,
				(WPARAM)plugin->window->canvas_window, MAKELPARAM(HTCLIENT, 0));  
				
			roi_tool->interact_type = INTERACT_HANDLE_MOVE;

			return 1;
		}
	}			

	return 0;
}


static int AreWeMovingROI(RoiTool *roi_tool, POINT down_point)
{
	ImageWindowPlugin* plugin =  (ImageWindowPlugin*) roi_tool;        
	
	if(roi_tool->draw_rect == 0)
		return 0;
	
	// Rectangle too small
	if(!IsCanvasRectAtLeast10x10(roi_tool))
		return 0;
	
	if(!PtInRect(&(roi_tool->canvas_rect), down_point))
		return 0;	
		
	roi_tool->interact_type = INTERACT_GENERAL_MOVE; 
	
	roi_tool->selection_offset.x = down_point.x - roi_tool->canvas_rect.left;
	roi_tool->selection_offset.y = down_point.y - roi_tool->canvas_rect.top;
			
	plugin->window->cursor_type = CURSOR_SIZE_ALL;
			
	PostMessage(plugin->window->canvas_window, WM_SETCURSOR,
		(WPARAM)plugin->window->canvas_window, MAKELPARAM(HTCLIENT, 0));  
			
	return 1;
 
}

static void on_mouse_down (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	RoiTool *roi_tool = (RoiTool *) plugin;   
	
	if(!IsToolActive((Tool*) roi_tool))
		return;
		
	if(!ImageViewer_IsPointWithinDisplayedImageRect(plugin->window->canvas_window, data2.point))
		return;

	roi_tool->mouse_button_is_down = 1;
	roi_tool->selectedHandle = -1;       
	roi_tool->interact_type = INTERACT_NONE;  
	
	// First check if we are selecting a handle
	if(HaveWeSelectedAHandle(roi_tool, data2.point))
		return;
	
	// Are we doing a general move ?
	if(AreWeMovingROI(roi_tool, data2.point))
		return;
	
	if(roi_tool->prevent_resize == 1)
		return;

	roi_tool->interact_type = INTERACT_DRAW;
	
	// New canvas left top position    
	roi_tool->canvas_rect.left = data2.point.x;
	roi_tool->canvas_rect.top = data2.point.y; 
}


static void on_mouse_up (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	RoiTool *roi_tool = (RoiTool *) plugin; 
	
	if(!IsToolActive((Tool*) roi_tool))
		return;
		
	NormaliseRect(&(roi_tool->canvas_rect));
	NormaliseRect(&(roi_tool->image_rect));  
	
	roi_tool->draw_rect = 0;
	
	roi_tool->mouse_button_is_down = 0;  
	
	plugin->window->cursor_type = CURSOR_NORMAL;
			
	PostMessage(plugin->window->canvas_window, WM_SETCURSOR,
		(WPARAM)plugin->window->canvas_window, MAKELPARAM(HTCLIENT, 0));  
	
	roi_tool->draw_rect = 1;   

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "RoiMoveOrResizeCompleted", GCI_VOID_POINTER,
		plugin->window);
	
	ImageViewer_Redraw(plugin->window->canvas_window);    
	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	
	Plugin_UnDimMenuPathItem(plugin, "Processing//Crop");
}



static int TranslateCanvasRectToImageRect(ImageWindowPlugin *plugin, RECT *image_rect)
{
	RoiTool* roi_tool = (RoiTool *) plugin;  
	
	POINT canvas_lt_point, canvas_rb_point;
	POINT image_lt_point, image_rb_point; 
	
	canvas_lt_point.x = roi_tool->canvas_rect.left;
	canvas_lt_point.y = roi_tool->canvas_rect.top;   
	canvas_rb_point.x = roi_tool->canvas_rect.right;
	canvas_rb_point.y = roi_tool->canvas_rect.bottom; 
	
	ImageViewer_TranslateWindowPointToImagePoint(plugin->window->canvas_window, 
				canvas_lt_point, &(image_lt_point)); 
	
//	printf("mouse_move image rect l t %d, %d\n", roi_tool->image_rect.left, roi_tool->image_rect.top); 
	
	
	ImageViewer_TranslateWindowPointToImagePoint(plugin->window->canvas_window, 
				canvas_rb_point, &(image_rb_point)); 
	
	return SetRect(image_rect,image_lt_point.x, image_lt_point.y, image_rb_point.x, image_rb_point.y);
}


static int TranslateImageRectToCanvasRect(ImageWindowPlugin *plugin, RECT *image_rect, RECT *canvas_rect)
{
	RoiTool* roi_tool = (RoiTool *) plugin;  
	
	POINT canvas_lt_point, canvas_rb_point;
	POINT image_lt_point, image_rb_point; 
	
	image_lt_point.x = roi_tool->image_rect.left;
	image_lt_point.y = roi_tool->image_rect.top;   
	image_rb_point.x = roi_tool->image_rect.right;
	image_rb_point.y = roi_tool->image_rect.bottom; 
	
	ImageViewer_TranslateImagePointToWindowPoint(plugin->window->canvas_window, 
				image_lt_point, &(canvas_lt_point)); 
	
	ImageViewer_TranslateImagePointToWindowPoint(plugin->window->canvas_window, 
				image_rb_point, &(canvas_rb_point)); 
	
	if(canvas_lt_point.x == -1)
					canvas_lt_point.x = -1;
	
	return SetRect(canvas_rect,canvas_lt_point.x, canvas_lt_point.y, canvas_rb_point.x, canvas_rb_point.y);
}

static void on_mouse_move(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	RoiTool *roi_tool = (RoiTool *) plugin;  
	int drawn_rect_width, drawn_rect_height;
	POINT p1, p2;
	
	if(!IsToolActive((Tool*) roi_tool))
		return;
		
	if(roi_tool->mouse_button_is_down == 0)
		return;
	
	drawn_rect_width =  GetWidthFromRECT(&(roi_tool->canvas_rect));
	drawn_rect_height = GetHeightFromRECT(&(roi_tool->canvas_rect));
	

	switch(roi_tool->interact_type)
	{
		case INTERACT_GENERAL_MOVE:
		{
			RECT zoomed_rect;
			
			ImageViewer_GetDisplayedImageRect(plugin->window->canvas_window, &zoomed_rect);   
			
			roi_tool->canvas_rect.left = data2.point.x - roi_tool->selection_offset.x - 1;
			roi_tool->canvas_rect.top = data2.point.y - roi_tool->selection_offset.y - 1;   
			roi_tool->canvas_rect.right = roi_tool->canvas_rect.left + drawn_rect_width - 1;
			roi_tool->canvas_rect.bottom = roi_tool->canvas_rect.top + drawn_rect_height - 1;

			if(roi_tool->canvas_rect.left < zoomed_rect.left) {
				roi_tool->canvas_rect.left = zoomed_rect.left;
				roi_tool->canvas_rect.right = roi_tool->canvas_rect.left + drawn_rect_width - 1;	
			}
		
			if(roi_tool->canvas_rect.top < zoomed_rect.top) {
				roi_tool->canvas_rect.top = zoomed_rect.top;
				roi_tool->canvas_rect.bottom = roi_tool->canvas_rect.top + drawn_rect_height - 1;	
			}
		
		
			if(roi_tool->canvas_rect.right >zoomed_rect.right) {
				roi_tool->canvas_rect.right = zoomed_rect.right;
				roi_tool->canvas_rect.left = roi_tool->canvas_rect.right - drawn_rect_width + 1;	
			}
		
	
			if(roi_tool->canvas_rect.bottom > zoomed_rect.bottom) {
				roi_tool->canvas_rect.bottom = zoomed_rect.bottom;
				roi_tool->canvas_rect.top = roi_tool->canvas_rect.bottom - drawn_rect_height + 1;	
			}
		
			break;
		}
		
		case INTERACT_HANDLE_MOVE:
		{
			RectHandle handle = roi_tool->rectHandles[roi_tool->selectedHandle];
	
			if(roi_tool->prevent_resize)
				return;

			switch(handle.position)
			{
				case HANDLE_TOPLEFT:
				{
					roi_tool->canvas_rect.left = data2.point.x;
					roi_tool->canvas_rect.top = data2.point.y;
					
					break;
				}
				
				case HANDLE_TOP:
				{
					roi_tool->canvas_rect.top = data2.point.y;  
					
					break;	
				}
				
				case HANDLE_TOPRIGHT:
				{
					roi_tool->canvas_rect.top = data2.point.y;  
					roi_tool->canvas_rect.right = data2.point.x;  
					
					break;	
				}
				
				case HANDLE_RIGHT:
				{
					roi_tool->canvas_rect.right = data2.point.x;    
					
					break;	
				}
				
				case HANDLE_RIGHTBOTTOM:
				{
					roi_tool->canvas_rect.right = data2.point.x;    
					roi_tool->canvas_rect.bottom = data2.point.y;    
					
					break;	
				}
				
				case HANDLE_BOTTOM:
				{
					roi_tool->canvas_rect.bottom = data2.point.y;  
					
					break;	
				}
				
				case HANDLE_LEFTBOTTOM:
				{
					roi_tool->canvas_rect.left = data2.point.x; 
					roi_tool->canvas_rect.bottom = data2.point.y;  
					
					break;	
				}
				
				case HANDLE_LEFT:
				{
					roi_tool->canvas_rect.left = data2.point.x;   
					
					break;	
				}
			}  
			
			break;
		}
		
		
		case INTERACT_DRAW:
		{
			roi_tool->canvas_rect.right = data2.point.x;
			roi_tool->canvas_rect.bottom = data2.point.y;
			
			break;
		}
		
	}
   
	TranslateCanvasRectToImageRect(plugin, &(roi_tool->image_rect)); 
	
	// Set status bar
	SetStatusbarText(plugin->window, STATUS_BAR, "Image roi: [%d, %d, %d, %d]",
				roi_tool->image_rect.left, roi_tool->image_rect.top,
				roi_tool->image_rect.right, roi_tool->image_rect.bottom);  
	
	roi_tool->draw_rect = 1;   
	
	ImageViewer_Redraw(plugin->window->canvas_window);
	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	
	p1.x = roi_tool->image_rect.left;
	p1.y = roi_tool->image_rect.top;
	p2.x = roi_tool->image_rect.right;
	p2.y = roi_tool->image_rect.bottom;
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(plugin->window), "RoiChanged", GCI_VOID_POINTER,
		plugin->window, GCI_POINT, p1, GCI_POINT, p2);

	return;
}

static void on_buffer_paint(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	Tool *tool =  (Tool *) plugin;  
	RoiTool *roi_tool = (RoiTool *) plugin;
	int i;
	RECT canvas_rect;
	
	if(IsToolActive(tool) || IsToolLocked(tool)) {      

		if(!roi_tool->draw_rect)
			return;

		// The roi is locked so we need to account for zoom
		TranslateImageRectToCanvasRect(plugin, &(roi_tool->image_rect), &(canvas_rect));     
	
		ImageViewer_DrawRectangle(plugin->window->canvas_window, canvas_rect, 1, RGB(255,0,0)); 
		
		if(roi_tool->prevent_resize == 0) {
		
		  CreateHandles(roi_tool, &(canvas_rect));    
		
		  // Draw Handles
		  for(i=0; i < 8; i++)
		  {
			//TranslateImageRectToCanvasRect(plugin, &(roi_tool->rectHandles[i].rect), &handle_canvas_rect);    
			
			  ImageViewer_DrawSolidRect(plugin->window->canvas_window,
				roi_tool->rectHandles[i].rect, RGB(255,0,0));
		  }
		}
	}
}


static void on_tool_status_change(Tool *tool, int main_ctrl_changed, int active, int locked, void *callback_data)
{
	ImageWindowPlugin *plugin =  (ImageWindowPlugin *) tool;
	RoiTool* roi_tool = (RoiTool *) plugin;
	
	// We dont want pan mode
	if(active) {
		roi_tool->draw_rect = 0;  
		plugin->window->cursor_type = CURSOR_CROSS;  
		ImageViewer_SetInteractMode(plugin->window->canvas_window, NO_INTERACT_MODE);  
	}
	else {
	
		// Erase statusbar panel
		SetStatusbarText(plugin->window, STATUS_BAR, "");     
		// Reset tool
		SetRectEmpty(&(roi_tool->canvas_rect));
		SetRectEmpty(&(roi_tool->image_rect));
	}
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	RoiTool* roi_tool = (RoiTool *) callbackData; 
	ImageWindowPlugin *plugin =  (ImageWindowPlugin *) callbackData;
	
	char temp_string[500]; 
	int width, height;
	double x_extents, y_extents;  

	FIBITMAP *dib = FreeImage_Copy(plugin->window->panel_dib, roi_tool->image_rect.left, roi_tool->image_rect.top,
		roi_tool->image_rect.right, roi_tool->image_rect.bottom);

	Plugin_DimAndUncheckMenuItems(plugin);    
	
	DeactivateTool((Tool *)plugin->window->roi_tool); 
	UnlockTool((Tool *)plugin->window->roi_tool);
	
	SetMenuBarAttribute (MENUBAR_ID(plugin), roi_tool->crop_menu_item, ATTR_DIMMED, 1);    
	
	// Set new MetaData
	plugin->window->filename[0] = '\0';

	assert(dib != NULL);   
	
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib); 
	
	x_extents = Plugin_GetMicronsPerPixel(plugin) * width;  
	y_extents = Plugin_GetMicronsPerPixel(plugin) * height;  
	
	sprintf(temp_string, "%.2e %.2e", x_extents, y_extents);    	
	
	GCI_ImagingWindow_SetMetaDataKey(plugin->window, "extents", temp_string);

	sprintf(temp_string, "%d %d", width, height);   
	
	GCI_ImagingWindow_SetMetaDataKey(plugin->window, "dimensions", temp_string);

	plugin->window->cropping = 1;
	
	GCI_ImagingWindow_LoadImage(plugin->window, dib); 
	
	plugin->window->cropping = 0;
}

static void on_disk_file_loaded (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	//GCI_ImagingWindow_DisableRoiTool(plugin->window);
}

static int on_validate_plugin(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	RoiTool* roi_tool = (RoiTool *) plugin;
	
	if(plugin->window->live_mode) {
		DimTool((Tool *)plugin);  
		return 0;
	}	
	
//	if(!IsToolActive((Tool *) plugin)) {
//		DimTool((Tool *)plugin);  
//		return 0;
//	}
	
// PB we should undim button even if we do not need to draw the roi
	UnDimTool((Tool *)plugin); 	 

	if(!roi_tool->draw_rect)
		return 0;
	
	return 1;
}

int	IW_DLL_CALLCONV
GCI_ImagingWindow_GetROIImageRECT(IcsViewerWindow *window, RECT *rect)
{
	// Translate to image coordinates first
	return TranslateCanvasRectToImageRect(window->roi_tool, rect);     
}	


int	IW_DLL_CALLCONV
GCI_ImagingWindow_GetROICanvasRECT(IcsViewerWindow *window, RECT *rect)
{
	RoiTool* roi_tool = (RoiTool *) window->roi_tool;        
	
	return CopyRect(rect, &(roi_tool->canvas_rect));   		
}


int	IW_DLL_CALLCONV
GCI_ImagingWindow_SetROIImageRECT(IcsViewerWindow *window, RECT *rect)
{
	RoiTool* roi_tool = (RoiTool *) window->roi_tool;  
	ImageWindowPlugin *plugin =  (ImageWindowPlugin *) roi_tool; 
	
	CopyRect(&(roi_tool->image_rect), rect);   	
	
	// Translate to image coordinates first
	TranslateImageRectToCanvasRect(plugin,&(roi_tool->image_rect), &(roi_tool->canvas_rect)) ; 
	
	roi_tool->draw_rect = 1;
	
	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));     
	
	return 0;
}

int	IW_DLL_CALLCONV
GCI_ImagingWindow_PreventRoiResize(IcsViewerWindow *window)
{
	RoiTool* roi_tool = (RoiTool *) window->roi_tool;       
	
	roi_tool->prevent_resize = 1;
  
	return 0;;
}

int	IW_DLL_CALLCONV
GCI_ImagingWindow_AllowRoiResize(IcsViewerWindow *window)
{
	RoiTool* roi_tool = (RoiTool *) window->roi_tool;       
	
	roi_tool->prevent_resize = 0;

	return 0;;
}

static void on_zoom_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	RoiTool* roi_tool = (RoiTool *) plugin->window->roi_tool;        
	
	TranslateImageRectToCanvasRect(plugin, &(roi_tool->image_rect), &(roi_tool->canvas_rect));       
}

static void on_tool_activated (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	RoiTool* roi_tool = (RoiTool *) plugin->window->roi_tool;           
}

ImageWindowPlugin* roi_tool_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewToolPluginType(window, "RoiTool",
		sizeof(RoiTool), "Roi", "roi_icon.bmp", 1, on_tool_status_change, NULL);

	RoiTool* roi_tool = (RoiTool*) plugin;

	roi_tool->crop_menu_item = Plugin_AddMenuItem(plugin, "Processing//Crop",
		VAL_MENUKEY_MODIFIER | 'X', on_menu_clicked, plugin);
	
	roi_tool->prevent_resize = 0;
	roi_tool->draw_rect = 0;
	roi_tool->mouse_button_is_down = 0;
	roi_tool->selectedHandle = -1;
	roi_tool->active = 1;
	
	SetRectEmpty(&(roi_tool->canvas_rect));
	SetRectEmpty(&(roi_tool->image_rect));
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_buffer_paint) = on_buffer_paint;
	PLUGIN_VTABLE(plugin, on_mouse_down) = on_mouse_down;
	PLUGIN_VTABLE(plugin, on_mouse_up) = on_mouse_up;
	PLUGIN_VTABLE(plugin, on_mouse_move) = on_mouse_move;
	PLUGIN_VTABLE(plugin, on_disk_file_loaded) = on_disk_file_loaded;    
	PLUGIN_VTABLE(plugin, on_zoom_changed) = on_zoom_changed;       
	PLUGIN_VTABLE(plugin, on_tool_activated) = on_tool_activated;
	
	return plugin;
}
