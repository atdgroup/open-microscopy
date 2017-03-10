#include "icsviewer_window.h"
#include "ImageViewer.h"
#include "gci_utils.h"
#include "icsviewer_uir.h"
#include "ImageViewer_Drawing.h"
#include "scalebar_plugin.h"

#include "string_utils.h"

static void scalebar_set_micron_length(ScaleBarPlugin* scalebar_plugin, int length_in_microns)
{
	double bar_pixel_length_without_zoom;
	
	double zoom_factor = ImageViewer_GetZoomFactor(PLUGIN_WINDOW(scalebar_plugin)->canvas_window);  
	
	scalebar_plugin->scale_len_in_microns = length_in_microns;
	
	bar_pixel_length_without_zoom = (double) scalebar_plugin->scale_len_in_microns / \
		(PLUGIN_WINDOW(scalebar_plugin)->binning_size * PLUGIN_WINDOW(scalebar_plugin)->microns_per_pixel);
		
	scalebar_plugin->scale_len_in_pixels = (int) (floor( bar_pixel_length_without_zoom + 0.5) * zoom_factor);
}

static void on_buffer_paint (ImageWindowPlugin *plugin, EventData data1, EventData data2) 
{
	ScaleBarPlugin *scalebar_plugin = (ScaleBarPlugin *) plugin;
	RECT client_rect;
	
	POINT start_point, p1, p2;
	char text[20]; 
	int width, height, ear_length=10;
	
	if(!scalebar_plugin->active)
		return;
	
	GetClientRect(PLUGIN_CANVAS(plugin), &client_rect);
	scalebar_set_micron_length(scalebar_plugin, scalebar_plugin->scale_len_in_microns);

	width = client_rect.right - client_rect.left;
    height = client_rect.bottom - client_rect.top;
    
	start_point.x = width - 50 - scalebar_plugin->scale_len_in_pixels;
	start_point.y = height - 50;
	
	// Draw the left ear
	p1 = start_point;
	p2.x = p1.x;
	p2.y = p1.y + ear_length;
	
	ImageViewer_DrawLine(plugin->window->canvas_window, p1, p2, 2, CviColourToColorRef(scalebar_plugin->colour), PS_SOLID, NULL); 
	
	// Draw the conecting middle line
	p1 = start_point;
	p1.y += ear_length / 2;
	p2.y = p1.y; 
	p2.x = p1.x + scalebar_plugin->scale_len_in_pixels;
	
	ImageViewer_DrawLine(plugin->window->canvas_window, p1, p2, 2, CviColourToColorRef(scalebar_plugin->colour), PS_SOLID, NULL); 
	
	// Draw the right ear
	p1.x = p2.x;
	p1.y = start_point.y;
	p2.y = p1.y + ear_length;
	
	ImageViewer_DrawLine(plugin->window->canvas_window, p1, p2, 2, CviColourToColorRef(scalebar_plugin->colour), PS_SOLID, NULL); 
 
	sprintf(text, "%d um", scalebar_plugin->scale_len_in_microns);

	p1 = start_point;
	p1.y = p1.y + ear_length;
	
	ImageViewer_DrawText(plugin->window->canvas_window, CviColourToColorRef(scalebar_plugin->colour), p1, text);
}


static int CVICALLBACK onScalebarPanelOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	//int len;
	ScaleBarPlugin* scalebar_plugin = (ScaleBarPlugin*) callbackData;
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) scalebar_plugin;
	
	switch (event)
		{
		case EVENT_COMMIT:
	
			GetCtrlVal(scalebar_plugin->panel_id, SCALE_PNL_SCALEBAR_LEN_RING, &(scalebar_plugin->scale_len_in_microns));

			if(scalebar_plugin->scale_len_in_microns == 0)
				GetCtrlVal(scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, &(scalebar_plugin->scale_len_in_microns));

			scalebar_set_micron_length(scalebar_plugin, scalebar_plugin->scale_len_in_microns);

			DiscardPanel(scalebar_plugin->panel_id);
			scalebar_plugin->panel_id = -1;
			
			InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
			
			break;
		}
		
	return 0;
}


static int CVICALLBACK onScalebarPanelClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	ScaleBarPlugin* scalebar_plugin = (ScaleBarPlugin*) callbackData;
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) scalebar_plugin;    
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			if(scalebar_plugin->panel_id != -1) {
				DiscardPanel(scalebar_plugin->panel_id);
				scalebar_plugin->panel_id = -1;
			}
	
			scalebar_plugin->active = 0; 
			
			Plugin_UncheckMenuItems(plugin);  

			ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
			
			InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE); 
			
			break;
		}
		
	return 0;
}


static int CVICALLBACK onScalebarPanelColourChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	ScaleBarPlugin* scalebar_plugin = (ScaleBarPlugin*) callbackData;
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) scalebar_plugin;    
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			GetCtrlVal (scalebar_plugin->panel_id, SCALE_PNL_COLOUR_NUMERIC, &(scalebar_plugin->colour) );
			
			ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
			
			InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE); 
			
			break;
		}
		
	return 0;
}



static int CVICALLBACK onScalebarPanelRingChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	ScaleBarPlugin* scalebar_plugin = (ScaleBarPlugin*) callbackData;
	ImageWindowPlugin * plugin = (ImageWindowPlugin *) scalebar_plugin; 
	int current_len;
	
	switch (event)
		{
		case EVENT_COMMIT:
		case EVENT_VAL_CHANGED:
	
			GetCtrlVal(scalebar_plugin->panel_id, SCALE_PNL_SCALEBAR_LEN_RING, &current_len);
		
			if(current_len == 0) {
			
				SetCtrlAttribute(scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, ATTR_CTRL_MODE, VAL_NORMAL);
				SetCtrlAttribute(scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, ATTR_TEXT_BGCOLOR, VAL_WHITE);
			}
			else {
				SetCtrlAttribute(scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, ATTR_TEXT_BGCOLOR, VAL_GRAY);
				SetCtrlAttribute(scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, ATTR_CTRL_MODE, VAL_INDICATOR); 
			}
			
			GetCtrlVal(scalebar_plugin->panel_id, SCALE_PNL_SCALEBAR_LEN_RING, &(scalebar_plugin->scale_len_in_microns));

			if(scalebar_plugin->scale_len_in_microns == 0)
				GetCtrlVal(scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, &(scalebar_plugin->scale_len_in_microns));

			scalebar_set_micron_length(scalebar_plugin, scalebar_plugin->scale_len_in_microns);
			
			ImageViewer_Redraw(plugin->window->canvas_window); 
			
			InvalidateRect(plugin->window->canvas_window, NULL, FALSE);     
			
			break;
		}
		
	return 0;
}


static void on_zoom_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ScaleBarPlugin * scalebar_plugin = (ScaleBarPlugin *) plugin;
	
	scalebar_set_micron_length(scalebar_plugin, scalebar_plugin->scale_len_in_microns);
			
	ImageViewer_Redraw(plugin->window->canvas_window); 

	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
}

static void on_microns_per_pixel_changed (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ScaleBarPlugin * scalebar_plugin = (ScaleBarPlugin *) plugin;
	
	scalebar_set_micron_length(scalebar_plugin, scalebar_plugin->scale_len_in_microns);
			
	ImageViewer_Redraw(plugin->window->canvas_window); 

	InvalidateRect(plugin->window->canvas_window, NULL, FALSE);
}



void scalebar_show_dialog(ImageWindowPlugin *plugin, IcsViewerWindow *window) 
{
	ScaleBarPlugin* scalebar_plugin = (ScaleBarPlugin*) plugin;

	if(scalebar_plugin->panel_id <= 0) {

		scalebar_plugin->panel_id = LoadPanel(0, uir_file_path, SCALE_PNL);  
		
		//MoveWindowToOtherWindow (window->panel_id, scalebar_plugin->panel_id, 0, 0, 0);
		
		SetCtrlAttribute(scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, ATTR_TEXT_BGCOLOR, VAL_GRAY);
		
		/* Setup the ok call back */
		if ( InstallCtrlCallback (scalebar_plugin->panel_id, SCALE_PNL_OK_BUTTON, onScalebarPanelOk, scalebar_plugin) < 0) {
			return;
		}
			
		/* Setup the close call back */
		if ( InstallCtrlCallback (scalebar_plugin->panel_id, SCALE_PNL_CANCEL_BUTTON, onScalebarPanelClose, scalebar_plugin) < 0) {
			return;
		}
			
		/* Setup the colour numeric call back */
		if ( InstallCtrlCallback (scalebar_plugin->panel_id, SCALE_PNL_SCALEBAR_LEN_RING, onScalebarPanelRingChanged, scalebar_plugin) < 0) {
			return;
		}
			
		/* Setup the colour numeric call back */
		if ( InstallCtrlCallback (scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, onScalebarPanelRingChanged, scalebar_plugin) < 0) {
			return;
		}
		
		/* Setup the colour numeric call back */
		if ( InstallCtrlCallback (scalebar_plugin->panel_id, SCALE_PNL_COLOUR_NUMERIC, onScalebarPanelColourChanged, scalebar_plugin) < 0) {
			return;
		}
	}

	GetCtrlVal(scalebar_plugin->panel_id, SCALE_PNL_SCALEBAR_LEN_RING, &(scalebar_plugin->scale_len_in_microns));

	if(scalebar_plugin->scale_len_in_microns == 0)
		GetCtrlVal(scalebar_plugin->panel_id, SCALE_PNL_MICRON_LEN_NUMERIC, &(scalebar_plugin->scale_len_in_microns));

	scalebar_set_micron_length(scalebar_plugin, scalebar_plugin->scale_len_in_microns);
	
	DisplayPanel(scalebar_plugin->panel_id);
	
	scalebar_plugin->active = 1;
	
	ImageViewer_Redraw(plugin->window->canvas_window);        
	
	InvalidateRect(PLUGIN_WINDOW(scalebar_plugin)->canvas_window, NULL, FALSE);
}



static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)  
{
	ScaleBarPlugin *scalebar_plugin = (ScaleBarPlugin *) plugin;   

	if(Plugin_GetMicronsPerPixel(plugin) != 1.0) { 
		return 1;
	}
	else
		scalebar_plugin->active = 0; 
		
		
	return 0;
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	ScaleBarPlugin *scalebar_plugin = (ScaleBarPlugin *) callbackData;
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) callbackData; 
	
	if(IS_MENU_CHECKED) {
	
		UNCHECK_MENU_ITEM
		
		scalebar_plugin->active = 0;      
		
		ImageViewer_Redraw(plugin->window->canvas_window);
	
		InvalidateRect(plugin->window->canvas_window, NULL, FALSE);  

		if (scalebar_plugin->panel_id > 0){
			DiscardPanel(scalebar_plugin->panel_id);
			scalebar_plugin->panel_id = -1;
		}

	}
	else {
	
		scalebar_plugin->active = 1;
		
		CHECK_MENU_ITEM
		
		scalebar_show_dialog(plugin, plugin->window);      
	}
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	ScaleBarPlugin *scalebar_plugin = (ScaleBarPlugin *) plugin;  
}

ImageWindowPlugin* scalebar_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "ScaleBarPlugin", sizeof(ScaleBarPlugin));

	ScaleBarPlugin *scalebar_plugin = (ScaleBarPlugin *) plugin;

	scalebar_plugin->active = 0;  
	scalebar_plugin->scale_len_in_microns = 100;
	scalebar_plugin->scale_len_in_pixels = 0;
	scalebar_plugin->colour = 16711680;   // Red default colour 
	
	Plugin_AddMenuItem(plugin, "Options//Scale Bar",
		VAL_MENUKEY_MODIFIER | 'B', on_menu_clicked, plugin);
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin; 
	PLUGIN_VTABLE(plugin, on_buffer_paint) = on_buffer_paint;		   
	PLUGIN_VTABLE(plugin, on_zoom_changed) = on_zoom_changed;
	PLUGIN_VTABLE(plugin, on_microns_per_pixel_changed) = on_microns_per_pixel_changed;
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
