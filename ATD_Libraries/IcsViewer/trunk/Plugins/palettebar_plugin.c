#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "icsviewer_uir.h"
#include "palettebar_plugin.h"
#include "ImageViewer.h"

#include "string_utils.h"
#include "gci_menu_utils.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Utilities.h"

#define NUMBER_OF_FRAMES_BEFORE_DRAW 5

#define PALETTEBAR_MENU_ITEM_NAME "Options//Palette Bar"

static void DrawColourPaletteScale(PaletteBarPlugin* palettebar_plugin, FIBITMAP *dib)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) palettebar_plugin; 
	
	int width, height; 
	int i, current_entry, r, g, b;
	double ratio;
	Rect scale_rect, temp;
	
	if(palettebar_plugin->canvas_id < 1)
		return;

	if(plugin->window->current_palette == NULL)
		return;
		
	GetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_WIDTH, &width);
	GetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_HEIGHT, &height);

	ratio = 256.0 / height;   

	scale_rect.left = 0;
	scale_rect.top = 0;
	scale_rect.width = width;
	scale_rect.height = height;

	temp.left = scale_rect.left;
	temp.width = scale_rect.width;

	CanvasStartBatchDraw (plugin->window->panel_id, palettebar_plugin->canvas_id);

    // loop to create the gradient 
	for(i=height-1; i >= 0; i--) 
	{ 
		current_entry = (int) ((i * ratio) + 0.5);
	
    	r = plugin->window->current_palette[current_entry].rgbRed; 
  		g = plugin->window->current_palette[current_entry].rgbGreen; 
  		b = plugin->window->current_palette[current_entry].rgbBlue;
        
        temp.height = scale_rect.height - i;
        
        temp.top = temp.height - 1;
 
 		SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_PEN_FILL_COLOR, MakeColor (r, g, b));
 		
 		CanvasDrawRect (plugin->window->panel_id, palettebar_plugin->canvas_id, temp, VAL_DRAW_INTERIOR);
     }
     
     CanvasEndBatchDraw (plugin->window->panel_id, palettebar_plugin->canvas_id);

     return;
}


static void PositionMinMaxLabelHorzontally(PaletteBarPlugin* palettebar_plugin, int label_id)
{
	int panel_width;
	int label_width, label_left, canvas_center, palettebar_left;
	
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) palettebar_plugin;
	
	IcsViewerWindow *window = plugin->window;
	
	GetPanelAttribute(plugin->window->panel_id, ATTR_WIDTH, &panel_width);  
	
	palettebar_left = panel_width - IMAGE_VIEW_BORDER - PALETTEBAR_WIDTH;
	
	GetCtrlAttribute(window->panel_id, label_id, ATTR_WIDTH, &label_width);
	
	canvas_center = palettebar_left + PALETTEBAR_WIDTH / 2;
	
	label_left = canvas_center - label_width / 2;
	
	SetCtrlAttribute(window->panel_id, label_id, ATTR_LEFT, label_left); 
}



static void CenterLabelOnWithCanvas(PaletteBarPlugin* palettebar_plugin, int label_id)
{
	int canvas_left, canvas_width, label_width, label_left, canvas_center;
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) palettebar_plugin;
	
	IcsViewerWindow *window = plugin->window;
	
	GetCtrlAttribute(window->panel_id, palettebar_plugin->canvas_id, ATTR_WIDTH, &canvas_width);
	GetCtrlAttribute(window->panel_id, palettebar_plugin->canvas_id, ATTR_LEFT, &canvas_left);
	
	GetCtrlAttribute(window->panel_id, label_id, ATTR_WIDTH, &label_width);
	
	canvas_center = canvas_left + canvas_width / 2;
	
	label_left = canvas_center - label_width / 2;
	
	SetCtrlAttribute(window->panel_id, label_id, ATTR_LEFT, label_left);  
	
}


static void SetPaletteBarText(PaletteBarPlugin* palettebar_plugin, int label_ctrl, double value)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) palettebar_plugin;  

	char text[50], old_text[50];  

	format_intensity_string(plugin->window, value, text); 

	GetCtrlVal(plugin->window->panel_id, label_ctrl, old_text);     
	
	// If the label text has changed we update.
	if( strcmp(old_text, text) != 0 ) {
		SetCtrlVal(plugin->window->panel_id, label_ctrl, text);
		CenterLabelOnWithCanvas(palettebar_plugin, label_ctrl);  
	}
}


static void Show_PaletteBar(PaletteBarPlugin* palettebar_plugin, int force)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) palettebar_plugin;  
	int width, height;    
	double min, max, old_min, old_max;
	char old_min_str[50], old_max_str[50];
	
	if(FreeImage_GetBPP(plugin->window->panel_dib) == 8) {
		min = 0;
		max = 255;
	}
	else {
		min = plugin->window->panel_min_pixel_value;
		max = plugin->window->panel_max_pixel_value;
	}
	
	GetCtrlVal(plugin->window->panel_id, palettebar_plugin->max_intensity_label, old_max_str); 
	GetCtrlVal(plugin->window->panel_id, palettebar_plugin->min_intensity_label, old_min_str);
	
	sscanf(old_min_str, "%lf", &old_min);
	sscanf(old_max_str, "%lf", &old_max);      
	
	// Values unchanged don't do any repainting
	if(force == 0 && old_max == max && old_min == min)
		return;
	
 	if(!Plugin_IsMenuPathItemChecked(plugin, "Display//Linear Scale")) { 
 	
 		SetPaletteBarText(palettebar_plugin, palettebar_plugin->min_intensity_label, min);
 		SetPaletteBarText(palettebar_plugin, palettebar_plugin->max_intensity_label, max); 
 	}
 	
	Plugin_CheckMenuPathItem(plugin, PALETTEBAR_MENU_ITEM_NAME); 
		
	plugin->window->panel_palettebar_status = 1;      
	
	
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->max_intensity_label, ATTR_VISIBLE, 1);
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->min_intensity_label, ATTR_VISIBLE, 1);
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_VISIBLE, 1);
		
	DrawColourPaletteScale(palettebar_plugin, plugin->window->panel_dib);

	GetPanelAttribute(plugin->window->panel_id, ATTR_WIDTH, &width);
	GetPanelAttribute(plugin->window->panel_id, ATTR_HEIGHT, &height);    
	
	PostMessage(plugin->window->panel_window_handle, WM_SIZE, 0, MAKELPARAM(width, height)); 
}
	  	
	  							
static void Hide_PaletteBar(PaletteBarPlugin* palettebar_plugin)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) palettebar_plugin;  
	int width, height;    
	
	// Added to remove flicker for live colour as in the JVC ?
	if(plugin->window->panel_palettebar_status == 0)
		return;

	Plugin_UnCheckMenuPathItem(plugin, PALETTEBAR_MENU_ITEM_NAME);  
		
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->max_intensity_label, ATTR_VISIBLE, 0);
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->min_intensity_label, ATTR_VISIBLE, 0);
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_VISIBLE, 0);
		
	plugin->window->panel_palettebar_status = 0;
		
	GetPanelAttribute(plugin->window->panel_id, ATTR_WIDTH, &width);
	GetPanelAttribute(plugin->window->panel_id, ATTR_HEIGHT, &height);    
	
	PostMessage(plugin->window->panel_window_handle, WM_SIZE, 0, MAKELPARAM(width, height));  
}

void IW_DLL_CALLCONV
GCI_ImagingWindow_ShowPaletteBar(IcsViewerWindow *window)
{
	Show_PaletteBar((PaletteBarPlugin*)window->palettebar_tool, 1); 
}


void IW_DLL_CALLCONV
GCI_ImagingWindow_HidePaletteBar(IcsViewerWindow *window)
{
	 Hide_PaletteBar((PaletteBarPlugin*)window->palettebar_tool); 
}

void SetMaximumValueOnPaletteBar(PaletteBarPlugin* palettebar_plugin, double value)
{
	SetPaletteBarText(palettebar_plugin, palettebar_plugin->max_intensity_label, value);
}

void SetMinimumValueOnPaletteBar(PaletteBarPlugin* palettebar_plugin, double value)
{
	SetPaletteBarText(palettebar_plugin, palettebar_plugin->min_intensity_label, value);
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	PaletteBarPlugin* palettebar_plugin = (PaletteBarPlugin*) plugin;  
	
	PROFILE_START("palettebar on_validate_plugin");   
	
	if(FIA_IsGreyScale(data1.dib)) {
		//Plugin_CheckMenuPathItem(plugin, PALETTEBAR_MENU_ITEM_NAME);     
		PROFILE_STOP("palettebar on_validate_plugin");  
		return 1; 
	}
	
	Plugin_UnCheckMenuPathItem(plugin, PALETTEBAR_MENU_ITEM_NAME);     
	Hide_PaletteBar(palettebar_plugin);  
	
	PROFILE_STOP("palettebar on_validate_plugin");    
	
	return 0;
}


static void on_image_displayed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	PaletteBarPlugin* palettebar_plugin = (PaletteBarPlugin*) plugin;

	if(plugin->window->panel_palettebar_status == 0)
		return;

	if(!Plugin_IsMenuPathItemChecked(plugin, PALETTEBAR_MENU_ITEM_NAME))
		return;
	
	PROFILE_START("Show_PaletteBar");  
	
	palettebar_plugin->number_of_frames++;
	
	// If live must we throttle the amount of times we change the palette bar 
	if(plugin->window->live_mode) {
		
		if(palettebar_plugin->number_of_frames >= NUMBER_OF_FRAMES_BEFORE_DRAW) {
			Show_PaletteBar(palettebar_plugin, 0);
			palettebar_plugin->number_of_frames = 0;	
		}
	}
	else {
		
		Show_PaletteBar(palettebar_plugin, 0);       	
	}
	
	PROFILE_STOP("Show_PaletteBar"); 
}
	  

static void on_palette_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	PaletteBarPlugin* palettebar_plugin = (PaletteBarPlugin*) plugin;

	PROFILE_START("Show_PaletteBar");  
	
	Show_PaletteBar(palettebar_plugin, 1); 
	
	PROFILE_STOP("Show_PaletteBar"); 
	
	PROFILE_START("Plugin_UnDimMenuPathItem");        
	
	Plugin_UnDimMenuPathItem(plugin, PALETTEBAR_MENU_ITEM_NAME);  
	
	PROFILE_STOP("Plugin_UnDimMenuPathItem");          
}

	  	
static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	PaletteBarPlugin* palettebar_plugin = (PaletteBarPlugin*) callbackData;    
	
	if(IsMenuItemChecked(menubar, menuItem))
		Hide_PaletteBar(palettebar_plugin);
	else
		Show_PaletteBar(palettebar_plugin, 1);
}


static void CreatePaletteBar(PaletteBarPlugin* palettebar_plugin)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) palettebar_plugin;

	RECT viewer_client_rect;
	int viewer_client_width, viewer_client_height;    
	
	if(palettebar_plugin->canvas_id >= 1 || palettebar_plugin->canvas_id != -1)
		return;
	
	GetClientRect(plugin->window->canvas_window, &viewer_client_rect);
	viewer_client_width = viewer_client_rect.right - viewer_client_rect.left;
	viewer_client_height = viewer_client_rect.bottom - viewer_client_rect.top;	
	
	// Max Label
	palettebar_plugin->max_intensity_label = NewCtrl (plugin->window->panel_id, CTRL_TEXT_MSG , "", 0, 0);
	
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->max_intensity_label, ATTR_WIDTH, PALETTEBAR_WIDTH); 
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->max_intensity_label, ATTR_SIZE_TO_TEXT, 1);
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->max_intensity_label, ATTR_TEXT_JUSTIFY, VAL_CENTER_JUSTIFIED);

	// Canvas
	palettebar_plugin->canvas_id = NewCtrl (plugin->window->panel_id, CTRL_CANVAS, "", 0,0);
						 
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_WIDTH, PALETTEBAR_WIDTH);
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_HEIGHT, viewer_client_height - PALETTEBAR_WIDTH);

	// Min Label
 	palettebar_plugin->min_intensity_label = NewCtrl (plugin->window->panel_id, CTRL_TEXT_MSG , "", 0, 0);
 	
 	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->min_intensity_label, ATTR_WIDTH, PALETTEBAR_WIDTH);
 	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->min_intensity_label, ATTR_TEXT_JUSTIFY, VAL_CENTER_JUSTIFIED);
 	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->min_intensity_label, ATTR_SIZE_TO_TEXT, 1);
}


static void on_resize (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	PaletteBarPlugin* palettebar_plugin = (PaletteBarPlugin *) plugin;  
	
	RECT viewer_client_rect; 
	int panel_width, palettebar_left, palettebar_top;
	int viewer_client_width, viewer_client_height;
	int width = data1.width;
	int palettebar_height;
	
	GetPanelAttribute(plugin->window->panel_id, ATTR_WIDTH, &panel_width);  
	
	palettebar_left = panel_width - IMAGE_VIEW_BORDER - PALETTEBAR_WIDTH;
	
	GetClientRect(plugin->window->canvas_window, &viewer_client_rect);
	viewer_client_width = viewer_client_rect.right - viewer_client_rect.left;   
	viewer_client_height = viewer_client_rect.bottom - viewer_client_rect.top;
	
	palettebar_top = VIEWER_TOP + 20;
	
	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_TOP, palettebar_top);

	SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_LEFT, width - PALETTEBAR_WIDTH - 10);
	
	palettebar_height = viewer_client_height - 40;
	
	if(palettebar_height > 0) {
	
		SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->canvas_id, ATTR_HEIGHT, viewer_client_height - 40);

		SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->min_intensity_label, ATTR_TOP, palettebar_top + palettebar_height + 5);
		SetCtrlAttribute(plugin->window->panel_id, palettebar_plugin->max_intensity_label, ATTR_TOP, VIEWER_TOP);

		PositionMinMaxLabelHorzontally(palettebar_plugin, palettebar_plugin->min_intensity_label); 
		PositionMinMaxLabelHorzontally(palettebar_plugin, palettebar_plugin->max_intensity_label); 
	
		DrawColourPaletteScale(palettebar_plugin, plugin->window->panel_dib);  
	}
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	PaletteBarPlugin* palettebar_plugin = (PaletteBarPlugin *) plugin;

}

ImageWindowPlugin* palettebar_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "PaletteBarPlugin", sizeof(PaletteBarPlugin));
	
	PaletteBarPlugin* palettebar_plugin = (PaletteBarPlugin *) plugin;

	plugin->window->panel_palettebar_status = -1; 
	palettebar_plugin->canvas_id = -1;
	palettebar_plugin->min_intensity_label = -1;
	palettebar_plugin->max_intensity_label = -1;
	palettebar_plugin->number_of_frames = 0; 
	
	Plugin_AddMenuItem(plugin, PALETTEBAR_MENU_ITEM_NAME,
		VAL_MENUKEY_MODIFIER | 'B', on_menu_clicked, plugin);
			
	Plugin_UnDimMenuPathItem(plugin, PALETTEBAR_MENU_ITEM_NAME);
	Plugin_CheckMenuPathItem(plugin, PALETTEBAR_MENU_ITEM_NAME);     
		
	CreatePaletteBar(palettebar_plugin);
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin; 
	PLUGIN_VTABLE(plugin, on_image_displayed) = on_image_displayed;
	PLUGIN_VTABLE(plugin, on_palette_changed) = on_palette_changed; 
	PLUGIN_VTABLE(plugin, on_resize) = on_resize;  
	PLUGIN_VTABLE(plugin, on_exit_resize) = on_resize; 
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 

	return plugin;
}
