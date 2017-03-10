#include <userint.h>
#include "icsviewer_window.h"
#include "icsviewer_private.h" 
#include "linearscale_plugin.h"
#include "icsviewer_uir.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "GL_CVIRegistry.h"

#include "icsviewer_tools.h"

#include "FreeImageAlgorithms.h"
#include "FreeImageAlgorithms_LinearScale.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_Palettes.h"


static void PlotLinearScale(LinearScalePlugin* linear_scale_plugin, double min, double max)
{
	int i, int_range, *y_array;
	double *x_array, range, scale;
	
	if(max <= min) {
		DeleteGraphPlot (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
		return;
	}

	if((range = (max - min)) <= 0)
		return;
		
	scale = 255.0 / range;

	int_range = (int) ceil(range);

	x_array = (double *) malloc ( sizeof(double) * (int_range + 1) );
	y_array = (int *) malloc ( sizeof(int) * (int_range + 1) );
	memset(x_array, 0, sizeof(double) * (int_range + 1));
	memset(y_array, 0, sizeof(int) * (int_range + 1));
		
	for(i=0; i <= int_range; i++) {
			
		x_array[i] = i + min;     
		y_array[i] = (int) (scale * i);
	}

	// Deletes all the plots from the graph 
	DeleteGraphPlot (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, -1, VAL_IMMEDIATE_DRAW);

	PlotXY (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, x_array, y_array, int_range + 1, VAL_DOUBLE, VAL_INTEGER,
		VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_BLACK);
			
	free(x_array);
	free(y_array);  
}


static int IsGraphVisible(LinearScalePlugin* linear_scale_plugin)
{
	int visible;  
	
	if(linear_scale_plugin->panel_id <= 0)    
		return 0;

	GetPanelAttribute(linear_scale_plugin->panel_id, ATTR_VISIBLE, &visible);      
	
	if(!visible)
		return 0;
		
	return 1;
}


static void HideMinCursor(LinearScalePlugin* linear_scale_plugin)
{
	SetCursorAttribute (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 2, ATTR_CROSS_HAIR_STYLE, VAL_NO_CROSS);	
	SetCursorAttribute (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 2, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
}

static void HideMaxCursor(LinearScalePlugin* linear_scale_plugin)
{
	SetCursorAttribute (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 1, ATTR_CROSS_HAIR_STYLE, VAL_NO_CROSS);	
	SetCursorAttribute (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 1, ATTR_CURSOR_COLOR, VAL_TRANSPARENT);
}


static void ShowMinCursor(LinearScalePlugin* linear_scale_plugin)
{
	SetCursorAttribute (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 2, ATTR_CROSS_HAIR_STYLE, VAL_VERTICAL_LINE);
	SetCursorAttribute (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 2, ATTR_CURSOR_COLOR, VAL_RED);
}

static void ShowMaxCursor(LinearScalePlugin* linear_scale_plugin)
{
	SetCursorAttribute (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 1, ATTR_CROSS_HAIR_STYLE, VAL_VERTICAL_LINE);
	SetCursorAttribute (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 1, ATTR_CURSOR_COLOR, VAL_BLUE);
}


static void SetLinearScale(LinearScalePlugin* linear_scale_plugin, double min_scale_value, double max_scale_value)
{
	int mode;
	double x_min, x_max;
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) linear_scale_plugin;      
	
	if(!FIA_IsGreyScale(plugin->window->panel_dib))
		return;
	
	if(IsGraphVisible(linear_scale_plugin)) {
	
		GetAxisScalingMode(linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, VAL_BOTTOM_XAXIS, &mode, &x_min, &x_max);
		if (min_scale_value < x_min) x_min = min_scale_value;
		if (max_scale_value > x_max) x_max = max_scale_value;
		SetAxisScalingMode(linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, VAL_BOTTOM_XAXIS, VAL_MANUAL, x_min, x_max);

		SetGraphCursor (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 1, max_scale_value, 0.0);
		ShowMaxCursor(linear_scale_plugin);                

		SetGraphCursor (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 2, min_scale_value, 0.0);
		ShowMinCursor(linear_scale_plugin);
		
		SetCtrlVal(linear_scale_plugin->panel_id, LSCALE_PNL_UPPER_NUMERIC, max_scale_value);   
		SetCtrlVal(linear_scale_plugin->panel_id, LSCALE_PNL_LOWER_NUMERIC, min_scale_value);
	
		PlotLinearScale(linear_scale_plugin, min_scale_value, max_scale_value); 		
		
		SetMinOnPaletteBar(plugin->window, linear_scale_plugin->min_scale_value);
		SetMaxOnPaletteBar(plugin->window, linear_scale_plugin->max_scale_value);
	}
			
	DisplayImage(plugin->window, plugin->window->panel_dib); 
}


static int CVICALLBACK onLinearScalePanelUpperNumericChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	LinearScalePlugin* linear_scale_plugin = (LinearScalePlugin*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			GetCtrlVal(panel, control, &(linear_scale_plugin->max_scale_value));
		
			SetLinearScale(linear_scale_plugin, linear_scale_plugin->min_scale_value, linear_scale_plugin->max_scale_value);
			
			break;
		}
	}
	
	return 0;
}


static int CVICALLBACK onLinearScalePanelLowerNumericChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	LinearScalePlugin* linear_scale_plugin = (LinearScalePlugin*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			GetCtrlVal(panel, control, &(linear_scale_plugin->min_scale_value));
		
			SetLinearScale(linear_scale_plugin, linear_scale_plugin->min_scale_value, linear_scale_plugin->max_scale_value);
			
			break;
		}
	}
	
	return 0;
}


static int CVICALLBACK onLinearScalePanelGraphChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double y;
	LinearScalePlugin* linear_scale_plugin = (LinearScalePlugin*) callbackData; 
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) linear_scale_plugin; 
	
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			plugin->window->prevent_image_load = 1;
		
			GetGraphCursor (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 1, &(linear_scale_plugin->max_scale_value), &y);
			GetGraphCursor (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 2, &(linear_scale_plugin->min_scale_value), &y);
		
			if(linear_scale_plugin->last_min_cursor_pos == linear_scale_plugin->min_scale_value
				&&  linear_scale_plugin->last_max_cursor_pos == linear_scale_plugin->max_scale_value) {
			
				plugin->window->prevent_image_load = 0; 
				
				return 0;
			}
		
			// x1 is the max cursor position
			if(linear_scale_plugin->max_scale_value <= linear_scale_plugin->min_scale_value) {
				
				// The max cursor has changed
				if(eventData1 == 1)
					linear_scale_plugin->max_scale_value = linear_scale_plugin->last_max_cursor_pos; 
				else 
					linear_scale_plugin->min_scale_value = linear_scale_plugin->last_min_cursor_pos;
			}
			else {
			
				linear_scale_plugin->last_min_cursor_pos = linear_scale_plugin->min_scale_value;
				linear_scale_plugin->last_max_cursor_pos = linear_scale_plugin->max_scale_value;
			}
			
			SetLinearScale(linear_scale_plugin, linear_scale_plugin->min_scale_value, linear_scale_plugin->max_scale_value); 
			
			break;
		}
		
		case EVENT_COMMIT:
		{
			plugin->window->prevent_image_load = 0;
		
			break;
		}
	}
		
	return 0;
}


static void LinearScalePanelClose(LinearScalePlugin* linear_scale_plugin)
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) linear_scale_plugin; 
	
	if(linear_scale_plugin->panel_id > 0) {
			
		ics_viewer_registry_save_panel_size_position(plugin->window, linear_scale_plugin->panel_id);

		DiscardPanel(linear_scale_plugin->panel_id);
		linear_scale_plugin->panel_id = -1;
		
		plugin->window->prevent_image_load = 0;
	}
}

static int CVICALLBACK onLinearScalePanelOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	LinearScalePlugin* linear_scale_plugin = (LinearScalePlugin*) callbackData;  
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) linear_scale_plugin; 
	
	switch (event)
		{
		case EVENT_COMMIT:

				LinearScalePanelClose(linear_scale_plugin);  
			
				if(linear_scale_plugin->tmp_dib != NULL) {
					FreeImage_Unload(linear_scale_plugin->tmp_dib);
					linear_scale_plugin->tmp_dib = NULL;
				}
			
				plugin->window->prevent_image_load = 0;
			
			break;
		}
		
	return 0;
}


static void linear_scale_display_panel(LinearScalePlugin* linear_scale_plugin) 
{
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) linear_scale_plugin;           
	
	if(linear_scale_plugin->panel_id <= 0)
	{
		linear_scale_plugin->panel_id = LoadPanel(0, uir_file_path, LSCALE_PNL);
		
		ics_viewer_set_panel_to_top_left_of_window(plugin->window, linear_scale_plugin->panel_id);

		if ( InstallCtrlCallback (linear_scale_plugin->panel_id, LSCALE_PNL_OK_BUTTON, onLinearScalePanelOk, linear_scale_plugin) < 0)
			return;
			
		/* Setup the higher numeric call back */
		if ( InstallCtrlCallback (linear_scale_plugin->panel_id, LSCALE_PNL_UPPER_NUMERIC, onLinearScalePanelUpperNumericChanged, linear_scale_plugin) < 0)
			return;
					
		/* Setup the lower numeric call back */
		if ( InstallCtrlCallback (linear_scale_plugin->panel_id, LSCALE_PNL_LOWER_NUMERIC, onLinearScalePanelLowerNumericChanged, linear_scale_plugin) < 0)
			return;
	
		/* Setup the cursor call back */
		if ( InstallCtrlCallback (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, onLinearScalePanelGraphChanged, linear_scale_plugin) < 0)
			return;
	
		if(linear_scale_plugin->min_scale_value == 0.0 && linear_scale_plugin->max_scale_value == 0.0) {
			linear_scale_plugin->min_scale_value = plugin->window->panel_min_pixel_value;
			linear_scale_plugin->max_scale_value = plugin->window->panel_max_pixel_value;
		}

		SetAxisScalingMode(linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, 0.0, 255.0);

		SetCtrlVal(linear_scale_plugin->panel_id, LSCALE_PNL_UPPER_NUMERIC, linear_scale_plugin->max_scale_value);
		SetCtrlVal(linear_scale_plugin->panel_id, LSCALE_PNL_LOWER_NUMERIC, linear_scale_plugin->min_scale_value); 
	
		SetAxisScalingMode(linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, VAL_BOTTOM_XAXIS, VAL_MANUAL,
			plugin->window->panel_min_pixel_value, plugin->window->panel_max_pixel_value);


		if((FreeImage_GetImageType(plugin->window->panel_dib) == FIT_FLOAT) && 
			(fabs(plugin->window->panel_max_pixel_value) < 0.01 || fabs(plugin->window->panel_max_pixel_value) > 999)) {
			
			SetCtrlAttribute(linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, ATTR_XFORMAT, VAL_SCIENTIFIC_FORMAT); 
		}
		else
			SetCtrlAttribute(linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, ATTR_XFORMAT, VAL_FLOATING_PT_FORMAT);	


		PlotLinearScale(linear_scale_plugin, linear_scale_plugin->min_scale_value, linear_scale_plugin->max_scale_value);
		
		// Cursor 1 is the max 
		// Cursor 2 is the min 
		if(linear_scale_plugin->max_scale_value <= plugin->window->panel_max_pixel_value)
			SetGraphCursor (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 1, linear_scale_plugin->max_scale_value, 0.0);
		else
			HideMaxCursor(linear_scale_plugin);
		
		if(linear_scale_plugin->min_scale_value >= plugin->window->panel_min_pixel_value)	
			SetGraphCursor (linear_scale_plugin->panel_id, LSCALE_PNL_GRAPH, 2, linear_scale_plugin->min_scale_value, 0.0);
		else
			HideMinCursor(linear_scale_plugin);  
		
		linear_scale_plugin->last_min_cursor_pos = linear_scale_plugin->min_scale_value;
		linear_scale_plugin->last_max_cursor_pos = linear_scale_plugin->max_scale_value;
	}
		
	DisplayPanel(linear_scale_plugin->panel_id);
	SetLinearScale(linear_scale_plugin, linear_scale_plugin->min_scale_value, linear_scale_plugin->max_scale_value);
}


static void CVICALLBACK 
	on_menu_clicked (int menubar, int menuItem, void *callbackData, int panel)
{
	LinearScalePlugin* linear_scale_plugin = (LinearScalePlugin*) callbackData;  
	ImageWindowPlugin* plugin = (ImageWindowPlugin*) linear_scale_plugin;           
	
	if(!IS_MENU_CHECKED) {
	
		SetMenuBarAttribute (menubar, menuItem, ATTR_CHECKED, 1); 
		
		if(linear_scale_plugin->last_min_scale_value >= 0.0 && linear_scale_plugin->last_max_scale_value != 0.0) {
			linear_scale_plugin->min_scale_value = linear_scale_plugin->last_min_scale_value;
			linear_scale_plugin->max_scale_value = linear_scale_plugin->last_max_scale_value;
		}
		
		linear_scale_display_panel(linear_scale_plugin);
	}
	else {
	
		SetMenuBarAttribute (menubar, menuItem, ATTR_CHECKED, 0);     
		
		linear_scale_plugin->last_min_scale_value = linear_scale_plugin->min_scale_value;
		linear_scale_plugin->last_max_scale_value = linear_scale_plugin->max_scale_value;
		
		if (FreeImage_GetBPP(plugin->window->panel_dib) == 8) {
			linear_scale_plugin->min_scale_value = 0;
			linear_scale_plugin->max_scale_value = 255;
		}
		else {
			linear_scale_plugin->min_scale_value = plugin->window->panel_min_pixel_value;
			linear_scale_plugin->max_scale_value = plugin->window->panel_max_pixel_value;
		}
		
		SetLinearScale(linear_scale_plugin, linear_scale_plugin->min_scale_value, linear_scale_plugin->max_scale_value);   
		LinearScalePanelClose(linear_scale_plugin);  
	}

}


static int on_validate_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	if(!FIA_IsGreyScale(data1.dib))
		return 0;
		
	if(plugin->window->panel_min_pixel_value == 0.0 &&
		plugin->window->panel_max_pixel_value == 0.0) {
	
		return 0;
	}
	
 	return 1;
}

static void on_destroy_plugin (ImageWindowPlugin *plugin, EventData data1, EventData data2)
{
	LinearScalePlugin *linear_scale_plugin = (LinearScalePlugin *) plugin;          
}

ImageWindowPlugin* linear_scale_plugin_constructor(IcsViewerWindow *window)
{
	ImageWindowPlugin* plugin = Plugin_NewPluginType(window, "LinearScalePlugin", sizeof(LinearScalePlugin));

	LinearScalePlugin *linear_scale_plugin = (LinearScalePlugin *) plugin;

	linear_scale_plugin->panel_id = -1;
	linear_scale_plugin->tmp_dib = NULL;
	linear_scale_plugin->min_scale_value = 0.0; 
	linear_scale_plugin->max_scale_value = 0.0;
	linear_scale_plugin->last_min_scale_value = 0.0;
	linear_scale_plugin->last_max_scale_value = 0.0;
		
	Plugin_AddMenuItem(plugin, "Display//Linear Scale",
		0, on_menu_clicked, plugin);
	
	PLUGIN_VTABLE(plugin, on_validate_plugin) = on_validate_plugin;
	PLUGIN_VTABLE(plugin, on_destroy) = on_destroy_plugin; 
	
	return plugin;
}
