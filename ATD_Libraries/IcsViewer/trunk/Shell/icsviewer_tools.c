#include "icsviewer_window.h"
#include "icsviewer_tools.h"
#include "icsviewer_private.h"

#include "ImageViewer.h"  

#include "gci_utils.h"

#include "roi_tool.h"
#include "zoom_tool.h"
#include "line_tool.h"
#include "palettebar_plugin.h"

static void DisableAllOtherTools(Tool *tool)
{
	int i; ImageWindowPlugin *plugin = NULL; 
	
	for(i=1; i <= PLUGIN_WINDOW(tool)->number_of_plugins; i++) {  
	
		plugin = GetPluginPtrInList(PLUGIN_WINDOW(tool), i);    
		
		if(tool == (Tool*) plugin)
			continue;
		
		if(plugin->type != TOOL_PLUGIN)
			continue;
		
		DeactivateTool((Tool*) plugin);  
	} 
}

static int ToolButtonToggle (int panel, int control, Tool *tool, int prevent_disable_other_tools)
{
	int button_status;
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) tool; 
	EventData eventdataTool = NewEventData();  
	
	GetCtrlVal (panel, control, &button_status); 
	
	ImageViewer_SetInteractMode(plugin->window->canvas_window, PANNING_MODE); 
	plugin->window->cursor_type = CURSOR_PANNING; 
			
	if(prevent_disable_other_tools == 0)
		DisableAllOtherTools(tool);
		
	tool->eventFunction (tool, 1, button_status, tool->locked, tool->callback_data);   
			
	ImageViewer_Redraw(plugin->window->canvas_window);
	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
	
	eventdataTool.tool = tool;
			
	SEND_EVENT(plugin->window, on_tool_status_changed, eventdataTool, NewEventData(),
	  "on_tool_status_changed")          
				
	return 0;
}

static int CVICALLBACK OnToolButtonToggle (int panel, int control, int event,
	void *callbackData, int eventData1, int eventData2)
{
	Tool *tool = (Tool *) callbackData; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			ToolButtonToggle (panel, control, tool, 0);
			
			break;
		}
		
	return 0;
}


static int CVICALLBACK OnLockToggle (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	Tool* tool = (Tool*) callbackData;    
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) tool;   
	EventData eventdata1 = NewEventData(); 
	int button_status;    
	
	switch (event)
		{
		case EVENT_LEFT_CLICK:
		case EVENT_LEFT_DOUBLE_CLICK:

			if(tool->locked == 0)
				LockTool(tool);
			else
				UnlockTool(tool);
			
			GetCtrlVal (panel, tool->button_id, &button_status);          
			
			tool->eventFunction (tool, 0, button_status, tool->locked, tool->callback_data);  
			
			eventdata1.tool = tool;  
			
			SEND_EVENT(plugin->window, on_tool_status_changed, eventdata1, NewEventData(), "on_tool_status_changed")          
				
			break;
		}
		
	return 0;
}


void DimTool(Tool *tool)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;  
	
	SetCtrlAttribute(plugin->window->panel_id, tool->button_id, ATTR_DIMMED, 1);
	SetCtrlAttribute(plugin->window->panel_id, tool->lock_id, ATTR_DIMMED, 1);
}

void UnDimTool(Tool *tool)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;  
	
	SetCtrlAttribute(plugin->window->panel_id, tool->button_id, ATTR_DIMMED, 0);
	
	if(tool->lock_id)
		SetCtrlAttribute(plugin->window->panel_id, tool->lock_id, ATTR_DIMMED, 0); 
}


void ActivateTool(Tool *tool)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;  
	
	SetCtrlVal(plugin->window->panel_id, tool->button_id, 1);
	
	SEND_EVENT(plugin->window, on_tool_activated, NewEventData(), NewEventData(),
	  "on_tool_on_tool_activated")          			
}

void DeactivateTool(Tool *tool)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;  
	
	if(plugin == NULL)
		return;

	SetCtrlVal(plugin->window->panel_id, tool->button_id, 0);
	
	SEND_EVENT(plugin->window, on_tool_deactivated, NewEventData(), NewEventData(),
	  "on_tool_on_tool_deactivated")   
}

void HideTool(Tool *tool)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;  
	
	if(plugin == NULL)
		return;

	SetCtrlAttribute(plugin->window->panel_id, tool->button_id, ATTR_VISIBLE, 0);
}

void ShowTool(Tool *tool)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;  
	
	if(plugin == NULL)
		return;

	SetCtrlAttribute(plugin->window->panel_id, tool->button_id, ATTR_VISIBLE, 1);
}

void LockTool(Tool *tool)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;   

	tool->locked = 1;
	DisplayImageFileAtDirectory(plugin->window->panel_id, tool->lock_id, icon_file_path, "pin_in.bmp");   
}


void UnlockTool(Tool *tool)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;   
	
	tool->locked = 0;
	DisplayImageFileAtDirectory(plugin->window->panel_id, tool->lock_id, icon_file_path, "pin_out.bmp");
}


int IsToolActive(Tool *tool)
{
	int active;
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) tool;    
	
	if(plugin == NULL)
		return 0;

	GetCtrlVal(plugin->window->panel_id, tool->button_id, &active);
	
	if(active)
		return 1;
		
	return 0;
}


int IsToolLocked(Tool *tool)
{
	if(tool == NULL)
		return 0;

	return tool->locked;
}


ImageWindowPlugin* Plugin_NewToolPluginType(IcsViewerWindow *window, char *name, size_t size, char *label,
	char *icon_name, int create_lock, ToolCallbackPtr eventFunction, void *callback_data)
{
	Tool* tool = (Tool*) malloc (size);

	ImageWindowPlugin* plugin = (ImageWindowPlugin*) tool;
	
	memset(plugin, 0, sizeof(ImageWindowPlugin));	// Set plugin memebers entries to NULL   
	
	plugin->vtable = (PluginOperations*) malloc(sizeof(PluginOperations));
	memset(plugin->vtable, 0, sizeof(PluginOperations)); // Set vtable entries to NULL   

	strcpy(plugin->name, name);
	plugin->window = window;
	plugin->type = TOOL_PLUGIN;
	plugin->menuItemList = ListCreate (sizeof(int));   
	
	tool->eventFunction = eventFunction;
	tool->callback_data = callback_data;
	
	tool->button_id = NewCtrl(window->panel_id, CTRL_PICTURE_TOGGLE_BUTTON, label, window->tool_button_top, 15);
	SetCtrlAttribute(window->panel_id, tool->button_id, ATTR_WIDTH, 30);
	SetCtrlAttribute(window->panel_id, tool->button_id, ATTR_HEIGHT, 30);
	SetCtrlAttribute(window->panel_id, tool->button_id, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
	
	LoadIconIntoButton(window->panel_id, tool->button_id, icon_file_path, icon_name);  

	SetCtrlAttribute(window->panel_id, tool->button_id, ATTR_VISIBLE, 1);         
	
	tool->locked = 0;
 	tool->lock_id = 0;
 	
 	if(create_lock) {
 	
 		// Lock Control
		tool->lock_id = NewCtrl(window->panel_id, CTRL_PICTURE, "", window->tool_button_top + 5, 45);
		
		SetCtrlAttribute(window->panel_id, tool->lock_id, ATTR_WIDTH, 25);
		SetCtrlAttribute(window->panel_id, tool->lock_id, ATTR_HEIGHT, 25);
		SetCtrlAttribute(window->panel_id, tool->lock_id, ATTR_FRAME_VISIBLE, 0);

		DisplayImageFileAtDirectory(window->panel_id, tool->lock_id, icon_file_path, "pin_out.bmp");   
		
		SetCtrlAttribute(window->panel_id, tool->lock_id, ATTR_VISIBLE, 1);
	}
	
 	window->tool_button_top += 60; 
 	 
 	InstallCtrlCallback (window->panel_id, tool->button_id, OnToolButtonToggle, tool);
 	
 	if(create_lock)
 		InstallCtrlCallback (window->panel_id, tool->lock_id, OnLockToggle, tool); 	
 	
	return plugin;
}


void SetMinOnPaletteBar(IcsViewerWindow *window, double value)
{
	SetMinimumValueOnPaletteBar((PaletteBarPlugin *)window->palettebar_tool, value);
}

void SetMaxOnPaletteBar(IcsViewerWindow *window, double value)
{
	SetMaximumValueOnPaletteBar((PaletteBarPlugin *) window->palettebar_tool, value);   
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableLineTool(IcsViewerWindow *window)
{
	Tool *tool = (Tool*) window->line_tool;
	
	//UnDimTool(tool);
	ActivateTool(tool);     
	
	ToolButtonToggle (window->panel_id, tool->button_id, tool, 0);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableLineTool(GCIWindow *window)
{
	Tool *tool = (Tool*) window->line_tool;
	
	DeactivateTool(tool);   
	
	ToolButtonToggle (window->panel_id, tool->button_id, tool, 0);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DimProfileTool(IcsViewerWindow *window, int dim)
{
    Tool *tool = (Tool*) window->line_tool;
    
    if(dim)
      DimTool(tool);
    else
      UnDimTool(tool);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableCrossHair(IcsViewerWindow *window)
{
	Tool *tool = (Tool*) window->crosshair_tool;
	
	//UnDimTool(tool);
	ActivateTool(tool);     
	
	ToolButtonToggle (window->panel_id, tool->button_id, tool, 0);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableCrossHair(GCIWindow *window)
{
	Tool *tool = (Tool*) window->crosshair_tool;
	
	DeactivateTool(tool);   
	
	ToolButtonToggle (window->panel_id, tool->button_id, tool, 0);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DimCrossHairTool(IcsViewerWindow *window, int dim)
{
    Tool *tool = (Tool*) window->crosshair_tool;
    
    if(dim)
      DimTool(tool);
    else
      UnDimTool(tool);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableZoomTool(GCIWindow *window)
{
	Tool *tool = (Tool*) window->zoom_tool;
	
	//UnDimTool(tool);
	ActivateTool(tool);     
	
	ToolButtonToggle (window->panel_id, tool->button_id, tool, 0);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableZoomTool(GCIWindow *window)
{
	Tool *tool = (Tool*) window->zoom_tool;
	
	DeactivateTool(tool);   
	
	ToolButtonToggle (window->panel_id, tool->button_id, tool, 0);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DimZoomTool(IcsViewerWindow *window, int dim)
{
    Tool *tool = (Tool*) window->zoom_tool;
    
    if(dim)
      DimTool(tool);
    else
      UnDimTool(tool);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_EnableRoiTool(GCIWindow *window)
{
	Tool *tool = (Tool*) window->roi_tool;
	
	//UnDimTool(tool);
	ActivateTool(tool);     
	
	ToolButtonToggle (window->panel_id, tool->button_id, tool, 0);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_DisableRoiTool(GCIWindow *window)
{
	Tool *tool = (Tool*) window->roi_tool;
	
	if(tool == NULL)
		return;

	DeactivateTool(tool);   
	
	ToolButtonToggle (window->panel_id, tool->button_id, tool, 1);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_DimRoiTool(IcsViewerWindow *window, int dim)
{
    Tool *tool = (Tool*) window->roi_tool;
    
	if(tool == NULL)
		return;

    if(dim)
      DimTool(tool);
    else
      UnDimTool(tool);
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_LockProfileButton(GCIWindow *window)
{
	Tool *tool = (Tool*) window->line_tool; 
	
	if(tool == NULL)
		return;

	LockTool(tool);
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockProfileButton(GCIWindow *window)
{
	Tool *tool = (Tool*) window->line_tool; 
	
	if(tool == NULL)
		return;

    UnlockTool(tool);		
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_LockCrossHairButton(GCIWindow *window)
{
	Tool *tool = (Tool*) window->crosshair_tool; 
	
	if(tool == NULL)
		return;

	LockTool(tool);		
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockCrossHairButton(GCIWindow *window)
{
	Tool *tool = (Tool*) window->crosshair_tool; 
	
	if(tool == NULL)
		return;

    UnlockTool(tool);				
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_LockRoiButton(GCIWindow *window)
{
 	Tool *tool = (Tool*) window->roi_tool; 
	
	if(tool == NULL)
		return;

	LockTool(tool);		
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_UnLockRoiButton(GCIWindow *window)
{
	Tool *tool = (Tool*) window->roi_tool; 
	
	if(tool == NULL)
		return;

    UnlockTool(tool);				
}
