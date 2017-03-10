#include "icsviewer_window.h"
#include "icsviewer_private.h"
#include "grid_plugin.h"
#include "ImageViewer_Drawing.h"
#include "icsviewer_uir.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "GL_CVIRegistry.h"

static int CVICALLBACK onGridPanelOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	GridPlugin *grid_plugin = (GridPlugin *) callbackData;
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) grid_plugin;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			GetCtrlVal (grid_plugin->panel_id, GRID_PANEL_GRID_SIZE, &(grid_plugin->grid_x_size) );
			GetCtrlVal (grid_plugin->panel_id, GRID_PANEL_GRID_SIZE, &(grid_plugin->grid_y_size) );
			GetCtrlVal (grid_plugin->panel_id, GRID_PANEL_COLOUR_NUMERIC, &(grid_plugin->colour) );
 
			ics_viewer_registry_save_panel_size_position(plugin->window, grid_plugin->panel_id);

			DiscardPanel(grid_plugin->panel_id);
			grid_plugin->panel_id = 0;
			
			InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);

			break;
		}
	}
		
	return 0;
}


static int CVICALLBACK onGridPanelSize (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	GridPlugin *grid_plugin = (GridPlugin *) callbackData; 
	ImageWindowPlugin *plugin = (ImageWindowPlugin *) grid_plugin; 
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			GetCtrlVal (grid_plugin->panel_id, GRID_PANEL_GRID_SIZE, &(grid_plugin->grid_x_size) );
			GetCtrlVal (grid_plugin->panel_id, GRID_PANEL_GRID_SIZE, &(grid_plugin->grid_y_size) );
			
			ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
			
			InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE); 
			
			break;
		 }
		 
	return 0;
}


static void CloseGridPanel(GridPlugin *grid_plugin)
{
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) grid_plugin;

	ics_viewer_registry_save_panel_size_position(plugin->window, grid_plugin->panel_id);

	DiscardPanel(grid_plugin->panel_id);

	grid_plugin->panel_id = 0;
}


static int CVICALLBACK onGridPanelCancel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	GridPlugin *grid_plugin = (GridPlugin *) callbackData; 
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) grid_plugin;
	
	switch (event)
		{
		case EVENT_COMMIT:

			CloseGridPanel(grid_plugin); 
			
			grid_plugin->active = 0;    
			
			Plugin_UncheckMenuItems(plugin);  
			
			ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
	
			InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);  
			
			break;
		}
		
	return 0;
}


static int CVICALLBACK onGridPanelColourChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	GridPlugin *grid_plugin = (GridPlugin *) callbackData; 
	ImageWindowPlugin *plugin = (ImageWindowPlugin*) grid_plugin;   
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			GetCtrlVal (grid_plugin->panel_id, GRID_PANEL_COLOUR_NUMERIC, &(grid_plugin->colour) );
			
			ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
			
			InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE); 
			
			break;
		}
		
	return 0;
}


static void grid_show_dialog(ImageWindowPlugin *plugin, IcsViewerWindow *window) 
{
	GridPlugin *grid_plugin = (GridPlugin *) plugin;
	
	if(grid_plugin->panel_id <= 0) {
	
		grid_plugin->panel_id = LoadPanel(0, uir_file_path, GRID_PANEL);  
		
		ics_viewer_set_panel_to_top_left_of_window(plugin->window, grid_plugin->panel_id);

		/* Setup the ok call back */
		if ( InstallCtrlCallback (grid_plugin->panel_id, GRID_PANEL_OK_BUTTON, onGridPanelOk, grid_plugin) < 0) {
			return;
		}
		
		/* Setup the cancel call back */
		if ( InstallCtrlCallback (grid_plugin->panel_id, GRID_PANEL_CANCEL_BUTTON, onGridPanelCancel, grid_plugin) < 0) {
			return;
		}
			
		/* Setup the spinbutton call back */
		if ( InstallCtrlCallback (grid_plugin->panel_id, GRID_PANEL_GRID_SIZE, onGridPanelSize, grid_plugin) < 0) {
			return;
		}
			
		/* Setup the colour numeric call back */
		if ( InstallCtrlCallback (grid_plugin->panel_id, GRID_PANEL_COLOUR_NUMERIC, onGridPanelColourChanged, grid_plugin) < 0) {
			return;
		}
			
		SetCtrlAttribute(grid_plugin->panel_id, GRID_PANEL_COLOUR_NUMERIC, ATTR_DFLT_VALUE, grid_plugin->colour);
		SetCtrlVal(grid_plugin->panel_id, GRID_PANEL_COLOUR_NUMERIC, grid_plugin->colour); 
		
		SetCtrlVal(grid_plugin->panel_id, GRID_PANEL_GRID_SIZE, grid_plugin->grid_x_size);    
	}
	
	DisplayPanel(grid_plugin->panel_id);
	
	grid_plugin->active = 1;  
	
	ImageViewer_Redraw(PLUGIN_CANVAS(plugin));        
	
	InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);
}


void on_buffer_paint(ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	RECT drawing_rect, rect;
	POINT p1, p2;
	int sizex, sizey, width, height, i, vert_scroll, horz_scroll, image_width, image_height;
	double zoom_factor;
	
	GridPlugin *grid_plugin = (GridPlugin *) plugin;
	
	if(!grid_plugin->active)
		return;
	
	ImageViewer_GetDisplayedImageRect(PLUGIN_CANVAS(plugin), &rect);
	
	zoom_factor = ImageViewer_GetZoomFactor(PLUGIN_CANVAS(plugin));
	
	horz_scroll = ImageViewer_GetHorizontalScrollPosition(PLUGIN_CANVAS(plugin)); 
	vert_scroll = ImageViewer_GetVerticalScrollPosition(PLUGIN_CANVAS(plugin));  
	
	image_width = FreeImage_GetWidth(plugin->window->panel_dib);
	image_height = FreeImage_GetHeight(plugin->window->panel_dib);
	
	drawing_rect.left = rect.left - horz_scroll;
	drawing_rect.right = (int) (rect.left + image_width * zoom_factor - horz_scroll);
	drawing_rect.top = rect.top  - vert_scroll;	
	drawing_rect.bottom = (int) (rect.top + image_height * zoom_factor - vert_scroll);	

	width = drawing_rect.right - drawing_rect.left;
	height = drawing_rect.bottom - drawing_rect.top;

	sizex = width / grid_plugin->grid_x_size;
	sizey = height / grid_plugin->grid_y_size;

	p1.y = drawing_rect.top ; 
	p2.y = p1.y + height;
	
	for (i=1; i < grid_plugin->grid_x_size; i++) {
	
		p1.x = sizex * i + drawing_rect.left;											 
		p2.x = p1.x;
		
		ImageViewer_DrawLine(PLUGIN_CANVAS(plugin), p1, p2, grid_plugin->line_width, CviColourToColorRef(grid_plugin->colour), PS_SOLID, NULL); 
	}
	
	p1.x = drawing_rect.left;
	p2.x = p1.x + width;
	
	for (i=1; i < grid_plugin->grid_y_size; i++) {
	
		p1.y = sizey * i + drawing_rect.top;
		p2.y = p1.y;
   	
   		ImageViewer_DrawLine(PLUGIN_CANVAS(plugin), p1, p2, grid_plugin->line_width, CviColourToColorRef(grid_plugin->colour), PS_SOLID, NULL); 
	}
}


static void CVICALLBACK
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	GridPlugin *grid_plugin = (GridPlugin *) callbackData;
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) grid_plugin;

	if(!IS_MENU_CHECKED) {
	
		SetMenuBarAttribute (menubar, menuItem, ATTR_CHECKED, 1);       
		grid_show_dialog(plugin, plugin->window);
	}
	else {
	
		SetMenuBarAttribute (menubar, menuItem, ATTR_CHECKED, 0);   
		
		grid_plugin->active = 0;
		
		ImageViewer_Redraw(PLUGIN_CANVAS(plugin));
	
		InvalidateRect(PLUGIN_CANVAS(plugin), NULL, FALSE);  

		CloseGridPanel(grid_plugin);
	}
}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	return 1;
}


static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	GridPlugin *grid_plugin = (GridPlugin *) plugin;     
}

ImageWindowPlugin* grid_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "GridPlugin", sizeof(GridPlugin));

	GridPlugin *grid_plugin = (GridPlugin *) plugin;

	grid_plugin->panel_id = 0;
	grid_plugin->colour = 16711680;   // Red default colour  
	grid_plugin->line_width = 2;
	grid_plugin->grid_x_size = 2;
	grid_plugin->grid_y_size = 2;
	grid_plugin->active = 0;  
	
	Plugin_AddMenuItem(plugin, "Options//Grid",
		VAL_MENUKEY_MODIFIER | 'G', on_menu_clicked, plugin);
	
	PLUGIN_VTABLE(plugin, on_buffer_paint) = on_buffer_paint;		   
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
